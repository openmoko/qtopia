/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include <qstorage.h>

#include <QSettings>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QHash>
#include <QTimer>
#ifndef QTOPIA_HOST
#if defined(Q_WS_QWS)
#include <qwsdisplay_qws.h>
#elif defined(Q_WS_X11)
#include <qx11info_x11.h>
#endif
#endif

#include <qtopiachannel.h>
#include <qtopialog.h>
#include <qtopianamespace.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <mntent.h>

#ifdef QT_LSB
#include <sys/statvfs.h>
#endif

/*!
  \class QStorageMetaInfo
  \mainclass
  \brief The QStorageMetaInfo class describes the disks mounted on the file system.

  This class provides access to the mount information for the Linux
  filesystem. Each mount point is represented by the QFileSystem class.
  To ensure this class has the most up to date size information, call update().

  \ingroup io
  \ingroup content
  \sa QFileSystem
*/

class QStorageMetaInfoPrivate
{
public:
    QStorageMetaInfoPrivate()
        : channel( 0 )
        , documentsFileSystem( 0 )
        , applicationsFileSystem( 0 )
        , suppressMessages(false)
    {
    }

    QList<QFileSystem*> fileSystems;
    QtopiaChannel *channel;
    QFileSystem *documentsFileSystem;
    QFileSystem *applicationsFileSystem;
    bool suppressMessages;
};

/*! Constructor that determines the current mount points of the filesystem.
  The standard \a parent parameters is passed on to QObject. QStorageMetaInfo::instance()
  is the preferred method of obtaining a copy of this class.

  \sa instance()
 */
QStorageMetaInfo::QStorageMetaInfo( QObject *parent )
    : QObject( parent )
{
    d = new QStorageMetaInfoPrivate;

#ifndef QTOPIA_HOST
#if defined(Q_WS_QWS)
    if ( qt_fbdpy )
#elif defined(Q_WS_X11)
    if (QX11Info::display())
#endif
    {
        d->channel = new QtopiaChannel( "QPE/QStorage", this );
        connect( d->channel, SIGNAL(received(QString,QByteArray)),
                this, SLOT(cardMessage(QString,QByteArray)) );
    }
#endif
    update();
}

/*!
  Destroys the QStorageMetaInfo object.
*/
QStorageMetaInfo::~QStorageMetaInfo()
{
    foreach(QFileSystem *f, d->fileSystems)
        delete f;

    delete d;
}

/*! Returns the longest matching QFileSystem that starts with the
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
    for (QListIterator<QFileSystem> i(d->fileSystems); i.current(); ++i) {
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
        QString path = !fs->path().isEmpty() ? fs->path() : fs->prevPath();
        if ( path.length() > bestLen && filename.startsWith( path ) ) {
            currLen = path.length();
            if ( currLen == 1 )
                currLen = 0; // Fix checking '/' root mount which is a special case
            if ( filename.length() == currLen ||
                 filename[currLen] == '/' ||
                 filename[currLen] == '\\') {
                bestMatch = fs;
                bestLen = path.length();
            }
        }
    }
    if (bestMatch != NULL)
        bestMatch->update();
    return bestMatch;
#endif
}

/*!
    Returns a pointer to the file system on which the default documents path is located.
*/
const QFileSystem *QStorageMetaInfo::documentsFileSystem()
{
    return d->documentsFileSystem;
}

/*!
    Returns a pointer to the file system on which the default applications path is located.
 */
const QFileSystem *QStorageMetaInfo::applicationsFileSystem()
{
    return d->applicationsFileSystem;
}

/*!
    \internal System hook to listen for notifications that a new file system has been connected
    and we need to refresh our internal information.
*/

void QStorageMetaInfo::cardMessage( const QString& message, const QByteArray& data )
{
    if ( message == "updateStorage()" )
        update();
    else if( message == "mounting(QString)" || message == "unmounting(QString)" ) {
        QDataStream in(data);
        QString mountpoint;
        in >> mountpoint;

        update(mountpoint, message == "mounting(QString)" ? true : false);
    }
}

