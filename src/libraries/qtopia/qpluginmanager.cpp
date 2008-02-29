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

#include "qpluginmanager.h"
#include <signal.h>
#ifndef QTOPIA_CONTENT_INSTALLER
#include <qtopiaapplication.h>
#endif
#include <qtopianamespace.h>

#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QPluginLoader>
#include <QSettings>
#include <QTimer>
#include <QTranslator>
#include <QMap>
#include <QPointer>

#include <stdlib.h>
#ifndef Q_OS_WIN32
# include <sys/stat.h>
# include <sys/types.h>
# include <fcntl.h>
# include <unistd.h>
# include <signal.h>
# include <dirent.h>
#endif

// Qtopia can try to disabled broken plugins automatically
// on an individual basis. This has performance implications.
//
//#define QTOPIA_DISABLE_ONLY_BROKEN_PLUGINS

Q_DECLARE_METATYPE(QPluginLoader *);

#ifdef SINGLE_EXEC
typedef QObject *(*qtopiaPluginCreateFunc_t)();
typedef QMap <QString, QPointer<QObject> > PluginInstanceDict;
typedef QMap <QString, PluginInstanceDict> PluginTypeInstanceDict;
typedef QMap <QString, qtopiaPluginCreateFunc_t> PluginNameDict;
typedef QMap <QString, PluginNameDict> PluginTypeNameDict;
static PluginTypeNameDict &se_ptd()
{
    static PluginTypeNameDict *d = 0;
    if ( !d )
        d = new PluginTypeNameDict();
    return *d;
}
static PluginTypeInstanceDict &se_pid()
{
    static PluginTypeInstanceDict *d = 0;
    if ( !d )
        d = new PluginTypeInstanceDict();
    return *d;
}
void registerPlugin(const char *name, const char *type, qtopiaPluginCreateFunc_t createFunc)
{
    se_ptd()[QLatin1String(type)][QLatin1String(name)] = createFunc;
}
#else
class PluginLibraryManager : public QObject
{
    Q_OBJECT
public:
    PluginLibraryManager();
    ~PluginLibraryManager();

    QPluginLoader *refLibrary( const QString &file );

public slots:
    void derefLibrary( QPluginLoader *lib );

private:
    QMap<QString,QPluginLoader*> libs;
};

static QPointer<PluginLibraryManager> manager = 0;

PluginLibraryManager *pluginLibraryManagerInstance()
{
    if ( !manager )
        manager = new PluginLibraryManager;

    return manager;
}
#endif

static QString configFilename( const QString &name )
{
    QString homeDirPath = Qtopia::homePath();

    QDir dir = (homeDirPath + "/Settings");
    if ( !dir.exists() ) {
#ifndef Q_OS_WIN32
        mkdir(dir.path().toLocal8Bit(),0700);
#else
        dir.mkdir(dir.path());
#endif
    }

    return dir.path() + "/" + name + ".conf";
}

#ifndef Q_OS_WIN32
static bool lockFile( QFile &f )
{
    if (!f.isOpen())
        return false;

    struct flock fileLock;

    fileLock.l_whence = SEEK_SET;
    fileLock.l_start = 0;
    fileLock.l_len = f.size();
    fileLock.l_type = F_WRLCK;

    return (::fcntl(f.handle(), F_SETLKW, &fileLock) == 0);
}

static bool unlockFile( QFile &f )
{
    if (!f.isOpen())
        return false;

    struct flock fileLock;

    fileLock.l_whence = SEEK_SET;
    fileLock.l_start = 0;
    fileLock.l_len = f.size();
    fileLock.l_type = F_UNLCK;

    return (::fcntl(f.handle(), F_SETLK, &fileLock) == 0);
}

#else

static bool lockFile( QFile &f )
{
    return f.isOpen();
}

static bool unlockFile( QFile & /*f*/ )
{
    return true;
}

#endif

static const char *cfgName()
{
    return "PluginManager";
}


//===========================================================================

