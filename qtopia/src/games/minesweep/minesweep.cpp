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

#include "minesweep.h"
#include "minefield.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>
#include <qtopia/config.h>

#ifdef QTOPIA_PHONE
#include <qtopia/contextmenu.h>
#include <qtopia/contextbar.h>
#include <qaction.h>
#else
#include <qtopia/qpetoolbar.h>
#include <qtopia/qpemenubar.h>
#include <qpopupmenu.h>
#include <qlcdnumber.h>
#include <qpushbutton.h>
#endif
#include <qmessagebox.h>
#ifndef QTOPIA_PHONE
#include <qtimer.h>
#endif
#include <qpalette.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qlabel.h>

#include <stdlib.h>
#include <time.h>

class ResultIndicator : private QLabel
{
public:   
    static void showResult( QWidget *ref, bool won );
    static ResultIndicator* getInstance();
private:    
    ResultIndicator( QWidget *parent, const char *name, WFlags f)
	:QLabel( parent, name, f ) {}
    
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
    pInstance = new ResultIndicator( ref, 0, WStyle_Customize | WStyle_Tool | WType_TopLevel );
    
    pInstance->setAlignment( AlignCenter );
    pInstance->setFrameStyle( Sunken|StyledPanel );
    if ( won ) {
	pInstance->setText( MineSweep::tr("You won!") );
	pInstance->center();
	pInstance->show();
	pInstance->twoStage = FALSE;
	pInstance->timerId = pInstance->startTimer(1500);
    } else {
	QPalette p( red );
	pInstance->setPalette( p );
	pInstance->setText( MineSweep::tr("You exploded!") );
	pInstance->resize( ref->size() );
	pInstance->move( ref->mapToGlobal(QPoint(0,0)) );
	pInstance->show();
	pInstance->twoStage = TRUE;
	pInstance->timerId =pInstance->startTimer(200);
    }
}

void ResultIndicator::center()
{
    QWidget *w = parentWidget();

    QPoint pp = w->mapToGlobal( QPoint(0,0) ); 
    QSize s = sizeHint()*3;
    s.setWidth( QMIN(s.width(), w->width()) );
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
	twoStage = FALSE;
	timerId = startTimer( 1000 );
    } else {
        pInstance = 0;
	delete this;
    }
}


class MineFrame : public QFrame
{
public:
    MineFrame( QWidget *parent, const char *name = 0 )
	:QFrame( parent, name ), field(0) {
	setFrameStyle(NoFrame);
        //setBackgroundMode(NoBackground);
    }
    void setField( MineField *f ) {
	field = f;
	setMinimumSize( field->sizeHint() );
    }
protected:
    void resizeEvent( QResizeEvent *e ) {
	field->setAvailableRect( contentsRect());
	QFrame::resizeEvent(e); 
    }
    
private:
    MineField *field;
};



