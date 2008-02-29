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

/*********************************************************************
** Asteroids(R) is a registered trademark of Atari Corporation.  Please
** note that the Asteroids(R) games included in Qtopia was not developed
** by nor is endorsed by Atari Corporation. Trolltech respectfully
** acknowledges Atari Corporation's ownership of the trademark."
**********************************************************************/

/*
 * KAsteroids - Copyright (c) Martin R. Jones 1997
 *
 * Part of the KDE project
 */
//	--- toplevel.cpp ---

#include "toplevel.h"
#include "ledmeter.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>
#include <qtopia/devicebuttonmanager.h>
#include <qtopia/contextbar.h>

#include <qaccel.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlcdnumber.h>
#include <qpushbutton.h>


#define SB_SCORE	1
#define SB_LEVEL	2
#define SB_SHIPS	3

struct SLevel
{
    int    nrocks;
    double rockSpeed;
};

#define MAX_LEVELS	16

#ifndef QTOPIA_PHONE
SLevel levels[MAX_LEVELS] =
{
    { 1, 0.4 },
    { 1, 0.6 },
    { 2, 0.5 },
    { 2, 0.7 },
    { 2, 0.8 },
    { 3, 0.6 },
    { 3, 0.7 },
    { 3, 0.8 },
    { 4, 0.6 },
    { 4, 0.7 },
    { 4, 0.8 },
    { 5, 0.7 },
    { 5, 0.8 },
    { 5, 0.9 },
    { 5, 1.0 }
};
#else
SLevel levels[MAX_LEVELS] =
{
    { 1, 0.1 },
    { 1, 0.2 },
    { 1, 0.3 },
    { 1, 0.4 },
    { 2, 0.1 },
    { 2, 0.2 },
    { 2, 0.3 },
    { 2, 0.4 },
    { 3, 0.1 },
    { 3, 0.2 },
    { 3, 0.3 },
    { 3, 0.4 },
    { 4, 0.1 },
    { 4, 0.2 },
    { 4, 0.3 }
};
#endif

