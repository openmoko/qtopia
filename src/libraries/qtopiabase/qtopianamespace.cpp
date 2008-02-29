/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include <qtopianamespace.h>
#include <version.h>
#include <custom.h>
#include <qstorage.h>
#include <private/qactionconfirm_p.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <float.h>

#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>

#ifdef Q_WS_QWS
#include <qdawg.h>
#endif

#ifdef QTOPIA_DESKTOP
#include <desktopsettings.h>
#endif
#include <QMap>
#include <QDir>
#include <QFileInfo>
#include <QUuid>
#include <QSettings>
#include <QLibraryInfo>
#include <QMutex>
#include <qdebug.h>

#ifndef Q_OS_WIN32
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <mntent.h>
#else
#include <Windows.h>
#include <sys/locking.h>
#include <io.h>
#include <string.h> // for strerror function
#include <stdlib.h>
#endif

#if defined(Q_WS_WIN32)
#include <objbase.h>
#elif defined(Q_WS_MAC)
#include <CoreFoundation/CoreFoundation.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#ifndef Q_OS_WIN
#include <unistd.h>
#endif


#if defined(Q_OS_UNIX) && defined(Q_WS_QWS)
extern int qws_display_id;
#endif

class QtopiaPathHelper
{
public:
    static QtopiaPathHelper *instance();
    QStringList installPaths() const { return r; }
private:
    QString packagePath() const;
    mutable QStringList r;  // cache of the path
};

/*!
  \internal
  Cache all the path lookup in a singleton so it only has to be done once
  per process.  Previous implementations also did this, but lets make it
  thread-safe while we're on the job.
*/
QtopiaPathHelper *QtopiaPathHelper::instance()
{
    static QtopiaPathHelper ph;
    static QBasicAtomic initialized = Q_ATOMIC_INIT(0);

    if ( initialized.testAndSet( 0, 1 ))
    {
        QChar sl = QDir::separator();
        const char *d = getenv("QTOPIA_PATH");
        if ( d ) {
            ph.r = QString(d).split(":");
            for (QStringList::Iterator it=ph.r.begin(); it!=ph.r.end(); ++it) {
                if ( (*it)[(*it).length()-1] != sl )
                    (*it) += sl;
            }
        }
        // The installation directory is always searched first
        QString qt_prefix = QLibraryInfo::location(QLibraryInfo::PrefixPath);
        if ( qt_prefix[qt_prefix.length()-1] != sl )
            qt_prefix += sl;
        ph.r.prepend(qt_prefix);

        // Package paths are last
        QString pp = ph.packagePath();
        if ( !pp.isEmpty() )
            ph.r.append( pp );
        QDir::root().mkpath( pp + "/pics" );
        QDir::root().mkpath( pp + "/sounds" );

        initialized = 2;
    }
    else
    {
        while ( initialized != 2 )
            Qtopia::msleep( 5 );
    }

    return &ph;
}

/*!
  \internal
  Find the package path from the Storage.conf file

  This code duplicates somewhat code already in qstorage.cpp, but there's
  objections to using that when a QApplication object hasn't been constructed.

  This should only ever be called from the instance method above.
*/
QString QtopiaPathHelper::packagePath() const
{
    QSettings storage( QLatin1String("Trolltech"), QLatin1String("Storage") );
    QString mountPoint = "HOME";
    QString pp = "packages/";
    if ( storage.childGroups().contains( "Packages" ))
    {
        storage.beginGroup( "Packages" );
        mountPoint = storage.value( QLatin1String("MountPoint") ).toString();
        pp = storage.value( QLatin1String("Path") ).toString();
        storage.endGroup();
    }
    if ( mountPoint == "HOME" )
    {
        if ( pp.right(1) != "/" )
            pp.append("/");
        return pp.prepend( Qtopia::homePath() + "/" );
    }

    storage.beginGroup( mountPoint );
    QString mountPath = storage.value( QLatin1String("Path") ).toString();
    storage.endGroup();
    if ( mountPoint.isEmpty() || pp.isEmpty() || mountPath.isEmpty() )
    {
        qWarning( "Storage.conf does not specify package path!" );
        return QString();
    }
    if ( pp.right(1) != "/" )
        pp.append("/");
    if ( pp.left(1) != "/" )
        pp.prepend( "/" );

#ifdef Q_OS_LINUX
    mntent *me;
    FILE *mntfp = NULL;
    mntfp = setmntent( "/proc/mounts", "r" );
    me = getmntent(mntfp);
    while ( me != NULL )
    {
        if ( mountPath == me->mnt_fsname )
            return pp.prepend( me->mnt_dir );
        me = getmntent(mntfp);
    }
    endmntent(mntfp);
#endif

    qWarning( "Could not found filesystem name %s in /proc/mounts - bad Storage.conf",
            qPrintable( mountPath ));
    return QString();
}


