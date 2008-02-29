/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QTOPIA_INTERNAL_PRELOADACCESS
#define QTOPIA_INTERNAL_PRELOADACCESS
#endif

#include "server.h"
#include "serverapp.h"

#ifdef QTOPIA_PHONE
#include "phone/phoneimpl.h"
#else
#include "pda/launcher.h"
#endif

#include "pda/startmenu.h"
#include "transferserver.h"
#include "qcopbridge.h"
#include "irserver.h"
#include "packageslave.h"
#include "qrsync.h"
#include "syncdialog.h"
#include "shutdownimpl.h"
#include "applauncher.h"
#include "suspendmonitor.h"
#include "documentlist.h"
#include "qrr.h"

#include <qtopia/applnk.h>
#include <qtopia/categories.h>
#include <qtopia/mimetype.h>
#include <qtopia/config.h>
#include <qtopia/services.h>
#include <qtopia/devicebuttonmanager.h>
#include <qtopia/pluginloader.h>
#include <qtopia/resource.h>
#include <qtopia/version.h>
#include <qtopia/storage.h>
#include <qtopia/qprocess.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/global.h>
#include <qtopia/custom.h>
#include <qtopia/timezone.h>
#include <qtopia/timeconversion.h>

#if defined(QPE_NEED_CALIBRATION)
#include "../settings/calibrate/calibrate.h"
#endif

#include <qwindowsystem_qws.h>
#include <qgfx_qws.h>

#ifdef Q_OS_WIN32
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#endif
#include <qmainwindow.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <qtextstream.h>
#include <qsound.h>

#include <sys/time.h>
#include <time.h>
#include <stdlib.h>

#define QSS_DEBUG


extern QRect qt_maxWindowRect;

#if defined(QPE_NEED_CALIBRATION)
static QWidget *calibrate(bool)
{
    Calibrate *c = new Calibrate;
    c->show();
    return c;
}
#endif

#ifdef QTOPIA_PHONE
#include <qtopia/services.h>
static QWidget *callhistory(bool)
{
    QCopEnvelope env( "QPE/Application/qpe", "showCallHistory(int,QString)" );
    env << 0 << QString::null;
    return 0;
}
#endif

#define FACTORY(T) \
    static QWidget *new##T( bool maximized ) { \
	QWidget *w = new T( 0, 0, QWidget::WDestructiveClose | QWidget::WGroupLeader ); \
	if ( maximized ) { \
	    if ( qApp->desktop()->width() <= 350 ) { \
		w->showMaximized(); \
	    } else { \
		w->resize( QSize( 300, 300 ) ); \
	    } \
	} \
	w->show(); \
	return w; \
    }


#ifdef SINGLE_APP
#define APP(a,b,c,d) FACTORY(b)
#include "apps.h"
#undef APP
#endif // SINGLE_APP

static Global::Command builtins[] = {

#ifdef SINGLE_APP
#define APP(a,b,c,d) { a, new##b, c, d },
#include "apps.h"
#undef APP
#endif

#if defined(QPE_NEED_CALIBRATION)
    { "calibrate",          calibrate,           1, 0 }, // No tr
#endif
#if !defined(QT_QWS_CASSIOPEIA)
    { "shutdown",           Global::shutdown,    1, 0 }, // No tr
//  { "run",                run,                 1, 0 }, // No tr
#endif
#ifdef QTOPIA_PHONE
    { "callhistory",        callhistory,         1, 0 },
#endif
//  { 0,                    calibrate,           0, 0 }
    { 0,                    0,                   0, 0 } // calibrate app is not always provided, so end with a zero pointer
};

#ifdef QPE_HAVE_DIRECT_ACCESS
extern void readyDirectAccess(QString cardInfo, QString installLocations);
extern const char *directAccessQueueFile();
#endif


//---------------------------------------------------------------------------

static Server *g_serverInstance = 0;

//===========================================================================

