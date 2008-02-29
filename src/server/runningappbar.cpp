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
**********************************************************************
*/

#define QTOPIA_INTERNAL_PRELOADACCESS

#include <qtopia/qpeglobal.h>

#include <signal.h>	// for killl
#include <stdlib.h>

#include <qtimer.h>
#include <qpopupmenu.h>
#include <qpainter.h>
#include <qmessagebox.h>

#include <qtopia/qprocess.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/applnk.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/mimetype.h>

#include "runningappbar.h"
#include "applauncher.h"

// 3600 seconds before a temporary disabled screensaver is reenabled by server
//#define QTOPIA_MAX_SCREEN_DISABLE_TIME ((int) 3600)
#ifdef QTOPIA_MAX_SCREEN_DISABLE_TIME
#define QTOPIA_MIN_SCREEN_DISABLE_TIME ((int) 300)  // min 5 minutes before forced suspend kicks in
#endif

static QStringList checkingApps;
static QStringList launchingApps;

RunningAppBar::RunningAppBar(QWidget* parent) 
  : QFrame(parent), m_AppLnkSet(0L), m_SelectedAppIndex(-1)
{
    m_AppLnkSet = new AppLnkSet( MimeType::appsFolderName() );

    tsmMonitor = new TempScreenSaverMonitor();
    connect(tsmMonitor, SIGNAL(forceSuspend()), this, SIGNAL(forceSuspend()) );
    
    QCopChannel* channel = new QCopChannel( "QPE/System", this );
    connect( channel, SIGNAL(received(const QCString&, const QByteArray&)),
	     this, SLOT(received(const QCString&, const QByteArray&)) );

    spacing = AppLnk::smallIconSize()+3;
    
    appLauncher = new AppLauncher(m_AppLnkSet, this);
    connect(appLauncher, SIGNAL(launched(int, const QString &)), this, SLOT(applicationLaunched(int, const QString &)) );
    connect(appLauncher, SIGNAL(terminated(int, const QString &)), this, SLOT(applicationTerminated(int, const QString &)) );
    connect(appLauncher, SIGNAL(connected(const QString &)), this, SLOT(applicationConnected(const QString &)) );
}

RunningAppBar::~RunningAppBar() 
{
    delete m_AppLnkSet;
    delete tsmMonitor;
    delete appLauncher;
}

void RunningAppBar::reloadApps()
{
    appLauncher->setAppLnkSet(0);
    delete m_AppLnkSet;
    m_AppLnkSet = new AppLnkSet( MimeType::appsFolderName() );
    appLauncher->setAppLnkSet(m_AppLnkSet);
}

void RunningAppBar::received(const QCString& msg, const QByteArray& data) {
    // Since fast apps appear and disappear without disconnecting from their
    // channel we need to watch for the showing/hiding events and update according.
    QDataStream stream( data, IO_ReadOnly );
    if ( msg == "fastAppShowing(QString)") {
	QString appName;
	stream >> appName;
	//    qDebug("fastAppShowing %s", appName.data() );
	const AppLnk* f = m_AppLnkSet->findExec(appName);
	if ( f ) addTask(*f);
    } else if ( msg == "fastAppHiding(QString)") {
	QString appName;
	stream >> appName;
	const AppLnk* f = m_AppLnkSet->findExec(appName);
	if ( f ) removeTask(*f);
    } else if ( msg == "setTempScreenSaverMode(int,int)" ) {
	int mode, pid;
	stream >> mode >> pid;
	tsmMonitor->setTempMode(mode, pid);
    }
}

void RunningAppBar::addTask(const AppLnk& appLnk) {
//    qDebug("Added %s to app list.", appLnk.name().latin1());
    AppLnk* newApp = new AppLnk(appLnk);
    newApp->setExec(appLnk.exec());
    m_AppList.prepend(newApp);
    update();
}

void RunningAppBar::removeTask(const AppLnk& appLnk) {
  unsigned int i = 0;
  for (; i < m_AppList.count() ; i++) {
    AppLnk* target = m_AppList.at(i);
    if (target->exec() == appLnk.exec()) {
//       qDebug("Removing %s from app list.", appLnk.name().latin1());
      m_AppList.remove();
      delete target;
    }
  }
  update();
}

