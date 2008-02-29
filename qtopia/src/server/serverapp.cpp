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

#include "serverapp.h"

#ifdef QTOPIA_PHONE
# include "phone/phonelauncher.h"
# include "phone/homescreen.h"
# include <qtopia/contextbar.h>
#endif

#include <qtopia/password.h>
#include <qtopia/config.h>
#include <qtopia/power.h>
#include <qtopia/devicebuttonmanager.h>
#include <qtopia/pluginloader.h>
#include <qtopia/storage.h>
#ifdef QTOPIA_PHONE
#include <qtopia/qcopbridge/qcopbridgeserver.h>
#include <qtopia/qcopbridge/qcopbridgeclient.h>
#endif

#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/global.h>
#include <qtopia/custom.h>

#ifdef Q_WS_QWS
#include <qgfx_qws.h>
#endif
#ifdef Q_OS_WIN32
#include <io.h>
#include <process.h>
#else
#include <time.h>
#include <unistd.h>
#endif
#include <qmessagebox.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qpixmapcache.h>

#include <stdlib.h>

static ServerApplication *serverApp = 0;
static int loggedin=0;



/*  Apply light/power settings for current power source */
static void applyLightSettings(PowerStatus *p)
{
    int initbright, intervalDim, intervalLightOff, intervalSuspend;
    bool dim, lightoff, suspend;

    {
	Config config("qpe");
	bool defsus;
        QStringList modes;
	/*	For compability	*/
        modes.append("Screensaver");
	if ( p->acStatus() == PowerStatus::Online ) {
	    config.setGroup("ExternalPower");
	    defsus = FALSE;
            modes.append("ExternalPower");
	} else {
	    config.setGroup("BatteryPower");
	    defsus = TRUE;
            modes.append("BatteryPower");
	}

	intervalDim = config.readNumEntry( "Interval_Dim", 20 );
	intervalLightOff = config.readNumEntry("Interval_LightOff", 30);
	intervalSuspend = config.readNumEntry("Interval", 240);
	initbright = config.readNumEntry("Brightness", 255);
	dim = config.readBoolEntry("Dim", TRUE);
	lightoff = config.readBoolEntry("LightOff", FALSE );
	suspend = config.readBoolEntry("Suspend", defsus );

        for( uint i=0; i<modes.count(); i++) {
            config.setGroup((*modes.at(i)));
            config.writeEntry( "Dim", dim );
            config.writeEntry( "LightOff", lightoff );
            config.writeEntry( "Suspend", suspend );
            config.writeEntry( "Interval_Dim", intervalDim );
            config.writeEntry( "Interval_LightOff", intervalLightOff );
            config.writeEntry( "Interval", intervalSuspend );
            config.writeEntry( "Brightness", initbright );
        }
    }

    int i_dim =      (dim ? intervalDim : 0);
    int i_lightoff = (lightoff ? intervalLightOff : 0);

#ifndef QTOPIA_PHONE
    int i_suspend =  (suspend ? intervalSuspend : 0);
#endif

#ifndef QT_NO_COP
    QCopEnvelope eB("QPE/System", "setBacklight(int)" );
    eB << -3; //forced on

    QCopEnvelope e("QPE/System", "setScreenSaverIntervals(int,int,int)" );
#ifndef QTOPIA_PHONE
    e << i_dim << i_lightoff << i_suspend;
#else
    e << i_dim << i_lightoff << 0;
#endif
#endif
}

//---------------------------------------------------------------------------

/*
  Priority is defined as the number of alerts that are needed to pop-up
an alert.
 */
class DesktopPowerAlerter : public QMessageBox
{
    Q_OBJECT
public:
    DesktopPowerAlerter( QWidget *parent, const char *name = 0 )
	: QMessageBox( tr("Battery Status"), tr("Low Battery"),
		       QMessageBox::Critical,
		       QMessageBox::Ok | QMessageBox::Default,
		       QMessageBox::NoButton, QMessageBox::NoButton,
		       parent, name, FALSE )
    {
	currentPriority = lastBlockedPriority = INT_MAX;
	alertCount = 0;
        blockTimer = new QTimer( this, "blockTimer" );
        connect(blockTimer, SIGNAL( timeout() ), this, SLOT( enableAlert() ));
    }

    void alert( const QString &text, int priority );
    void hideEvent( QHideEvent * );
private slots:
    void enableAlert();
private:
    QTimer *blockTimer;
    int lastBlockedPriority;
    int currentPriority;
    int alertCount;
};

