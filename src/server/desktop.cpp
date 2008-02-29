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

#include "desktop.h"
#include "launcher.h"
#include "qcopbridge.h"
#include "shutdownimpl.h"
#include "startmenu.h"
#include "taskbar.h"
#include "transferserver.h"
#include "irserver.h"
#include "packageslave.h"
//#include "info.h"
//#include "mrulist.h"

#include <qpe/applnk.h>
#include <qpe/mimetype.h>
#include <qpe/password.h>
#include <qpe/config.h>
#include <qpe/power.h>
#include <qpe/services/services.h>
#include "devicebuttonmanager.h"
#ifdef QWS
#include <qpe/qcopenvelope_qws.h>
#endif
#include <qpe/global.h>
#ifdef QT_QWS_CUSTOM
#include "qpe/custom.h"
#endif

#ifdef QWS
#include <qgfx_qws.h>
#endif
#include <qmainwindow.h>
#include <qmessagebox.h>
#include <qtimer.h>

#include <stdlib.h>
#include <unistd.h>

/*  Apply light/power settings for current power source */
static void applyLightSettings(PowerStatus *p)
{
    int initbright, intervalDim, intervalLightOff, intervalSuspend;
    bool dim, lightoff, suspend;
    
    {
	Config config("qpe");
	bool defsus;
	if ( p->acStatus() == PowerStatus::Online ) {
	    config.setGroup("ExternalPower");
	    defsus = FALSE;
	} else {
	    config.setGroup("BatteryPower");
	    defsus = TRUE;
	}

	intervalDim = config.readNumEntry( "Interval_Dim", 20 );
	intervalLightOff = config.readNumEntry("Interval_LightOff", 30);
	intervalSuspend = config.readNumEntry("Interval", 240);
	initbright = config.readNumEntry("Brightness", 255);
	dim = config.readBoolEntry("Dim", TRUE);
	lightoff = config.readBoolEntry("LightOff", FALSE );
	suspend = config.readBoolEntry("Suspend", defsus );

	/*	For compability	*/
	config.setGroup("Screensaver");
	config.writeEntry( "Dim", dim );
	config.writeEntry( "LightOff", lightoff );
	config.writeEntry( "Suspend", suspend );
	config.writeEntry( "Interval_Dim", intervalDim );
	config.writeEntry( "Interval_LightOff", intervalLightOff );
	config.writeEntry( "Interval", intervalSuspend );
	config.writeEntry( "Brightness", initbright );
    }

    int i_dim =      (dim ? intervalDim : 0);
    int i_lightoff = (lightoff ? intervalLightOff : 0);
    int i_suspend =  suspend;

#ifndef QT_NO_COP
    QCopEnvelope eB("QPE/System", "setBacklight(int)" );
    eB << initbright;
    
    QCopEnvelope e("QPE/System", "setScreenSaverIntervals(int,int,int)" );
    e << i_dim << i_lightoff << i_suspend;
#endif
}

static Desktop* qpedesktop = 0;
static int loggedin=0;
static void login(bool at_poweron)
{
    if ( !loggedin ) {
	Global::terminateBuiltin("calibrate"); // No tr
	Password::authenticate(at_poweron);
	loggedin=1;
#ifndef QT_NO_COP
	QCopEnvelope e( "QPE/Desktop", "unlocked()" );
#endif
    }
}

bool Desktop::screenLocked()
{
    return loggedin == 0;
}

/*
  Priority is number of alerts that are needed to pop up
  alert.
 */
class DesktopPowerAlerter : public QMessageBox
{
public:
    DesktopPowerAlerter( QWidget *parent, const char *name = 0 )
	: QMessageBox( tr("Battery Status"), tr("Low Battery"),
		       QMessageBox::Critical,
		       QMessageBox::Ok | QMessageBox::Default,
		       QMessageBox::NoButton, QMessageBox::NoButton,
		       parent, name, FALSE )
    {
	currentPriority = INT_MAX;
	alertCount = 0;
    }

    void alert( const QString &text, int priority );
    void hideEvent( QHideEvent * );
private:
    int currentPriority;
    int alertCount;
};

