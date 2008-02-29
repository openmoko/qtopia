/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "pluginloader.h"
#include "pluginloaderlib_p.h"
#include <signal.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qdir.h>
#include <qfile.h>
#include <qdict.h>
#include <qptrdict.h>
#include <qtimer.h>
#include <qmessagebox.h>

#include <stdlib.h>
#ifndef Q_OS_WIN32
# include <sys/stat.h>
# include <sys/types.h>
# include <fcntl.h>
# include <unistd.h>
# include <signal.h>
# include <dirent.h>
#endif

#ifdef PLUGINLOADER_INTERN
# define LOADER_INSTANCE pluginLibraryManagerInstanceIntern
#else
# define LOADER_INSTANCE pluginLibraryManagerInstance
#endif

extern PluginLibraryManager *LOADER_INSTANCE();

static QString configFilename( const QString &name )
{
    QString homeDirPath = QDir::homeDirPath();

    QDir dir = (homeDirPath + "/Settings");
    if ( !dir.exists() )
#ifndef Q_OS_WIN32
	mkdir(dir.path().local8Bit(),0700);
#else
    dir.mkdir(dir.path());
#endif

    return dir.path() + "/" + name + ".conf";
}

#ifndef Q_OS_WIN32
static bool lockFile( QFile &f )
{
    if (!f.isOpen())
	return FALSE;

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
	return FALSE;

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

static bool unlockFile( QFile &f )
{
    return TRUE;
}

#endif

static const char *cfgName();

//===========================================================================

/*!
  \class PluginLoader pluginloader.h
  \brief The PluginLoader class simplifies plugin loading and allows plugins to be
  enabled/disabled.

  PluginLoader simplifies loading plugins.  The most common use is to
  iterate over the list of plugins and load each one:

  \code
    PluginLoader loader( "Effects" );
    QStringList list = pluginLoader.list();
    QStringList::Iterator it;
    QValueList<EffectsInterface*> effectList;
    for ( it = list.begin(); it != list.end(); ++it ) {
	EffectsInterface *iface = 0;
	if ( pluginLoader.queryInterface( *it, IID_Effects, (QUnknownInterface**)&iface ) == QS_OK && iface ) {
	    effectList.append( iface );
	}
    }
  \endcode

  \ingroup qtopiaemb
*/

class PluginLoaderPrivate
{
public:
    QString type;
    QStringList plugins;
    QStringList disabled;
    QPtrDict<QLibrary> interfaces;
};

/*!
  Creates a PluginLoader for plugins of type \a type.

  The plugins must be installed in the $QPEDIR/plugins/<i>type<i> directory.
*/

PluginLoader::PluginLoader( const QString &type )
{
    d = new PluginLoaderPrivate;
    d->type = type;
    init();
    initType();
}

/*!
  Destroys the PluginLoader.  Note that this does not release any
  interfaces.
*/
PluginLoader::~PluginLoader()
{
    delete d;
}

/*!
  Releases all interfaces.
*/
void PluginLoader::clear()
{
    QPtrDictIterator<QLibrary> it( d->interfaces );
    while ( it.current() ) {
	QUnknownInterface *iface = (QUnknownInterface *)it.currentKey();
	++it;
	releaseInterface( iface );
    }
}

/*! \internal
*/
void PluginLoader::init()
{
    LOADER_INSTANCE();
}

/*!
  Returns the list of plugins that are available.
*/
const QStringList &PluginLoader::list() const
{
    return d->plugins;
}

/*!
  Returns the list of plugins that have been disabled.
*/
const QStringList &PluginLoader::disabledList() const
{
    return d->disabled;
}

/*!
  Query the plugin for the interface specified by \a id in plugin \a name.
  If the interface is available, \a iface will contain the pointer to it.

  Returns QS_OK if the interface was found or QS_FALSE otherwise.
*/
QRESULT PluginLoader::queryInterface( const QString &name, const QUuid &id, QUnknownInterface **iface )
{
    QRESULT result = QS_FALSE;
    *iface = 0;

    QString lname = stripSystem( name );

#ifndef Q_OS_WIN32
    QString libFile = QPEApplication::qpeDir() + "plugins/" + d->type + "/lib" + lname + ".so";
#else
    QString libFile = QPEApplication::qpeDir() + "plugins/" + d->type + "/" + lname + ".dll";
#endif
    QLibrary *lib = LOADER_INSTANCE()->refLibrary( libFile );
    if ( !lib ) {
	qDebug( "Plugin not loaded: %s", lname.latin1() );
	return result;
    }

    bool enabled = isEnabled( name );
    if (enabled)
	setEnabled( name, FALSE );
    if ( lib->queryInterface( id, iface ) == QS_OK && iface ) {
	d->interfaces.insert( *iface, lib );
	QString type = name;
#ifndef Q_OS_WIN32
	type = "lib" + type;
#endif
	QStringList langs = languageList();
	for (QStringList::ConstIterator lit = langs.begin(); lit!=langs.end(); ++lit) {
	    QString lang = *lit;
	    QString tfn = QPEApplication::qpeDir()+"i18n/"+lang+"/"+type+".qm";
	    if ( QFile::exists(tfn) ) {
		QTranslator * trans = new QTranslator(qApp);
		if ( trans->load( tfn ))
		    qApp->installTranslator( trans );
		else
		    delete trans;
	    }
	}
	result = QS_OK;
    } else {
	LOADER_INSTANCE()->derefLibrary( lib );
    }
    if (enabled)
	setEnabled( name, TRUE );

    return result;
}

/*!
  Releases the interface \a iface.  The interface must have previously been
  returned by queryInterface.
*/
void PluginLoader::releaseInterface( QUnknownInterface *iface )
{
    if ( iface ) {
	QLibrary *lib = d->interfaces.take( iface );
	iface->release();
	LOADER_INSTANCE()->derefLibrary( lib );
    }
}

void PluginLoader::initType()
{
    QString path = QPEApplication::qpeDir() + "plugins/";
    path += d->type;
#ifndef Q_OS_WIN32
    DIR *dir = opendir( path.latin1() );
    if ( !dir )
	return;

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

    bool safeMode = FALSE;

    QString cfgFilename( configFilename(cfgName()) + ".lock" );
    QFile lf( cfgFilename );
    lf.open( lf.exists() ? IO_ReadOnly : IO_WriteOnly );
    lockFile( lf );
    Config cfg( cfgName() );
    cfg.setGroup( "Global" );
    safeMode = cfg.readEntry( "Mode", "Normal" ) == "Safe";
    cfg.setGroup( d->type );
    d->disabled = cfg.readListEntry( "Disabled", ',' );
    unlockFile( lf );

    QStringList required;
    if ( QFile::exists( path + "/.directory" ) ) {
	Config config( path + "/.directory", Config::File );
	required = config.readListEntry( "Required", ',' );
    }

    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	QString name = stripSystem(*it);
	if ( (!safeMode && isEnabled(name)) || required.contains(name) )
	    d->plugins += name;
    }
}

