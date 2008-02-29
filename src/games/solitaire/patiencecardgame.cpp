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
#include "patiencecardgame.h"
#include "canvascardwindow.h"


int highestZ = 0;


PatienceCardGame::PatienceCardGame(QCanvas *c, bool snap, QWidget *parent) : CanvasCardGame(*c, snap, parent)
{
    numberOfTimesThroughDeck = 0;
    highestZ = 0;

    int interCardSize = CardMetrics::width() + CardMetrics::interCardGap();
    int newXOff = CardMetrics::xOffset() - 6;

    for (int i = 0; i < 4; i++) {
	discardPiles[i] = new PatienceDiscardPile( newXOff + 10 + (i + 3) * interCardSize, 10, canvas() );
	addCardPile(discardPiles[i]);
    }
    for (int i = 0; i < 7; i++) {
	workingPiles[i] = new PatienceWorkingPile( newXOff + 5 + i * interCardSize, CardMetrics::height() + 15, canvas() );
	addCardPile(workingPiles[i]);
    }

    faceDownDealingPile = new PatienceFaceDownDeck( newXOff, 10, canvas() );
    faceUpDealingPile   = new PatienceFaceUpDeck( newXOff + interCardSize, 10, canvas() );

    int radius = CardMetrics::width() * 80 / 100; // 80% of card width
    circleCross = new CanvasCircleOrCross( newXOff + CardMetrics::width() / 10, 11 + (CardMetrics::height() - radius) / 2, radius, canvas() );
}


PatienceCardGame::~PatienceCardGame()
{
    delete circleCross;
    delete faceDownDealingPile;
    delete faceUpDealingPile;
}


void PatienceCardGame::deal(void)
{
    highestZ = 1;
    int t = 0;

    beginDealing();

    for (int i = 0; i < 7; i++) {
	cards[t]->setFace(TRUE);
	for (int k = i; k < 7; k++, t++) {
	    Card *card = cards[t];
	    workingPiles[k]->addCardToTop(card);
	    card->setCardPile( workingPiles[k] );
	    QPoint p = workingPiles[k]->getCardPos( card );
	    ((CanvasCard*)card)->setPos( p.x(), p.y(), highestZ );
	    ((CanvasCard*)card)->showCard();
	    highestZ++;
	}
    }
    
    for ( ; t < 52; t++) {
	Card *card = cards[t];
	faceDownDealingPile->addCardToTop(card);
	card->setCardPile( faceDownDealingPile );
	QPoint p = faceDownDealingPile->getCardPos( card );
	((CanvasCard*)card)->setPos( p.x(), p.y(), highestZ );
	((CanvasCard*)card)->showCard();
	highestZ++;
    }

    endDealing();
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
    createDeck();

    // How many times through the deck have we been
    numberOfTimesThroughDeck = cfg.readNumEntry("NumberOfTimesThroughDeck");

    // restore state to the circle/cross under the dealing pile
    if ( canTurnOverDeck() )
	circleCross->setCircle();
    else
	circleCross->setCross();

    // Move the cards to their piles (deal them to their previous places)
    beginDealing();

    highestZ = 1;

    for (int k = 0; k < 7; k++) {
	QString pile;
	pile.sprintf( "WorkingPile%i", k );
	readPile( cfg, workingPiles[k], pile, highestZ );
    }

    for (int k = 0; k < 4; k++) {
	QString pile;
	pile.sprintf( "DiscardPile%i", k );
	readPile( cfg, discardPiles[k], pile, highestZ );
    }

    readPile( cfg, faceDownDealingPile, "FaceDownDealingPile", highestZ );
    readPile( cfg, faceUpDealingPile,   "FaceUpDealingPile", highestZ );

    highestZ++;

    endDealing();
}


void PatienceCardGame::writeConfig( Config& cfg )
{
    cfg.setGroup("GameState");
    cfg.writeEntry("NumberOfTimesThroughDeck", numberOfTimesThroughDeck);

    for ( int i = 0; i < 7; i++ ) {
	QString pile;
	pile.sprintf( "WorkingPile%i", i );
	workingPiles[i]->writeConfig( cfg, pile );
    }
    for ( int i = 0; i < 4; i++ ) {
	QString pile;
	pile.sprintf( "DiscardPile%i", i );
	discardPiles[i]->writeConfig( cfg, pile );
    }
    faceDownDealingPile->writeConfig( cfg, "FaceDownDealingPile" );
    faceUpDealingPile->writeConfig( cfg, "FaceUpDealingPile" );
}


bool PatienceCardGame::mousePressCard( Card *card, QPoint p )
{
    CanvasCard *item = (CanvasCard *)card;
    if (item->isFacing() != TRUE) {
	// From facedown stack
	if ( item->getCardPile() == faceDownDealingPile ) { 
	    // flip 1 or 3 cards depending on cardsDrawn()
	    int flipped = 0;
	    QCanvasItemList l = canvas()->collisions( p );
	    for (QCanvasItemList::Iterator it = l.begin(); (it != l.end()) && (flipped != cardsDrawn()); ++it) {
		if ( (*it)->rtti() == canvasCardId ) {
		    CanvasCard *item = (CanvasCard *)*it;
		    if (item->animated())
			continue;
		    item->setZ(highestZ);
		    highestZ++;
		    flipped++;

		    // Added Code
		    faceDownDealingPile->removeCard(item);
		    faceUpDealingPile->addCardToTop(item);
		    item->setCardPile( faceUpDealingPile );

		    item->flipTo( faceUpDealingPile->getX(), faceUpDealingPile->getY(), 5 * flipped );
		}
	    }
	}
	moving = NULL;
	moved = FALSE;
	return TRUE;
    }

    return FALSE;
}


void PatienceCardGame::mousePress(QPoint p)
{
    if ( canTurnOverDeck() && 
         (p.x() > faceDownDealingPile->getX() ) && (p.x() < faceDownDealingPile->getX() + CardMetrics::width() ) &&
	 (p.y() > faceDownDealingPile->getY() ) && (p.y() < faceDownDealingPile->getY() + CardMetrics::height() ) ) {

	beginDealing();
	Card *card = faceUpDealingPile->cardOnTop();
	while ( card ) {
	    ((CanvasCard*)card)->setPos( faceDownDealingPile->getX(), faceDownDealingPile->getY(), highestZ );
	    card->setFace( FALSE );
	    faceUpDealingPile->removeCard( card );
	    faceDownDealingPile->addCardToTop( card );
	    card->setCardPile( faceDownDealingPile );
	    card = faceUpDealingPile->cardOnTop();
    	    highestZ++;
	}
	endDealing();

	throughDeck();
	
	moved = TRUE;
    }
}