void DesktopPowerAlerter::alert( const QString &text, int priority )
{
    alertCount++;
    if ( alertCount < priority )
	return;
    if ( priority > currentPriority )
	return;
    currentPriority = priority;
    setText( text );
    show();
}


void DesktopPowerAlerter::hideEvent( QHideEvent *e )
{
    QMessageBox::hideEvent( e );
    alertCount = 0;
    currentPriority = INT_MAX;
}

KeyFilter::KeyFilter(QObject* parent) : QObject(parent), held_tid(0), heldButton(0)
{
}

void KeyFilter::timerEvent(QTimerEvent* e)
{
    if ( e->timerId() == held_tid ) {
	// button held
	if ( heldButton ) {
	    emit activate(heldButton, TRUE);
	    heldButton = 0;
	}
    }
    killTimer(e->timerId());
}

bool KeyFilter::filter(int /*unicode*/, int keycode, int modifiers, bool press,
		  bool /*autoRepeat*/)
{
    if ( !loggedin && keycode != Key_F34 )
	return TRUE;
    if ( !((DesktopApplication*)qApp)->keyboardGrabbed() ) {
	// First check to see if DeviceButtonManager knows something about this button:
	const DeviceButton* button = DeviceButtonManager::instance().buttonForKeycode(keycode);
	if (button) {
	    if ( held_tid )
		killTimer(held_tid);
	    if ( button->heldAction().isNull() ) {
		if ( press )
		    emit activate(button, FALSE);
	    } else if ( press ) {
	        heldButton = button;
	        held_tid = startTimer(1000);
	    } else if ( heldButton ) {
		heldButton = 0;
		emit activate(button, FALSE);
	    }
	    QWSServer::screenSaverActivate(FALSE);
	    return TRUE;
	}
    }
    if ( keycode == Key_F34 ) {
	if ( press ) emit power();
	return TRUE;
    }
    if ( keycode == Key_F35 ) {
	if ( press ) emit backlight();
	return TRUE;
    }
    if ( keycode == Key_F32 ) {
#ifndef QT_NO_COP
	if ( press ) QCopEnvelope e( "QPE/Desktop", "startSync()" );
#endif
	return TRUE;
    }
    if ( keycode == Key_F31 && !modifiers ) {
	if ( press ) emit symbol();
	QWSServer::screenSaverActivate(FALSE);
	return TRUE;
    }
    if ( keycode == Key_NumLock ) {
	if ( press ) emit numLockStateToggle();
    }
    if ( keycode == Key_CapsLock ) {
	if ( press ) emit capsLockStateToggle();
    }
    if ( press )
	qpedesktop->keyClick();
    return FALSE;
}


DesktopApplication::DesktopApplication( int& argc, char **argv, Type t )
    : QPEApplication( argc, argv, t )
{

    QTimer *t = new QTimer( this );
    connect( t, SIGNAL(timeout()), this, SLOT(psTimeout()) );
    t->start( 10000 );
    ps = new PowerStatus;
    pa = new DesktopPowerAlerter( 0 );
    KeyFilter* kf = new KeyFilter(this);
    qwsServer->setKeyboardFilter(kf);
    connect(kf,SIGNAL(launch()),this,SIGNAL(launch()));
    connect(kf,SIGNAL(power()),this,SIGNAL(power()));
    connect(kf,SIGNAL(backlight()),this,SIGNAL(backlight()));
    connect(kf,SIGNAL(symbol()),this,SIGNAL(symbol()));
    connect(kf,SIGNAL(numLockStateToggle()),this,SIGNAL(numLockStateToggle()));
    connect(kf,SIGNAL(capsLockStateToggle()),this,SIGNAL(capsLockStateToggle()));
    connect(kf,SIGNAL(activate(const DeviceButton*,bool)),this,SIGNAL(activate(const DeviceButton*,bool)));
}


DesktopApplication::~DesktopApplication()
{
    delete ps;
    delete pa;
}


enum MemState { Unknown, VeryLow, Low, Normal } memstate=Unknown;