/*!
  \class Qtopia
  \brief The Qtopia namespace contains miscellaneous functionality.

  \section1 Word list related

  A list of words relevant to the current locale is maintained by the
  system. The list is held in a \l {QDawg}{DAWG}
  (implemented by the QDawg class). This list is used, for example, by
  the pickboard input method.

  The global QDawg is returned by fixedDawg(); this cannot be updated.
  An updatable copy of the global QDawg is returned by addedDawg().
  Applications may have their own word lists stored in \l{QDawg}s
  which are returned by dawg(). Use addWords() to add words to the
  updateable copy of the global QDawg or to named application
  \l{QDawg}s.

  \ingroup qtopiaemb
*/

/*!
   Returns the list of base directories into which Qtopia packages
   may have been installed.

   The first element in the list is the directory into which
   Qtopia packages that are always available are installed.
   The other elements may be on removable media, etc.

   Some directories in the list, or their subdirectories,
   may not be writable.

   The first element is equals to [qt_prefix] and additional values
   are set by the colon-seperated environment variable QTOPIA_PATH.
*/

QStringList Qtopia::installPaths()
{
    return QtopiaPathHelper::instance()->installPaths();
}

/*
   Returns the base directory in which downloaded 3rd party packages
   will be searched for.

   This is the last item in the installPaths() list.

   This is determined by the [Packages] group in the Storage.conf file.
*/
QString Qtopia::packagePath()
{
#ifndef QTOPIA_DESKTOP
    return installPaths().last();
#else
    return QString();
#endif
}

/*!
   Returns the base directory into which Qtopia system packages
   are installed.

   This is the first item in the installPaths() list.
 */
QString Qtopia::qtopiaDir()
{
#ifdef QTOPIA_DESKTOP
    return DesktopSettings::installedDir();
#endif

#ifdef Q_WS_QWS
    return *installPaths().begin();
#endif // Q_WS_QWS
    return QString();
}

/*!
  Returns the user's current Document directory. There is a trailing "/".
*/
QString Qtopia::documentDir()
{
    return QFileSystem::documentsFileSystem().documentsPath() + '/';
}

/*!
  Returns the name of the directory to be used as the current
  users home directory.
*/
QString Qtopia::homePath()
{
#ifdef QTOPIA_DESKTOP
    return DesktopSettings::homePath();
#else
    return QDir::homePath();
#endif
}


#ifdef QTOPIA4_TODO
#error "Remove Qtopia::defaultButtonsFile"
#endif
/*!
  This function will change in the final Qtopia 4 release.

  \internal

  Return the name of the defaultbuttons.conf file.
  This allows Qtopia to use a QVFb-supplied defaultbuttons.conf file
  (if one exists).
*/
QString Qtopia::defaultButtonsFile()
{
#if defined(Q_OS_UNIX) && defined(Q_WS_QWS) && !defined(QT_NO_QWS_VFB)
    QString r = QString("/tmp/qtembedded-%1/defaultbuttons.conf").arg(qws_display_id);
    if ( QFileInfo(r).exists() )
        return r;
#endif
    return qtopiaDir()+"etc/defaultbuttons.conf";
}

/*!
  Returns a list of directory names where help files are found.
*/
QStringList Qtopia::helpPaths()
{
    QStringList path;
    QStringList langs = languageList();
    QStringList qpepaths = installPaths();
    for (QStringList::ConstIterator qit = qpepaths.begin(); qit!=qpepaths.end(); ++qit) {
        QStringListIterator it(langs);
        it.toBack();
        while (it.hasPrevious()) {
            QString lang = it.previous();
            if ( !lang.isEmpty() )
                path += *qit + "help/" + lang + "/html";
        }
        path += *qit + "pics";
        path += *qit + "help/html";
        path += *qit + "docs";
    }
    return path;
}

/*!
    Shows a dialog with \a caption to confirm deletion of \a object with user.

    If \a parent is 0, the dialog becomes an application-global modal dialog
    box. If \a parent is a widget, the dialog becomes modal relative to
    \a parent.
*/
bool Qtopia::confirmDelete( QWidget *parent, const QString & caption, const QString & object )
{
    QString msg = "<qt>" + qApp->translate( "Qtopia::confirmDelete", "Are you sure you want to delete: %1?").arg( object ) + "</qt>";
    int r = QMessageBox::warning( parent, caption, msg, QMessageBox::Yes, QMessageBox::No|QMessageBox::Default| QMessageBox::Escape, 0 );
    return r == QMessageBox::Yes;
}