Server::Server() :
    QWidget( 0, 0, WStyle_Tool | WStyle_Customize ),
    qcopBridge( 0 ),
    transferServer( 0 ),
    packageHandler( 0 ),
    syncDialog( 0 ),
    soundserver( 0 ),
    qrr( 0 ),
    lastStartedApp( 0 )
{
    g_serverInstance = this;

    Global::setBuiltinCommands(builtins);

    tid_xfer = 0;

    tsmMonitor = new TempScreenSaverMonitor();
    connect( tsmMonitor, SIGNAL(forceSuspend()), qApp, SIGNAL(power()) );

#ifdef QTOPIA_PHONE
    serverGui = new PhoneImpl;
#else
    serverGui = new Launcher;
#endif
    serverGui->createGUI();

    docList = new DocumentList( serverGui );
    appLauncher = new AppLauncher(this);
    connect(appLauncher, SIGNAL(raised(const QString&)), this, SLOT(applicationRaised(const QString&)) );
    connect(appLauncher, SIGNAL(launched(int,const QString&)), this, SLOT(applicationLaunched(int,const QString&)) );
    connect(appLauncher, SIGNAL(terminated(int,const QString&)), this, SLOT(applicationTerminated(int,const QString&)) );
    connect(appLauncher, SIGNAL(connected(const QString&)), this, SLOT(applicationConnected(const QString&)) );

    storage = new StorageInfo( this );
    connect( storage, SIGNAL(disksChanged()), this, SLOT(storageChanged()) );

#ifdef QPE_HAVE_DIRECT_ACCESS
    QCopChannel *desktopChannel = new QCopChannel( "QPE/Desktop", this );
    connect( desktopChannel, SIGNAL(received( const QCString &, const QByteArray & )),
	     this, SLOT(desktopMessage( const QCString &, const QByteArray & )) );
#endif

    // Gives system 5 seconds to start up (on PDA it needs that much time)
    qssTimerId = startTimer(10);

    // start services
    startTransferServer();
    (void) new IrServer( this );

    packageHandler = new PackageHandler( this );
    connect(qApp, SIGNAL(activate(const DeviceButton*,bool)),this,SLOT(activate(const DeviceButton*,bool)));

    setGeometry( -10, -10, 9, 9 );

    QCopChannel *channel = new QCopChannel("QPE/System", this);
    connect(channel, SIGNAL(received(const QCString&,const QByteArray&)),
	    this, SLOT(systemMsg(const QCString&,const QByteArray&)) );

    QCopChannel *tbChannel = new QCopChannel( "QPE/TaskBar", this );
    connect( tbChannel, SIGNAL(received(const QCString&,const QByteArray&)),
	    this, SLOT(receiveTaskBar(const QCString&,const QByteArray&)) );

    connect( qApp, SIGNAL(prepareForRestart()), this, SLOT(terminateServers()) );
    connect( qApp, SIGNAL(timeChanged()), this, SLOT(pokeTimeMonitors()) );

    preloadApps();
}

Server *Server::instance() 
{
    return g_serverInstance;
}

DocumentList *Server::documentList() const
{
    return docList;
}

void Server::show()
{
    QWidget::show();
    serverGui->showGUI();
}

Server::~Server()
{
    serverGui->destroyGUI();
    delete docList;
    delete qcopBridge;
    delete transferServer;
    delete serverGui;
    delete tsmMonitor;
    delete lastStartedApp;
}

static bool hasVisibleWindow(const QString& clientname, bool partial)
{
#ifdef QWS
    const QList<QWSWindow> &list = qwsServer->clientWindows();
    QWSWindow* w;
    for (QListIterator<QWSWindow> it(list); (w=it.current()); ++it) {
	if ( w->client()->identity() == clientname ) {
	    if ( partial && !w->isFullyObscured() )
		return TRUE;
	    if ( !partial && !w->isFullyObscured() && !w->isPartiallyObscured() ) {
# if QT_VERSION < 0x030000
		QRect mwr = qt_screen->mapToDevice(qt_maxWindowRect,
			QSize(qt_screen->width(),qt_screen->height()) );
# else
		QRect mwr = qt_maxWindowRect;
# endif
		if ( mwr.contains(w->requested().boundingRect()) )
		    return TRUE;
	    }
	}
    }
#endif
    return FALSE;
}

void Server::activate(const DeviceButton* button, bool held)
{
    Global::terminateBuiltin("calibrate"); // No tr
    ServiceRequest sr;
    if ( held ) {
	sr = button->heldAction();
    } else {
	sr = button->pressedAction();
    }
    // A button with no action defined, will return a null ServiceRequest.  Don't attempt
    // to send/do anything with this as it will crash
    if ( !sr.isNull() ) {
	QString app = sr.app();
	bool vis = hasVisibleWindow(app, app != "qpe");
	if ( sr.message() == "raise()" && vis ) {
	    sr.setMessage("nextView()");
	} else {
	    // "back door"
	    sr << (int)vis;
	}

	sr.send();
    }
}


typedef struct KeyOverride {
#ifdef QT_QWS_TIP2
    uint
#else
    ushort
#endif
    scan_code;

    QWSServer::KeyMap map;
};