#ifdef Q_WS_QWS
bool DesktopApplication::qwsEventFilter( QWSEvent *e )
{
    qpedesktop->checkMemory();

    if ( e->type == QWSEvent::Mouse ) {
	QWSMouseEvent *me = (QWSMouseEvent *)e;
	static bool up = TRUE;
	if ( me->simpleData.state&LeftButton ) {
	    if ( up ) {
		up = FALSE;
		qpedesktop->screenClick();
	    }
	} else {
	    up = TRUE;
	}
    }

    return QPEApplication::qwsEventFilter( e );
}
#endif

void DesktopApplication::psTimeout()
{
    qpedesktop->checkMemory(); // in case no events are being generated

    PowerStatus prev = *ps;	
    
    *ps = PowerStatusManager::readStatus();

    if ( prev != *ps ) {
	// power source changed, read settings applying to current powersource
	applyLightSettings(ps);
    }

    
    if ( (ps->batteryStatus() == PowerStatus::VeryLow ) ) {
	pa->alert( tr( "Battery is running very low." ), 6 );
    }

    if (  ps->batteryStatus() == PowerStatus::Critical ) {
	pa->alert(  tr( "Battery level is critical!\n"
			"Keep power off until power restored!" ), 1 );
    }

    if (  ps->backupBatteryStatus() == PowerStatus::VeryLow ) {
	pa->alert( tr( "The Back-up battery is very low.\nPlease charge the back-up battery." ), 3 );
    }
}

#if defined(QPE_HAVE_MEMALERTER)
QPE_MEMALERTER_IMPL
#endif

#if defined(CUSTOM_SOUND_IMPL)
CUSTOM_SOUND_IMPL
#endif

//===========================================================================

Desktop::Desktop() :
    QWidget( 0, 0, WStyle_Tool | WStyle_Customize ),
    qcopBridge( 0 ),
    transferServer( 0 ),
    packageHandler( 0 )
{
#ifdef CUSTOM_SOUND_INIT
    CUSTOM_SOUND_INIT;
#endif

    qpedesktop = this;

//    bg = new Info( this );
    tb = new TaskBar;

    launcher = new Launcher( 0, 0, WStyle_Customize | QWidget::WGroupLeader );

    connect(launcher, SIGNAL(busy()), tb, SLOT(startWait()));
    connect(launcher, SIGNAL(notBusy(const QString&)), tb, SLOT(stopWait(const QString&)));

    int displayw = qApp->desktop()->width();
    int displayh = qApp->desktop()->height();


    QSize sz = tb->sizeHint();

    setGeometry( 0, displayh-sz.height(), displayw, sz.height() );
    tb->setGeometry( 0, displayh-sz.height(), displayw, sz.height() );

    tb->show();
    launcher->showMaximized();
    launcher->show();
    launcher->raise();
#if defined(QPE_HAVE_MEMALERTER)
    initMemalerter();
#endif
    // start services
    startTransferServer();
    (void) new IrServer( this );
    rereadVolumes();

    packageHandler = new PackageHandler( this );
    connect(qApp, SIGNAL(volumeChanged(bool)), this, SLOT(rereadVolumes()));
    connect(qApp, SIGNAL(activate(const DeviceButton*,bool)),this,SLOT(activate(const DeviceButton*,bool)));

    qApp->installEventFilter( this );
}

void Desktop::show()
{
    login(TRUE);
    QWidget::show();
}

Desktop::~Desktop()
{
    delete launcher;
    delete tb;
    delete qcopBridge;
    delete transferServer;
}

bool Desktop::recoverMemory()
{
    return tb->recoverMemory();
}

void Desktop::checkMemory()
{
#if defined(QPE_HAVE_MEMALERTER)
    static bool ignoreNormal=FALSE;
    static bool existingMessage=FALSE;

    if(existingMessage)
	return; // don't show a second message while still on first

    existingMessage = TRUE;
    switch ( memstate ) {
	case Unknown:
	    break;
	case Low:
	    memstate = Unknown;
	    if ( recoverMemory() )
		ignoreNormal = TRUE;
	    else
		QMessageBox::warning( 0 , tr("Memory Status"),
		    tr("Memory Low\nPlease save data.") );
	    break;
	case Normal:
	    memstate = Unknown;
	    if ( ignoreNormal )
		ignoreNormal = FALSE;
	    else
		QMessageBox::information ( 0 , tr("Memory Status"),
		    "Memory OK" );
	    break;
	case VeryLow:
	    memstate = Unknown;
	    QMessageBox::critical( 0 , tr("Memory Status"),
		tr("Critical Memory Shortage\n"
		"Please end this application\n"
		"immediately.") );
	    recoverMemory();
    }
    existingMessage = FALSE;
#endif
}