MineSweep::MineSweep( QWidget* parent, const char* name, WFlags f )
: QMainWindow( parent, name, f )
{
    QPEApplication::setInputMethodHint(this,QPEApplication::AlwaysOff);
    srand(::time(0));
    setCaption( tr("Mine Hunt") );
    setIcon( Resource::loadPixmap( "MineHunt" ) );

#ifndef QTOPIA_PHONE
    QPEToolBar *toolBar = new QPEToolBar( this );
    toolBar->setHorizontalStretchable( TRUE );

    QPEMenuBar *menuBar = new QPEMenuBar( toolBar );

    QPopupMenu *gameMenu = new QPopupMenu( this );
    gameMenu->insertItem( tr("Beginner"), this, SLOT( beginner() ) );
    gameMenu->insertItem( tr("Advanced"), this, SLOT( advanced() ) );

    if (qApp->desktop()->width() >= 240) {
	gameMenu->insertItem( tr("Expert"), this, SLOT( expert() ) );
    }

    menuBar->insertItem( tr("Game"), gameMenu );
    
    guessLCD = new QLCDNumber( toolBar );
    toolBar->setStretchableWidget( guessLCD );

    QPalette lcdPal( red );
    lcdPal.setColor( QColorGroup::Background, QApplication::palette().active().background() );
    lcdPal.setColor( QColorGroup::Button, QApplication::palette().active().button() );
    
//    guessLCD->setPalette( lcdPal );
    guessLCD->setSegmentStyle( QLCDNumber::Flat );
    guessLCD->setFrameStyle( QFrame::NoFrame );
    guessLCD->setNumDigits( 2 );
    guessLCD->setBackgroundMode( PaletteButton );
    newGameButton = new QPushButton( toolBar );
    newGameButton->setPixmap( mNewPM );
    newGameButton->setFocusPolicy(QWidget::NoFocus);
    connect( newGameButton, SIGNAL(clicked()), this, SLOT(newGame()) );
    
    timeLCD = new QLCDNumber( toolBar );
//    timeLCD->setPalette( lcdPal );
    timeLCD->setSegmentStyle( QLCDNumber::Flat );
    timeLCD->setFrameStyle( QFrame::NoFrame );
    timeLCD->setNumDigits( 5 ); // "mm:ss"
    timeLCD->setBackgroundMode( PaletteButton );
    
    setToolBarsMovable ( FALSE );

    addToolBar( toolBar );
#endif

    MineFrame *mainframe = new MineFrame( this );

    field = new MineField( mainframe );
#ifdef QTOPIA_PHONE
    connect( field, SIGNAL(newGameSelected()), this, SLOT(newGame()) );
#endif
    mainframe->setField( field );

    QFont fnt = field->font();
    fnt.setBold( TRUE );
    field->setFont( QFont( fnt ) );
    field->setFocus();
    setCentralWidget( mainframe );
    
    connect( field, SIGNAL( gameOver(bool) ), this, SLOT( gameOver(bool) ) );
    connect( field, SIGNAL( mineCount(int) ), this, SLOT( setCounter(int) ) );
    connect( field, SIGNAL( gameStarted()), this, SLOT( startPlaying() ) );

#ifdef QTOPIA_PHONE
    ContextMenu *contextMenu = new ContextMenu(field);

    QAction *beginnerAction = new QAction( tr( "Beginner" ), QString::null, 0, this, 0 );
    QAction *advancedAction = new QAction( tr( "Advanced" ), QString::null, 0, this, 0 );
    connect( beginnerAction, SIGNAL(activated()), this, SLOT(beginner()) );
    connect( advancedAction, SIGNAL(activated()), this, SLOT(advanced()) );
    beginnerAction->addTo( contextMenu );
    advancedAction->addTo( contextMenu );
#endif

#ifndef QTOPIA_PHONE
    timer = new QTimer( this );
    connect( timer, SIGNAL( timeout() ), this, SLOT( updateTime() ) );
#endif

    mWorriedPM = Resource::loadPixmap( "worried" );
    mNewPM = Resource::loadPixmap( "new" );
    mHappyPM = Resource::loadPixmap( "happy" );
    mDeadPM = Resource::loadPixmap( "dead" );

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
	ContextBar::setLabel( field, Qt::Key_Select, "happy", QString::null );
#endif
    } else {
#ifndef QTOPIA_PHONE
	newGameButton->setPixmap( mDeadPM );
#else
	ContextBar::setLabel( field, Qt::Key_Select, "dead", QString::null );
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
	ContextBar::setLabel( field, Qt::Key_Select, "new", QString::null );
#endif
}

void MineSweep::startPlaying()
{
#ifndef QTOPIA_PHONE
    newGameButton->setPixmap( mWorriedPM );
    starttime = QDateTime::currentDateTime();
    timer->start( 1000 );
#else
	ContextBar::setLabel( field, Qt::Key_Select, "worried", QString::null );
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
    Q_UNUSED(c)
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
    Config cfg("MineSweep");
    cfg.setGroup("Panel");
#ifndef QTOPIA_PHONE
    cfg.writeEntry("Time",
	    timer->isActive() ? starttime.secsTo(QDateTime::currentDateTime()) : -1);
#endif
    field->writeConfig(cfg);
}

void MineSweep::readConfig()
{
    Config cfg("MineSweep");
    field->readConfig(cfg);
    cfg.setGroup("Panel");
    int s = cfg.readNumEntry("Time",-1);
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