static const KeyOverride jp109keys[] = {
   { 0x03, {   Qt::Key_2,      '2'     , 0x22     , 0xffff  } },
   { 0x07, {   Qt::Key_6,      '6'     , '&'     , 0xffff  } },
   { 0x08, {   Qt::Key_7,      '7'     , '\''     , 0xffff  } },
   { 0x09, {   Qt::Key_8,      '8'     , '('     , 0xffff  } },
   { 0x0a, {   Qt::Key_9,      '9'     , ')'     , 0xffff  } },
   { 0x0b, {   Qt::Key_0,      '0'     , 0xffff  , 0xffff  } },
   { 0x0c, {   Qt::Key_Minus,      '-'     , '='     , 0xffff  } },
   { 0x0d, {   Qt::Key_AsciiCircum,'^'     , '~'     , '^' - 64  } },
   { 0x1a, {   Qt::Key_At,     '@'     , '`'     , 0xffff  } },
   { 0x1b, {   Qt::Key_BraceLeft, '['     , '{'  , '[' - 64  } },
   { 0x27, {   Qt::Key_Semicolon,  ';'     , '+'     , 0xffff  } },
   { 0x28, {   Qt::Key_Colon,  ':'    ,  '*'     , 0xffff  } },
   { 0x29, {   Qt::Key_Zenkaku_Hankaku,  0xffff  , 0xffff     , 0xffff  } },
   { 0x2b, {   Qt::Key_BraceRight,  ']'    , '}'     , ']'-64  } },
   { 0x70, {   Qt::Key_Hiragana_Katakana,    0xffff  , 0xffff  , 0xffff  } },
   { 0x73, {   Qt::Key_Backslash,  '\\'    , '_'  ,    0xffff  } },
   { 0x79, {   Qt::Key_Henkan,     0xffff  , 0xffff  , 0xffff  } },
   { 0x7b, {   Qt::Key_Muhenkan,   0xffff  , 0xffff  , 0xffff  } },
   { 0x7d, {   Qt::Key_yen,        0x00a5  , '|'     , 0xffff  } },
   { 0x00, {   0,          0xffff  , 0xffff  , 0xffff  } }
};

#ifdef QTOPIA_PHONE
// slightly hacky way of simulating this in the virtual framebuffer
static const KeyOverride keypadkeys[] = {
   { Qt::Key_Home,     {   Qt::Key_Select,    0xffff, 0xffff, 0xffff  } },
   { Qt::Key_End,    {   Qt::Key_Back,	    0xffff, 0xffff, 0xffff  } },
   { Qt::Key_Insert,    {   Qt::Key_Context1,  0xffff, 0xffff, 0xffff  } },
   { 0,	     {   0,		    0xffff, 0xffff, 0xffff  } }
};
#endif

bool Server::setKeyboardLayout( const QString &kb )
{
#if defined(QPE_OVERRIDE_KEYMAP)
    const KeyOverride *devicekeys = qtopia_override_keys();
#else
#   ifdef QTOPIA_PHONE
    const KeyOverride *devicekeys = keypadkeys;
#   else
    const KeyOverride *devicekeys = 0;
#   endif
#endif

    QIntDict<QWSServer::KeyMap> *om = 0;

    //if ( kb == "us101" ) { // No tr
    // no override for us101.
    if ( kb == "jp109" ) {
	om = new QIntDict<QWSServer::KeyMap>(37);
	const KeyOverride *k = jp109keys;
	while ( k->scan_code ) {
	    om->insert( k->scan_code, &k->map );
	    k++;
	}
    }
    // now do the device keys.

    if (devicekeys) {
	if (!om)
	    om = new QIntDict<QWSServer::KeyMap>(37);
	while ( devicekeys->scan_code ) {
	    om->insert( devicekeys->scan_code, &devicekeys->map );
	    devicekeys++;
	}
    }

    QWSServer::setOverrideKeys( om );

    return TRUE;
}

