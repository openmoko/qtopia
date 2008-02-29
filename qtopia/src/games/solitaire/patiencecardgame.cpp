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
#include "patiencecardgame.h"
#include "canvascardpile.h"


PatienceCardGame::PatienceCardGame(QCanvas *c, QCanvasView *view, bool snap, bool casinoRules) :
    CanvasCardGame(c, view, snap, casinoRules)
{
    int interCardSize = CardMetrics::width() + CardMetrics::interCardGap();
    int newXOff = CardMetrics::xOffset() - 6;
    int i;

    numberOfTimesThroughDeck = 0;

    faceDownDealingPile = new PatienceFaceDownDeck(QPoint(newXOff, 10), "FaceUpDealingPile", canvas());
    faceUpDealingPile   = new PatienceFaceUpDeck(QPoint(newXOff + interCardSize, 10), "FaceDownDealingPile", canvas());
    faceDownDealingPile->setDropTarget(false);
    faceUpDealingPile->setDropTarget(false);
    append(faceDownDealingPile);
    append(faceUpDealingPile);

    QString pile;
    for ( i = 0; i < 4; i++) {
	pile.sprintf("DiscardPile%i", i);
	discardPiles[i] = new PatienceDiscardPile(QPoint(newXOff + 10 + (i + 3) * interCardSize, 10), pile, canvas());
	append(discardPiles[i]);
    }
    for ( i = 0; i < 7; i++) {
	pile.sprintf("WorkingPile%i", i);
	workingPiles[i] = new PatienceWorkingPile(QPoint(newXOff + 5 + i * interCardSize, CardMetrics::height() + 15), pile, canvas());
	append(workingPiles[i]);
    }

    int radius = CardMetrics::width() * 80 / 100; // 80% of card width
    circleCross = new CanvasCircleOrCross(QPoint(newXOff + CardMetrics::width() / 10, 10 + (CardMetrics::height() - radius) / 2), radius, canvas());
}


PatienceCardGame::~PatienceCardGame()
{
    delete circleCross;
}


void PatienceCardGame::deal(void)
{
    int t = 0;

    for (int i = 0; i < 7; i++) {
	undealtPile->at(t)->setFacing(true);
	for (int k = i; k < 7; k++, t++) {
	    Card *card = undealtPile->at(t);
	    workingPiles[k]->addCardToTop(card);
	    card->setVisible(true);
	}
    }
    
    for ( ; t < 52; t++) {
	Card *card = undealtPile->at(t);
	faceDownDealingPile->addCardToTop(card);
	card->setVisible(true);
    }
}


void PatienceCardGame::readConfig( Config& cfg )
{
    cfg.setGroup("GameState");

    // Do we have a config file to read in?
    if ( !cfg.hasKey("NumberOfTimesThroughDeck") ) {
	// if not, create a new game
	newGame();
	return;
    }
    // We have a config file, lets read it in and use it

    // Create Cards, but don't shuffle or deal them yet
    createCards();

    // How many times through the deck have we been
    numberOfTimesThroughDeck = cfg.readNumEntry("NumberOfTimesThroughDeck");

    // restore state to the circle/cross under the dealing pile
    setCircleCrossState();

    // Move the cards to their piles (deal them to their previous places)
    beginDealing();
    readPiles(cfg);
    endDealing();
}


void PatienceCardGame::writeConfig( Config& cfg )
{
    cfg.setGroup("GameState");
    if ( haveWeWon() ) {
	cfg.clearGroup();
	return;
    }
    
    cfg.writeEntry("NumberOfTimesThroughDeck", numberOfTimesThroughDeck);

    writePiles(cfg);
}


#ifdef QTOPIA_PHONE 
static const int animDuration = 150;
#else
static const int animDuration = 150;
#endif


void PatienceCardGame::flipOverCards()
{
    int height = 1;

    // Tidy pile in to neat stack as you would in real life!
    for (Card *c = faceUpDealingPile->first(); c != 0; c = faceUpDealingPile->next()) {
	c->setFacing(true);
	c->setHeight(height++);
	if ( c->pos() != faceUpDealingPile->pos() )
	    c->moveTo(faceUpDealingPile->pos(), animDuration/2);
    }

    // flip 1 or 3 cards depending on cardsDrawn()
    for ( int flipped = 0; flipped != cardsDrawn(); flipped++ ) {
	Card *item = faceDownDealingPile->cardOnTop();
	if ( item ) {
	    // Added Code
	    faceDownDealingPile->removeCard(item);
	    faceUpDealingPile->addCardToTop(item);

	    QPoint p = faceUpDealingPile->pos();
	    QPoint p2(p.x() + flipped*8, p.y());

	    item->setPos(faceDownDealingPile->pos());
	    item->setHeight(QMAX(item->height(),height++));
	    item->flipTo(p2, animDuration*(flipped+1));
	}
    }
}


void PatienceCardGame::turnOverDeck()
{
    if ( canTurnOverDeck() ) {
	beginDealing();
	Card *card = faceUpDealingPile->cardOnTop();
	while ( card ) {
	    card->setFacing(false);
	    faceUpDealingPile->removeCard( card );
	    faceDownDealingPile->addCardToTop( card );
	    card->moveTo(faceDownDealingPile->pos(), 0);
	    card = faceUpDealingPile->cardOnTop();
	}
	endDealing();
	throughDeck();
    }
}


bool PatienceCardGame::pileSelected(CardPile *pile)
{
    if (pile == faceDownDealingPile) {
	if (faceDownDealingPile->cardOnTop()) 
	    flipOverCards();
	else 
	    turnOverDeck();
	return true;
    }
    return false;
}


bool PatienceCardGame::cardSelected(Card *card)
{
    if (!card->isFacing()) {
	// From facedown stack
	if (card->cardPile() == faceDownDealingPile) 
	    flipOverCards();
	return true;
    }
    return !card->canBeMoved();
}


// Cursor movement tables

// Selects the pile to move a card from according to which arrow key is pressed
static int leftSelectingTab[]  = { 12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
static int rightSelectingTab[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0 };
static int upSelectingTab[]    = { 12, 6, 8, 9, 10, 11, 0, 1, 7, 2, 3, 4, 5 };
static int downSelectingTab[]  = { 6, 7, 9, 10, 11, 12, 1, 8, 2, 3, 4, 5, 0 };

// Selects the pile a card can be moved to
static int leftSelectingPileTab[]   = { 6, 6, 12, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
static int rightSelectingPileTab[]  = { 6, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 2 };
static int upSelectingPileTab[]     = { 6, 6, 8, 9, 10, 11, 12, 6, 7, 2, 3, 4, 5 };
static int downSelectingPileTab[]   = { 6, 6, 9, 10, 11, 12, 7, 8, 2, 3, 4, 5, 6 };
static int selectSelectingPileTab[] = { 6, 6, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

static int *pileForCursorMovement[9] = { leftSelectingTab, rightSelectingTab,
				    upSelectingTab, downSelectingTab,
				    leftSelectingPileTab, rightSelectingPileTab,
				    upSelectingPileTab, downSelectingPileTab,
				    selectSelectingPileTab };


int PatienceCardGame::pileForKey(int curPile, int key)
{
    if ( curPile >= 0 && curPile < 13 && key >= 0 && key < 9 )
	return pileForCursorMovement[key][curPile];
    else
	return 0;
}

