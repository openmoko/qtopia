/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "minesweep.h"
#include "minefield.h"
//#include "mineframe.h"

#include <qtopiaapplication.h>
#include <QSettings>

#include <QMenu>
#include <QScrollArea>

#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#include <QAction>
#include <QPushButton>
#else
#include <QToolbar>
#include <QMenuBar>
#include <QLCDnumber>
#include <QPushButton>
#endif
#include <qmessagebox.h>
#ifndef QTOPIA_PHONE
#include <qtimer.h>
#endif
#include <qpalette.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qdesktopwidget.h>

#include <stdlib.h>
#include <time.h>

class ResultIndicator : private QLabel
{
public:
    static void showResult( QWidget *ref, bool won );
    static ResultIndicator* getInstance();
private:
    ResultIndicator( QWidget *parent, Qt::WFlags f)
        :QLabel( parent, f ) {}

    static ResultIndicator* pInstance;

    void timerEvent( QTimerEvent *);
    void center();
    bool twoStage;
    int timerId;
};

ResultIndicator* ResultIndicator::pInstance = 0;

ResultIndicator* ResultIndicator::getInstance()
{
    return ResultIndicator::pInstance;
}

void ResultIndicator::showResult( QWidget *ref, bool won )
{
    if (pInstance != 0)
        delete pInstance;
    pInstance = new ResultIndicator( ref,Qt::Window | Qt::Tool );

    pInstance->setAlignment( Qt::AlignCenter );
    pInstance->setFrameStyle( Sunken|StyledPanel );
    if ( won ) {
        pInstance->setText( MineSweep::tr("You won!") );
        pInstance->center();
        pInstance->show();
        pInstance->twoStage = false;
        pInstance->timerId = pInstance->startTimer(1500);
    } else {
        QPalette p( Qt::red );
        pInstance->setPalette( p );
        pInstance->setText( MineSweep::tr("You exploded!") );
        pInstance->resize( ref->size() );
        pInstance->move( ref->mapToGlobal(QPoint(0,0)) );
        pInstance->show();
        pInstance->twoStage = true;
        pInstance->timerId =pInstance->startTimer(200);
    }
}

void ResultIndicator::center()
{
    QWidget *w = parentWidget();

    QPoint pp = w->mapToGlobal( QPoint(0,0) );
    QSize s = sizeHint()*3.0;
    s.setWidth( qMin(s.width(), w->width()) );
    pp = QPoint( pp.x() + w->width()/2 - s.width()/2,
                pp.y() + w->height()/ 2 - s.height()/2 );

    setGeometry( QRect(pp, s) );

}

void ResultIndicator::timerEvent( QTimerEvent *te )
{
    if ( te->timerId() != timerId )
        return;
    killTimer( timerId );
    if ( twoStage ) {
        center();
        twoStage = false;
        timerId = startTimer( 1000 );
    } else {
        pInstance = 0;
        delete this;
    }
}