void Server::systemMsg(const QCString &msg, const QByteArray &data)
{
    QDataStream stream( data, IO_ReadOnly );

    if ( msg == "securityChanged()" ) {
	if ( transferServer )
	   transferServer->authorizeConnections();
	if ( qcopBridge )
	    qcopBridge->authorizeConnections();
    } else if ( msg == "setTempScreenSaverMode(int,int)" ) {
	int mode, pid;
	stream >> mode >> pid;
	tsmMonitor->setTempMode(mode, pid);
    } else if ( msg == "linkChanged(QString)" ) {
	QString link;
	stream >> link;
	qDebug( "desktop.cpp systemMsg -> linkchanged( %s )", link.latin1() );
	docList->linkChanged(link);
    } else if ( msg == "serviceChanged(QString)" ) {
	MimeType::updateApplications();
    } else if ( msg == "mkdir(QString)" ) {
	QString dir;
	stream >> dir;
	if ( !dir.isEmpty() )
	    mkdir( dir );
    } else if ( msg == "rdiffGenSig(QString,QString)" ) {
	QString baseFile, sigFile;
	stream >> baseFile >> sigFile;
	QRsync::generateSignature( baseFile, sigFile );
    } else if ( msg == "rdiffGenDiff(QString,QString,QString)" ) {
	QString baseFile, sigFile, deltaFile;
	stream >> baseFile >> sigFile >> deltaFile;
	QRsync::generateDiff( baseFile, sigFile, deltaFile );
    } else if ( msg == "rdiffApplyPatch(QString,QString)" ) {
	QString baseFile, deltaFile;
	stream >> baseFile >> deltaFile;
	if ( !QFile::exists( baseFile ) ) {
	    QFile f( baseFile );
	    f.open( IO_WriteOnly );
	    f.close();
	}
	QRsync::applyDiff( baseFile, deltaFile );
#ifndef QT_NO_COP
	QCopEnvelope e( "QPE/Desktop", "patchApplied(QString)" );
	e << baseFile;
#endif
    } else if ( msg == "rdiffCleanup()" ) {
	mkdir( "/tmp/rdiff" );
	QDir dir;
	dir.setPath( "/tmp/rdiff" );
	QStringList entries = dir.entryList();
	for ( QStringList::Iterator it = entries.begin(); it != entries.end(); ++it )
	    dir.remove( *it );
    } else if ( msg == "sendHandshakeInfo()" ) {
	QString home = getenv( "HOME" );
#ifndef QT_NO_COP
	QCopEnvelope e( "QPE/Desktop", "handshakeInfo(QString,bool)" );
	e << home;
	int locked = (int) ServerApplication::screenLocked();
	e << locked;
#endif

    } else if ( msg == "sendVersionInfo()" ) {
	QCopEnvelope e( "QPE/Desktop", "versionInfo(QString,QString)" );
	QString v = QPE_VERSION;
	e << Global::version() << Global::architecture();
    } else if ( msg == "sendCardInfo()" ) {
#ifndef QT_NO_COP
	QCopEnvelope e( "QPE/Desktop", "cardInfo(QString)" );
	e << cardInfoString();
#endif
    } else if ( msg == "sendInstallLocations()" ) {
#ifndef QT_NO_COP
	QCopEnvelope e( "QPE/Desktop", "installLocations(QString)" );
	e << installLocationsString();
#endif
    } else if ( msg == "sendSyncDate(QString)" ) {
	QString app;
	stream >> app;
	Config cfg( "qpe" );
	cfg.setGroup("SyncDate");
#ifndef QT_NO_COP
	QCopEnvelope e( "QPE/Desktop", "syncDate(QString,QString)" );
	e  << app  << cfg.readEntry( app );
#endif
	//qDebug("QPE/System sendSyncDate for %s: response %s", app.latin1(),
	//cfg.readEntry( app ).latin1() );
    } else if ( msg == "setSyncDate(QString,QString)" ) {
	QString app, date;
	stream >> app >> date;
	Config cfg( "qpe" );
	cfg.setGroup("SyncDate");
	cfg.writeEntry( app, date );
	//qDebug("setSyncDate(QString,QString) %s %s", app.latin1(), date.latin1());
    } else if ( msg == "startSync(QString)" ) {
	QString what;
	stream >> what;
	delete syncDialog;
	syncDialog = new SyncDialog( this, what );
	syncDialog->show();
	connect( syncDialog, SIGNAL(cancel()), SLOT(cancelSync()) );
    } else if ( msg == "stopSync()") {
	delete syncDialog;
	syncDialog = 0;
    } else if (msg == "restoreDone(QString)") {
	docList->restoreDone();
    } else if ( msg == "getAllDocLinks()" ) {
	docList->sendAllDocLinks();
    }
#ifdef QPE_HAVE_DIRECT_ACCESS
    else if ( msg == "prepareDirectAccess()" ) {
	prepareDirectAccess();
    } else if ( msg == "postDirectAccess()" ) {
	postDirectAccess();
    }
#endif
    else if ( msg == "setMouseProto(QString)" ) {
	QString mice;
	stream >> mice;
	setenv("QWS_MOUSE_PROTO",mice.latin1(),1);
	qwsServer->openMouse();
    } else if ( msg == "setKeyboard(QString)" ) {
	QString kb;
	stream >> kb;
	setenv("QWS_KEYBOARD",kb.latin1(),1);
	qwsServer->openKeyboard();

    } else if ( msg == "setKeyboardAutoRepeat(int,int)" ) {
	int delay, period;
	stream >> delay >> period;
	qwsSetKeyboardAutoRepeat( delay, period );
	Config cfg( "qpe" );
	cfg.setGroup("Keyboard");
	cfg.writeEntry( "RepeatDelay", delay );
	cfg.writeEntry( "RepeatPeriod", period );
    } else if ( msg == "setKeyboardLayout(QString)" ) {
	QString kb;
	stream >> kb;
	setKeyboardLayout( kb );
	Config cfg( "qpe" );
	cfg.setGroup("Keyboard");
	cfg.writeEntry( "Layout", kb );
    }
#ifdef QPE_SYNC_CLOCK_FROM_QD
    else if ( msg == "setClockFromQD(QString)" ) {
        // Datebook on Qtopia Desktop sends the "current time"
        // to Qtopia after it has finished synchronizing.
        // Set the system clock using the time received so
        // that the clock on the PDA is kept in sync with the
        // desktop.
        QString val;
        stream >> val;
        bool ok;
        // unwrap the string (stupid old-style QCop) to get the time_t value
        time_t utc_time = (time_t)val.toLong(&ok);
        if ( ok ) {
            qDebug( "Qtopia Desktop told me to set the clock to %s", TimeConversion::fromUTC(utc_time).toString().latin1() );
#ifndef QPE_SYNC_CLOCK_FROM_QD_FAKE
            struct timeval myTv;
            myTv.tv_sec = utc_time;
            myTv.tv_usec = 0;

            if ( myTv.tv_sec != -1 )
                ::settimeofday( &myTv, 0 );
            Global::writeHWClock();
#endif

#ifndef QT_NO_COP
            // Make sure everyone updates their clocks
            QString tz = TimeZone::current().id();
            QCopEnvelope e( "QPE/System", "timeChange(QString)" );
            e << tz;
#endif
        }
    }
#endif
}