/*! Updates the mount and free space information for each mount
  point. This function is automatically called when a mount point is mounted or
  unmounted.
*/
void QStorageMetaInfo::update()
{
    QSettings cfg(QLatin1String("Trolltech"), QLatin1String("Storage"));
#if !defined(QT_LSB)
    mntent *me;
    FILE *mntfp = NULL;
    QHash<QString, QString> mountEntries;
    QString disk, path, prevPath, options, name, documentsPath, applicationsPath;
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

    QString documentsMount = cfg.value( QLatin1String( "DocumentsDefault" ), QLatin1String("HOME") ).toString();
    QString applicationsMount = cfg.value( QLatin1String( "ApplicationsDefault" ), QLatin1String("HOME") ).toString();

    qLog(DocAPI) << "cfg.mountpoints =" << cfg.value(QLatin1String("MountPoints")).toString();
    qLog(DocAPI) << "mountpointslist =" << mountpointslist;
    cfg.endGroup();

    if(d->fileSystems.count() != mountpointslist.count() + 2)
    {
        // save the path info (so we can record the "previous path")
        QHash<QString, QString> prevPaths;
        foreach(QFileSystem *f, d->fileSystems) {
            prevPaths[f->disk()] = (!f->path().isEmpty()?f->path():f->prevPath());
            delete f;
        }
        d->fileSystems.clear();
        // refresh d->fileSystems
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
            documentsPath    = cfg.value(QLatin1String("DocumentsPath")).toString();
            applicationsPath = cfg.value(QLatin1String("ApplicationsPath")).toString();
            removable       = cfg.value(QLatin1String("Removable"), false).toBool();
            applications    = cfg.value(QLatin1String("Applications"), false).toBool();
            documents       = cfg.value(QLatin1String("Documents"), false).toBool();
            contentDatabase = cfg.value(QLatin1String("ContentDatabase"), documents).toBool();
            connected       = removable?false:true;
            qLog(DocAPI) << "disk, path, options, name, documentsPath, applicationsPath, removable, applications, "
                            "documents, contentDatabase, connected ="
                         << disk << path << options << name << documentsPath << applicationsPath << removable
                         << applications << documents << contentDatabase << connected;
            d->fileSystems.append(new QFileSystem(disk, path, prevPath, options, name, documentsPath, applicationsPath,
                                removable, applications, documents, contentDatabase, connected));

            if( entry == documentsMount )
                d->documentsFileSystem = d->fileSystems.last();
            if( entry == applicationsMount )
                d->applicationsFileSystem = d->fileSystems.last();

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
            path             = QDir::homePath();
            options          = cfg.value(QLatin1String("Options"), QLatin1String("rw")).toString();
            name             = cfg.value(QLatin1String("Name[]"), QLatin1String("HOME")).toString();
            documentsPath    = cfg.value(QLatin1String("DocumentsPath"), QLatin1String("/Documents")).toString();
            applicationsPath = cfg.value(QLatin1String("ApplicationsPath"), QLatin1String("/Applications")).toString();
            removable        = cfg.value(QLatin1String("Removable"), false).toBool();
            applications     = cfg.value(QLatin1String("Applications"), true).toBool();
            documents        = cfg.value(QLatin1String("Documents"), true).toBool();
            contentDatabase  = false; // This setting makes no sense because HOME's database is always loaded.
            connected        = removable?false:true;
            qLog(DocAPI) << "disk, path, options, name, documentsPath, applicationsPath, removable, applications, "
                    "documents, contentDatabase, connected ="
                    << disk << path << options << name << documentsPath << applicationsPath << removable
                    << applications << documents << contentDatabase << connected;
            d->fileSystems.append(new QFileSystem(disk, path, path, options, name, documentsPath, applicationsPath,
                                removable, applications, documents, contentDatabase, connected));

            if( QLatin1String("HOME") == documentsMount )
                d->documentsFileSystem = d->fileSystems.last();
            if( QLatin1String("HOME") == applicationsMount )
                d->applicationsFileSystem = d->fileSystems.last();

            cfg.endGroup();
        } while (0);

        // add [PREFIX]
        do {
            cfg.beginGroup(QLatin1String("PREFIX"));
            disk             = QLatin1String("PREFIX");
            path             = QDir::cleanPath( Qtopia::qtopiaDir() );
            options          = cfg.value(QLatin1String("Options"), QLatin1String("ro")).toString();
            name             = cfg.value(QLatin1String("Name[]"), QLatin1String("PREFIX")).toString();
            documentsPath    = cfg.value(QLatin1String("DocumentsPath")).toString();
            applicationsPath = cfg.value(QLatin1String("ApplicationsPath")).toString();
            removable        = cfg.value(QLatin1String("Removable"), false).toBool();
            applications     = cfg.value(QLatin1String("Applications"), false).toBool();
            documents        = cfg.value(QLatin1String("Documents"), false).toBool();
            contentDatabase  = cfg.value(QLatin1String("ContentDatabase"), true).toBool();
            connected        = removable?false:true;
            qLog(DocAPI) << "disk, path, options, name, documentsPath, applicationsPath, removable, applications, "
                    "documents, contentDatabase, connected ="
                    << disk << path << options << name << documentsPath << applicationsPath << removable
                    << applications << documents << contentDatabase << connected;
            d->fileSystems.append(new QFileSystem(disk, path, path, options, name, documentsPath, applicationsPath,
                                removable, applications, documents, contentDatabase, connected));

            if( QLatin1String("PREFIX") == documentsMount )
                d->documentsFileSystem = d->fileSystems.last();
            if( QLatin1String("PREFIX") == applicationsMount )
                d->applicationsFileSystem = d->fileSystems.last();

            cfg.endGroup();
        } while (0);
    }

    foreach(QFileSystem *fs, d->fileSystems)
    {
        bool connected = !fs->isRemovable();
        QString path;

        if (fs->disk() != QLatin1String("HOME") &&
            fs->disk() != QLatin1String("PREFIX") )
        {
            connected = mountEntries.contains(fs->disk());
            if( connected )
                path = mountEntries[fs->disk()];

            fs->update( connected, path );
        }
        else
            fs->update();
    }
    if(d->suppressMessages == false)
        emit disksChanged();