KAstTopLevel::KAstTopLevel( QWidget *parent, const char *name, WFlags fl )
    : QMainWindow( parent, name, fl ),
    shipDestroyed("qasteroids/shipdestroyed"),
    rockDestroyed("qasteroids/rockdestroyed"),
    missileFired("qasteroids/missilefired")
{
    QPEApplication::grabKeyboard();

#ifdef QTOPIA_PHONE
    contextMenu = 0;
#endif

    setCaption( tr("Asteroids") );
    QWidget *border = new QWidget( this );
    border->setBackgroundColor( black );
    setCentralWidget( border );

    QVBoxLayout *borderLayout = new QVBoxLayout( border );
    
    QWidget *mainWin = new QWidget( border );
    borderLayout->addWidget( mainWin, 2, AlignHCenter );

    view = new KAsteroidsView( mainWin );
    connect( view, SIGNAL( shipKilled() ), SLOT( slotShipKilled() ) );
    connect( view, SIGNAL( missileFired() ), SLOT( slotMissileFired() ) );
    connect( view, SIGNAL( rockHit(int) ), SLOT( slotRockHit(int) ) );
    connect( view, SIGNAL( rocksRemoved() ), SLOT( slotRocksRemoved() ) );
    connect( view, SIGNAL( updateVitals() ), SLOT( slotUpdateVitals() ) );

    QVBoxLayout *vb = new QVBoxLayout( mainWin );
    QHBoxLayout *hb = new QHBoxLayout;
    vb->addLayout( hb );

    QFont labelFont( QApplication::font() );
    labelFont.setPointSize( 12 );
    QColorGroup grp( darkGreen, black, QColor( 128, 128, 128 ),
	    QColor( 64, 64, 64 ), black, darkGreen, black );
    QPalette pal( grp, grp, grp );

    mainWin->setPalette( pal );

    QLabel *label;
    if (qApp->desktop()->width() > 200) {
	label = new QLabel( tr("Score"), mainWin );
	label->setFont( labelFont );
	label->setPalette( pal );
	//    label->setFixedWidth( label->sizeHint().width() );
	hb->addWidget( label );
    }

    scoreLCD = new QLCDNumber( 5, mainWin );
    scoreLCD->setFrameStyle( QFrame::NoFrame );
    scoreLCD->setSegmentStyle( QLCDNumber::Flat );
    scoreLCD->setFixedHeight( 16 );
    scoreLCD->setPalette( pal );
    hb->addWidget( scoreLCD );
    hb->addStretch( 1 );

    label = new QLabel( tr("Level"), mainWin );
    label->setFont( labelFont );
    label->setPalette( pal );
//    label->setFixedWidth( label->sizeHint().width() );
    hb->addWidget( label );

    levelLCD = new QLCDNumber( 2, mainWin );
    levelLCD->setFrameStyle( QFrame::NoFrame );
    levelLCD->setSegmentStyle( QLCDNumber::Flat );
    levelLCD->setFixedHeight( 16 );
    levelLCD->setPalette( pal );
    hb->addWidget( levelLCD );
    hb->addStretch( 1 );

#ifdef QTOPIA_PHONE
    label = new QLabel( mainWin );
    label->setPixmap( Resource::loadPixmap("ship/ship0000") );
#else
    label = new QLabel( tr("Ships"), mainWin );
#endif
    label->setFont( labelFont );
//    label->setFixedWidth( label->sizeHint().width() );
    label->setPalette( pal );
    hb->addWidget( label );

    shipsLCD = new QLCDNumber( 1, mainWin );
    shipsLCD->setFrameStyle( QFrame::NoFrame );
    shipsLCD->setSegmentStyle( QLCDNumber::Flat );
    shipsLCD->setFixedHeight( 16 );
    shipsLCD->setPalette( pal );
    hb->addWidget( shipsLCD );

//    hb->addStrut( 14 );

    vb->addWidget( view, 10 );

// -- bottom layout:
    QHBoxLayout *hbd = new QHBoxLayout(vb, 0);
    int hbdSpace = qApp->desktop()->width() > 200 ? 5 : 3;

    //QFont smallFont( "helvetica", 12 );
    hbd->addSpacing( hbdSpace );

/*
    label = new QLabel( tr( "T" ), mainWin );
    label->setFont( labelFont );
    label->setFixedWidth( label->sizeHint().width() );
    label->setPalette( pal );
    hbd->addWidget( label );

    teleportsLCD = new QLCDNumber( 1, mainWin );
    teleportsLCD->setFrameStyle( QFrame::NoFrame );
    teleportsLCD->setSegmentStyle( QLCDNumber::Flat );
    teleportsLCD->setPalette( pal );
    teleportsLCD->setFixedHeight( 18 );
    hbd->addWidget( teleportsLCD );

    hbd->addSpacing( 10 );
*/
    label = new QLabel( mainWin );
    label->setPixmap( Resource::loadPixmap("powerups/brake") );
    label->setFixedWidth( 16 );
    label->setPalette( pal );
    hbd->addWidget( label );

    brakesLCD = new QLCDNumber( 1, mainWin );
    brakesLCD->setFrameStyle( QFrame::NoFrame );
    brakesLCD->setSegmentStyle( QLCDNumber::Flat );
    brakesLCD->setPalette( pal );
    brakesLCD->setFixedHeight( 16 );
    hbd->addWidget( brakesLCD );

    hbd->addSpacing( hbdSpace );

    label = new QLabel( mainWin );
    label->setPixmap( Resource::loadPixmap("powerups/shield") );
    label->setFixedWidth( 16 );
    label->setPalette( pal );
    hbd->addWidget( label );

    shieldLCD = new QLCDNumber( 1, mainWin );
    shieldLCD->setFrameStyle( QFrame::NoFrame );
    shieldLCD->setSegmentStyle( QLCDNumber::Flat );
    shieldLCD->setPalette( pal );
    shieldLCD->setFixedHeight( 16 );
    hbd->addWidget( shieldLCD );

    hbd->addSpacing( hbdSpace );

    label = new QLabel( mainWin );
    label->setPixmap( Resource::loadPixmap("powerups/shoot") );
    label->setFixedWidth( 16 );
    label->setPalette( pal );
    hbd->addWidget( label );

    shootLCD = new QLCDNumber( 1, mainWin );
    shootLCD->setFrameStyle( QFrame::NoFrame );
    shootLCD->setSegmentStyle( QLCDNumber::Flat );
    shootLCD->setPalette( pal );
    shootLCD->setFixedHeight( 16 );
    hbd->addWidget( shootLCD );

    hbd->addStretch( 1 );

    if (qApp->desktop()->width() > 200) {
	label = new QLabel( tr( "Fuel" ), mainWin );
	label->setFont( labelFont );
	label->setFixedWidth( label->sizeHint().width() + 5 );
	label->setPalette( pal );
	hbd->addWidget( label );
    }

    powerMeter = new KALedMeter( mainWin );
    powerMeter->setFrameStyle( QFrame::Box | QFrame::Plain );
    powerMeter->setRange( MAX_POWER_LEVEL );
    powerMeter->addColorRange( 10, darkRed );
    powerMeter->addColorRange( 20, QColor(160, 96, 0) );
    powerMeter->addColorRange( 70, darkGreen );
    powerMeter->setCount( qApp->desktop()->width() > 200 ? 15 : 10 );
    powerMeter->setPalette( pal );
    powerMeter->setFixedSize( qApp->desktop()->width() > 200 ? 60 : 40, 12 );
    hbd->addWidget( powerMeter );

    shipsRemain = 3;
    showHiscores = FALSE;

    actions.insert( Qt::Key_Up, Thrust );
    actions.insert( Qt::Key_Left, RotateLeft );
    actions.insert( Qt::Key_Right, RotateRight );
    actions.insert( Qt::Key_Down, Brake );
#ifndef QTOPIA_PHONE
    actions.insert( Qt::Key_Enter, Shoot );
    actions.insert( Qt::Key_Z, Teleport );
    actions.insert( Qt::Key_Space, Shoot );
    actions.insert( Qt::Key_P, Pause );

#else
    actions.insert( Qt::Key_Select, Shoot );
    actions.insert( Qt::Key_Context1, Shield );
    
    ContextBar::setLabel(this, Key_Select, "qasteroids/ship/ship0000", tr("Launch"));
    contextMenu = new ContextMenu(this);
#endif

#ifndef QTOPIA_PHONE
    const QValueList<DeviceButton>& buttons = DeviceButtonManager::instance().buttons();
    actions.insert( buttons[0].keycode(), Launch );
    actions.insert( buttons[1].keycode(), Shield );
    launchButtonText = buttons[0].userText();
#else
    launchButtonText = tr("Select Key");
#endif

    view->showText( tr( "Press '%1'\nto start playing", "e.g. Select Key" ).
	arg(launchButtonText), yellow );

    setFocusPolicy( StrongFocus );
}

