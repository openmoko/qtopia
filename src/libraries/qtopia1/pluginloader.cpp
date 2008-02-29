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
    return if.isOpen();
}

static bool unlockFile( QFile &f )
{
    return TRUE;
}

#endif

//===========================================================================

PluginLibraryManager::PluginLibraryManager()
{
}

PluginLibraryManager::~PluginLibraryManager()
{
    if ( qApp->type() == QApplication::GuiServer ) {
	Config cfg( "PluginLoader" );
	cfg.setGroup( "Global" );
	cfg.writeEntry( "Mode", "Normal" );
    }
}

QLibrary *PluginLibraryManager::refLibrary( const QString &file )
{
    QLibrary *lib = libs.find( file );
    if ( !lib ) {
#if (QT_VERSION < 300)
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
	qDebug( "Unloading lib: %s", file.latin1() );
	lib->unload();
	delete lib;
    }
}

//===========================================================================

class PluginLoaderPrivate
{
public:
    QString type;
    QStringList plugins;
    QStringList disabled;
    QPtrDict<QLibrary> interfaces;
};

PLUGINLOADER::PLUGINLOADER( const QString &type )
{
    d = new PluginLoaderPrivate;
    d->type = type;
    init();
    initType();
}


PLUGINLOADER::~PLUGINLOADER()
{
    delete d;
}

void PLUGINLOADER::clear()
{
    QPtrDictIterator<QLibrary> it( d->interfaces );
    while ( it.current() ) {
	QUnknownInterface *iface = (QUnknownInterface *)it.currentKey();
	++it;
	releaseInterface( iface );
    }
}

void PLUGINLOADER::init()
{
    LOADER_INSTANCE();
}

const QStringList &PLUGINLOADER::list() const
{
    return d->plugins;
}

const QStringList &PLUGINLOADER::disabledList() const
{
    return d->disabled;
}

QRESULT PLUGINLOADER::queryInterface( const QString &name, const QUuid &id, QUnknownInterface **iface )
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
	LOADER_INSTANCE()->derefLibrary( lib );
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
	    QTranslator * trans = new QTranslator(qApp);
	    QString tfn = QPEApplication::qpeDir()+"i18n/"+lang+"/"+type+".qm";
	    if ( trans->load( tfn ))
		qApp->installTranslator( trans );
	    else
		delete trans;
	}
	result = QS_OK;
    } else {
	LOADER_INSTANCE()->derefLibrary( lib );
    }
    if (enabled)
	setEnabled( name, TRUE );

    return result;
}

void PLUGINLOADER::releaseInterface( QUnknownInterface *iface )
{
    if ( iface ) {
	QLibrary *lib = d->interfaces.take( iface );
	iface->release();
	LOADER_INSTANCE()->derefLibrary( lib );
    }
}

void PLUGINLOADER::initType()
{
    QString path = QPEApplication::qpeDir() + "plugins/";
    path += d->type;
#ifndef Q_OS_WIN32
    QDir dir( path, "lib*.so" );
#else
    QDir dir (path, "*.dll");
#endif

    bool safeMode = FALSE;

    QString cfgFilename( configFilename("PluginLoader") + ".lock" );
    QFile lf( cfgFilename );
    lf.open( lf.exists() ? IO_ReadOnly : IO_WriteOnly );
    lockFile( lf );
    Config cfg( "PluginLoader" );
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

    QStringList list = dir.entryList();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	QString name = stripSystem(*it);
	if ( (!safeMode && isEnabled(name)) || required.contains(name) )
	    d->plugins += name;
    }
}

QStringList PLUGINLOADER::languageList() const
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

void PLUGINLOADER::setEnabled( const QString &name, bool enabled )
{
    QString lname = stripSystem(name);
    QString cfgFilename( configFilename("PluginLoader") + ".lock" );
    QFile lf( cfgFilename );
    lf.open( lf.exists() ? IO_ReadOnly : IO_WriteOnly );
    lockFile( lf );
    Config cfg( "PluginLoader" );
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
    unlockFile( lf );
}

bool PLUGINLOADER::isEnabled( const QString &name ) const
{
    QString lname = stripSystem(name);
    return d->disabled.find( lname ) == d->disabled.end();
}

bool PLUGINLOADER::inSafeMode()
{
    Config cfg( "PluginLoader" );
    cfg.setGroup( "Global" );
    QString mode = cfg.readEntry( "Mode", "Normal" );
    return ( mode == "Safe" ); // No tr
}

QString PLUGINLOADER::stripSystem( const QString &libFile ) const
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