/*!
  \class QPluginManager
  \brief The QPluginManager class simplifies plug-in loading and allows plugins to be
  enabled/disabled.

  QPluginManager simplifies loading plugins.  The most common use is to
  iterate over the list of plugins and load each one as follows:

  \code
    QPluginManager pluginManager( "Effects" );
    QStringList list = pluginManager.list();
    QStringList::Iterator it;
    QList<EffectsInterface*> effectList;
    for ( it = list.begin(); it != list.end(); ++it ) {
        QObject *instance = pluginManager->instance(*it);
        EffectsInterface *iface = 0;
        iface = qobject_cast<EffectsInterface*>(instance);
        if (iface) {
            effectList.append( iface );
        }
    }
  \endcode

  \ingroup plugins
*/

class QPluginManagerPrivate
{
public:
    QString type;
    QStringList plugins;
    QStringList disabled;
    QMap<QObject*,QPluginLoader*> interfaces;
};

/*!
  Creates a QPluginManager for plugins of type \a type.

  The plugins must be installed in the [qt_prefix]/plugins/\i{type}  directory.
  \a parent is the standard QObject parameter.
*/

QPluginManager::QPluginManager(const QString &type, QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QPluginLoader *>("QPluginLoader *");
    d = new QPluginManagerPrivate;
    d->type = type;
    init();
    initType();
}

/*!
  Destroys the QPluginManager and releases any resources allocated by
  the PluginManager.
*/
QPluginManager::~QPluginManager()
{
    clear();
    delete d;
}

/*!
  Releases all resources allocated by the QPluginManager
*/
void QPluginManager::clear()
{
    QMutableMapIterator<QObject*,QPluginLoader*> it(d->interfaces);
    while (it.hasNext()) {
        it.next();
        QObject *iface = it.key();
        delete iface;
    }
}

/*! \internal
*/
void QPluginManager::init()
{
#ifndef SINGLE_EXEC
    pluginLibraryManagerInstance();
#endif
}

/*!
  Returns the list of plugins that are available.
*/
const QStringList &QPluginManager::list() const
{
    return d->plugins;
}

/*!
  Returns the list of plugins that have been disabled.
*/
const QStringList &QPluginManager::disabledList() const
{
    return d->disabled;
}

/*!
  Query the plug-in for the interface specified by \a name.

  Returns the named interface if found, otherwise 0.

  \code
    QObject *instance = pluginManager->instance("name");
    if (instance) {
        EffectsInterface *iface = 0;
        iface = qobject_cast<EffectsInterface*>(instance);
        if (iface) {
            // We have an instance of the desired type.
        }
    }
  \endcode

  If an instance is no longer required and resources need to be released,
  simply delete the returned instance.
*/
QObject *QPluginManager::instance( const QString &name )
{
    QObject *iface = 0;
#ifndef SINGLE_EXEC
    QString lname = stripSystem( name );

    QString libFile;
    QStringList qpepaths = Qtopia::installPaths();
    for (QStringList::ConstIterator qit = qpepaths.begin(); qit!=qpepaths.end(); ++qit) {
#ifndef Q_OS_WIN32
        libFile = *qit + "plugins/" + d->type + "/lib" + lname + ".so";
#else
        libFile = *qit + "plugins/" + d->type + "/" + lname + ".dll";
#endif
        if ( QFile::exists(libFile) )
            break;
    }
    QPluginLoader *lib = pluginLibraryManagerInstance()->refLibrary( libFile );
    if ( !lib ) {
        qWarning( "Plugin not loaded: %s", (const char *)lname.toLatin1() );
        return 0;
    }

#ifdef QTOPIA_DISABLE_ONLY_BROKEN_PLUGINS
    bool enabled = isEnabled( name );
    if (enabled)
        setEnabled( name, false );
#endif
    if ( (iface = lib->instance()) ) {
        loaded(iface, lib, name);
    } else {
        pluginLibraryManagerInstance()->derefLibrary( lib );
    }
#ifdef QTOPIA_DISABLE_ONLY_BROKEN_PLUGINS
    if (enabled)
        setEnabled( name, true );
#endif
#else
    iface = se_pid()[d->type][name];
    if ( !iface ) {
        qtopiaPluginCreateFunc_t createFunc = se_ptd()[d->type][name];
        if ( createFunc ) {
            se_pid()[d->type][name] = createFunc();
            iface = se_pid()[d->type][name];
            if ( iface )
                loaded(iface, 0, name);
        }
    }
#endif

    return iface;
}