void DesktopPowerAlerter::alert( const QString &text, int priority )
{
    if (blockTimer->isActive() && priority >= lastBlockedPriority)
        return;
    alertCount++;
    if ( alertCount < priority )
	return;
    if ( priority > currentPriority )
	return;
    currentPriority = priority;
    setText( "<qt>" + text + "</qt>" );
    show();
}


void DesktopPowerAlerter::hideEvent( QHideEvent *e )
{
    lastBlockedPriority = currentPriority;
    blockTimer->start(5 * 60 * 1000, TRUE); 
    QMessageBox::hideEvent( e );
    alertCount = 0;
    currentPriority = INT_MAX;
}

void DesktopPowerAlerter::enableAlert()
{
    lastBlockedPriority = INT_MAX;
}
//---------------------------------------------------------------------------

#ifdef QTOPIA_PHONE
class StorageAlertBox : public QMessageBox
{
    public:
        StorageAlertBox( const QString& caption, const QString &text, Icon icon,
		 int b0, int b1, int b2,
		 QWidget *parent=0, const char *name=0, bool modal=TRUE,
		 WFlags f=WStyle_DialogBorder  )
           : QMessageBox(caption, text, icon, b0, b1, b2, parent, name, modal,
                  f )
        {
        };
    protected:
        void showEvent(QShowEvent *se) {
            ContextBar::setLabel(this, Qt::Key_Back, "trash", "");
            QMessageBox::showEvent(se);
        };
       
};
#endif

class StorageMonitor : public QObject 
{
    Q_OBJECT
public:
    StorageMonitor(QObject *o, const char *name = 0)
        : QObject(o, name), box(0), suppressNotification(FALSE)
    {
        sinfo = new StorageInfo( this );
        QCopChannel *channel = new QCopChannel("QPE/System", this);
        connect(channel, SIGNAL(received(const QCString&,const QByteArray&)),
	    this, SLOT(systemMsg(const QCString&,const QByteArray&)) );
    }

    ~StorageMonitor()
    {
        if (box)
            delete box;
    }
    
public slots:
    void checkAvailStorage();
    void systemMsg(const QCString &msg, const QByteArray &data);

private slots:
    void showNotification();
    void showCleanupWizard();
    
private:
    void outOfSpace(QString& text);
    long fileSystemMetrics(const FileSystem* fs);
    
    StorageInfo *sinfo;
    QMessageBox *box;
    bool suppressNotification;
    
};

long StorageMonitor::fileSystemMetrics(const FileSystem *fs)
{
    long mult = 0;
    long div = 0;
    if (fs == 0)
        return 0;

    if ( fs->blockSize() ) {
        mult = fs->blockSize() / 1024;
        div = 1024 / fs->blockSize();
    }
    if ( !mult ) mult = 1;
    if ( !div ) div = 1;

    return (fs->availBlocks() * mult / div);
}

void StorageMonitor::checkAvailStorage()
{
    if (suppressNotification)
        return;

    sinfo->update();
    const FileSystem *fs;
    bool haveWritableFS = FALSE;
    fs = sinfo->fileSystemOf(QPEApplication::documentDir());
    if (fs == 0) {
        qDebug( "No file systems found for %s",QPEApplication::documentDir().local8Bit().data());
        return;
    }
    if( fs->isWritable() )
	haveWritableFS = TRUE;

    long availStorage = fileSystemMetrics(fs);
   
    //check all additional filesystems 
    const QList<FileSystem>& filesystems(sinfo->fileSystems());
    QListIterator<FileSystem> iter(filesystems);
    
    for ( ; iter.current(); ++iter )
    {
        if ((*iter)->isRemovable()) {
            availStorage += fileSystemMetrics(*iter);
	    if( (*iter)->isWritable() )
		haveWritableFS = TRUE;
        }
    }

    if( !haveWritableFS )
	return; // no writable filesystems, lack of free space is irrelevant
    
    //qDebug(QString("Free storage: %1 kB").arg(availStorage));
    
    //for now read config file each time until we have notification in place
    Config cfg("qpe"); 
    cfg.setGroup("StorageMonitor");
    int notificationLimit = cfg.readNumEntry("MinimalStorageLimit", 20);
    if (notificationLimit >= 0 && availStorage <= notificationLimit ) {  
        QString msg = tr("<qt>The device has no free storage space. "
               "Please delete unwanted documents.</qt>");
        outOfSpace(msg);
    }
}