/*!
    Displays a message dialog containing the specified \a pixmap and \a text
    for a short time.
*/
void Qtopia::actionConfirmation(const QPixmap &pixmap, const QString &text)
{
#ifdef Q_WS_QWS
    QActionConfirm::display(pixmap, text);
#else
    Q_UNUSED(pixmap);
    Q_UNUSED(text);
#endif
}

/*!
    Sounds the audible system alarm. This is used for applications such
    as Calendar when it needs to inform the user of an event.
*/
void Qtopia::soundAlarm()
{
    QtopiaServiceRequest e( "Alert", "soundAlert()" );
    e.send();
}

/*!
  Displays a status \a message to the user. This usually appears
  in the taskbar for a short amount of time, then disappears.
*/
void Qtopia::statusMessage(const QString& message)
{
    QtopiaIpcEnvelope e( "QPE/TaskBar", "message(QString)" );
    e << message;
}


/*

   Translation and language functions

 */

/*!
  Returns the list of language identifiers for the languages the user
  has selected as meaningful to them.

  The first string in the list is the users primary
  language. All information presented to the user should be in the
  language earliest in the list.
*/
QStringList Qtopia::languageList()
{
    QString lang;
    QStringList langs;
#ifdef QTOPIA_DESKTOP
    langs = DesktopSettings::languages();
#else
    if (lang.isEmpty())
        lang = getenv("LANG");

    int i  = lang.indexOf(".");
    if ( i > 0 )
        lang = lang.left( i );
    langs.append(lang);
    i = lang.indexOf( "_" );
    if ( i > 0 )
        langs.append(lang.left(i));
#endif
    return langs;
}

static QMap<QString,QList<QTranslator*>* > *trmap=0;

static void clean_trmap()
{
    if ( trmap ) {
        QList<QString> keys = trmap->keys();
        foreach ( QString k, keys ) {
            QList<QTranslator*> *l = trmap->take( k );
            if ( l ) {
                while ( !l->isEmpty() )
                    delete l->takeLast();
                delete l;
            }
        }
        delete trmap;
        trmap = 0;
    }
}

static QList<QTranslator*>* qLocalTranslations(const QString& key)
{

    if ( !trmap ) {
        trmap = new QMap<QString,QList<QTranslator*>* >;
        qAddPostRoutine( clean_trmap );
    }

    QList<QTranslator*> *l = 0;
    if (trmap->contains(key)) {
        l = trmap->value(key);
        return l;
    }

    l = new QList<QTranslator*>;

    QStringList langs = Qtopia::languageList();

    QStringList qpepaths = Qtopia::installPaths();
    for (QStringList::ConstIterator qit = qpepaths.begin(); qit!=qpepaths.end(); ++qit) {
        for (QStringList::ConstIterator it = langs.begin(); it!=langs.end(); ++it) {
            QString lang = *it;

            QString d = *qit + "i18n/" + lang + "/";
            QDir dir(d, key + ".qm", QDir::Unsorted, QDir::Files);
            if (!dir.exists())
                continue;
            for ( int i=0; i<(int)dir.count(); i++ ) {
                QTranslator * trans = new QTranslator();
                if ( trans->load( d+dir[i] ))
                    l->append(trans);
                else
                    delete trans;
            }
        }
    }

    trmap->insert(key,l);

    return l;
}

/*!
Translate \a str in the context \a c, using only the translation files defined by \a key.
The \a key may be either a single name, such as "QtopiaSettings", or it may be a
'wildcard', such as "Categories-*". The \a key thus defines the set of translation files
in which to search for a translation.
*/

QString Qtopia::translate(const QString& key, const QString& c, const QString& str)
{
    static QMutex translationMutex;
    QMutexLocker locker( &translationMutex );
    QList<QTranslator*>* l = qLocalTranslations(key);
    if ( !l || l->isEmpty() ) {
        return str;
    }
    for (QList<QTranslator*>::ConstIterator it = l->begin(); it!=l->end(); ++it) {
        QString msg = (*it)->translate(c.toLatin1(),str.toUtf8());
        if ( !msg.isEmpty() )
            return msg;
    }
    return str;
}

/*

String manipulation functions

*/

/*!
  Returns the result of using QString::simplified() on \a multiLine, but with
  linebreaks preserved.
*/
QString Qtopia::simplifyMultiLineSpace( const QString &multiLine )
{
    QString result;
    QStringList lines =  multiLine.split('\n');
    for ( QStringList::Iterator it = lines.begin(); it != lines.end(); ++it ) {
        if ( it != lines.begin() )
            result += "\n";
        result += (*it).simplified();
    }
    return result;
}