void QPluginManager::instanceDestroyed()
{
    QObject *iface = sender();
    QPluginLoader *lib = d->interfaces.take(iface);
#ifndef SINGLE_EXEC
    QMetaObject::invokeMethod(pluginLibraryManagerInstance(), "derefLibrary",
        Qt::QueuedConnection, Q_ARG(QPluginLoader *, lib));
#endif
}

void QPluginManager::initType()
{
#ifndef SINGLE_EXEC
    QStringList qpepaths = Qtopia::installPaths();
    for (QStringList::ConstIterator qit = qpepaths.begin(); qit!=qpepaths.end(); ++qit) {
        QString path = *qit + "plugins/";
        path += d->type;
#ifndef Q_OS_WIN32
        DIR *dir = opendir( path.toLatin1() );
        if ( !dir )
            continue;

        QStringList list;
        dirent *file;
        while ( (file = readdir(dir)) ) {
            if ( !strncmp( file->d_name, "lib", 3 ) ) {
                if ( !strcmp( file->d_name+strlen(file->d_name)-3, ".so" ) )
                    list.append( file->d_name );
            }
        }
        closedir(dir);
#else
        QDir dir (path, "*.dll");
        QStringList list = dir.entryList();
#endif

        bool safeMode = false;

        QString cfgFilename( configFilename(cfgName()) + ".lock" );
        QFile lf( cfgFilename );
        lf.open( lf.exists() ? QIODevice::ReadOnly : QIODevice::WriteOnly );
        lockFile( lf );
        {
            QSettings cfg("Trolltech",cfgName());
            cfg.beginGroup( "Global" );
            safeMode = cfg.value( "Mode", "Normal" ).toString() == "Safe";
            cfg.endGroup();
            cfg.beginGroup( d->type );
            d->disabled = cfg.value( "Disabled").toString().split( ',' );
        }
        unlockFile( lf );

        QStringList required;
        if ( QFile::exists( path + "/.directory" ) ) {
            QSettings config(path + "/.directory", QSettings::IniFormat);
            required = config.value( "Required").toString().split( ',' );
        }

        QStringList::Iterator it;
        for ( it = list.begin(); it != list.end(); ++it ) {
            QString name = stripSystem(*it);
            if ( (!safeMode && isEnabled(name)) || required.contains(name) )
                // Discard duplicate plugins preferring those found first
                if( !d->plugins.contains( name ) )
                    d->plugins += name;
        }
    }
#else
    QStringList *pnsl = new QStringList();
    const PluginNameDict &pnd = se_ptd()[d->type];
    foreach ( const QString &key, pnd.keys() )
        pnsl->append(key);
    d->plugins = *pnsl;
#endif
}

QStringList QPluginManager::languageList() const
{
    return Qtopia::languageList();
}

/*!
  Enables or disables plug-in \a name depending on the value of \a enabled.
  A disabled plug-in can still be queried, but it will not be returned by list().
*/
void QPluginManager::setEnabled( const QString &name, bool enabled )
{
    QString lname = stripSystem(name);
    QString cfgFilename( configFilename(cfgName()) + ".lock" );
    QFile lf( cfgFilename );
    lf.open( lf.exists() ? QIODevice::ReadOnly : QIODevice::WriteOnly );
    lockFile( lf );
    {
        QSettings cfg("Trolltech",cfgName());
        cfg.beginGroup( d->type );
        d->disabled = cfg.value( "Disabled").toString().split( ',' );
        bool wasEnabled = d->disabled.contains( lname ) == 0;
        if ( wasEnabled != enabled ) {
            if ( enabled ) {
                d->disabled.removeAll( lname );
            } else {
                d->disabled += lname;
            }
            cfg.setValue("Disabled", d->disabled.join(QString(',' )));
        }
    }
    unlockFile( lf );
}