MineSweep::MineSweep( QWidget* parent, Qt::WFlags f )
: QMainWindow( parent, f )
{
    QtopiaApplication::setInputMethodHint(this,QtopiaApplication::AlwaysOff);
    srand(::time(0));
    setWindowTitle( tr("Mine Hunt") );
    setWindowIcon( QPixmap( ":image/MineHunt" ) );

#ifndef QTOPIA_PHONE
    mWorriedPM = QPixmap( ":image/worried" );
    mNewPM = QPixmap( ":image/new" );
    mHappyPM = QPixmap( ":image/happy" );
    mDeadPM = QPixmap( ":image/dead" );

    QToolBar *toolBar = new QToolBar( this );

    QMenuBar *menuBar = new QMenuBar( toolBar );

    QMenu *gameMenu = new QMenu( this );
    gameMenu->insertItem( tr("Beginner"), this, SLOT( beginner() ) );
    gameMenu->insertItem( tr("Advanced"), this, SLOT( advanced() ) );

    QDesktopWidget *desktop = QApplication::desktop();
    if (desktop->screenGeometry(desktop->primaryScreen()).width() >= 240) {
        gameMenu->insertItem( tr("Expert"), this, SLOT( expert() ) );
    }

    menuBar->insertItem( tr("Game"), gameMenu );

    guessLCD = new QLCDNumber( toolBar );
    toolBar->addWidget( guessLCD );

    QPalette lcdPal( Qt::red );
    lcdPal.setColor( QColorGroup::Background, QApplication::palette().active().background() );
    lcdPal.setColor( QColorGroup::Button, QApplication::palette().active().button() );

//    guessLCD->setPalette( lcdPal );
    guessLCD->setSegmentStyle( QLCDNumber::Flat );
    guessLCD->setFrameStyle( QFrame::NoFrame );
    guessLCD->setNumDigits( 2 );
    guessLCD->setBackgroundMode( Qt::PaletteButton );
    newGameButton = new QPushButton( toolBar );
    newGameButton->setPixmap( mNewPM );
    newGameButton->setFocusPolicy(Qt::NoFocus);
    connect( newGameButton, SIGNAL(clicked()), this, SLOT(newGame()) );

    timeLCD = new QLCDNumber( toolBar );
//    timeLCD->setPalette( lcdPal );
    timeLCD->setSegmentStyle( QLCDNumber::Flat );
    timeLCD->setFrameStyle( QFrame::NoFrame );
    timeLCD->setNumDigits( 5 ); // "mm:ss"
    timeLCD->setBackgroundMode( Qt::PaletteButton );

    toolBar->setMovable ( false );

    addToolBar( toolBar );
#endif


    scroll = new QScrollArea(this);
    setCentralWidget(scroll);

    layoutHolder = new QWidget();
    scroll->setWidget(layoutHolder);
    layout = new QHBoxLayout();
    field = new MineField();
    layout->setMargin(0);
    layout->addWidget(field);
    layoutHolder->setLayout(layout);
    layoutHolder->setFocusProxy(field);
    layoutHolder->setBackgroundRole ( QPalette::Dark );
    QSize layoutSize = layout->sizeHint();

    layout->setAlignment(Qt::AlignCenter);
//    if(!Qtopia::mousePreferred())
        scroll->setFocusPolicy ( Qt::NoFocus );

    scroll->setWidgetResizable(true);

    field->setFocus();

#ifdef QTOPIA_PHONE
    connect( field, SIGNAL(newGameSelected()), this, SLOT(newGame()) );
    QSoftMenuBar::setLabel((QWidget*)field, Qt::Key_Select, QSoftMenuBar::Select, QSoftMenuBar::AnyFocus);
#endif
    QFont fnt = field->font();
    fnt.setBold( true );
    field->setFont( QFont( fnt ) );

    connect( field, SIGNAL( gameOver(bool) ), this, SLOT( gameOver(bool) ) );
    connect( field, SIGNAL( mineCount(int) ), this, SLOT( setCounter(int) ) );
    connect( field, SIGNAL( gameStarted()), this, SLOT( startPlaying() ) );

    connect( field, SIGNAL( currentPointChanged(QPoint)), this, SLOT(showPoint(QPoint)));

#ifdef QTOPIA_PHONE
    QMenu *contextMenu = QSoftMenuBar::menuFor(this);

    contextMenu->addAction( QIcon(":image/dead"), tr( "Expert" ), this, SLOT(expert()) );
    contextMenu->addAction( QIcon(":image/worried"), tr( "Advanced" ), this, SLOT(advanced()) );
    contextMenu->addAction( QIcon(":image/happy") , tr( "Beginner" ), this, SLOT(beginner()) );
#endif

#ifndef QTOPIA_PHONE
    timer = new QTimer( this );
    connect( timer, SIGNAL( timeout() ), this, SLOT( updateTime() ) );
#endif

    readConfig();
}

MineSweep::~MineSweep()
{
}

void MineSweep::closeEvent(QCloseEvent *e) {
    ResultIndicator* r = ResultIndicator::getInstance();
    if (r) {
        delete r;
    }
    writeConfig();
    QMainWindow::closeEvent(e);
}