#endif
}

/*!
    Update the system mounted at \a mountpoint, marking whether it is \a connected.
*/
void QStorageMetaInfo::update(QString& mountpoint, bool connected)
{
    d->suppressMessages = true;
    update();
    d->suppressMessages = false;
    foreach(QFileSystem *fs, d->fileSystems)
    {
        if(fs->disk() == mountpoint && fs->isRemovable())
        {
            QString path;
            mntent *me;
            FILE *mntfp = NULL;
            QHash<QString, QString> mountEntries;

            mntfp = setmntent( "/proc/mounts", "r" );
            me = getmntent(mntfp);
            while(me != NULL)
            {
                mountEntries[me->mnt_fsname] = me->mnt_dir;
                me = getmntent(mntfp);
            }
            endmntent(mntfp);
            if(mountEntries.contains(fs->disk()))
                path=mountEntries[fs->disk()];

            fs->update(connected, path);
        }
    }
    emit disksChanged();
}

/*!
  Returns a string containing the name, path, size and read/write parameters
  of all known filesystems

  \sa installLocationsString()
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

/*!
  \internal Returns infostrings for each of the \a filesystems passed, appending \a extension
  to the path.

  \sa cardInfoString(), installLocationsString()
*/
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

  \sa fileSystemNames(), QFileSystemFilter
*/
QList<QFileSystem*> QStorageMetaInfo::fileSystems( QFileSystemFilter *filter, bool connectedOnly )
{
    if(d->fileSystems.count() == 0)
        update();
    QList<QFileSystem*> ret;
    foreach ( QFileSystem *fs, d->fileSystems ) {
        if(fs->isConnected() || connectedOnly == false) {
            fs->update();
            if ( !filter || filter->filter( fs ) )
                ret << fs;
        }
    }
    return ret;
}

/*!
  Returns a list of file system names matching the \a filter. Use \a connectedOnly to return
  a list of only the filesystems that are connected.

  \sa fileSystems(), QFileSystemFilter
*/
QStringList QStorageMetaInfo::fileSystemNames( QFileSystemFilter *filter, bool connectedOnly )
{
    if(d->fileSystems.count() == 0)
        update();
    QStringList strings;
    foreach ( QFileSystem *fs, fileSystems(filter, connectedOnly) )
        strings << fs->name();
    return strings;
}

/*! \fn void QStorageMetaInfo::disksChanged()
  This signal is emitted whenever a disk has been mounted or unmounted, such as when
  a CF card has been inserted or removed.
*/

/*!
    Returns a pointer to a static instance of QStorageMetaInfo.
*/
Q_GLOBAL_STATIC(QStorageMetaInfo, storageMetaInfoInstance);