static bool isVisibleWindow(int wid)
{
#ifdef QWS
    const QList<QWSWindow> &list = qwsServer->clientWindows();
    QWSWindow* w;
    for (QListIterator<QWSWindow> it(list); (w=it.current()); ++it) {
	if ( w->winId() == wid )
	    return !w->isPartiallyObscured();
    }
#endif
    return FALSE;
}

static bool hasVisibleWindow(const QString& clientname)
{
#ifdef QWS
    const QList<QWSWindow> &list = qwsServer->clientWindows();
    QWSWindow* w;
    for (QListIterator<QWSWindow> it(list); (w=it.current()); ++it) {
	if ( w->client()->identity() == clientname && !w->isFullyObscured() )
	    return TRUE;
    }
#endif
    return FALSE;
}

void Desktop::raiseLauncher()
{
}

void Desktop::activate(const DeviceButton* button, bool held)
{
    Global::terminateBuiltin("calibrate"); // No tr
    ServiceRequest sr;
    if ( held ) {
	sr = button->heldAction();
    } else {
	sr = button->pressedAction();
	QString app = sr.app();
	bool vis = app=="qpe" ? isVisibleWindow(launcher->winId()) : hasVisibleWindow(app);
	if ( sr.message() == "raise()" && vis ) {
	    sr.setMessage("nextView()");
	} else {
	    // "back door" 
	    sr << (int)vis;
	}
    }
    sr.send();
}

void Desktop::appMessage(const QCString& message, const QByteArray&)
{
    if ( message == "nextView()" )
	launcher->nextView();
}

void Desktop::raiseMenu()
{
    Global::terminateBuiltin("calibrate"); // No tr
    tb->startMenu()->launch();
}

#if defined(QPE_HAVE_TOGGLELIGHT)
#include <qpe/config.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <time.h>
#endif

static bool blanked=FALSE;

static void blankScreen()
{
#ifdef QWS
    if ( !qt_screen ) return;
    /* Should use a big black window instead.
    QGfx* g = qt_screen->screenGfx();
    g->fillRect(0,0,qt_screen->width(),qt_screen->height());
    delete g;
    */
    blanked = TRUE;
#endif
}

static void darkScreen()
{
    extern void qpe_setBacklight(int);
    qpe_setBacklight(0); // force off
}


void Desktop::togglePower()
{
    bool wasloggedin = loggedin;
    loggedin=0;
    darkScreen();
    if ( wasloggedin )
	blankScreen();

    static int haveAPM = -1;
    if ( haveAPM < 0 ) {
	if ( QFile::exists( "/proc/apm" ) ) {
	    haveAPM = 1;
	} else {
	    haveAPM = 0;
	    qWarning( "Cannot suspend - no APM support in kernel" );
	}
    }
    if ( haveAPM ) {
	system("apm --suspend");
    }
#ifndef QT_NO_COP
    QWSServer::screenSaverActivate( FALSE );
    {
	QCopEnvelope("QPE/Card", "mtabChanged()" ); // might have changed while asleep
	QCopEnvelope e("QPE/System", "setBacklight(int)");
	e << -3; // Force on
    }
#endif
    if ( wasloggedin )
	login(TRUE);
    //qcopBridge->closeOpenConnections();
    //qDebug("called togglePower()!!!!!!");
}

void Desktop::toggleLight()
{
#ifndef QT_NO_COP
    QCopEnvelope e("QPE/System", "setBacklight(int)");
    e << -2; // toggle
#endif
}

void Desktop::toggleSymbolInput()
{
    tb->toggleSymbolInput();
}

void Desktop::toggleNumLockState()
{
    tb->toggleNumLockState();
}