QString Server::cardInfoString()
{
    storage->update();
    const QList<FileSystem> &fs = storage->fileSystems();
    QListIterator<FileSystem> it ( fs );
    QString s;
    QString homeDir = getenv("HOME");
    QString homeFs, homeFsPath;
    for ( ; it.current(); ++it ) {
	int k4 = (*it)->blockSize()/256;
	if ( (*it)->isRemovable() ) {
	    s += (*it)->name() + "=" + (*it)->path() + "/Documents " // No tr
		 + QString::number( (*it)->availBlocks() * k4/4 )
		 + "K " + (*it)->options() + ";";
	} else if ( homeDir.contains( (*it)->path() ) &&
		  (*it)->path().length() > homeFsPath.length() ) {
	    homeFsPath = (*it)->path();
	    homeFs =
		(*it)->name() + "=" + homeDir + "/Documents " // No tr
		+ QString::number( (*it)->availBlocks() * k4/4 )
		+ "K " + (*it)->options() + ";";
	}
    }
    if ( !homeFs.isEmpty() )
	s += homeFs;
    return s;
}

QString Server::installLocationsString()
{
    storage->update();
    const QList<FileSystem> &fs = storage->fileSystems();
    QListIterator<FileSystem> it ( fs );
    QString s;
    QString homeDir = getenv("HOME");
    QString homeFs, homeFsPath;
    for ( ; it.current(); ++it ) {
	int k4 = (*it)->blockSize()/256;
	if ( (*it)->isRemovable() ) {
	    s += (*it)->name() + "=" + (*it)->path() + " " // No tr
		 + QString::number( (*it)->availBlocks() * k4/4 )
		 + "K " + (*it)->options() + ";";
	} else if ( homeDir.contains( (*it)->path() ) &&
		    (*it)->path().length() > homeFsPath.length() ) {
	    homeFsPath = (*it)->path();
	    homeFs =
		(*it)->name() + "=" + homeDir + " " // No tr
		+ QString::number( (*it)->availBlocks() * k4/4 )
		+ "K " + (*it)->options() + ";";
	}
    }
    if ( !homeFs.isEmpty() )
	s = homeFs + s;
    return s;
}

void Server::receiveTaskBar(const QCString &msg, const QByteArray &data)
{
    QDataStream stream( data, IO_ReadOnly );

    if ( msg == "reloadApps()" ) {
	docList->reloadAppLnks();
    } else if ( msg == "soundAlarm()" ) {
	soundAlarm();
    }
#ifdef CUSTOM_LEDS
    else if ( msg == "setLed(int,bool)" ) {
	int led, status;
	stream >> led >> status;
	CUSTOM_LEDS( led, status );
    }
#endif
}

void Server::cancelSync()
{
#ifndef QT_NO_COP
    QCopEnvelope e( "QPE/Desktop", "cancelSync()" );
#endif
    delete syncDialog;
    syncDialog = 0;
}