/*!
    Singleton accessor for an application wide QStorageMetaInfo. This is the preferred means for
    creating a QStorageMetaInfo object.
*/
QStorageMetaInfo *QStorageMetaInfo::instance()
{
    return storageMetaInfoInstance();
}

//---------------------------------------------------------------------------

class QFileSystemPrivate : public QSharedData
{
public:
    QFileSystemPrivate()
        : removable( false )
        , applications( false )
        , documents( false )
        , contentDatabase( false )
        , connected( false )
        , blockSize( 512 )
        , totalBlocks( 0 )
        , availBlocks( 0 )
    {
    }

    QString disk;
    QString path;
    QString prevPath;
    QString options;
    QString name;
    QString documentsPath;
    QString applicationsPath;
    bool removable;
    bool applications;
    bool documents;
    bool contentDatabase;
    bool connected;

    long blockSize;
    long totalBlocks;
    long availBlocks;
};


/*! 
    \class QFileSystem
    \brief The QFileSystem class describes a single mount point.

    This class is an informational result structure returned by the QStorageMetaInfo class.
    This class should not be created directly, but should rather be used in tandem
    with the QStorageMetaInfo class.

    \ingroup io
    \ingroup content
*/

/*!
    Construct an empty QFileSystem object.
*/
QFileSystem::QFileSystem()
{
    d = new QFileSystemPrivate;
}

/*!
    Construct a copy of \a other.
*/
QFileSystem::QFileSystem( const QFileSystem &other )
    : d( other.d )
{
}

/*!
    \internal Called by QStorageMetaInfo to construct and initialise a QFileSystem object
    for use.
*/

QFileSystem::QFileSystem( const QString &disk, const QString &path, const QString &prevPath, const QString &options,
                        const QString &name, const QString &documentsPath, const QString &applicationsPath, bool removable,
                        bool applications, bool documents, bool contentDatabase, bool connected)
{
    d = new QFileSystemPrivate;

    d->disk = disk;
    d->path = path;
    d->prevPath = prevPath;
    d->options = options;
    d->name = name;
    d->documentsPath = path + documentsPath;
    d->applicationsPath = path + applicationsPath;
    d->removable = removable;
    d->applications = applications;
    d->documents = documents;
    d->contentDatabase = contentDatabase;
    d->connected = connected;
}

/*!
    Destroys this QFileSystem object.
*/
QFileSystem::~QFileSystem()
{
}

/*!
    Assign the contents of \a other to this object.
*/
QFileSystem &QFileSystem::operator =( const QFileSystem &other )
{
    d = other.d;

    return *this;
}

/*!
    \internal Update the size infromation for this QFileSystem.
*/

void QFileSystem::update()
{
    QFileSystemPrivate *_d = const_cast< QFileSystemPrivate * >( d.constData() ); // Circumvent the copy on write.

    _d->blockSize = 0;
    _d->totalBlocks = 0;
    _d->availBlocks = 0;
#ifdef QT_LSB
    struct statvfs fs;
    if (_d->connected && statvfs(_d->path.toLocal8Bit().constData(), &fs) == 0) {
        _d->blockSize = fs.f_bsize;
        _d->totalBlocks = fs.f_blocks;
        _d->availBlocks = fs.f_bavail;
    }
#else
    struct statfs fs;
    if ( _d->connected && statfs( _d->path.toLocal8Bit(), &fs ) ==0 ) {
        _d->blockSize = fs.f_bsize;
        _d->totalBlocks = fs.f_blocks;
        _d->availBlocks = fs.f_bavail;
    }
#endif
}

/*!
    \internal Update the size infromation for this QFileSystem only if the \a connected state has
    changed, passing \a path in, in case of disconnection to keep track of where a filesystem was mounted.
*/
void QFileSystem::update( bool connected, const QString &path )
{
    if( d.constData()->connected != connected )
    {
        QFileSystemPrivate *_d = const_cast< QFileSystemPrivate * >( d.constData() ); // Circumvent the copy on write.

        _d->connected = connected;

        _d->documentsPath = path + _d->documentsPath.mid( _d->path.length() );
        _d->applicationsPath = path + _d->applicationsPath.mid( _d->path.length() );

        _d->prevPath = _d->path;
        _d->path = path;
    }
    update();
}