void MineSweep::gameOver( bool won )
{
    field->showMines();
    if ( won ) {
#ifndef QTOPIA_PHONE
        newGameButton->setPixmap( mHappyPM );
#else
        //QSoftMenuBar::setLabel( field, Qt::Key_Select, "happy", QString(), QSoftMenuBar::AnyFocus); //needs 16x16 icons
#endif
    } else {
#ifndef QTOPIA_PHONE
        newGameButton->setPixmap( mDeadPM );
#else
//        QSoftMenuBar::setLabel((QWidget*)field, Qt::Key_Select, QSoftMenuBar::Select, QSoftMenuBar::AnyFocus); // using minesweep/dead would be better here, if we had the 16x16 icon
#endif
    }
    ResultIndicator::showResult( this, won );
#ifndef QTOPIA_PHONE
    timer->stop();
#endif
}

void MineSweep::newGame()
{
    newGame(field->level());
}

void MineSweep::newGame(int level)
{
#ifndef QTOPIA_PHONE
    timeLCD->display( "0:00" );
#endif
    field->setup( level );
#ifndef QTOPIA_PHONE
    newGameButton->setPixmap( mNewPM );
    timer->stop();
#else
    // using minesweep icons would be better here, if we had the 16x16 icon
    //QSoftMenuBar::setLabel( field, Qt::Key_Select, "minesweep/happy", QString() );
    QSoftMenuBar::setLabel((QWidget*)field, Qt::Key_Select, QSoftMenuBar::Select, QSoftMenuBar::AnyFocus);
#endif
}

void MineSweep::startPlaying()
{
#ifndef QTOPIA_PHONE
    newGameButton->setPixmap( mWorriedPM );
    starttime = QDateTime::currentDateTime();
    timer->start( 1000 );
#else
        //QSoftMenuBar::setLabel( field, Qt::Key_Select, "minesweep/worried", QString() ); // This needs 16x16 icons
#endif
}

void MineSweep::beginner()
{
    newGame(1);
}

void MineSweep::advanced()
{
    newGame(2);
}

void MineSweep::expert()
{
    newGame(3);
}

void MineSweep::setCounter( int c )
{
#ifndef QTOPIA_PHONE
    if ( !guessLCD )
        return;

    guessLCD->display( c );
#else
//suppress warning
    Q_UNUSED(c);
#endif
}

void MineSweep::updateTime()
{
#ifndef QTOPIA_PHONE
    if ( !timeLCD )
        return;

    int s = starttime.secsTo(QDateTime::currentDateTime());
    if ( s/60 > 99 )
        timeLCD->display( "-----" );
    else
        timeLCD->display( QString().sprintf("%2d:%02d",s/60,s%60) );
#endif
}

void MineSweep::writeConfig() const
{
    QSettings cfg("Trolltech","MineSweep");
    cfg.beginGroup("Panel");
#ifndef QTOPIA_PHONE
    cfg.setValue("Time",
            timer->isActive() ? starttime.secsTo(QDateTime::currentDateTime()) : -1);
#endif
    field->writeConfig(cfg);
}

void MineSweep::readConfig()
{
    QSettings cfg("Trolltech","MineSweep");
    field->readConfig(cfg);
    cfg.beginGroup("Panel");
    int s = cfg.value("Time",-1).toInt();
    if ( s<0 ) {
        newGame();
    } else {
        startPlaying();
#ifndef QTOPIA_PHONE
        starttime = QDateTime::currentDateTime().addSecs(-s);
#endif
        updateTime();
    }
}

void MineSweep::showPoint(QPoint point){
    // 16/1/01:  At time of writing the qt code for this ensureVisible()
    // is bugged, resulting in incorrect scrolling in this program.
    // Fix has been proposed.
    scroll->ensureVisible(point.x(), point.y());
}

void MineSweep::resizeEvent( QResizeEvent *e ) {
    QMainWindow::resizeEvent(e);
    QRect scrollRect=(scroll->geometry());
    field->setAvailableRect( scrollRect);
}
