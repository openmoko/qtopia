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

#include "startmenu.h"
#include "inputmethods.h"
#include "runningappbar.h"
#include "systray.h"
#include "wait.h"
#include "appicons.h"

#include "taskbar.h"
#include "server.h"

#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>

#include <custom.h>
#include <qpluginmanager.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif
#include <QStackedWidget>
#include <QDesktopWidget>
#include <QMenu>

#if defined( Q_WS_QWS )
#include <qwsdisplay_qws.h>
#endif


static bool initNumLock()
{
#ifdef QPE_INITIAL_NUMLOCK_STATE
    QPE_INITIAL_NUMLOCK_STATE
#endif
    return false;
}

//---------------------------------------------------------------------------

class SafeMode : public QWidget
{
    Q_OBJECT
public:
    SafeMode( QWidget *parent ) : QWidget( parent ), menu(0)
    {
        message = tr("Safe Mode");
        QFont f( font() );
        f.setWeight( QFont::Bold );
        setFont( f );
    }

    void mousePressEvent( QMouseEvent *);
    QSize sizeHint() const;
    void paintEvent( QPaintEvent* );

private slots:
    void action(QAction *);

private:
    QString message;
    QMenu *menu;
    QAction *managerAction;
    QAction *restartAction;
    QAction *helpAction;
};

void SafeMode::mousePressEvent( QMouseEvent *)
{
    if ( !menu ) {
        menu = new QMenu(this);
        managerAction = menu->addAction( tr("Plugin Manager...") );
        restartAction = menu->addAction( tr("Restart Qtopia") );
        helpAction = menu->addAction( tr("Help...") );
        connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(action(QAction*)));
    }
    QPoint curPos = mapToGlobal( QPoint(0,0) );
    QSize sh = menu->sizeHint();
    menu->popup( curPos-QPoint((sh.width()-width())/2,sh.height()) );
}

void SafeMode::action(QAction *action)
{
    if (action == managerAction)
        Qtopia::execute( "pluginmanager" );
    else if (action == restartAction)
        QtopiaChannel::send( "QPE/System", "restart()" );
    else if (action == helpAction)
        Qtopia::execute( "helpbrowser", "safemode.html" );
}

QSize SafeMode::sizeHint() const
{
    QFontMetrics fm = fontMetrics();

    return QSize( fm.width(message), fm.height() );
}

void SafeMode::paintEvent( QPaintEvent* )
{
    QPainter p(this);
    p.drawText( rect(), Qt::AlignCenter, message );
}

//---------------------------------------------------------------------------

class LockKeyState : public QWidget
{
public:
    LockKeyState( QWidget *parent ) :
        QWidget(parent),
        nl(initNumLock()), cl(false)
    {
        nl_pm = QPixmap(":image/numlock");
        cl_pm = QPixmap(":image/capslock");
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

//---------------------------------------------------------------------------

TaskBar::~TaskBar()
{
}


TaskBar::TaskBar() :
    QWidget(0, Qt::Tool | Qt::WindowStaysOnTopHint)
{
    setAttribute(Qt::WA_GroupLeader);
    QHBoxLayout *hbl = new QHBoxLayout(this);
    hbl->setMargin(0);
    hbl->setSpacing(0);

    QPalette::ColorRole mode = QPalette::Button;
    setBackgroundRole(mode);

    sm = StartMenu::create(this);
    hbl->addWidget(sm);
    sm->setBackgroundRole(mode);
    connect( sm, SIGNAL(tabSelected(const QString&)), this,
            SIGNAL(tabSelected(const QString&)) );

    inputMethods = new InputMethods( this );
    hbl->addWidget(inputMethods);
    inputMethods->setBackgroundRole(mode);
    connect( inputMethods, SIGNAL(inputToggled(bool)),
             this, SLOT(calcMaxWindowRect()) );

    stack = new QStackedWidget( this );
    hbl->addWidget(stack);
    stack->setBackgroundRole(mode);
    stack->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum ) );
    label = new QLabel(stack);
    label->setBackgroundRole(mode);
    stack->addWidget(label);