void Desktop::toggleCapsLockState()
{
    tb->toggleCapsLockState();
}

void Desktop::styleChange( QStyle &s )
{
    QWidget::styleChange( s );
    int displayw = qApp->desktop()->width();
    int displayh = qApp->desktop()->height();

    QSize sz = tb->sizeHint();

    tb->setGeometry( 0, displayh-sz.height(), displayw, sz.height() );
}

void DesktopApplication::shutdown()
{
    if ( type() != GuiServer )
	return;
    ShutdownImpl *sd = new ShutdownImpl( 0, 0, WDestructiveClose );
    connect( sd, SIGNAL(shutdown(ShutdownImpl::Type)),
	     this, SLOT(shutdown(ShutdownImpl::Type)) );
    sd->showMaximized();
}

void DesktopApplication::shutdown( ShutdownImpl::Type t )
{
    switch ( t ) {
	case ShutdownImpl::ShutdownSystem:
	    execlp("shutdown", "shutdown", "-h", "now", (void*)0); // No tr
	    break;
	case ShutdownImpl::RebootSystem:
	    execlp("shutdown", "shutdown", "-r", "now", (void*)0); // No tr
	    break;
	case ShutdownImpl::RestartDesktop:
	    restart();
	    break;
	case ShutdownImpl::TerminateDesktop:
	    prepareForTermination(FALSE);
	    quit();
	    break;
    }
}

void DesktopApplication::restart()
{
    prepareForTermination(TRUE);

#ifdef Q_WS_QWS
    for ( int fd = 3; fd < 100; fd++ )
	close( fd );
#if defined(QT_DEMO_SINGLE_FLOPPY)
    execl( "/sbin/init", "qpe", 0 );
#elif defined(QT_QWS_CASSIOPEIA)
    execl( "/bin/sh", "sh", 0 );
#else
    execl( (qpeDir()+"/bin/qpe").latin1(), "qpe", 0 );
#endif
    exit(1);
#endif
}

void Desktop::startTransferServer()
{
    if ( !qcopBridge ) {
	// start qcop bridge server
	qcopBridge = new QCopBridge( 4243 );
	if ( qcopBridge->ok() ) {
	    // ... OK
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
	startTimer( 2000 );
}

void Desktop::timerEvent( QTimerEvent *e )
{
    killTimer( e->timerId() );
    startTransferServer();
}

void Desktop::terminateServers()
{
    delete transferServer;
    delete qcopBridge;
    transferServer = 0;
    qcopBridge = 0;
}

void Desktop::rereadVolumes()
{
    Config cfg("Sound");
    cfg.setGroup("System");
    touchclick = cfg.readBoolEntry("Touch");
    keyclick = cfg.readBoolEntry("Key");
}

void Desktop::pokeTimeMonitors()
{
    // inform all TimeMonitors
    QStrList tms = Service::channels("TimeMonitor");
    for (const char* ch = tms.first(); ch; ch=tms.next()) {
	QString t = getenv("TZ");
	QCopEnvelope e(ch, "timeChange(QString)");
	e << t;
    }
}

void Desktop::keyClick()
{
#ifdef CUSTOM_SOUND_KEYCLICK
    if ( keyclick )
	CUSTOM_SOUND_KEYCLICK;
#endif
}

void Desktop::screenClick()
{
#ifdef CUSTOM_SOUND_TOUCH
    if ( touchclick )
	CUSTOM_SOUND_TOUCH;
#endif
}

void Desktop::soundAlarm()
{
#ifdef CUSTOM_SOUND_ALARM
    CUSTOM_SOUND_ALARM;
#endif
}

bool Desktop::eventFilter( QObject *, QEvent *ev )
{
#ifdef QT_QWS_CUSTOM
    if ( ev->type() == QEvent::KeyPress ) {
	QKeyEvent *ke = (QKeyEvent *)ev;
	if ( ke->key() == Qt::Key_F11 ) { // menu key
	    QWidget *active = qApp->activeWindow();
	    if ( active && active->isPopup() )
		active->close();
	    else
		raiseMenu();
	    return TRUE;
	}
    }
#endif
    return FALSE;
}