/*!
  Returns the string equivalent to \a s, but with any soft-hyphens removed.
*/
QString Qtopia::dehyphenate(const QString& s)
{
    QChar shy(0x00ad);
    const QChar *data = s.data();
    int i=0;
    while (i<(int)s.length() && *data!=shy) {
        i++;
        data++;
    }
    if ( i==(int)s.length() )
        return s;
    QString r = s.left(i);
    for (; i<(int)s.length(); ++i) {
        if ( s[i]!=shy )
            r += s[i];
    }
    return r;
}

/*!
  System independant sleep. Suspend process for \a secs seconds.

  \sa QThread::sleep()
 */
void Qtopia::sleep( unsigned long secs )
{
#ifdef Q_OS_WIN32
    ::Sleep( (unsigned long)(secs * 1000) );
#else
    ::sleep( secs );
#endif
}

/*!
  System independant sleep. Suspend process for \a msecs milliseconds.

  \sa QThread::msleep()
 */
void Qtopia::msleep( unsigned long msecs )
{
#ifdef Q_OS_WIN32
    ::Sleep( msecs );
#else
    usleep( msecs * 1000 );
#endif
}

/*!
  System independant sleep. Suspend process for \a usecs microseconds.

  \sa QThread::usleep()
 */
void Qtopia::usleep( unsigned long usecs )
{
#ifdef Q_OS_WIN32
    ::Sleep( ( usecs / 1000 ) + 1 );
#else
    if ( usecs >= 1000000 )
        ::sleep( usecs / 1000000 );
    ::usleep( usecs % 1000000 );
#endif
}

#ifdef Q_WS_QWS

/*!
  Returns the Qtopia version string. This is of the form:
  \i{major} .\i{minor} .\i{patchlevel}  (eg. "1.2.3"),
  possibly followed by a space and special information
  (eg. "1.2.3 beta4").
*/
QString Qtopia::version()
{
    return QPE_VERSION;
}

/*!
  Returns the device architecture string. This is a sequence
  of identifiers separated by "/", from most general to most
  specific (eg. "IBM/PC").
*/
QString Qtopia::architecture()
{
#ifndef QPE_ARCHITECTURE
# define QPE_ARCHITECTURE "Uncustomized Device" // No tr
#endif
    return QPE_ARCHITECTURE;
}

/*!
  Returns a unique ID for this device. The value can change, if
  for example, the device is reset.
*/
QString Qtopia::deviceId()
{
    QSettings cfg("Trolltech","Security");
    cfg.beginGroup("Sync");
    QString r=cfg.value("serverid").toString();
    if ( r.isEmpty() ) {
        QUuid uuid = QUuid::createUuid();
        cfg.setValue("serverid",(r = uuid.toString()));
    }
    return r;
}

/*!
  Returns the name of the owner of the device.
*/
QString Qtopia::ownerName()
{
    QSettings cfg("Trolltech","Security");
    cfg.beginGroup("Sync");
    QString r=cfg.value("ownername").toString();
    return r;
}


#endif

#ifdef Q_OS_WIN32
/*!
  Truncate file to size specified
  \a f must be an open file
  \a size must be a positive value
 */
bool Qtopia::truncateFile(QFile &f, int size)
{
    if (!f.isOpen())
        return false;

    if (size == -1)
        size = f.size();

    if (::chsize(f.handle(), size) != -1)
        return true;
    else
        return false;
}
#else   // Q_OS_WIN32
/*!
  Truncate file to size specified
  \a f must be an open file
  \a size must be a positive value
 */
bool Qtopia::truncateFile(QFile &f, int size){
    if (!f.isOpen())
        return false;

    return ::ftruncate(f.handle(), size) != -1;
}
#endif  // Q_OS_WIN32

/*!
  Returns the default system path for storing temporary files. The path is
  unique to the display to which the application is connected. The path has
  a trailing directory separator character.

  The returned directory is created if it does not exist.

  \sa QDir::tempPath()
 */
QString Qtopia::tempDir()
{
    QString result;

#if defined(Q_OS_UNIX) && defined(Q_WS_QWS)
    result = QString("/tmp/qtopia-%1/").arg(QString::number(qws_display_id));
#else
    result = QDir::tempPath();
#endif

    QDir d( result );
    if ( !d.exists() ) {
#ifndef Q_OS_WIN
        mkdir(result.toLatin1(), 0700);
#else
        d.mkdir(result);
#endif
    }

    return result;
}

