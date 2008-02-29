/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QSettings>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QHash>
#include <qstorage.h>
#include <QTimer>

#include <qtopiachannel.h>
#include <qtopialog.h>
#include <qtopianamespace.h>

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef Q_OS_LINUX
#include <unistd.h>
#include <sys/vfs.h>
#include <mntent.h>
#endif

#ifdef Q_WS_QWS
#include <qwsdisplay_qws.h>
#endif

/*!
  \class QStorageMetaInfo
  \brief The QStorageMetaInfo class describes the disks mounted on the file system.

  This class provides access to the mount information for the Linux
  filesystem. Each mount point is represented by the \c QFileSystem class.
  To ensure this class has the most up to date size information, call
  the \c update() method. Note that this will automatically be signaled
  by the operating system when a disk has been mounted or unmounted.

  \ingroup qtopiaemb
*/

/*! Constructor that determines the current mount points of the filesystem.
  The standard \a parent parameters is passed on to QObject.
 */
QStorageMetaInfo::QStorageMetaInfo( QObject *parent )
    : QObject( parent )
{
#ifdef Q_WS_QWS
    if ( qt_fbdpy )
    {
        channel = new QtopiaChannel( "QPE/Card", this );
        connect( channel, SIGNAL(received(const QString&,const QByteArray&)),
                this, SLOT(cardMessage(const QString&,const QByteArray&)) );
    }
#endif
    update();
}

/*!
  Destroys the QStorageMetaInfo object.
*/
QStorageMetaInfo::~QStorageMetaInfo()
{
    foreach(QFileSystem *f, mFileSystems)
        delete f;
}

/*! Returns the longest matching \c QFileSystem that starts with the
   same prefix as \a filename as its mount point. Use \a connectedOnly to search
   only the filesystems that are connected.
*/
const QFileSystem *QStorageMetaInfo::fileSystemOf( const QString &filename, bool connectedOnly )
{
#if 0
    // The filesystem way of doing things...
    // NOT updated for the new Storage.conf behaviour
    QString existingfilename = filename;
    struct stat st;
    while (stat(QFile::encodeName(existingfilename),&st)) {
        int x = existingfilename.findRev('/');
        if ( x < 0 )
            return 0;
        if ( x )
            existingfilename.truncate(x-1);
        else
            break;
    }
    int devno = st.st_dev;
    for (QListIterator<QFileSystem> i(mFileSystems); i.current(); ++i) {
        if ( 0==stat(QFile::encodeName((*i)->path()),&st) ) {
            if ( st.st_dev == devno ) {
                return *i;
            }
        }
    }
    return 0;
#else
    // The filename way (doesn't understand symlinks)
    QFileSystem *bestMatch = 0;
    int bestLen = 0;
    int currLen;

    foreach ( QFileSystem *fs, fileSystems( NULL, connectedOnly) ) {
        if ( filename.startsWith( fs->path() ) ) {
            currLen = fs->path().length();
            if ( currLen == 1 )
                currLen = 0; // Fix checking '/' root mount which is a special case
            if ( filename.length() == currLen ||
                 filename[currLen] == '/' ||
                 filename[currLen] == '\\') {
                if ( currLen > bestLen || bestLen == 0 ) {
                    bestMatch = fs;
                    bestLen = currLen;
                }
            }
        }
    }
    if (bestMatch != NULL)
        bestMatch->update();
    return bestMatch;
#endif
}


void QStorageMetaInfo::cardMessage( const QString& msg, const QByteArray& )
{
    if ( msg == "mtabChanged()" )
        update();
}