bool Server::mkdir(const QString &localPath)
{
    QDir fullDir(localPath);
    if (fullDir.exists())
	return true;

    // at this point the directory doesn't exist
    // go through the directory tree and start creating the directories
    // that don't exist; if we can't create the directories, return false

    QString dirSeps = "/";
    int dirIndex = localPath.find(dirSeps);
    QString checkedPath;

    // didn't find any seps; weird, use the cur dir instead
    if (dirIndex == -1) {
	//qDebug("No seperators found in path %s", localPath.latin1());
	checkedPath = QDir::currentDirPath();
    }

    while (checkedPath != localPath) {
	// no more seperators found, use the local path
	if (dirIndex == -1)
	    checkedPath = localPath;
	else {
	    // the next directory to check
	    checkedPath = localPath.left(dirIndex) + "/";
	    // advance the iterator; the next dir seperator
	    dirIndex = localPath.find(dirSeps, dirIndex+1);
	}

	QDir checkDir(checkedPath);
	if (!checkDir.exists()) {
	    //qDebug("mkdir making dir %s", checkedPath.latin1());

	    if (!checkDir.mkdir(checkedPath)) {
		qDebug("Unable to make directory %s", checkedPath.latin1());
		return FALSE;
	    }
	}

    }
    return TRUE;
}

void Server::styleChange( QStyle &s )
{
    QWidget::styleChange( s );
}

void Server::startTransferServer()
{
    if ( !qcopBridge ) {
	// start qcop bridge server
	qcopBridge = new QCopBridge( 4243 );
	if ( qcopBridge->ok() ) {
	    // ... OK
	    connect( qcopBridge, SIGNAL(connectionClosed(const QHostAddress&)),
		    this, SLOT(syncConnectionClosed(const QHostAddress&)) );
	} else {
	    delete qcopBridge;
	    qcopBridge = 0;
	}
    }
    if ( !transferServer ) {
	// start transfer server
	transferServer = new TransferServer( 4242 );
	if ( transferServer->ok() ) {
	    // ... OK
	} else {
	    delete transferServer;
	    transferServer = 0;
	}
    }
    if ( !transferServer || !qcopBridge )
	tid_xfer = startTimer( 2000 );
}

void Server::timerEvent( QTimerEvent *e )
{
    if ( e->timerId() == tid_xfer ) {
	killTimer( tid_xfer );
	tid_xfer = 0;
	startTransferServer();
    }
    if ( e->timerId() == qssTimerId ) {
	if (!soundserver) {
	    soundserver = new QProcess(this, 0);
	    connect(soundserver, SIGNAL(processExited()),
		    this, SLOT(soundServerExited()));
#ifdef QSS_DEBUG
	    connect(soundserver, SIGNAL(readyReadStdout()),
		    this, SLOT(soundServerReadyStdout()));
	    connect(soundserver, SIGNAL(readyReadStderr()),
		    this, SLOT(soundServerReadyStderr()));
#endif
	}
	soundserver->clearArguments();
        soundserver->addArgument(QPEApplication::qpeDir() + "bin/qss");

#if defined(QTOPIA_PHONE) || defined(QTOPIA_USE_QSS_VOLUME)
        // set up initial volume
#ifdef QTOPIA_PHONE
        Config c("PhoneProfile"); // no tr
        c.setGroup("Profiles"); // no tr
        int pid = c.readNumEntry("Selected");
        c.setGroup("Profile " + QString::number(pid));
        int vol = c.readNumEntry("Volume", 3) * 20;
#else
        Config c("Sound");
        c.setGroup("System");
        int vol = c.readNumEntry("Volume", 50);
        if (c.readBoolEntry("Muted", false))
            vol = 0;
#endif
        soundserver->addArgument("-systemvolume");
        soundserver->addArgument(QString::number(vol));
#endif
        soundserver->start();

	killTimer(qssTimerId);
	qssTimerId = 0;
    }
}

void Server::terminateServers()
{
    delete transferServer;
    delete qcopBridge;
    transferServer = 0;
    qcopBridge = 0;
}

void Server::syncConnectionClosed( const QHostAddress & )
{
    qDebug( "Lost sync connection" );
    delete syncDialog;
    syncDialog = 0;
}

void Server::pokeTimeMonitors()
{
    // inform all TimeMonitors
    QStrList tms = Service::channels("TimeMonitor");
    for (const char* ch = tms.first(); ch; ch=tms.next()) {
	QString t = getenv("TZ");
	QCopEnvelope e(ch, "timeChange(QString)");
	e << t;
    }
}

void Server::applicationRaised(const QString &app)
{
    serverGui->applicationStateChanged( app, ServerInterface::Raised );
}

