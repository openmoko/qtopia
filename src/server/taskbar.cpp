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

#include "startmenu.h"
#include "inputmethods.h"
#include "mrulist.h"
#include "runningappbar.h"
#include "systray.h"
#include "calibrate.h"
#include "wait.h"
#include "appicons.h"

#include "taskbar.h"
#include "desktop.h"

#include <qtopia/qpeapplication.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/global.h>
#include <qtopia/custom.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>
#ifdef QWS
#include <qwindowsystem_qws.h>
#endif
#include <qwidgetstack.h>

#if defined( Q_WS_QWS )
#include <qwsdisplay_qws.h>
#include <qgfx_qws.h>
#endif


#define FACTORY(T) \
    static QWidget *new##T( bool maximized ) { \
	QWidget *w = new T( 0, "test", QWidget::WDestructiveClose | QWidget::WGroupLeader ); \
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
        { "calibrate",          TaskBar::calibrate,	1, 0 }, // No tr
#endif
#if !defined(QT_QWS_CASSIOPEIA)
	{ "shutdown",           Global::shutdown,		1, 0 }, // No tr
//	{ "run",                run,			1, 0 }, // No tr
#endif

	{ 0,            TaskBar::calibrate,	0, 0 },
};

static bool initNumLock()
{
#ifdef QPE_INITIAL_NUMLOCK_STATE
    QPE_INITIAL_NUMLOCK_STATE
#endif
    return FALSE;
}

class LockKeyState : public QWidget
{
public:
    LockKeyState( QWidget *parent ) :
	QWidget(parent),
	nl(initNumLock()), cl(FALSE)
    {
	nl_pm = Resource::loadPixmap("numlock");
	cl_pm = Resource::loadPixmap("capslock");
    }
    QSize sizeHint() const
    {
	return QSize(nl_pm.width()+2,nl_pm.width()+nl_pm.height()+1);
    }
    void toggleNumLockState()
    {
	nl = !nl; repaint();
    }
    void toggleCapsLockState()
    {
	cl = !cl; repaint();
    }
    void paintEvent( QPaintEvent * )
    {
	int y = (height()-sizeHint().height())/2;
	QPainter p(this);
	if ( nl )
	    p.drawPixmap(1,y,nl_pm);
	if ( cl )
	    p.drawPixmap(1,y+nl_pm.height()+1,cl_pm);
    }
private:
    QPixmap nl_pm, cl_pm;
    bool nl, cl;
};

TaskBar::~TaskBar()
{
}


TaskBar::TaskBar() : QHBox(0, 0, WStyle_Customize | WStyle_Tool | WStyle_StaysOnTop | WGroupLeader)
{
    Global::setBuiltinCommands(builtins);

    sm = new StartMenu( this );

    inputMethods = new InputMethods( this );
    connect( inputMethods, SIGNAL(inputToggled(bool)),
	     this, SLOT(calcMaxWindowRect()) );
    //new QuickLauncher( this );
    
    stack = new QWidgetStack( this );
    stack->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum ) );
    label = new QLabel(stack);

//     mru = new MRUList( stack );
//     stack->raiseWidget( mru );
    runningAppBar = new RunningAppBar(stack);
    stack->raiseWidget(runningAppBar);

    waitIcon = new Wait( this );
    (void) new AppIcons( this );

    sysTray = new SysTray( this );

    // ## make customizable in some way?
#ifdef QT_QWS_CUSTOM
    lockState = new LockKeyState( this );
#else
    lockState = 0;
#endif

#if defined(Q_WS_QWS)
#if !defined(QT_NO_COP)
    QCopChannel *channel = new QCopChannel( "QPE/TaskBar", this );
    connect( channel, SIGNAL(received(const QCString&, const QByteArray&)),
	this, SLOT(receive(const QCString&, const QByteArray&)) );
#endif
#endif
    waitTimer = new QTimer( this );
    connect( waitTimer, SIGNAL( timeout() ), this, SLOT( stopWait() ) );
    clearer = new QTimer( this );
    QObject::connect(clearer, SIGNAL(timeout()), SLOT(clearStatusBar()));
    QObject::connect(clearer, SIGNAL(timeout()), sysTray, SLOT(show()));
}