KAstTopLevel::~KAstTopLevel()
{
}

void KAstTopLevel::keyPressEvent( QKeyEvent *event )
{
    if ( event->isAutoRepeat() || !actions.contains( event->key() ) )
    {
	event->ignore();
        return;
    }

    havePress = TRUE;

    Action a = actions[ event->key() ];

    switch ( a )
    {
        case RotateLeft:
            view->rotateLeft( TRUE );
            break;

        case RotateRight:
            view->rotateRight( TRUE );
            break;

        case Thrust:
            view->thrust( TRUE );
            break;

        case Shoot:
            view->shoot( TRUE );
            break;

        case Shield:
            view->setShield( TRUE );
            break;

        case Teleport:
            view->teleport( TRUE );
            break;

        case Brake:
            view->brake( TRUE );
            break;

        default:
	    event->ignore();
            return;
    }
    event->accept();
}

void KAstTopLevel::keyReleaseEvent( QKeyEvent *event )
{
    if ( event->isAutoRepeat() || !actions.contains( event->key() ) )
    {
        event->ignore();
        return;
    }

    Action a = actions[ event->key() ];

    switch ( a )
    {
        case RotateLeft:
            view->rotateLeft( FALSE );
            break;

        case RotateRight:
            view->rotateRight( FALSE );
            break;

        case Thrust:
            view->thrust( FALSE );
            break;

        case Brake:
            view->brake( FALSE );
            break;

        case Shield:
            view->setShield( FALSE );
            break;

        case Teleport:
            view->teleport( FALSE );
            break;

        case Shoot:
#ifndef QTOPIA_PHONE
            view->shoot( FALSE );
            break;
#else
	    if ( !view->gameOver() && !waitShip ) {
		view->shoot(FALSE);
		break;
	    }
	    // rollover intended
#endif
        case Launch:
	    if ( havePress && view->gameOver() ) {
		slotNewGame();
	    } else if ( havePress && waitShip ) {
                view->newShip();
                waitShip = FALSE;
                view->hideText();
#ifdef QTOPIA_PHONE
		ContextBar::setLabel(this, Key_Select, "qasteroids/powerups/shoot", tr("Shoot") );
		updateContext1();
#endif
            } else {
                event->ignore();
		havePress = FALSE;
                return;
            }
            break;
	
	case NewGame:
	    slotNewGame();
	    break;
/*
        case Pause:
            {
                view->pause( TRUE );
                QMessageBox::information( this,
                                          tr("KAsteroids is paused"),
                                          tr("Paused") );
                view->pause( FALSE );
            }
            break;
*/
        default:
            event->ignore();
	    havePress = FALSE;
            return;
    }

    havePress = FALSE;
    event->accept();
}