/*!
  Returns true if the plug-in \a name is enabled.
*/
bool QPluginManager::isEnabled( const QString &name ) const
{
    QString lname = stripSystem(name);
    return d->disabled.indexOf( lname ) == -1;
}

/*!
  Returns true if Qtopia is currently in \i{Safe Mode}.  In safe mode
  list() will return an empty list and no plugins should be loaded.  This
  is to allow misbehaving plugins to be disabled.
*/
bool QPluginManager::inSafeMode()
{
    QSettings cfg("Trolltech",cfgName());
    cfg.beginGroup( "Global" );
    QString mode = cfg.value( "Mode", "Normal" ).toString();
    return ( mode == "Safe" ); // No tr
}

QString QPluginManager::stripSystem( const QString &libFile ) const
{
    QString name = libFile;
#ifndef Q_OS_WIN32
    if ( libFile.lastIndexOf(".so") == (int)libFile.length()-3 ) {
        name = libFile.left( libFile.length()-3 );
        if ( name.indexOf( "lib" ) == 0 )
            name = name.mid( 3 );
    }
#else
    if ( libFile.lastIndexOf(".dll") == (int)libFile.length()-4 )
        name = libFile.left( libFile.length()-4 );
#endif

    return name;
}

void QPluginManager::loaded( QObject *iface, QPluginLoader *lib, QString name )
{
    d->interfaces.insert( iface, lib );
    connect(iface, SIGNAL(destroyed()), this, SLOT(instanceDestroyed()));
#ifndef Q_OS_WIN32
    QString type = QLatin1String("/lib") + name + QLatin1String(".qm");
#else
    QString type = QLatin1Char('/') + name + QLatin1String(".qm");
#endif
    QStringList langs = languageList();
    QStringList qpepaths = Qtopia::installPaths();
    for (QStringList::ConstIterator qit = qpepaths.begin(); qit!=qpepaths.end(); ++qit) {
        QString path(*qit+QLatin1String("i18n/"));
        for (QStringList::ConstIterator lit = langs.begin(); lit!=langs.end(); ++lit) {
            QString tfn = path + *lit + QLatin1Char('/') + type;
            if ( QFile::exists(tfn) ) {
                QTranslator * trans = new QTranslator(qApp);
                if ( trans->load( tfn ))
                    qApp->installTranslator( trans );
                else
                    delete trans;
            }
        }
    }
}

#ifndef SINGLE_EXEC
//===========================================================================
// Only compile this once under Win32 and single process
#if !(defined(Q_OS_WIN32) && defined(PLUGINLOADER_INTERN)) && \
    !(defined(SINGLE_EXEC) && defined(PLUGINLOADER_INTERN)) && \
    !(defined(QPE_NO_COMPAT) && defined(PLUGINLOADER_INTERN))

PluginLibraryManager::PluginLibraryManager() : QObject( qApp )
{
}

PluginLibraryManager::~PluginLibraryManager()
{
    if ( qApp->type() == QApplication::GuiServer ) {
        QSettings cfg("Trolltech",cfgName());
        cfg.beginGroup( "Global" );
        cfg.setValue( "Mode", "Normal" );
    }
}

QPluginLoader *PluginLibraryManager::refLibrary( const QString &file )
{
    QPluginLoader *lib = 0;
    QMap<QString,QPluginLoader*>::const_iterator it = libs.find(file);
    if (it != libs.end())
        lib = *it;
    if ( !lib ) {
        lib = new QPluginLoader( file );
        lib->load();
        if ( !lib->isLoaded() ) {
            delete lib;
            return 0;
        }
    }
    libs.insertMulti( file, lib );

    return lib;
}

void PluginLibraryManager::derefLibrary( QPluginLoader *lib )
{
    if ( !lib )
        return;

    QString file = lib->fileName();
    libs.take( file );
    if ( !libs.contains(file) ) {
        lib->unload();
        delete lib;
    }
}
#endif
#endif

#include "qpluginmanager.moc"