/*! Updates the mount and free space information for each mount
  point and is automatically called when a disk is mounted or
  unmounted.
*/
void QStorageMetaInfo::update()
{
    QSettings cfg(QLatin1String("Trolltech"), QLatin1String("Storage"));
#ifdef Q_OS_LINUX
    mntent *me;
    FILE *mntfp = NULL;
    QHash<QString, QString> mountEntries;
    QString disk, path, prevPath, options, name;
    bool removable, applications, documents, contentDatabase, connected;

    mntfp = setmntent( "/proc/mounts", "r" );
    me = getmntent(mntfp);
    while(me != NULL)
    {
        mountEntries[me->mnt_fsname] = me->mnt_dir;
        me = getmntent(mntfp);
    }
    endmntent(mntfp);

    cfg.beginGroup(QLatin1String("MountTable"));
    QStringList mountpointslist;
    if (cfg.contains("MountPoints"))
        mountpointslist = cfg.value(QLatin1String("MountPoints")).toString().split(QLatin1String(","));
    qLog(DocAPI) << "cfg.mountpoints =" << cfg.value(QLatin1String("MountPoints")).toString();
    qLog(DocAPI) << "mountpointslist =" << mountpointslist;
    cfg.endGroup();

    if(mFileSystems.count() != mountpointslist.count() + 1)
    {
        // save the path info (so we can record the "previous path")
        QHash<QString, QString> prevPaths;
        foreach(QFileSystem *f, mFileSystems) {
            prevPaths[f->disk()] = (!f->path().isEmpty()?f->path():f->prevPath());
            delete f;
        }
        mFileSystems.clear();
        // refresh mFileSystems
        foreach(QString entry, mountpointslist)
        {
            cfg.beginGroup(entry);
            disk            = cfg.value(QLatin1String("Path")).toString();
            if ( mountEntries.contains(disk) ) {
                path        = mountEntries[disk];
                prevPath    = mountEntries[disk];
            } else {
                path        = QLatin1String("");
                prevPath    = prevPaths[disk];
            }
            options         = cfg.value(QLatin1String("Options"), QLatin1String("rw")).toString();
            name            = cfg.value(QLatin1String("Name[]"), disk).toString();
            removable       = cfg.value(QLatin1String("Removable"), false).toBool();
            applications    = cfg.value(QLatin1String("Applications"), false).toBool();
            documents       = cfg.value(QLatin1String("Documents"), false).toBool();
            contentDatabase = cfg.value(QLatin1String("ContentDatabase"), documents).toBool();
            connected       = removable?false:true;
            qLog(DocAPI) << "disk, path, options, name, removable, applications, documents, contentDatabase, connected ="
                    << disk << path << options << name << removable << applications << documents << contentDatabase << connected;
            mFileSystems.append(new QFileSystem(disk, path, prevPath, options, name, removable, applications, documents, contentDatabase, connected));
            cfg.endGroup();
        }

        // add [HOME]
        do {
            cfg.beginGroup(QLatin1String("HOME"));
            disk            = cfg.value(QLatin1String("Path"), QLatin1String("HOME")).toString();
            // If your HOME is on a partition that's specified by Storage.conf and it doesn't contain
            // Applications or Documents you might want to hide it. Set "Path = HIDE" to do this.
            if ( disk == QLatin1String("HIDE") ) {
                cfg.endGroup();
                break;
            } else {
                disk        = QLatin1String("HOME");
            }
            path            = QDir::homePath();
            options         = cfg.value(QLatin1String("Options"), QLatin1String("rw")).toString();
            name            = cfg.value(QLatin1String("Name[]"), QLatin1String("HOME")).toString();
            removable       = cfg.value(QLatin1String("Removable"), false).toBool();
            applications    = cfg.value(QLatin1String("Applications"), true).toBool();
            documents       = cfg.value(QLatin1String("Documents"), true).toBool();
            contentDatabase = false; // This setting makes no sense because HOME's database is always loaded.
            connected       = removable?false:true;
            qLog(DocAPI) << "disk, path, options, name, removable, applications, documents, contentDatabase, connected ="
                    << disk << path << options << name << removable << applications << documents << contentDatabase << connected;
            mFileSystems.append(new QFileSystem(disk, path, path, options, name, removable, applications, documents, contentDatabase, connected));
            cfg.endGroup();
        } while (0);

        // add [PREFIX]
        do {
            cfg.beginGroup(QLatin1String("PREFIX"));
            disk            = QLatin1String("PREFIX");
            path            = Qtopia::qtopiaDir();
            options         = cfg.value(QLatin1String("Options"), QLatin1String("ro")).toString();
            name            = cfg.value(QLatin1String("Name[]"), QLatin1String("PREFIX")).toString();
            removable       = cfg.value(QLatin1String("Removable"), false).toBool();
            applications    = cfg.value(QLatin1String("Applications"), false).toBool();
            documents       = cfg.value(QLatin1String("Documents"), false).toBool();
            contentDatabase = cfg.value(QLatin1String("ContentDatabase"), true).toBool();
            connected       = removable?false:true;
            qLog(DocAPI) << "disk, path, options, name, removable, applications, documents, contentDatabase, connected ="
                    << disk << path << options << name << removable << applications << documents << contentDatabase << connected;
            mFileSystems.append(new QFileSystem(disk, path, path, options, name, removable, applications, documents, contentDatabase, connected));
            cfg.endGroup();
        } while (0);
    }

    foreach(QFileSystem *fs, mFileSystems)
    {
        if (fs->disk() != QLatin1String("HOME") &&
            fs->disk() != QLatin1String("PREFIX") )
        {
            fs->mConnected = mountEntries.contains(fs->mDisk);
            if (fs->mConnected)
                fs->mPath = mountEntries[fs->mDisk];
        }
        fs->update();
    }
    emit disksChanged();
#endif
}