void Server::applicationLaunched(int, const QString &app)
{
    if (lastStartedApp)
        delete lastStartedApp;
    lastStartedApp = new QString(app);
    serverGui->applicationStateChanged( app, ServerInterface::Launching );
#ifdef QPE_LAZY_APPLICATION_SHUTDOWN
    // This restricts the maximum number of apps we leave running.
    // If we use a memory monitor then we could just rely on it to
    // close applications when mem is tight.
    const QStringList &running = appLauncher->running();
    if (running.count() > 6) {
	QStringList::ConstIterator it(running.fromLast());
	for (; it != running.begin(); --it) {
	    const AppLnk* app = DocumentList::appLnkSet->findExec(*it);
	    if ( !app ) continue;
	    if ( !app->isPreloaded() ) {
		QCopEnvelope e("QPE/Application/"+(*it).local8Bit(), "quitIfInvisible()");
		break;
	    }
	}
    }
#endif
}

void Server::applicationTerminated(int pid, const QString &app)
{
    if (lastStartedApp) {
        delete lastStartedApp;
        lastStartedApp = 0;
    }
    serverGui->applicationStateChanged( app, ServerInterface::Terminated );
    tsmMonitor->applicationTerminated( pid );
}

void Server::applicationConnected(const QString &app)
{
    serverGui->applicationStateChanged( app, ServerInterface::Running );
}

void Server::storageChanged()
{
    system( "qtopia-update-symlinks" );
    serverGui->storageChanged( storage->fileSystems() );
    docList->storageChanged();
}


void Server::soundAlarm()
{
#ifdef CUSTOM_SOUND_ALARM
    CUSTOM_SOUND_ALARM;
#else
#ifndef QT_NO_SOUND
    QSound::play(Resource::findSound("alarm")); // No tr
#endif
#endif
}

void Server::preloadApps()
{
    Config cfg("Launcher");
    cfg.setGroup("Preload");
    QStringList apps = cfg.readListEntry("Apps",',');
    for (QStringList::ConstIterator it=apps.begin(); it!=apps.end(); ++it) {
#ifndef QT_NO_COP
	QCopEnvelope e("QPE/Application/"+(*it).local8Bit(), "enablePreload()");
#endif
    }
}

// This is only called if QPE_HAVE_DIRECT_ACCESS is defined
void Server::prepareDirectAccess()
{
    qDebug( "Server::prepareDirectAccess()" );
    // Put up a pretty dialog
    syncDialog = new SyncDialog( this, tr("USB Lock") );
    syncDialog->show();

    // Prevent the PDA from acting as a PDA
    terminateServers();

    // suspend the mtab monitor
#ifndef QT_NO_COP
    {
	QCopEnvelope e( "QPE/Stabmon", "suspendMonitor()" );
    }
#endif

    // send out a flush message
    // once flushes are done call runDirectAccess()
    // We just count the number of apps and set a timer.
    // Either the timer expires or the correct number of apps responds.
    // Note: quicklauncher isn't in the runningApps list but it responds
    //       to the flush so we start the counter at 1
    pendingFlushes = 1;
    directAccessRun = FALSE;
    for ( QStringList::ConstIterator it =
	    appLauncher->running().begin();
	  it != appLauncher->running().end();
	  ++it ) {
	pendingFlushes++;
    }
#ifndef QT_NO_COP
    QCopEnvelope e1( "QPE/System", "flush()" );
#endif
    QTimer::singleShot( 10000, this, SLOT(runDirectAccess()) );
    QPEApplication::setTempScreenSaverMode(QPEApplication::DisableSuspend);
}

// This is only connected if QPE_HAVE_DIRECT_ACCESS is defined
// It fakes the presence of Qtopia Desktop
void Server::desktopMessage( const QCString &message, const QByteArray &data )
{
    QDataStream stream( data, IO_ReadOnly );
    if ( message == "flushDone(QString)" ) {
	QString app;
	stream >> app;
	qDebug( "flushDone from %s", app.latin1() );
	if ( --pendingFlushes == 0 ) {
	    qDebug( "pendingFlushes == 0, all the apps responded" );
	    runDirectAccess();
	}
    } else if ( message == "installStarted(QString)" ) {
	QString package;
	stream >> package;
	qDebug( "\tInstall Started for package %s", package.latin1() );
    } else if ( message == "installStep(QString)" ) {
	QString step;
	stream >> step;
	qDebug( "\tInstall Step %s", step.latin1() );
    } else if ( message == "installDone(QString)" ) {
	QString package;
	stream >> package;
	qDebug( "\tInstall Finished for package %s", package.latin1() );
    } else if ( message == "installFailed(QString,int,QString)" ) {
	QString package, error;
	int status;
	stream >> package >> status >> error;
	qDebug( "\tInstall Failed for package %s with error code %d and error message %s",
		package.latin1(), status, error.latin1() );
    } else if ( message == "removeStarted(QString)" ) {
	QString package;
	stream >> package;
	qDebug( "\tRemove Started for package %s", package.latin1() );
    } else if ( message == "removeDone(QString)" ) {
	QString package;
	stream >> package;
	qDebug( "\tRemove Finished for package %s", package.latin1() );
    } else if ( message == "removeFailed(QString)" ) {
	QString package;
	stream >> package;
	qDebug( "\tRemove Failed for package %s", package.latin1() );
    }

    if ( qrr && qrr->waitingForMessages )
	qrr->desktopMessage( message, data );
}