/*!
    Returns true if the QFileSystem is unitialised.
*/
bool QFileSystem::isNull() const
{
    return d->name.isEmpty();
}

/*!
  Returns the file system name, eg. /dev/hda3
*/
const QString &QFileSystem::disk() const
{
    return d->disk;
}

/*!
  Returns the mount path, eg. /home
*/
const QString &QFileSystem::path() const
{
    return d->path;
}

/*!
    Returns the path of the documents directory, eg. /home/Documents.
*/
const QString &QFileSystem::documentsPath() const
{
    return d->documentsPath;
}

/*!
    Returns the path of the applications directory, eg. /home/Applications.
*/
const QString &QFileSystem::applicationsPath() const
{
    return d->applicationsPath;
}

/*!
  Returns the previous mount path, eg. /home
  This is useful when a filesystem has been unmounted.
*/
const QString &QFileSystem::prevPath() const
{
    return d->prevPath;
}

/*!
  Returns the translated, human readable name for the mount directory.
*/
const QString &QFileSystem::name() const
{
    return d->name;
}

/*!
  Returns the mount options
*/
const QString &QFileSystem::options() const
{
    return d->options;
}

/*!
  Returns the size of each block on the file system.
*/
long QFileSystem::blockSize() const
{
    return d->blockSize;
}

/*!
  Returns the total number of blocks on the file system
*/
long QFileSystem::totalBlocks() const
{
    return d->totalBlocks;
}

/*!
  Returns the number of available blocks on the file system
 */
long QFileSystem::availBlocks() const
{
    return d->availBlocks;
}

/*!
  Returns flag indicating if the file system can be removed. For example, a CF card would be removable, but the internal memory would not.
*/
bool QFileSystem::isRemovable() const
{
    return d->removable;
}

/*!
  Returns flag indicating if the file system can contain applications.
*/
bool QFileSystem::applications() const
{
    return d->applications;
}

/*!
  Returns flag indicating if the file system can contain documents.
*/
bool QFileSystem::documents() const
{
    return d->documents;
}

/*!
  Returns flag indicating if the file system is mounted as writable or read-only.
  Returns false if read-only, true if read and write.
*/
bool QFileSystem::isWritable() const
{
    return d->options.contains("rw");
}

/*!
  Returns flag indicating if the file system has it's own content database stored.
 */
bool QFileSystem::contentDatabase() const
{
    return d->contentDatabase;
}

/*!
  Returns flag indicating if the file system is currently connected.
 */
bool QFileSystem::isConnected() const
{
    return d->connected;
}

/*!
    Returns a QFileSystem object describing the file system on which the file with the name \a fileName is located.

    If \a connectedOnly is true the QFileSystem will only be returned if it is currently connected.
*/
QFileSystem QFileSystem::fromFileName( const QString &fileName, bool connectedOnly )
{
    const QFileSystem *fs = QStorageMetaInfo::instance()->fileSystemOf( fileName, connectedOnly );

    return fs ? *fs : QFileSystem();
}

/*!
    Returns the QFileSystem which contains the default document storage path.

    The default document storage path can be obtained by calling \c{QFileSystem::documentsFileSystem().documentsPath()}.

    \sa documentsPath()
*/
QFileSystem QFileSystem::documentsFileSystem()
{
    const QFileSystem *fs = QStorageMetaInfo::instance()->documentsFileSystem();

    return fs ? *fs : QFileSystem();
}

/*!
    Returns the QFileSystem which contains the default application storage path.

    The default application storage path can be obtained by calling \c{QFileSystem::applicationsFileSystem().applicationsPath()}.

    \sa applicationsPath()
 */
QFileSystem QFileSystem::applicationsFileSystem()
{
    const QFileSystem *fs = QStorageMetaInfo::instance()->applicationsFileSystem();

    return fs ? *fs : QFileSystem();
}

// ====================================================================

/*! 
  \class QFileSystemFilter
  \brief The QFileSystemFilter class is used to restrict the available filesystems returned from QStorageMetaInfo.

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
        else
            return QFileSystemFilter::filter(fs);
    }

    QFileSystemFilter::FilterOption writable;
};
  \endcode

  \ingroup io
  \ingroup content
  \sa QStorageMetaInfo
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

/*! Returns true if the \a fs matches the filter; otherwise returns false.
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