void StorageMonitor::systemMsg(const QCString &msg, const QByteArray &data)
{
    QDataStream stream( data, IO_ReadOnly );
    
    if ( msg == "outOfDiskSpace(QString)" ) {
        QString text;
        stream >> text;
        if (!suppressNotification)
            outOfSpace( text );
    } else if ( msg == "checkDiskSpace()" ) {
        checkAvailStorage();
    }
    
}

static bool existingMessage = FALSE;

void StorageMonitor::outOfSpace(QString &msg) 
{
    if (existingMessage)
        return;

#ifdef QTOPIA_PHONE 
    if (HomeScreen::locked() || HomeScreen::guarded())
        return;
#endif
    
    existingMessage = suppressNotification = TRUE;

    // start cleanup wizard
    if (!box) {
#ifdef QTOPIA_PHONE
        box = new StorageAlertBox(tr("Out of storage space"), "", QMessageBox::Critical,
                QMessageBox::Ok | QMessageBox::Default, 
                QMessageBox::No, QMessageBox::NoButton);
#else
        box = new QMessageBox(tr("Out of storage space"), "", QMessageBox::Critical,
                QMessageBox::Ok | QMessageBox::Default, 
                QMessageBox::No, QMessageBox::NoButton);
#endif
        box->setButtonText(QMessageBox::No, tr("Cleanup"));
    }
    box->setText(msg);
#ifdef QTOPIA_PHONE
    switch (QPEApplication::execDialog(box)) {
#else
    switch (box->exec()) {
#endif
        case QMessageBox::Ok:
            break;
        case QMessageBox::No:
            qDebug("Starting cleanup wizard");
            showCleanupWizard();
            break;
    }
    
    //suppress next msg for 30 mins to avoid annoying msgbox
    QTimer::singleShot(30*60*1000, this, SLOT(showNotification()));
    existingMessage = FALSE;
}

void StorageMonitor::showNotification()
{
    suppressNotification = FALSE;
}

void StorageMonitor::showCleanupWizard() 
{
#ifdef QTOPIA_PHONE
    if (HomeScreen::locked() || HomeScreen::guarded())
        return;
#endif

    ServiceRequest req("CleanupWizard", "showCleanupWizard()");
    req.send();
}

//---------------------------------------------------------------------------

KeyFilter::KeyFilter(QObject* parent) : QObject(parent), held_tid(0), heldButton(0)
{
    qwsServer->setKeyboardFilter(this);
}

void KeyFilter::timerEvent(QTimerEvent* e)
{
    if ( e->timerId() == held_tid ) {
	killTimer(held_tid);
	// button held
	if ( heldButton ) {
	    emit activate(heldButton, TRUE);
	    heldButton = 0;
	}
	held_tid = 0;
    }
}

bool KeyFilter::filter(int unicode, int keycode, int modifiers, bool press,
		  bool autoRepeat)
{
    if ( !loggedin
	    // Permitted keys
	    && keycode != Key_0
	    && keycode != Key_1
	    && keycode != Key_2
	    && keycode != Key_3
	    && keycode != Key_4
	    && keycode != Key_5
	    && keycode != Key_6
	    && keycode != Key_7
	    && keycode != Key_8
	    && keycode != Key_9
	    && keycode != Key_Asterisk
	    && keycode != Key_NumberSign
#ifdef QTOPIA_PHONE
	    && keycode != Key_Context1
	    && keycode != Key_Context2
	    && keycode != Key_Select
	    && keycode != Key_Back
	    && keycode != Key_Yes
	    && keycode != Key_No
	    && keycode != Key_Call
	    && keycode != Key_Hangup
	    && keycode != Key_Flip
#endif
	    && keycode != Key_Menu
	    && keycode != Key_F34 // power
	    && keycode != Key_F30 // select
	    && keycode != Key_Enter
	    && keycode != Key_Return
	    && keycode != Key_Space
	    && keycode != Key_Left
	    && keycode != Key_Right
	    && keycode != Key_Up
	    && keycode != Key_Down ) 
	return TRUE;
    if ( !modifiers ) {
	if ( !((ServerApplication*)qApp)->keyboardGrabbed() ) {
	    // First check to see if DeviceButtonManager knows something about this button:
	    const DeviceButton* button = DeviceButtonManager::instance().buttonForKeycode(keycode);
	    if (button && !autoRepeat) {
		if ( held_tid ) {
		    killTimer(held_tid);
		    held_tid = 0;
		}
		if ( button->heldAction().isNull() ) {
		    if ( press )
			emit activate(button, FALSE);
		} else if ( press ) {
		    heldButton = button;
		    held_tid = startTimer(500);
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
	if ( keycode == Key_F31 ) {
	    if ( press ) emit symbol();
	    QWSServer::screenSaverActivate(FALSE);
	    return TRUE;
	}
    } else if ( modifiers == ControlButton ) {
        if ( keycode == Key_C || keycode == Key_V || keycode == Key_X ) 
            return FALSE;
    }
    if ( keycode == Key_NumLock ) {
	if ( press ) emit numLockStateToggle();
    }
    if ( keycode == Key_CapsLock ) {
	if ( press ) emit capsLockStateToggle();
    }
    if ( serverApp )
	serverApp->keyClick(unicode, keycode, modifiers, press, autoRepeat);
    return FALSE;
}

ServerApplication::MemState ServerApplication::memstate = ServerApplication::MemUnknown;

#if defined(QPE_MEMALERTER_IMPL)
QPE_MEMALERTER_IMPL
#endif

#if defined(CUSTOM_SOUND_IMPL)
CUSTOM_SOUND_IMPL
#endif

//---------------------------------------------------------------------------


bool ServerApplication::doRestart = FALSE;
bool ServerApplication::allowRestart = TRUE;
QArray<int> ServerApplication::pgFaults = QArray<int>();
#if defined(QPE_HAVE_MEMALERTER)
ServerApplication::VMMonitorType ServerApplication::VMStatType = VMUnknown;
#if !defined(QPE_MEMALERTER_IMPL)
static bool slowVMMonitor;
static const int MIN_MEM_LIMIT = 10000;
#endif
#endif
QStringList *ServerApplication::features = 0;


ServerApplication::ServerApplication( int& argc, char **argv, Type t )
    : QPEApplication( argc, argv, t ), userInfoBox( 0 )
{
#ifdef CUSTOM_SOUND_INIT
    CUSTOM_SOUND_INIT;
#endif

#if defined(QPE_MEMALERTER_IMPL)
    initMemalerter();
#endif

#ifdef QTOPIA_PHONE
    (void)new QCopBridgeServer(this);
    QCopBridgeClient::connect();
    QCopBridgeClient::importChannel("QPE/PhoneStatusServer", true);
#endif

    if (!features)
	features = new QStringList;
    features->clear();

    // We know we'll have lots of cached pixmaps due to App/DocLnks
    QPixmapCache::setCacheLimit(512);
#ifdef QTOPIA_PHONE
    PhoneLauncher::initExportedBackgrounds();
    Config btnCfg(Global::defaultButtonsFile(), Config::File);
    btnCfg.setGroup("Device");
    QStringList ilist = btnCfg.readListEntry("Input", ',');
    if (!ilist.isEmpty()) {
	*features += ilist;
    } else {
	if (Global::mousePreferred()) {
# ifdef QPE_NEED_CALIBRATION
	    features->append("Calibrate");
# endif
	    features->append("Touchscreen");
	} else {
	    features->append("Keypad");
	}
    }
#else
# ifdef QPE_NEED_CALIBRATION
    features->append("Calibrate");
# endif
    features->append("Touchscreen");
#endif

    QTimer *timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(psTimeout()) );
    timer->start( 10000 );
    
#if defined(QPE_HAVE_MEMALERTER)
    delayedVMInfo = new QTimer( this );
    connect(delayedVMInfo, SIGNAL(timeout()), this, SLOT(showVMStatusToUser()) );
    userInfoBox = new QMessageBox( tr("Memory Status"),
        tr("<qt>Critical Memory Shortage. "
      	   "Please end applications as soon as possible.</qt>"),
        QMessageBox::Critical, QMessageBox::Ok|QMessageBox::Default,
        QMessageBox::NoButton, QMessageBox::NoButton, 0, "UserInfoBox" );
    //force dialog to be loaded and generated to ensure quick response
    //if system is short of memory
    userInfoBox->show();
    userInfoBox->hide(); 

#if !defined(QPE_MEMALERTER_IMPL)
    pgFaults.fill(0, 10);
    QFile vmstat("/proc/vmstat"); //kernel 2.6+
    if (vmstat.exists() && vmstat.open( IO_ReadOnly )) {
        //qDebug("Memory Monitor: Linux 2.6");
        VMStatType  = VMLinux_2_6;
        vmstat.close();
    } else {
        VMStatType = VMLinux_2_4;
        //qDebug("Memory Monitor: Linux 2.4");
   }
    
    if (VMStatType != VMUnknown) { 
        vmMonitor = new QTimer( this );
        connect( vmMonitor, SIGNAL(timeout()), this, SLOT(memoryMonitor()));
        int freeMem = -1;
        readMemInfo(&freeMem);
        if (freeMem > MIN_MEM_LIMIT) {
            // plenty of mem available => reduce number of checks
            vmMonitor->start( 10000 ); 
            slowVMMonitor = TRUE;
        }
        else {
            vmMonitor->start( 1000 ); 
            slowVMMonitor = FALSE;
        }
    }
#endif
#endif

    Config cfg("qpe");
    cfg.setGroup("StorageMonitor");
    if (cfg.readBoolEntry("Enabled", TRUE)) {
        //qDebug("Storage Monitor enabled");
        sm = new StorageMonitor( this );
        QTimer *storageTimer = new QTimer( this );
        connect( storageTimer, SIGNAL(timeout()), sm, SLOT(checkAvailStorage()));
        storageTimer->start(cfg.readNumEntry("UpdatePeriod", 15)*1000);
    }
    
    ps = new PowerStatus;
    pa = new DesktopPowerAlerter( 0 );
    applyLightSettings( ps );

#ifndef QT_NO_COP
    QCopEnvelope eB("QPE/System", "setBacklight(int)" );
    eB << -3; //forced on
#endif

    if ( PluginLoader::inSafeMode() )
	QTimer::singleShot(500, this, SLOT(showSafeMode()) );
    QTimer::singleShot(20*1000, this, SLOT(clearSafeMode()) );

    KeyFilter* kf = new KeyFilter(this);

    connect( kf, SIGNAL(launch()), this, SIGNAL(launch()) );
    connect( kf, SIGNAL(power()), this, SIGNAL(power()) );
    connect( kf, SIGNAL(backlight()), this, SIGNAL(backlight()) );
    connect( kf, SIGNAL(symbol()), this, SIGNAL(symbol()));
    connect( kf, SIGNAL(numLockStateToggle()), this,SIGNAL(numLockStateToggle()));
    connect( kf, SIGNAL(capsLockStateToggle()), this,SIGNAL(capsLockStateToggle()));
    connect( kf, SIGNAL(activate(const DeviceButton*,bool)), this,SIGNAL(activate(const DeviceButton*,bool)));

    connect( kf, SIGNAL(power()), this, SLOT(togglePower()) );
    connect( kf, SIGNAL(backlight()), this, SLOT(toggleLight()) );

    connect( this, SIGNAL(volumeChanged(bool)), this, SLOT(rereadVolumes()) );
    rereadVolumes();

    serverApp = this;
}


ServerApplication::~ServerApplication()
{
    delete userInfoBox;
    delete ps;
    delete pa;
}

bool ServerApplication::screenLocked()
{
    return loggedin == 0;
}

void ServerApplication::lockScreen(bool b)
{
    loggedin = b ? 0 : 1;
}

void ServerApplication::login(bool at_poweron)
{
    if ( !loggedin ) {
	Global::terminateBuiltin("calibrate"); // No tr
	serverApp->authenticate(at_poweron);
#ifndef QT_NO_COP
	QCopEnvelope e( "QPE/Desktop", "unlocked()" );
#endif
    }
}

bool ServerApplication::haveFeature(const QString &f)
{
    return features->find(f) != features->end();
}

static bool blanked=FALSE;

static void blankScreen()
{
#ifdef QWS
    QWidget w(0, 0, Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WStyle_Tool | Qt::WStyle_StaysOnTop | Qt::WPaintUnclipped);
    w.resize( qt_screen->width(), qt_screen->height() );
    w.move(0, 0);
    
    QPainter p(&w);
    p.fillRect(w.rect(), QBrush(QColor(255,255,255)) );
    p.end();
    w.repaint();

    blanked = TRUE;
#endif
}

void ServerApplication::togglePower()
{
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
	bool wasloggedin = loggedin;
	loggedin=0;
	if ( wasloggedin ) {
	    Config cfg("Security");
	    cfg.setGroup("Passcode");
	    QString passcode = cfg.readEntry("passcode");
	    if ( !passcode.isEmpty() && cfg.readNumEntry("passcode_poweron",0) )
		blankScreen();
	}
	
	system("apm --suspend");
    
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
    } 
    
    //qcopBridge->closeOpenConnections();
    //qDebug("called togglePower()!!!!!!");
}

void ServerApplication::toggleLight()
{
#ifndef QT_NO_COP
    QCopEnvelope e("QPE/System", "setBacklight(int)");
    e << -2; // toggle
#endif
}


#ifdef Q_WS_QWS

bool ServerApplication::qwsEventFilter( QWSEvent *e )
{

#ifdef QTOPIA_TEST 
#ifndef QT_NO_QWS_IM
    if (e->type == QWSEvent::IMEvent) {
        server_slave.filterIMEvent(e);
    }
#endif
#endif
    checkMemory();

    if ( e->type == QWSEvent::Mouse ) {
	QWSMouseEvent *me = (QWSMouseEvent *)e;
	static bool up = TRUE;
	if ( me->simpleData.state&LeftButton ) {
	    if ( up ) {
		up = FALSE;
		screenClick(TRUE);
	    }
	} else if ( !up ) {
	    up = TRUE;
	    screenClick(FALSE);
	}
    }
#ifdef QTOPIA_PHONE
    if (screenLocked()) {
        if (e->type == QWSEvent::Key) {
            QWSKeyEvent *ke = (QWSKeyEvent *)e;
	    // Always let Key_Back and Key_no through in case we get a visible dialog.
	    if (ke->simpleData.keycode != Key_Back && ke->simpleData.keycode != Key_No) {
	        emit preProcessKeyLockEvent(ke->simpleData.keycode, ke->simpleData.unicode, ke->simpleData.is_press);
	        return TRUE;
	    }
        } else if (Global::mousePreferred() && e->type == QWSEvent::IMEvent)  {
            QWSIMEvent *ie = (QWSIMEvent *)e;
            if (ie->simpleData.type == QWSServer::IMEnd && ie->simpleData.textLen == 1) {
                emit preProcessKeyLockEvent(*(ie->text), ie->text->unicode(), TRUE);
                return TRUE;
            }
        }
    } 
#endif

    return QPEApplication::qwsEventFilter( e );
}
#endif

void ServerApplication::psTimeout()
{
#if defined(QPE_HAVE_MEMALERTER) && !defined(QPE_MEMALERTER_IMPL)
    evalMemory();
#endif
    checkMemory(); // in case no events are being generated

    PowerStatus::ACStatus oldStatus = ps->acStatus();

    *ps = PowerStatusManager::readStatus();

    if ( oldStatus != ps->acStatus() ) {
	// power source changed, read settings applying to current powersource
	applyLightSettings(ps);
    }


    if ( (ps->batteryStatus() == PowerStatus::VeryLow ) ) {
	pa->alert( tr( "Battery is running very low." ), 6 );
    }

    if (  ps->batteryStatus() == PowerStatus::Critical ) {
	pa->alert(  tr( "Battery level is critical!\n"
			"Please recharge the main battery!" ), 1 );
    }

    if (  ps->backupBatteryStatus() == PowerStatus::VeryLow ) {
	pa->alert( tr( "Back-up battery is very low.\nPlease charge the back-up battery." ), 3 );
    }
}

void ServerApplication::showSafeMode()
{
    if ( QMessageBox::warning(0, tr("Safe Mode"), tr("<qt>A system startup error occurred, "
		"and the system is now in Safe Mode. "
		"Plugins are not loaded in Safe Mode. "
		"You can use the Plugin Manager to "
		"disable plugins that cause system error.</qt>"),
		tr("OK"), tr("Plugin Manager..."), 0) == 1 ) {
	Global::execute( "pluginmanager" );
    }
}

void ServerApplication::clearSafeMode()
{
    // If we've been running OK for a while then we won't bother going into
    // safe mode immediately on the next crash.
    Config cfg( "PluginLoader" );
    cfg.setGroup( "Global" );
    QString mode = cfg.readEntry( "Mode", "Normal" );
    if ( mode == "MaybeSafe" ) {
	cfg.writeEntry( "Mode", "Normal" );
    }
}

void ServerApplication::shutdown()
{
    if ( type() != GuiServer )
	return;
    ShutdownImpl *sd = new ShutdownImpl( 0, 0, WDestructiveClose );
    connect( sd, SIGNAL(shutdown(ShutdownImpl::Type)),
	     this, SLOT(shutdown(ShutdownImpl::Type)) );
    sd->showMaximized();
}

void ServerApplication::shutdown( ShutdownImpl::Type t )
{
    switch ( t ) {
	case ShutdownImpl::ShutdownSystem:
#ifndef Q_OS_WIN32
	    execlp("shutdown", "shutdown", "-h", "now", (void*)0); // No tr
#else
	    qDebug("ServerApplication::ShutdownSystem");      
	    prepareForTermination(FALSE);
	    quit();
#endif
	    break;

	case ShutdownImpl::RebootSystem:
#ifndef Q_OS_WIN32
	    execlp("shutdown", "shutdown", "-r", "now", (void*)0); // No tr
#else
	    qDebug("ServerApplication::RebootSystem");      
	    restart();
#endif
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

void ServerApplication::restart()
{
    if ( allowRestart ) {
	prepareForTermination(TRUE);
	doRestart = TRUE;
	quit();
    }
}

void ServerApplication::rereadVolumes()
{
    Config cfg("Sound");
    cfg.setGroup("System");
    touchclick = cfg.readBoolEntry("Touch");
    keyclick = cfg.readBoolEntry("Key");
}


void ServerApplication::checkMemory()
{
#if defined(QPE_HAVE_MEMALERTER)

    switch ( memstate ) {
	case MemUnknown:
	    break;
	case MemLow: {
		memstate = MemUnknown;
		static time_t lastCheck = 0;
		if (time(0) - lastCheck > 10) {
		    emit recoverMemory(MemLow);
		    lastCheck = time(0);
		}
	    }
	    break;
	case MemNormal:
	    memstate = MemUnknown;
	    break;
	case MemVeryLow: 
	    memstate = MemUnknown;
            //delay dialog box because it increases
            //the page faults and thus causing a crash in a tight
            //situation
            delayedVMInfo->start(20000, TRUE);
             
	    emit recoverMemory(MemVeryLow);
            break;
        case MemCritical:
            memstate = MemUnknown;
            emit recoverMemory(MemCritical);
            //dont make it worse by poping up a 2nd dialog
            if (delayedVMInfo->isActive())
                delayedVMInfo->stop(); 
    }
#endif
}

void ServerApplication::keyClick(int unicode, int keycode, int modifiers, bool press, bool repeat)
{
#ifdef QTOPIA_TEST
// Before doing anything else... see if we want to record the keyevent
        QEvent::Type type ;
        if (press) {
            type = QEvent::KeyPress;
        } else {
            type = QEvent::KeyRelease;
        }
        QKeyEvent qe( type, keycode, unicode, modifiers, QString::null, repeat );//, state, text, autoRep, count );
        server_slave.eventFilter( &qe );
#else
    Q_UNUSED(unicode);
    Q_UNUSED(modifiers);
#endif

#ifdef CUSTOM_SOUND_KEYCLICK
    if ( keyclick )
	CUSTOM_SOUND_KEYCLICK(keycode,press,repeat);
#else
    Q_UNUSED( keycode );
    Q_UNUSED( press );
    Q_UNUSED( repeat );
#endif
}

void ServerApplication::screenClick(bool press)
{
#ifdef CUSTOM_SOUND_TOUCH
    if ( touchclick )
	CUSTOM_SOUND_TOUCH(press);
#else
    Q_UNUSED( press );
#endif
}

// This is the default memory monitor used when there is no other
// memory monitor available
void ServerApplication::memoryMonitor()
{
#if defined(QPE_HAVE_MEMALERTER) && !defined(QPE_MEMALERTER_IMPL)
    int freemem = -1;
    int r = readMemInfo( &freemem );
    if ( r < 0 ) 
        freemem = MIN_MEM_LIMIT - 1;
    //qDebug(QString("call to MemoryMonitor(%1 KB)").arg(freemem));
    if (freemem > MIN_MEM_LIMIT) {
        if (!slowVMMonitor) {
            vmMonitor->start(10000);
            slowVMMonitor = TRUE;
        }
        return;
    } else {
        if (slowVMMonitor) {
            vmMonitor->start(1000);
            slowVMMonitor = FALSE;
        }
    }
    
    static short count = 0;
    static long prevFaults = 0;
    
    if (VMStatType == VMLinux_2_6) {
        QFile vmstat("/proc/vmstat"); //kernel 2.6+
        if (vmstat.open( IO_ReadOnly )) {
            QTextStream t ( &vmstat );
            QString nLine, majfaults;
            nLine = t.readLine();       
            while (!nLine.isNull()) {
                //only interested in major page faults 
                if (nLine.contains("pgmajfault"))  {
                    majfaults = nLine.mid(nLine.find(' '));
                    break;
                }
                nLine = t.readLine();
            }
           
            bool ok;
            long newFaults = majfaults.toInt(&ok);
            if (prevFaults != 0  && ok) {
                pgFaults[count] = newFaults - prevFaults;
            }
            else
                pgFaults[count] = 0;
            
            count = (++count) % pgFaults.size();
            prevFaults = newFaults;
            vmstat.close();
        }
    } else if (VMStatType == VMLinux_2_4) {
        QFile vmstat("/proc/"+QString::number(::getpid())+"/stat");
        if (vmstat.open( IO_ReadOnly )) {
            QTextStream t(&vmstat);
            QString majFaults;
            for (int i = 0; i < 12; i++) 
                t >> majFaults;
            vmstat.close();
            bool ok;
            long newFaults = majFaults.toLong(&ok);
            if (prevFaults != 0 && ok) {
                pgFaults[count] = newFaults-prevFaults;
                if (pgFaults[count] > 250) {
                    qWarning("MemoryMonitor: killing last application started");
                    emit recoverMemory(MemCritical); 
                    pgFaults.fill(0);
                }
            } else 
                pgFaults[count] = 0;
            //qDebug(QString("faults: %1").arg(newFaults-prevFaults));
            count = (++count) % pgFaults.size();
            prevFaults = newFaults;
        }
    }
#endif
}

void ServerApplication::evalMemory() 
{
#if defined(QPE_HAVE_MEMALERTER) && !defined(QPE_MEMALERTER_IMPL)

    if (VMStatType == VMLinux_2_6 ) {
        int sum = 0;
        for (int i=0; i < (int)pgFaults.size(); i++) {
            sum += pgFaults[i];
        }
        int avg = sum/pgFaults.size();
        //qDebug(QString("page fault average: %1 (%2)").arg(avg).arg(sum));
        if (avg > 250) {
            //delete history to ensure timeout after kill
            pgFaults.fill(0); 
            memstate = MemCritical;
        } else if (avg > 120) 
            memstate = MemVeryLow;
        else if (avg > 60)
            memstate = MemLow;
        else
            memstate = MemNormal;
    } else if (VMStatType == VMLinux_2_4) {
        int sum = 0;
        for (int i=0; i < (int)pgFaults.size(); i++) {
            sum += pgFaults[i];
        }
        
        int avg = sum / pgFaults.size();
        //qDebug(QString("page fault average for qpe: %1 (%2)").arg(avg).arg(sum));
        
        if ( avg > 50 ) { 
            memstate = MemCritical;
        } else if ( avg > 25 )
            memstate = MemVeryLow;
        else if ( avg > 20 ) {
            memstate = MemLow;
        }
    } else { //not used atm
        int freeMem;
        int pcUsed = readMemInfo( &freeMem );
        //qDebug(QString("Alternative mem eval: %1").arg(pcUsed));
        if (pcUsed < 0) {
            memstate = MemUnknown;
            return;
        }
            
        if (pcUsed > 75)
            memstate = MemVeryLow;
        else if (pcUsed > 60)
            memstate = MemLow;
        else
            memstate = MemNormal;
    }
#endif
}

int ServerApplication::readMemInfo(int *freeMem) 
{
#if defined(QPE_HAVE_MEMALERTER) && !defined(QPE_MEMALERTER_IMPL)
    QFile file( "/proc/meminfo" );
    int pcUsed = -1;
    if ( file.open( IO_ReadOnly ) ) {
        int total, memfree, buffers, cached;
        QTextStream t( &file );
        
        // structure of meminfo changed in kernel 2.6
        QString word;

        while (!t.atEnd()) {
            t>>word;
            if (word == "MemTotal:") {
                t >> total;
            } else if (word == "MemFree:") {
                t >> memfree;
            } else if (word == "Buffers:") {
                t >> buffers;
            } else if (word == "Cached:") {
                t >> cached;
                break; //last entry to read
            }
        }
        *freeMem = buffers + cached + memfree;
        int realUsed = total - ( *freeMem );
        pcUsed = 100*realUsed/total;
    }
    return pcUsed;
#else
    Q_UNUSED(freeMem);
    return -1;
#endif
}
void ServerApplication::showVMStatusToUser()
{
#if defined(QPE_HAVE_MEMALERTER)
    static bool existingMessage = FALSE; 
    
    if(existingMessage)
        return; // don't show a second message while still on first

#if !defined(QPE_MEMALERTER_IMPL)
    evalMemory();
#endif        
    if (memstate == MemVeryLow || memstate == MemCritical) {
        delayedVMInfo->start(10000, TRUE); //system to busy atm
        return;
    }
    existingMessage = TRUE;

    /*QMessageBox::critical( 0 , tr("Memory Status"),
        tr("<qt>Critical Memory Shortage. "
      	   "Please end applications as soon as possible.</qt>") );*/

    userInfoBox->exec();
    existingMessage = FALSE;
#endif
}
#include "serverapp.moc"