/*!
  Returns a filename suitable for use as a temporary file using \a fname
  as a base. Currently returns the filename with "_new" inserted prior to
  the extension.

  No attempt is made to check that the file does not already exist.
*/
QString Qtopia::tempName(const QString &fname)
{
    QFileInfo fileInfo( fname );
    return fileInfo.absolutePath() + "/" + fileInfo.baseName() +
            "_new." + fileInfo.completeSuffix(); // No tr
    // XXX no point trying to check if already exists, since even
    // XXX if it doesn't now, it might before caller makes file.
    // XXX Could use a "highly unique" name.
}

/*!
  Returns the full path for the application called \a appname, with the
  given \a filename. Returns QString() if there was a problem creating
  the directory tree for \a appname.
  If \a filename contains "/", it is the caller's responsibility to
  ensure that those directories exist.
*/
QString Qtopia::applicationFileName(const QString& appname, const QString& filename)
{
    QDir d;
    QString r = Qtopia::homePath();
#ifndef QTOPIA_DESKTOP
    r += "/Applications/";
#endif
    if ( !QFile::exists( r ) )
    if ( d.mkdir(r) == false )
        return QString();
    r += appname;
    if ( !QFile::exists( r ) )
        if ( d.mkdir(r) == false )
            return QString();
    r += "/"; r += filename;

    return r;
}

/*!
  Returns true if \a file is the filename of a document.
*/
bool Qtopia::isDocumentFileName(const QString& file)
{
    if ( file.right(1) == "/" )
        return false;
    QStorageMetaInfo storage;
    QFileSystemFilter fsf;
    fsf.documents = QFileSystemFilter::Set;
    foreach ( QFileSystem *fs, storage.fileSystems(&fsf) ) {
        if ( file.indexOf(fs->path()+"/") == 0 )
            return true;
    }
    return false;
}

/*! \enum Qtopia::Lockflags
 This enum controls what type of locking is performed on a file.

 Current defined values are:

 \value LockShare Allow lock to be shared. Reserved for future use
 \value LockWrite Create at a write lock.
 \value LockBlock Block the process when lock is encountered. Under WIN32
                  this blocking is limited to ten(10) failed attempts to
                  access locked file. Reserved for future use.
*/

#ifndef Q_OS_WIN32

/*!
  Lock region of file. Any locks created should be released before
  the program exits. Returns true if sucessfull.

  \a f must be an open file
  \a flags any combination of LockShare, LockWrite, LockBlock
*/
bool Qtopia::lockFile(QFile &f, int flags)
{
    struct flock fileLock;

    if (!f.isOpen())
        return false;

    fileLock.l_whence = SEEK_SET;
    fileLock.l_start = 0;
    int lockCommand, lockMode;

    fileLock.l_len = f.size();


    if (flags == -1){
        lockMode =  F_RDLCK;
        if (f.openMode() == QIODevice::ReadOnly)
            fileLock.l_type = F_RDLCK;
        else
            fileLock.l_type = F_WRLCK;
        lockCommand = F_SETLK;
    }else{
        if (flags & LockWrite)
            fileLock.l_type = F_WRLCK;
        else
            fileLock.l_type = F_RDLCK;
        if (flags & LockBlock)
            lockCommand = F_SETLK;
        else
            lockCommand = F_SETLKW; // block process if possible
    }

    if (::fcntl(f.handle(), lockCommand, &fileLock) != -1)
        return true;
    else
        return false;
}


/*!
  Unlock a region of file
  \a f must be an open file previously locked
 */
bool Qtopia::unlockFile(QFile &f)
{
    struct flock fileLock;

    if (!f.isOpen())
        return false;

    fileLock.l_whence = SEEK_SET;
    fileLock.l_start = 0;

    fileLock.l_len = f.size();

    fileLock.l_type = F_UNLCK;

    if (::fcntl(f.handle(), F_SETLK, &fileLock) != -1)
        return true;
    else
        return false;

}

/*!
  Could a request to lock file with given flags succeed
  \a f must be an opened file
  \a flags the desired lock type required
 */
bool Qtopia::isFileLocked(QFile &f, int /* flags */)
{
    struct flock fileLock;

    if (!f.isOpen())
        return false;

    fileLock.l_whence = SEEK_SET;
    fileLock.l_start = 0;

    fileLock.l_len = f.size();

    if (f.openMode() == QIODevice::ReadOnly)
        fileLock.l_type = F_RDLCK;
    else
        fileLock.l_type = F_WRLCK;

    fileLock.l_pid = 0;

    if (::fcntl(f.handle(), F_SETLK, &fileLock) != -1)
        return false;

    return fileLock.l_pid != 0;
}