void TaskBar::setStatusMessage( const QString &text )
{
    if ( !text.isEmpty() ) {
	label->setText( text );
	stack->raiseWidget( label );
	if ( sysTray && ( label->fontMetrics().width( text ) > label->width() ) )
	    sysTray->hide();
	clearer->start( 3000, TRUE );
    } else {
	clearStatusBar();
    }
}

void TaskBar::clearStatusBar()
{
    label->clear();
    stack->raiseWidget(runningAppBar);
    //     stack->raiseWidget( mru );
}

void TaskBar::startWait()
{
    waitIcon->setWaiting( true );
    // a catchall stop after 10 seconds...
    waitTimer->start( 10 * 1000, true );
}

void TaskBar::stopWait(const QString&)
{
    waitTimer->stop();
//     mru->addTask(sm->execToLink(app));
    waitIcon->setWaiting( false );
}

void TaskBar::stopWait()
{
    waitTimer->stop();
    waitIcon->setWaiting( false );
}

void TaskBar::resizeEvent( QResizeEvent *e )
{
    bool imv = inputMethods->inputRect().isValid();
    if ( imv )
	inputMethods->hideInputMethod();
    QHBox::resizeEvent( e );
    calcMaxWindowRect();
    if ( imv )
	inputMethods->showInputMethod();
}

void TaskBar::styleChange( QStyle &s )
{
    QHBox::styleChange( s );
    calcMaxWindowRect();
}

void TaskBar::calcMaxWindowRect()
{
#ifdef Q_WS_QWS
    QRect wr;
    int displayWidth  = qApp->desktop()->width();
    QRect ir = inputMethods->inputRect();
    if ( ir.isValid() ) {
	wr.setCoords( 0, 0, displayWidth-1, ir.top()-1 );
    } else {
	wr.setCoords( 0, 0, displayWidth-1, y()-1 );
    }

#if QT_VERSION < 300
    QWSServer::setMaxWindowRect( qt_screen->mapToDevice(wr,
	QSize(qt_screen->width(),qt_screen->height()))
	);
#else
    QWSServer::setMaxWindowRect( wr );
#endif
#endif
}

void TaskBar::receive( const QCString &msg, const QByteArray &data )
{
    QDataStream stream( data, IO_ReadOnly );
    if ( msg == "message(QString)" ) {
        QString text;
        stream >> text;
        setStatusMessage( text );
    } else if ( msg == "hideInputMethod()" ) {
	inputMethods->hideInputMethod();
    } else if ( msg == "showInputMethod()" ) {
	inputMethods->showInputMethod();
    } else if ( msg == "showInputMethod(QString)" ) {
        QString name;
        stream >> name;
	inputMethods->showInputMethod(name);
    } else if ( msg == "reloadInputMethods()" ) {
	inputMethods->loadInputMethods();
    } else if ( msg == "reloadApps()" ) {
	sm->reloadApps();
    } else if ( msg == "reloadApplets()" ) {
	sysTray->clearApplets();
	sm->createMenu();
	sysTray->addApplets();
    } else if ( msg == "soundAlarm()" ) {
	Desktop::soundAlarm();
    }
#ifdef CUSTOM_LEDS    
    else if ( msg == "setLed(int,bool)" ) {
	int led, status;
	stream >> led >> status;
	CUSTOM_LEDS( led, status );
    }
#endif    
}

QWidget *TaskBar::calibrate(bool)
{
#ifdef Q_WS_QWS
    Calibrate *c = new Calibrate;
    c->show();
    return c;
#else
    return 0;
#endif
}

void TaskBar::toggleNumLockState()
{
    if ( lockState ) lockState->toggleNumLockState();
}

void TaskBar::toggleCapsLockState()
{
    if ( lockState ) lockState->toggleCapsLockState();
}

void TaskBar::toggleSymbolInput()
{
    QString unicodeInput = qApp->translate( "InputMethods", "Unicode" );
    if ( inputMethods->currentShown() == unicodeInput ) {
	inputMethods->hideInputMethod();
    } else {
	inputMethods->showInputMethod( unicodeInput );
    }
}

bool TaskBar::recoverMemory()
{
//     return mru->quitOldApps();
  return true;
}