void RunningAppBar::mousePressEvent(QMouseEvent *e)
{
  // Find out if the user is clicking on an app icon...
  // If so, snag the index so when we repaint we show it
  // as highlighed.
  m_SelectedAppIndex = 0;
  int x=0;
  QListIterator<AppLnk> it( m_AppList );
  for ( ; it.current(); ++it,++m_SelectedAppIndex,x+=spacing ) {
    if ( x + spacing <= width() ) {
      if ( e->x() >= x && e->x() < x+spacing ) {
	if ( m_SelectedAppIndex < (int)m_AppList.count() ) {
	  repaint(FALSE);
	  return;
	}
      }
    } else {
      break;
    }
  }
  m_SelectedAppIndex = -1;
  repaint( FALSE );
}

void RunningAppBar::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == QMouseEvent::RightButton) {
	return;
    }
    if ( m_SelectedAppIndex >= 0 ) {
	QString app = m_AppList.at(m_SelectedAppIndex)->exec(); 
	QString channel = QString("QPE/Application/") + app;
	
	// it might be started but not yet opened a qcop channel.  Test for both
	if ( appLauncher->isRunning(app) ) {
	    // qDebug("%s is running!", app.latin1() );
	    if ( QCopChannel::isRegistered(channel.latin1()) ) {
		// qDebug("%s is registered!", m_AppList.at(m_SelectedAppIndex)->exec().latin1());
		if ( checkingApps.find(app) == checkingApps.end() ) {
		    checkingApps.append( app );
		    QCopEnvelope e(channel.latin1(), "raise()");
		    // This class will delete itself after hearing from the app or the timer expiring
		    (void)new AppMonitor(*m_AppList.at(m_SelectedAppIndex), *this);
		} else {
		    // qDebug("already quering %s", app.data() );
		}
	    } else {
		if ( launchingApps.find( app ) == launchingApps.end() )
		    launchingApps.append( app );
	    }
	} else {
	    // this should never happen with the new implementation
	    removeTask(*m_AppList.at(m_SelectedAppIndex));
	}

	m_SelectedAppIndex = -1;
	update();
    }
}

void RunningAppBar::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    AppLnk *curApp;
    int x = 0;
    int y = (height() - AppLnk::smallIconSize()) / 2;
    int i = 0;

    p.fillRect( 0, 0, width(), height(), colorGroup().background() );

    QListIterator<AppLnk> it(m_AppList);
    
    for (; it.current(); i++, ++it ) {
      if ( x + spacing <= width() ) {
	curApp = it.current();
 	if ( (int)i == m_SelectedAppIndex )
 	  p.fillRect( x, y, spacing, curApp->pixmap().height()+1, colorGroup().highlight() );
  	else 
	  p.eraseRect( x, y, spacing, curApp->pixmap().height()+1 );
	p.drawPixmap( x, y, curApp->pixmap() );
	x += spacing;
      }
    }
}

QSize RunningAppBar::sizeHint() const
{
    return QSize( frameWidth(), AppLnk::smallIconSize()+frameWidth()*2+3 );
}

void RunningAppBar::applicationLaunched(int, const QString &appName)
{
    // qDebug("desktop:: app: %s launched with pid %d", appName.data(), pid);
    const AppLnk* newGuy = m_AppLnkSet->findExec(appName);
    if ( newGuy && !newGuy->isPreloaded() ) {
	addTask( *newGuy );
	QCopEnvelope e("QPE/System", "busy()");
    }
}

void RunningAppBar::applicationTerminated(int pid, const QString &app)
{
    const AppLnk* gone = m_AppLnkSet->findExec(app);
    if ( gone ) {
	removeTask(*gone);
	tsmMonitor->applicationTerminated(pid);
    }
}

void RunningAppBar::applicationConnected( const QString &app )
{
    QStringList::Iterator it = launchingApps.find( app );
    if ( it != launchingApps.end() ) {
	launchingApps.remove( it );
	if ( checkingApps.find(app) == checkingApps.end() ) {
	    QString channel = QString("QPE/Application/") + app;
	    QCopEnvelope e(channel.latin1(), "raise()");
	}
    }
}