    runningAppBar = new RunningAppBar(stack);
    runningAppBar->setBackgroundRole(mode);
    stack->addWidget(runningAppBar);
    stack->setCurrentIndex(stack->indexOf(runningAppBar));

    waitIcon = new Wait( this );
    hbl->addWidget(waitIcon);
    waitIcon->setBackgroundRole(mode);
    AppIcons *appIcons = new AppIcons( this );
    hbl->addWidget(appIcons);
    appIcons->setBackgroundRole(mode);

    sysTray = new SysTray( this );
    hbl->addWidget(sysTray);
    sysTray->setBackgroundRole(mode);

    if (QPluginManager::inSafeMode()) {
        SafeMode *safeMode = new SafeMode( this );
        hbl->addWidget(safeMode);
        safeMode->setBackgroundRole(mode);
    }

    // ## make customizable in some way?
#ifdef QT_QWS_SL5XXX
    lockState = new LockKeyState( this );
    hbl->addWidget(lockState);
    lockState->setBackgroundRole(mode);
#else
    lockState = 0;
#endif

    QtopiaChannel *channel = new QtopiaChannel( "QPE/TaskBar", this );
    connect( channel, SIGNAL(received(const QString&,const QByteArray&)),
        this, SLOT(receive(const QString&,const QByteArray&)) );

    waitTimer = new QTimer( this );
    waitTimer->setSingleShot(true);
    connect( waitTimer, SIGNAL( timeout() ), this, SLOT( stopWait() ) );
    clearer = new QTimer( this );
    clearer->setSingleShot(true);
    QObject::connect(clearer, SIGNAL(timeout()), SLOT(clearStatusBar()));

    connect( qApp, SIGNAL(symbol()), this, SLOT(toggleSymbolInput()) );
    connect( qApp, SIGNAL(numLockStateToggle()), this, SLOT(toggleNumLockState()) );
    connect( qApp, SIGNAL(capsLockStateToggle()), this, SLOT(toggleCapsLockState()) );

    ApplicationLauncher *l = qtopiaTask<ApplicationLauncher>();
    if(l) {
        QObject::connect(l, SIGNAL(applicationStateChanged(const QString &,ApplicationTypeLauncher::ApplicationState)), this, SLOT(applicationStateChanged(const QString &,ApplicationTypeLauncher::ApplicationState)));
    }
}

void TaskBar::setStatusMessage( const QString &text )
{
    if ( !text.isEmpty() ) {
        label->setText( text );
        if ( sysTray && ( label->fontMetrics().width( text ) > stack->width() ) )
            sysTray->hide();
        stack->setCurrentIndex(stack->indexOf(label));
        clearer->start(3000);
    } else {
        clearStatusBar();
    }
}

void TaskBar::clearStatusBar()
{
    label->clear();
    stack->setCurrentIndex(stack->indexOf(runningAppBar));
    if ( sysTray )
        sysTray->show();
    //     stack->raiseWidget( mru );
}

void TaskBar::startWait()
{
    waitIcon->setWaiting( true );
    // a catchall stop after 10 seconds...
    waitTimer->start(10 * 1000);
}

void TaskBar::stopWait(const QString&)
{
    waitTimer->stop();
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
    QWidget::resizeEvent( e );
    calcMaxWindowRect();
    if ( imv )
        inputMethods->showInputMethod();
}

void TaskBar::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    if (e->type() == QEvent::StyleChange)
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

    QWSServer::setMaxWindowRect( wr );
#endif
}

void TaskBar::receive( const QString &msg, const QByteArray &data )
{
    QDataStream stream( data );
    if ( msg == "message(QString)" ) {
        QString text;
        stream >> text;
        setStatusMessage( text );
    } else if ( msg == "reloadApplets()" ) {
        sysTray->clearApplets();
        sm->createMenu();
        sysTray->addApplets();
    }
}

void TaskBar::applicationStateChanged(const QString &name,
        ApplicationTypeLauncher::ApplicationState state)
{
    if(state == ApplicationTypeLauncher::Starting) {
        runningAppBar->applicationLaunched( name );
    } else if(state == ApplicationTypeLauncher::NotRunning) {
        runningAppBar->applicationTerminated( name );
    }
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

#include "taskbar.moc"