#else

/*!
  Lock region of file. Any locks created should be released before
  the program exits. Returns true if sucessfull.

  \a f must be an open file
  \a flags any combination of LockShare, LockWrite, LockBlock
*/
bool Qtopia::lockFile(QFile &f, int /*flags*/)
{
    // If the file has been opened then a lock has been achieved
    return f.isOpen();
}

/*!
  Unlock a region of file
  \a f must be an open file previously locked
 */
bool Qtopia::unlockFile(QFile & /*f*/)
{
    // No need to do anything as we do not open file using sharing
    return true;
}

/*!
  Could a request to lock file with given flags succeed
  \a f must be an opened file
  \a flags the desired lock type required
 */
bool Qtopia::isFileLocked(QFile &f, int /*flags*/)
{
    // if the file is open then we must have achieved a file lock
    return f.isOpen();
}

#endif

/*!
  Returns true if the application can expect the user to
  be able to easily produce \l {QMouseEvent}{mouse events}.

  Returns false if the user \i cannot produce mouse events,
  or chooses not to (eg. does not plug in a mouse).

  Applications may use this to tune interactions.

  Note that you should only call this function after an instance of
  QtopiaApplication has been created. This function will return an undefined
  value if called prior to this.
*/
bool Qtopia::mousePreferred()
{
#ifdef QTOPIA_PHONE
    return !qApp->keypadNavigationEnabled(); //keypad phone
#else
    return true;
#endif
}

/*!
  Returns true if \a key is available on the device.  The buttons
  may be either fixed function buttons, such as Key_Menu, or user
  mappable buttons, such as Key_F1.

  \sa QSoftMenuBar, QDeviceButtonManager
 */
bool Qtopia::hasKey(int key)
{
    static QList<int> *buttons = 0;

    if (!buttons) {
        buttons = new QList<int>;
        QSettings cfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat);
        cfg.beginGroup("SystemButtons");
        if (cfg.contains("Count")) {
            int count = cfg.value("Count", 0).toInt();
            if (count) {
                for (int i = 0; i < count; i++) {
                    QString is = QString::number(i);
                    buttons->append(QKeySequence(cfg.value("Key"+is).toString())[0]);
                }
            }
        }
    }

    return (*buttons).contains(key);
}

/*!
  Executes the application identfied by \a app, passing \a
  document if it isn't null.

  Note that a better approach might be to send a QCop message to the
  application's QPE/Application/\i{appname} channel.
*/
#ifdef Q_WS_QWS
void Qtopia::execute( const QString &app, const QString& document )
{
    if ( document.isNull() ) {
        QtopiaServiceRequest e( "Launcher", "execute(QString)" );
        e << app;
        e.send();
    } else {
        QtopiaServiceRequest e( "Launcher", "execute(QString,QString)" );
        e << app << document;
        e.send();
    }
}
#endif

/*!
  Returns the string \a s with the characters '\', '"', and '$' quoted
  by a preceeding '\', and enclosed by double-quotes (").

  \sa stringQuote()
*/
QString Qtopia::shellQuote(const QString& s)
{
    QString r="\"";
    for (int i=0; i<(int)s.length(); i++) {
        char c = s[i].toLatin1();
        switch (c) {
            case '\\': case '"': case '$':
                r+="\\";
        }
    r += s[i];
    }
    r += "\"";
    return r;
}

/*!
  Returns the string \a s with the characters '\' and '"' quoted by a
  preceeding '\'.

  \sa shellQuote()
*/
QString Qtopia::stringQuote(const QString& s)
{
    QString r="\"";
    for (int i=0; i<(int)s.length(); i++) {
        char c = s[i].toLatin1();
        switch (c) {
            case '\\': case '"':
                r+="\\";
        }
        r += s[i];
    }
    r += "\"";
    return r;
}


/*!
  Returns true if the user regards their week as starting on Monday.
  Returns false if the user regards their week as starting on Sunday.
*/
bool Qtopia::weekStartsOnMonday()
{
    QSettings config("Trolltech","qpe");
    config.beginGroup( "Time" );
    return config.value("MONDAY", false).toBool();
}

/*!
  Sets the day the user regards their week starting on.
  If \a v is true, then the week begins on Monday.
  If \a v is false, then the week begins on Sunday.
*/
void Qtopia::setWeekStartsOnMonday(bool v)
{
    QSettings config("Trolltech","qpe");
    config.beginGroup( "Time" );
    config.setValue( "MONDAY", v );
}


extern bool mkdirRecursive( QString path );

#ifdef Q_WS_QWS