void KAstTopLevel::showEvent( QShowEvent *e )
{
    QMainWindow::showEvent( e );
    view->pause( FALSE );
    setFocus();
}

void KAstTopLevel::hideEvent( QHideEvent *e )
{
    QMainWindow::hideEvent( e );
    view->pause( TRUE );
}

void KAstTopLevel::focusInEvent( QFocusEvent * )
{
    view->pause( FALSE );
    setFocus();
}

void KAstTopLevel::focusOutEvent( QFocusEvent * )
{
    view->pause( TRUE );
}

void KAstTopLevel::slotNewGame()
{
#ifdef QTOPIA_PHONE
    delete contextMenu;
    contextMenu = 0;
    ContextBar::setLabel(this, Key_Select, "qasteroids/powerups/shoot", tr("Shoot") );
#endif
    shipsRemain = 3;
    score = 0;
    scoreLCD->display( 0 );
    level = 0;
    levelLCD->display( level+1 );
    shipsLCD->display( shipsRemain );
    view->newGame();
    view->setRockSpeed( levels[0].rockSpeed );
    view->addRocks( levels[0].nrocks );
    view->newShip();
    waitShip = FALSE;
    view->hideText();
    isPaused = FALSE;
}

void KAstTopLevel::slotMissileFired()
{
    missileFired.play();
}

void KAstTopLevel::slotShipKilled()
{
    shipsRemain--;
    shipsLCD->display( shipsRemain );

    shipDestroyed.play();

    if ( shipsRemain > 0 )
    {
        waitShip = TRUE;
        view->showText( tr( "Ship Destroyed\nPress '%1'", "e.g. Select Key").arg(launchButtonText), yellow );
    }
    else
    {
        view->endGame();
	doStats();
    }
#ifdef QTOPIA_PHONE
    ContextBar::setLabel(this, Key_Select, "qasteroids/ship/ship0000", tr("Launch"));
    ContextBar::clearLabel(this, Key_Context1);
    if (!contextMenu)
	contextMenu = new ContextMenu( this );
#endif
}

void KAstTopLevel::slotRockHit( int size )
{
    switch ( size )
    {
	case 0:
	    score += 10;
	     break;

	case 1:
	    score += 20;
	    break;

	default:
	    score += 40;
      }

    rockDestroyed.play();

    scoreLCD->display( score );
}

void KAstTopLevel::slotRocksRemoved()
{
    level++;

    if ( level >= MAX_LEVELS )
	level = MAX_LEVELS - 1;

    view->setRockSpeed( levels[level-1].rockSpeed );
    view->addRocks( levels[level-1].nrocks );

    levelLCD->display( level+1 );
}

void KAstTopLevel::doStats()
{
    QString r( "0.00" );
    if ( view->shots() )
	 r = QString::number( (double)view->hits() / view->shots() * 100.0,
			     'g', 2 );

    view->showText( tr( "GAME OVER\nPress '%1'\nfor a new game.", "%1 = e.g. Select Key" ).arg(launchButtonText), yellow, FALSE );
}

void KAstTopLevel::slotUpdateVitals()
{
    brakesLCD->display( view->brakeCount() );
    shieldLCD->display( view->shieldCount() );
    shootLCD->display( view->shootCount() );
//    teleportsLCD->display( view->teleportCount() );
    powerMeter->setValue( view->power() );
    
#ifdef QTOPIA_PHONE
    updateContext1();
#endif
}

#ifdef QTOPIA_PHONE
void KAstTopLevel::updateContext1()
{
    static int lastLabel = -1;

    int label = view->shieldCount() && view->power() ? 1 : 0;

    if (label != lastLabel) {
	if (label) {
	    if(contextMenu) {
		delete contextMenu;
		contextMenu = 0;
	    }
	    ContextBar::setLabel(this, Key_Context1, "qasteroids/powerups/shield", tr("Shield"));
	} else {
	    ContextBar::clearLabel(this, Key_Context1);
	    if(waitShip) {
		if(!contextMenu)
		    contextMenu = new ContextMenu(this);
	    } else {
		if(contextMenu) {
		    delete contextMenu;
		    contextMenu = 0;
		}
	    }
	}
	lastLabel = label;
    }
}
#endif
