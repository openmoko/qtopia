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


CanvasCardWindow::CanvasCardWindow(QWidget* parent, const char* name, WFlags f) :
    QMainWindow(parent, name, f), canvas(1, 1), snapOn(TRUE), cardBack(0), gameType(0),
    cardGame(0), drawThree(TRUE)
{
    setIcon( Resource::loadPixmap( "cards" ) );

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


#ifdef _PATIENCE_USE_ACCELS_
    QPEMenuBar* menu = menuBar();

    QPopupMenu* file = new QPopupMenu;
    file->insertItem(tr("Patience"), this, SLOT(initPatience()), CTRL+Key_F);
    file->insertItem(tr("Freecell"), this, SLOT(initFreecell()), CTRL+Key_F);
    menu->insertItem(tr("&Game"), file);
    
    menu->insertSeparator();

    settings = new QPopupMenu;
    settings->insertItem(tr("&Change Card Backs"), this, SLOT(changeCardBacks()), Key_F2);
    snap_id = settings->insertItem(tr("&Snap To Position"), this, SLOT(snapToggle()), Key_F3);
    settings->setCheckable(TRUE);
    menu->insertItem(tr("&Settings"),settings);

    menu->insertSeparator();

    QPopupMenu* help = new QPopupMenu;
    help->insertItem(tr("&About"), this, SLOT(help()), Key_F1);
    help->setItemChecked(dbf_id, TRUE);
    menu->insertItem(tr("&Help"),help);
#else
    QMenuBar* menu = menuBar();

    QPopupMenu* file = new QPopupMenu;
    file->insertItem(tr("Patience"), this, SLOT(initPatience()));
    file->insertItem(tr("Freecell"), this, SLOT(initFreecell()));
    menu->insertItem(tr("Play"), file);
    
    menu->insertSeparator();

    settings = new QPopupMenu;
    settings->setCheckable(TRUE);
    settings->insertItem(tr("Change Card Backs"), this, SLOT(changeCardBacks()));
    snap_id = settings->insertItem(tr("Snap To Position"), this, SLOT(snapToggle()));
    QString m;

    drawId = settings->insertItem(tr("Turn One Card"), this, SLOT(drawnToggle()));
    menu->insertItem(tr("Settings"),settings);

#endif

    menu->show();

    // We need a dummy canvas view to get the correct size first
    // before we layout the cards in the resize handling code
    QCanvasView blankView( &canvas, this, "blankView" );
    setCentralWidget( &blankView );

    // wait for resize/show event
    resizeEvent( 0 );
}


CanvasCardWindow::~CanvasCardWindow()
{
    closeGame();
}


void CanvasCardWindow::initGame()
{
    Config cfg("Patience");
    cfg.setGroup( "GlobalSettings" );
    snapOn = cfg.readBoolEntry( "SnapOn", TRUE);
    settings->setItemChecked(snap_id, snapOn);
    gameType = cfg.readNumEntry( "GameType", -1 );
    drawThree = cfg.readBoolEntry( "DrawThree", TRUE);
    cardBack = cfg.readNumEntry( "CardBack", 0 );
    CardMetrics::setCardBack( cardBack );

    if ( gameType == 0 ) {
	cardGame = new PatienceCardGame( &canvas, snapOn, this );
	cardGame->setNumberToDraw(drawThree ? 3 : 1);
	setCaption(tr("Patience"));
	setCentralWidget(cardGame);
	cardGame->readConfig( cfg );
	setCardBacks();
	updateDraw();
    } else if ( gameType == 1 ) {
	cardGame = new FreecellCardGame( &canvas, snapOn, this );
	setCaption(tr("Freecell"));
	setCentralWidget(cardGame);
	cardGame->readConfig( cfg );
	setCardBacks();
	updateDraw();
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
    }
}


void CanvasCardWindow::showEvent( QShowEvent * )
{
    resizeEvent( 0 );
}


void CanvasCardWindow::resizeEvent( QResizeEvent * )
{
    if ( !centralWidget() ) 
	return;

    QSize s = centralWidget()->size();
    int fw = style().defaultFrameWidth();
    canvas.resize( s.width() - fw - 2, s.height() - fw - 2);

    CardMetrics::loadMetrics( width(), height() );

    // Reinitialise *everything* so it is layed out correctly
    closeGame();
    initGame();
}


void CanvasCardWindow::initPatience()
{
    // Create New Game 
    if ( cardGame )
	delete cardGame;
    cardGame = new PatienceCardGame( &canvas, snapOn, this );
    cardGame->setNumberToDraw(drawThree ? 3 : 1);
    gameType = 0;
    setCaption(tr("Patience"));
    setCentralWidget(cardGame);
    cardGame->newGame();
    setCardBacks();
    updateDraw();
}


void CanvasCardWindow::initFreecell()
{
    // Create New Game
    if ( cardGame ) 
	delete cardGame;
    cardGame = new FreecellCardGame( &canvas, snapOn, this );
    gameType = 1;
    setCaption(tr("Freecell"));
    setCentralWidget(cardGame);
    cardGame->newGame();
    setCardBacks();
}


void CanvasCardWindow::snapToggle()
{
    snapOn = !snapOn;
    settings->setItemChecked(snap_id, snapOn);
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