static bool docDirCreated = false;
static QDawg* fixed_dawg = 0;
static QMap<QString,QDawg*> *named_dawg = 0;

static void clean_fixed_dawg()
{
    if ( fixed_dawg ) {
        delete fixed_dawg;
        fixed_dawg = 0;
    }
}

static void clean_named_dawg()
{
    if ( named_dawg ) {
        QList<QString> keys = named_dawg->keys();
        foreach ( QString k, keys ) {
            QDawg* dawg = named_dawg->take( k );
            if ( dawg )
                delete dawg;
        }
        delete named_dawg;
        named_dawg = 0;
    }
}

static QString dictDir()
{
    // Directory for fixed dawgs
    return Qtopia::qtopiaDir() + "etc/dict";
}

void qtopia_load_fixedwords(QDawg* dawg, const QString& dictname, const QString& l)
{
    QString words_lang;
    QString basename = dictDir() + '/';
    //non-translatable dictionaries
    QString dawgfilename = basename + dictname + ".dawg";

    if ( l.isEmpty() ) {
        QStringList langs = Qtopia::languageList();
        foreach( QString lang, langs ) {
            words_lang = basename + lang + '/' + dictname;
            QString dawgfilename_lang = words_lang + ".dawg";
            if ( QFile::exists(dawgfilename_lang) ||
                    QFile::exists(words_lang) ) {
                dawgfilename = dawgfilename_lang;
                break;
            }
        }
    } else {
        words_lang = basename + l + '/' + dictname;
        QString dawgfilename_lang = words_lang + ".dawg";
        if ( QFile::exists(dawgfilename_lang) ||
                QFile::exists(words_lang) ) {
            dawgfilename = dawgfilename_lang;
        }
    }
    if ( dawgfilename.isEmpty() || !QFile::exists(dawgfilename) ) {
        // Not recommended to generate the dawgs from the word lists
        // on a device (slow), but we put it here so eg. SDK can easily
        // generate dawgs as required.
        QString fn = QFile::exists(words_lang) ? words_lang : QString();
        if ( fn.isEmpty() )
            return;
        qWarning("Generating '%s' dawg from word list.", (const char *)fn.toLatin1());
        QFile in(fn);
        QFile dawgfile(fn+".dawg");
        if ( in.open(QIODevice::ReadOnly) && dawgfile.open(QIODevice::WriteOnly) ) {
            dawg->createFromWords(&in);
            dawg->write(&dawgfile);
            dawgfile.close();
        }
    } else {
        dawg->readFile(dawgfilename);
    }
}

void qtopia_createDocDir()
{
    if ( !docDirCreated ) {
        QDir d;
        if (!d.exists(Qtopia::documentDir().toLatin1())){
            docDirCreated = true;
#ifndef Q_WS_WIN32
            mkdir( Qtopia::documentDir().toLatin1(), 0755 );
#else
            mkdirRecursive(Qtopia::documentDir());
#endif
        }else{
            docDirCreated = true;
        }
    }
}

static void setDawgWords(const QString& dictname, const QStringList& words)
{
    QDawg& d = (QDawg&)Qtopia::dawg(dictname);
    d.createFromWords(words);

    QString dawgfilename = Qtopia::applicationFileName("Dictionary", dictname) + ".dawg"; // No tr
    QString dawgfilenamenew = dawgfilename + ".new";
    QFile dawgfile(dawgfilenamenew);
    if ( dawgfile.open(QIODevice::WriteOnly) ) {
        d.write(&dawgfile);
        dawgfile.close();
        ::rename(QFile::encodeName(dawgfilenamenew), QFile::encodeName(dawgfilename)); // Cannot use QFile::rename() here as it wont overwrite dawgfilename
    }

    // Signal *other* processes to re-read.
    QtopiaIpcEnvelope e( "QPE/System", "wordsChanged(QString,int)" );
    e << dictname << (int)::getpid();
}


/*!
  Returns the unchangeable QDawg that contains general
  words for the current locale.

  \sa addedDawg()
*/
const QDawg& Qtopia::fixedDawg()
{
    if ( !fixed_dawg ) {
        if ( !docDirCreated )
            qtopia_createDocDir();

        fixed_dawg = new QDawg;
        qAddPostRoutine( clean_fixed_dawg );
        qtopia_load_fixedwords(fixed_dawg,"words", QString());
    }

    return *fixed_dawg;
}

/*!
  Returns the changeable QDawg that contains general
  words for the current locale.

  \sa fixedDawg()
*/
const QDawg& Qtopia::addedDawg()
{
    return dawg("local"); // No tr
}