// This is only connected if QPE_HAVE_DIRECT_ACCESS is defined
void Server::runDirectAccess()
{
#ifdef QPE_HAVE_DIRECT_ACCESS
    // The timer must have fired after all the apps responded
    // with flushDone(). Just ignore it.
    if ( directAccessRun )
	return;

    directAccessRun = TRUE;
    ::readyDirectAccess(cardInfoString(), installLocationsString());
#endif
}

// This is only called if QPE_HAVE_DIRECT_ACCESS is defined
void Server::postDirectAccess()
{
#ifdef QPE_HAVE_DIRECT_ACCESS
    qDebug( "Server::postDirectAccess()" );

    // Categories may have changed
    QCopEnvelope e1( "QPE/System", "categoriesChanged()" );
    // Apps need to reload their data
    QCopEnvelope e2( "QPE/System", "reload()" );
    // Reload DocLinks
    docList->storageChanged();
    // Restart the PDA server stuff
    startTransferServer();

    // restart the mtab monitor
#ifndef QT_NO_COP
    {
	QCopEnvelope e( "QPE/Stabmon", "restartMonitor()" );
    }
#endif

    // Process queued requests
    const char *queueFile = ::directAccessQueueFile();
    QFile *file = new QFile( queueFile );
    if ( !file->exists() ) {
	delete file;
	// Get rid of the dialog
	if ( syncDialog ) {
	    delete syncDialog;
	    syncDialog = 0;
	}
	QPEApplication::setTempScreenSaverMode(QPEApplication::Enable);
    } else {
	qrr = new QueuedRequestRunner( file, syncDialog );
	connect( qrr, SIGNAL(finished()),
		 this, SLOT(finishedQueuedRequests()) );
	QTimer::singleShot( 100, qrr, SLOT(process()) );
	// qrr will remove the sync dialog later
    }
#endif
}

void Server::finishedQueuedRequests()
{
    if ( qrr->readyToDelete ) {
	delete qrr;
	qrr = 0;
	// Get rid of the dialog
	if ( syncDialog ) {
	    delete syncDialog;
	    syncDialog = 0;
	}
	QPEApplication::setTempScreenSaverMode(QPEApplication::Enable);
    } else {
	qrr->readyToDelete = TRUE;
	QTimer::singleShot( 0, this, SLOT(finishedQueuedRequests()) );
    }
}

void Server::recoverMemory(ServerApplication::MemState state)
{
    if (state == ServerApplication::MemNormal || 
            state == ServerApplication::MemUnknown)
        return;

    qWarning( "MEMORY LOW: %d, recovering", state);
    

#ifdef QPE_LAZY_APPLICATION_SHUTDOWN
    const QStringList &running = appLauncher->running();
    if (running.count() > 1) {
	QStringList::ConstIterator it(running.fromLast());
	for (; it != running.begin(); --it) {
	    const AppLnk* app = DocumentList::appLnkSet->findExec(*it);
	    if ( !app ) continue;
	    if ( !app->isPreloaded() ) {
                if (lastStartedApp) {
                    delete lastStartedApp;
                    lastStartedApp = 0;
                }
		qWarning("MEMORY LOW: Shutdown %s", (*it).latin1());
		QCopEnvelope e("QPE/Application/"+(*it).local8Bit(), "quitIfInvisible()");
		if (state == ServerApplication::MemLow)
		    break;
	    }
	}
    }
#endif
    QCopEnvelope e("QPE/System", "RecoverMemory()");
    
    //kill if new app was started recently 
    if ( state == ServerApplication::MemCritical && lastStartedApp) {
        qDebug("Page-thrashing: Killing " + *lastStartedApp);
        appLauncher->criticalKill(*lastStartedApp);
        return;
    }

}

void Server::soundServerExited()
{
    if (!qssTimerId)
	qssTimerId = startTimer(10);
}

void Server::soundServerReadyStdout()
{
#ifdef QSS_DEBUG
    while ( soundserver->canReadLineStdout() )
	qDebug( "SS: %s", soundserver->readLineStdout().latin1() );
#endif
}

void Server::soundServerReadyStderr()
{
#ifdef QSS_DEBUG
    while ( soundserver->canReadLineStderr() )
	qDebug( "SS: %s", soundserver->readLineStderr().latin1() );
#endif
}

