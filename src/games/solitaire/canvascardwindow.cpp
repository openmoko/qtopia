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

#include "canvascardwindow.h"
#include "patiencecardgame.h"
#include "freecellcardgame.h"
#include "cardmetrics.h"

#include <qtopia/resource.h>

#include <qapplication.h>
#include <qmainwindow.h>
#include <qpopupmenu.h>
#include <qstyle.h>
#include <qlayout.h>


enum GameType
{
    SolitaireGame,
    FreecellGame
};


CanvasCardWindow::CanvasCardWindow(QWidget* parent, const char* name, WFlags f) :
    QMainWindow(parent, name, f), resizeTimeout( this, "resizeTimeout" ), canvas(1, 1),
    snapOn(TRUE), cardBack(0), gameType(0), drawThree(TRUE), cardGame(0) , resizing( false )
{
    setIcon( Resource::loadPixmap( "cards" ) );

    connect( &resizeTimeout, SIGNAL( timeout() ), this, SLOT( doResize() ) );

    // Create Playing Area for Games
    if ( QPixmap::defaultDepth() < 12 ) {
	canvas.setBackgroundColor(QColor(0x08, 0x98, 0x2D));
    } else {
        QPixmap bg;
	bg.convertFromImage( Resource::loadImage( "table_pattern" ), ThresholdDither );  
        canvas.setBackgroundPixmap(bg);
    }

#if defined( QT_QWS_CASSIOPEIA )
    canvas.setAdvancePeriod(70);
#else
    canvas.setAdvancePeriod(30);
#endif

    QMenuBar* menu = menuBar();

    QPopupMenu* file = new QPopupMenu;

#ifdef _PATIENCE_USE_ACCELS_
    file->insertItem(tr("&Patience"), this, SLOT(initPatience()), Key_F2);
    file->insertItem(tr("&Freecell"), this, SLOT(initFreecell()), Key_F3);
    menu->insertItem(tr("&Play"), file);
#else
    file->insertItem(tr("Patience"), this, SLOT(initPatience()));
    file->insertItem(tr("Freecell"), this, SLOT(initFreecell()));
    menu->insertItem(tr("Play"), file);
#endif
    
    menu->insertSeparator();

    settings = new QPopupMenu;
    settings->setCheckable(TRUE);
    snapId = settings->insertItem(tr("Snap To Position"), this, SLOT(snapToggle()));
    changeId = 0;
    drawId = 0;
    menu->insertItem(tr("&Settings"),settings);

    menu->show();

    // Temporary initial QCanvasView until a game is loaded or a new game started
    canvasView = new QCanvasView( &canvas, this );
    setCentralWidget( canvasView );
}


CanvasCardWindow::~CanvasCardWindow()
{
    closeGame();
}


void CanvasCardWindow::initGame()
{
{
    Config cfg("Patience");
    cfg.setGroup( "GlobalSettings" );
    snapOn = cfg.readBoolEntry( "SnapOn", TRUE);
    settings->setItemChecked(snapId, snapOn);
    gameType = cfg.readNumEntry( "GameType", -1 );
    drawThree = cfg.readBoolEntry( "DrawThree", TRUE);
    cardBack = cfg.readNumEntry( "CardBack", 0 );
}
    CardMetrics::setCardBack( cardBack );

    if ( gameType == 0 ) {
	initGame( false, 0 );
    } else if ( gameType == 1 ) {
	initGame( false, 1 );
    } else {
	// Probably there isn't a config file or it is broken
	// Start a new game
	initPatience();
    }
}


void CanvasCardWindow::closeGame()
{
    if ( cardGame ) {
	Config cfg("Patience");
	cfg.setGroup( "GlobalSettings" );
	cfg.writeEntry( "GameType", gameType );
	cfg.writeEntry( "SnapOn", snapOn );
	cfg.writeEntry( "DrawThree", drawThree);
	cfg.writeEntry( "CardBack", cardBack );
	cardGame->writeConfig( cfg );
	delete cardGame;
	setCentralWidget( canvasView );
    }
}