/*!
  Returns the QDawg with the given \a name.
  This is an application-specific word list.

  \a name should not contain "/". If \a name starts
  with "_", it is a read-only system word list.

  \a language allows the specific selection of a dictionary for
  a particular language. This option applies to read-only system
  word list only and is ignored otherwise. If no language is
  given the dictionary of the current language is loaded.

*/
const QDawg& Qtopia::dawg(const QString& name, const QString& language)
{
    QString augmentedName;
    if ( name[0] == '_' ) {
        augmentedName=name+QChar('.')+language;
    } else {
        augmentedName = name;
    };

    if ( !named_dawg ) {
        named_dawg = new QMap<QString,QDawg*>;
        qAddPostRoutine( clean_named_dawg );
    }
    QDawg* r = named_dawg->value(augmentedName);

    if ( !r ) {
        qtopia_createDocDir();
        r = new QDawg;
        (*named_dawg)[augmentedName] = r;

        if ( !r->root() ) {
            if ( augmentedName[0] == '_' ) {
                QString n = name.mid(1);
                qtopia_load_fixedwords(r, n, language);
            } else {
                Qtopia::qtopiaReloadWords(augmentedName);
            }
        }
    }

    return *r;
}

/*!
  Adds \a wordlist to the addedDawg(). Words that are already there
  are not added. Words that are in the 'deleted' dictionary are
  removed from there.

  Note that the addition of words persists between program executions
  (they are saved in the dictionary files), so you should confirm the
  words with the user before adding them.

  This is a slow operation. Call it once with a large list rather than
  multiple times with a small list.
*/
void Qtopia::addWords(const QStringList& wordlist)
{
    QStringList toadd;
    QStringList toundel;
    QDawg& del = (QDawg&)dawg("deleted"); // No tr
    for (QStringList::ConstIterator it=wordlist.begin(); it!=wordlist.end(); ++it) {
        if ( del.contains(*it) )
            toundel.append(*it);
        else
            toadd.append(*it);
    }
    addWords("local",toadd); // No tr
    removeWords("deleted",toundel); // No tr
}

/*!
  Adds \a wordlist to the dawg() named \a dictname.

  Note that the addition of words persists between program executions
  (they are saved in the dictionary files), so you should confirm the
  words with the user before adding them.

  This is a slow operation. Call it once with a large list rather than
  multiple times with a small list.
*/
void Qtopia::addWords(const QString& dictname, const QStringList& wordlist)
{
    if ( wordlist.isEmpty() )
        return;
    QDawg& d = (QDawg&)dawg(dictname);
    QStringList all = d.allWords() + wordlist;
    setDawgWords(dictname,all);
}

/*!
  Removes \a wordlist from the addedDawg(). If the words are in
  the fixed dictionary, they are added to the "deleted" dictionary.

  This is a slow operation. Call it once with a large list rather than
  multiple times with a small list.
*/
void Qtopia::removeWords(const QStringList& wordlist)
{
    if ( wordlist.isEmpty() )
        return;
    QDawg& d = (QDawg&)dawg("local");
    QStringList loc = d.allWords();
    int nloc = loc.count();
    QStringList del;
    for (QStringList::ConstIterator it=wordlist.begin(); it!=wordlist.end(); ++it) {
        loc.removeAll(*it);
        if ( fixedDawg().contains(*it) )
            del.append(*it);
    }
    if ( nloc != (int)loc.count() )
        setDawgWords("local",loc);
    addWords("deleted",del);
}

/*!
  Removes \a wordlist from the dawg() named \a dictname.

  This is a slow operation. Call it once with a large list rather than
  multiple times with a small list.
*/
void Qtopia::removeWords(const QString& dictname, const QStringList& wordlist)
{
    QDawg& d = (QDawg&)dawg(dictname);
    QStringList all = d.allWords();
    for (QStringList::ConstIterator it=wordlist.begin(); it!=wordlist.end(); ++it)
        all.removeAll(*it);
    setDawgWords(dictname,all);
}

/*!
  Reloads new words from a dawg dictionary called \a dictname.
*/
void Qtopia::qtopiaReloadWords(const QString& dictname)
{
    // Reload dictname, if we have it loaded.
    if ( named_dawg ) {
        QDawg* r = named_dawg->value(dictname);
        if ( r ) {
            QString dawgfilename = Qtopia::applicationFileName("Dictionary", dictname) + ".dawg"; // No tr
            QFile dawgfile(dawgfilename);
            if ( dawgfile.open(QIODevice::ReadOnly) )
                r->readFile(dawgfilename);
        }
    }
}

#endif //Q_WS_QWS