/*!
  Returns a string containing the name, path, size and read/write parameters
  of all known filesystems
*/
QString QStorageMetaInfo::cardInfoString()
{
    QFileSystemFilter fsf;
    fsf.documents = QFileSystemFilter::Set;
    return infoString( fileSystems( &fsf ), "" );
}

/*!
  Includes the same information as QStorageMetaInfo::cardInfoString() and the path
  where documents can be installed

  \sa cardInfoString()
*/
QString QStorageMetaInfo::installLocationsString()
{
    QFileSystemFilter fsf;
    fsf.applications = QFileSystemFilter::Set;
    return infoString( fileSystems( &fsf ), "/Documents" );
}

QString QStorageMetaInfo::infoString( QList<QFileSystem*> filesystems, const QString &extension )
{
    //storage->update();
    QString s;
    foreach ( QFileSystem *fs, filesystems ) {
        fs->update();
        s += fs->name() + "=" + fs->path() + extension + " "
             + QString::number( fs->availBlocks() * (fs->blockSize() / 256) / 4 )
             + "K " + fs->options() + ";";
    }
    return s;
}

/*!
  Returns a list of available mounted file systems matching the \a filter. Use \a connectedOnly to return
  a list of only the filesystems that are connected.
*/
QList<QFileSystem*> QStorageMetaInfo::fileSystems( QFileSystemFilter *filter, bool connectedOnly )
{
    if(mFileSystems.count() == 0)
        update();
    QList<QFileSystem*> ret;
    foreach ( QFileSystem *fs, mFileSystems ) {
        if(fs->isConnected() || connectedOnly == false) {
            fs->update();
            if ( !filter || filter->filter( fs ) )
                ret << fs;
        }
    }
    return ret;
}

/*!
  Returns a list of file system names (matching the \a filter ). Use \a connectedOnly to return
  a list of only the filesystems that are connected.
*/
QStringList QStorageMetaInfo::fileSystemNames( QFileSystemFilter *filter, bool connectedOnly )
{
    if(mFileSystems.count() == 0)
        update();
    QStringList strings;
    foreach ( QFileSystem *fs, fileSystems(filter, connectedOnly) )
        strings << fs->name();
    return strings;
}

/*! \fn void QStorageMetaInfo::disksChanged()
  Is emitted when a disk has been mounted or unmounted, such as when
  a CF card has been inserted.
*/

//---------------------------------------------------------------------------

QFileSystem::QFileSystem( const QString &disk, const QString &path, const QString &prevPath, const QString &options,
                        const QString &name, bool removable, bool applications, bool documents,
                        bool contentDatabase, bool connected)
    : mDisk( disk ), mPath( path ), mPrevPath( prevPath ), mOptions( options ), mName( name ),
    mRemovable( removable ), mApplications( applications ), mDocuments( documents ), mContentDatabase( contentDatabase ),
    mConnected(connected), mBlockSize( 512 ), mTotalBlocks( 0 ), mAvailBlocks( 0 )
{
    update();
}