QStringList PluginLoader::languageList() const
{
    QString lang;
    if (lang.isEmpty())
	lang = getenv("LANG");

    QStringList langs;
    langs.append(lang);
    int i  = lang.find(".");
    if ( i > 0 )
	lang = lang.left( i );
    i = lang.find( "_" );
    if ( i > 0 )
	langs.append(lang.left(i));
    return langs;
}

/*!
  Enables or disables plugin \a name depending on the value of \a enabled.
  A disabled plugin can still be queried, but it will not be returned by list().
*/
void PluginLoader::setEnabled( const QString &name, bool enabled )
{
    QString lname = stripSystem(name);
    QString cfgFilename( configFilename(cfgName()) + ".lock" );
    QFile lf( cfgFilename );
    lf.open( lf.exists() ? IO_ReadOnly : IO_WriteOnly );
    lockFile( lf );
    {
	Config cfg( cfgName() );
	cfg.setGroup( d->type );
	d->disabled = cfg.readListEntry( "Disabled", ',' );
	bool wasEnabled = d->disabled.contains( lname ) == 0;
	if ( wasEnabled != enabled ) {
	    if ( enabled ) {
		d->disabled.remove( lname );
	    } else {
		d->disabled += lname;
	    }
	    cfg.writeEntry( "Disabled", d->disabled, ',' );
	}
    }
    unlockFile( lf );
}

/*!
  Returns TRUE is the plugin \a name is enabled.
*/
bool PluginLoader::isEnabled( const QString &name ) const
{
    QString lname = stripSystem(name);
    return d->disabled.find( lname ) == d->disabled.end();
}

/*!
  Returns TRUE if Qtopia is currently in <i>Safe Mode</i>.  In Safe Mode
  list() will return an empty list and no plugins should be loaded.  This
  is to allow misbehaving plugins to be disbled.
*/
bool PluginLoader::inSafeMode()
{
    Config cfg( cfgName() );
    cfg.setGroup( "Global" );
    QString mode = cfg.readEntry( "Mode", "Normal" );
    return ( mode == "Safe" ); // No tr
}

QString PluginLoader::stripSystem( const QString &libFile ) const
{
    QString name = libFile;
#ifndef Q_OS_WIN32
    if ( libFile.findRev(".so") == (int)libFile.length()-3 ) {
	name = libFile.left( libFile.length()-3 );
	if ( name.find( "lib" ) == 0 )
	    name = name.mid( 3 );
    }
#else
    if ( libFile.findRev(".dll") == (int)libFile.length()-4 )
	name = libFile.left( libFile.length()-4 );
#endif

    return name;
}

//===========================================================================
// Only compile this once under Win32 and single process
#if !(defined(Q_OS_WIN32) && defined(PLUGINLOADER_INTERN)) && \
    !(defined(SINGLE_APP) && defined(PLUGINLOADER_INTERN))

PluginLibraryManager::PluginLibraryManager()
{
}

PluginLibraryManager::~PluginLibraryManager()
{
    if ( qApp->type() == QApplication::GuiServer ) {
	Config cfg( cfgName() );
	cfg.setGroup( "Global" );
	cfg.writeEntry( "Mode", "Normal" );
    }
}

QLibrary *PluginLibraryManager::refLibrary( const QString &file )
{
    QLibrary *lib = libs.find( file );
    if ( !lib ) {
#if (QT_VERSION < 0x030000)
	lib = new QLibrary( file, QLibrary::Immediately );
#else
	lib = new QLibrary( file );
#endif
	if ( !lib->isLoaded() ) {
	    delete lib;
	    return 0;
	}
    }
    libs.insert( file, lib );

    return lib;
}

void PluginLibraryManager::derefLibrary( QLibrary *lib )
{
    if ( !lib )
	return;

    QString file = lib->library();
    libs.remove( file );
    if ( !libs.find( file ) ) {
	lib->unload();
	delete lib;
    }
}
#endif
//===========================================================================

#undef PluginLoader
static const char *cfgName()
{
    return "PluginLoader";
}