/*	App Monitor	*/
const int AppMonitor::RAISE_TIMEOUT_MS = 2000;

AppMonitor::AppMonitor(const AppLnk& app, RunningAppBar& owner) 
  : QObject(0L), m_Owner(owner), m_App(app), m_PsProc(0L), m_AppKillerBox(0L) {
  QCopChannel* channel = new QCopChannel( "QPE/System", this );
  connect( channel, SIGNAL(received(const QCString&, const QByteArray&)),
	   this, SLOT(received(const QCString&, const QByteArray&)) );
  connect(&m_Timer, SIGNAL(timeout()), this, SLOT(timerExpired()));
  m_Timer.start(RAISE_TIMEOUT_MS, TRUE);
}

AppMonitor::~AppMonitor() 
{
    if (m_AppKillerBox) {
	delete m_AppKillerBox;
	m_AppKillerBox = 0L;
    }
    checkingApps.remove( m_App.exec() );
}

void AppMonitor::received(const QCString& msg, const QByteArray& data) {
  QDataStream stream( data, IO_ReadOnly );

  if (msg == "appRaised(QString)") {
    QString appName;
    stream >> appName;
    if (appName == m_App.exec()) {
      qDebug("Got a heartbeat from %s", appName.latin1());
      m_Timer.stop();
      // Check to make sure we're not waiting on user input...
      if (m_AppKillerBox) {
	// If we are, we kill the dialog box, and the code waiting on the result
	// will clean us up (basically the user said "no").
	delete m_AppKillerBox;
	m_AppKillerBox = 0L;
      }
      else {
	// Ok, we're not waiting on user input, so clean us up now.
	// WE DELETE OURSELVES HERE!  Don't do anything else!!
	delete this;
      }
    }
  }
}

void AppMonitor::timerExpired() {
  //   qDebug("Checking in on %s", m_App.name().latin1());
  // We store this incase the application responds while we're
  // waiting for user input so we know not to delete ourselves.  This
  // will be cleaned up in the destructor.
  m_AppKillerBox = new QMessageBox(tr("Application Problem"),
				   tr("<p>%1 is not responding.</p>").arg(m_App.name()) +
				   tr("<p>Would you like to force the application to exit?</p>"), 
				   QMessageBox::Warning, QMessageBox::Yes, 
				   QMessageBox::No | QMessageBox::Default, 
				   QMessageBox::NoButton);
  if (m_AppKillerBox->exec()  == QMessageBox::Yes) {    
    // qDebug("Killing the app!!! Bwuhahahaha!");
    m_PsProc = new  QProcess(QString("ps"));
    m_PsProc->addArgument("h");
    m_PsProc->addArgument("-C");
    m_PsProc->addArgument(m_App.exec());
    m_PsProc->addArgument("-o");
    m_PsProc->addArgument("pid");
    connect(m_PsProc, SIGNAL(processExited()), this, SLOT(psProcFinished()));
    m_PsProc->start();
  }
  else {
    // qDebug("Wuss..");
    // WE DELETE OURSELVES HERE!  Don't do anything else!!
    delete this;
  }
}

void AppMonitor::psProcFinished() {
  QString pid = m_PsProc->readLineStdout();
  delete m_PsProc;
  m_PsProc = 0L;

  // qDebug("Killing app %s", pid.latin1());
  if (pid.isEmpty()) {
    // Hmm.. did the application bail before we got there?
    qDebug("AppMonitor: Tried to kill application %s but ps couldn't find it.", m_App.exec().latin1());
  }
  else {
#ifndef Q_OS_WIN32
    int success = kill(pid.toUInt(), SIGKILL);
#else
    int success = 0;
#endif
    if (success == 0) {
      m_Owner.removeTask(m_App);
    }
    else {
      qWarning("Could not kill task %s", m_App.exec().latin1());
    }
  }

  // WE DELETE OURSELVES HERE!  Don't do anything else!!
  delete this;
}