void CanvasCardWindow::showEvent( QShowEvent * )
{
    resizeEvent( 0 );
}


void CanvasCardWindow::resizeEvent( QResizeEvent *re )
{
    if ( !resizing ) {
	closeGame();
	resizing = true;
    }

    QMainWindow::resizeEvent( re );

    QWidget *theView = centralWidget();
    if ( theView ) {
	if ( theView->inherits("QCanvasView") ) {
	    QCanvasView *currentView = (QCanvasView*)theView;
	    if ( currentView ) {
		QSize s = currentView->viewport()->size();
		canvas.resize( s.width(), s.height() );
	    }
	}

    }

    resizeTimeout.start( 100, TRUE );
}


void CanvasCardWindow::doResize()
{
    if ( resizing ) {

	QSize s = canvas.size();
	QWidget *theView = centralWidget();
	if ( theView ) {
	    if ( theView->inherits("QCanvasView") ) {
		QCanvasView *currentView = (QCanvasView*)theView;
		if ( currentView ) {
		    s = currentView->viewport()->size();
		}
	    }
	}
	CardMetrics::loadMetrics( s.width(), s.height() );
	initGame();
	canvas.resize( s.width(), s.height() );

	resizing = false;
    }
}


void CanvasCardWindow::initGame( bool newGame, int type )
{
    if ( type == SolitaireGame ) {
	if ( !changeId )
	    changeId = settings->insertItem(tr("Change Card Backs"), this, SLOT(changeCardBacks()));
	if ( !drawId )
	    drawId = settings->insertItem(tr("Turn One Card"), this, SLOT(drawnToggle()));
    } else {
	if ( changeId ) {
	    settings->removeItem( changeId );
	    changeId = 0;
	}
	if ( drawId ) {
	    settings->removeItem( drawId );
	    drawId = 0;
	}
    }

    // Create New Game 
    if ( newGame && cardGame ) 
	delete cardGame;

    if ( type == SolitaireGame )
        cardGame = new PatienceCardGame( &canvas, snapOn, this );
    else
	cardGame = new FreecellCardGame( &canvas, snapOn, this );

    cardGame->setNumberToDraw(drawThree ? 3 : 1);
    gameType = type;
    if ( type == SolitaireGame )
	setCaption(tr("Patience"));
    else
	setCaption(tr("Freecell"));
    setCentralWidget(cardGame);
    if ( newGame )
	cardGame->newGame();
    else {
	Config cfg("Patience");
	cfg.setGroup( "GlobalSettings" );
	cardGame->readConfig( cfg );
    }
    setCardBacks();
    if ( type == SolitaireGame )
	updateDraw();
}


void CanvasCardWindow::initPatience()
{
    initGame( true, SolitaireGame );
}


void CanvasCardWindow::initFreecell()
{
    initGame( true, FreecellGame );
}


void CanvasCardWindow::snapToggle()
{
    snapOn = !snapOn;
    settings->setItemChecked(snapId, snapOn);
    cardGame->toggleSnap();
}


void CanvasCardWindow::drawnToggle()
{
    drawThree = !drawThree;
    cardGame->toggleCardsDrawn();
    updateDraw();
}


void CanvasCardWindow::updateDraw()
{
    if(cardGame->cardsDrawn() == 3) 
	settings->changeItem(drawId, tr("Turn One Card"));
    else 
	settings->changeItem(drawId, tr("Turn Three Cards"));
}


void CanvasCardWindow::setCardBacks()
{
    QCanvasItemList l = canvas.allItems();
    for (QCanvasItemList::Iterator it = l.begin(); it != l.end(); ++it) {
	if ( (*it)->rtti() == canvasCardId )
	    ((CanvasCard *)(*it))->cardBackChanged();
    }
}


void CanvasCardWindow::changeCardBacks()
{
    cardBack++;
    if (cardBack == 5)
	cardBack = 0;
    CardMetrics::setCardBack( cardBack );
    setCardBacks();
}