void QFileSystem::update()
{
    mBlockSize = 0;
    mTotalBlocks = 0;
    mAvailBlocks = 0;
#ifdef Q_OS_LINUX
    struct statfs fs;
    if ( mConnected && statfs( mPath.toLocal8Bit(), &fs ) ==0 ) {
        mBlockSize = fs.f_bsize;
        mTotalBlocks = fs.f_blocks;
        mAvailBlocks = fs.f_bavail;
    }
#endif
}

/*! \class QFileSystem
  \brief The QFileSystem class describes a single mount point.

  This class returns mount point information including:
  \list
  \o file system name
  \o mount point
  \o human readable name
  \o size information
  \o mount options.
  \endlist

  \ingroup qtopiaemb

  \sa QStorageMetaInfo
*/

/*! \fn const QString &QFileSystem::disk() const
  Returns the file system name, such as /dev/hda3
*/

/*! \fn const QString &QFileSystem::path() const
  Returns the mount path, such as /home
*/

/*! \fn const QString &QFileSystem::prevPath() const
  Returns the previous mount path, such as /home (useful when a filesystem has been unmounted)
*/

/*! \fn const QString &QFileSystem::name() const
  Returns the translated, human readable name for the mount directory.
*/

/*! \fn const QString &QFileSystem::options() const
  Returns the mount options
*/

/*! \fn long QFileSystem::blockSize() const
  Returns the size of each block on the file system.
*/

/*! \fn long QFileSystem::totalBlocks() const
  Returns the total number of blocks on the file system
*/

/*! \fn long QFileSystem::availBlocks() const
  Returns the number of available blocks on the file system
 */

/*! \fn bool QFileSystem::isRemovable() const
  Returns flag indicating if the file system can be removed. For example, a CF card would be removable, but the internal memory would not.
*/

/*! \fn bool QFileSystem::applications() const
  Returns flag indicating if the file system can contain applications.
*/

/*! \fn bool QFileSystem::documents() const
  Returns flag indicating if the file system can contain documents.
*/

/*! \fn bool QFileSystem::isWritable() const
  Returns flag indicating if the file system is mounted as writable or read-only.
  Returns false if read-only, true if read and write.
*/

/*! \fn bool QFileSystem::contentDatabase() const
  Returns flag indicating if the file system has it's own content database stored.
 */

/*! \fn bool QFileSystem::isConnected() const
  Returns flag indicating if the file system is currently connected.
 */

// ====================================================================

/*! \class QFileSystemFilter
  \brief The QFileSystemFilter class is used to restrict the available filesystems.

  Extending the filter class is relatively simple.

  \code
class WriteableFileSystemFilter : public QFileSystemFilter
{
public:
    WriteableFileSystemFilter()
        : writable( QFileSystemFilter::Either )
    {
    }

    bool filter( QFileSystem *fs )
    {
        if ( (writable == QFileSystemFilter::Set && !fs->isWritable()) ||
             (writable == QFileSystemFilter::NotSet && fs->isWritable()) )
            return false;
        return QFileSystemFilter::filter();
    }

    QFileSystemFilter::FilterOption writable;
};
  \endcode

  \ingroup qtopiaemb
*/

/*!
  \enum QFileSystemFilter::FilterOption

  This enum is used to describe the filter conditions for file systems.

  \value Set The filter matches if the connected condition is set.
  \value NotSet The filter matches if the connected condition is not set
  \value Either The filter always matches.
*/

/*! Constructor that initializes the filter to allow any filesystem.
*/
QFileSystemFilter::QFileSystemFilter()
    : documents(Either), applications(Either), removable(Either),
      content(Either)
{
}

/*! Destructor
*/
QFileSystemFilter::~QFileSystemFilter()
{
}

/*! Does the \a fs match the filter?
*/
bool QFileSystemFilter::filter( QFileSystem *fs )
{
    if ( (documents == Set && !fs->documents()) ||
         (documents == NotSet && fs->documents()) )
        return false;
    if ( (applications == Set && !fs->applications()) ||
         (applications == NotSet && fs->applications()) )
        return false;
    if ( (removable == Set && !fs->isRemovable()) ||
         (removable == NotSet && fs->isRemovable()) )
        return false;
    if ( (content == Set && !fs->contentDatabase()) ||
         (content == NotSet && fs->contentDatabase()) )
        return false;
    return true;
}