TempScreenSaverMonitor::TempScreenSaverMonitor(QObject *parent, const char *name)
    : QObject(parent, name)
{
    currentMode = QPEApplication::Enable;
    timerId = 0;
}

void TempScreenSaverMonitor::setTempMode(int mode, int pid)
{
    removeOld(pid);
    switch(mode) {
	case QPEApplication::Disable: sStatus[0].append(pid); break;
	case QPEApplication::DisableLightOff: sStatus[1].append(pid); break;
	case QPEApplication::DisableSuspend: sStatus[2].append(pid); break;
	case QPEApplication::Enable: break;
	default: qWarning("Unrecognized temp power setting.  Ignored"); return;
    }
    updateAll();
}

// Returns true if app had set a temp Mode earlier
bool TempScreenSaverMonitor::removeOld(int pid)
{
    QValueList<int>::Iterator it;
    for (int i = 0; i < 3; i++) {
	it = sStatus[i].find(pid);
	if ( it != sStatus[i].end() ) {
	    sStatus[i].remove( it );
	    return TRUE;
	}
    }
    return FALSE;
}

void TempScreenSaverMonitor::updateAll()
{
    int mode = QPEApplication::Enable;
    if ( sStatus[0].count() ) {
	mode = QPEApplication::Disable;
    } else if ( sStatus[1].count() ) {
	mode = QPEApplication::DisableLightOff;
    } else if ( sStatus[2].count() ) {
	mode = QPEApplication::DisableSuspend;
    } 

    if ( mode != currentMode ) {
#ifdef QTOPIA_MAX_SCREEN_DISABLE_TIME
	if ( currentMode == QPEApplication::Enable) {
	    int tid = timerValue();
	    if ( tid )
		timerId = startTimer( tid * 1000 );
	} else if ( mode == QPEApplication::Enable ) {
	    if ( timerId ) {
		killTimer(timerId);
		timerId = 0;
	    }
	}
#endif
	currentMode = mode;
	QCopEnvelope("QPE/System", "setScreenSaverMode(int)") << mode;
    }
}

void TempScreenSaverMonitor::applicationTerminated(int pid)
{
    if ( removeOld(pid) )
	updateAll();
}

int TempScreenSaverMonitor::timerValue()
{
    int tid = 0;
#ifdef QTOPIA_MAX_SCREEN_DISABLE_TIME
    tid = QTOPIA_MAX_SCREEN_DISABLE_TIME;

    char *env = getenv("QTOPIA_DISABLED_APM_TIMEOUT");
    if ( !env ) 
	return tid;

    QString strEnv = env;
    bool ok = FALSE;
    int envTime = strEnv.toInt(&ok);

    if ( ok ) {
	if ( envTime < 0 )
	    return 0;
	else if ( envTime <= QTOPIA_MIN_SCREEN_DISABLE_TIME )
	    return tid;
	else 
	    return envTime;
    }
#endif

    return tid;
}

void TempScreenSaverMonitor::timerEvent(QTimerEvent *t)
{
#ifdef QTOPIA_MAX_SCREEN_DISABLE_TIME
    if ( timerId && (t->timerId() == timerId) ) {
	
	/*  Clean up	*/
	killTimer(timerId);
	timerId = 0;
	currentMode = QPEApplication::Enable;
	QCopEnvelope("QPE/System", "setScreenSaverMode(int)") << currentMode;
	
	// signal starts on a merry-go-round, which ends up in Desktop::togglePower()
	emit forceSuspend();
	// if we have apm we are asleep at this point, next line will be executed when we
	// awake from suspend.
	if ( QFile::exists( "/proc/apm" ) ) {
	    QTime t;
	    t = t.addSecs( timerValue() );
	    QString str =  tr("<qt>The running applications disabled the screen saver for more than the allowed time (%1).<p>The system was forced to suspend</p></qt>").arg( t.toString() );
	    QMessageBox::information(0, tr("Forced suspend"), str);
	}
	
	// Reset all requests.
	for (int i = 0; i < 3; i++)
	    sStatus[i].clear();

	updateAll();
    }
#endif
}

