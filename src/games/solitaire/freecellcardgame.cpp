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
#include "freecellcardgame.h"
#include "cardmetrics.h"


extern int highestZ;
int numberOfFreeCells = 4;


FreecellCardGame::FreecellCardGame(QCanvas *c, bool snap, QWidget *parent) : CanvasCardGame(*c, snap, parent)
{
    numberOfFreeCells = 4;
    highestZ = 0;

    int spaceBetweenPiles = CardMetrics::interCardGap() + CardMetrics::width(); // 21 + 7 * cardSizes;
    int xOrigin = CardMetrics::xOffset() - spaceBetweenPiles / 2;
    int spacing = 0;
    spaceBetweenPiles--;

    int i;
    for ( i = 0; i < 4; i++) {
	freecellPiles[i] = new FreecellFreecellPile( xOrigin + i * spaceBetweenPiles, 10, canvas() );
	addCardPile(freecellPiles[i]);
    }
    for ( i = 0; i < 4; i++) {
	discardPiles[i] = new FreecellDiscardPile( xOrigin + spacing + 6 + (i + 4) * spaceBetweenPiles, 10, canvas() );
	addCardPile(discardPiles[i]);
    }
    for ( i = 0; i < 8; i++) {
	workingPiles[i] = new FreecellWorkingPile( xOrigin + spacing + 2 + i * spaceBetweenPiles, 15 + CardMetrics::height(), canvas() );
	addCardPile(workingPiles[i]);
    }
}


void FreecellCardGame::deal(void)
{
    highestZ = 1;
    
    beginDealing();
    
    for (int i = 0; i < 52; i++) {
	Card *card = cards[i];
	card->setFace( TRUE );
	((CanvasCard*)card)->setPos( 0, 0, highestZ );
	card->setCardPile( workingPiles[i%8] );
	workingPiles[i%8]->addCardToTop( card );
	((CanvasCard*)card)->move( workingPiles[i%8]->getCardPos( card ) );
	((CanvasCard*)card)->showCard();
	highestZ++;
    }

    endDealing();
}


bool FreecellCardGame::mousePressCard( Card *c, QPoint p )
{
    Q_UNUSED(p);
    
    if ( !c->getCardPile()->isAllowedToBeMoved(c) ) {
	moving = NULL;
	return TRUE;
    }

    return FALSE;
}


void FreecellCardGame::readConfig( Config& cfg )
{
    cfg.setGroup("GameState");
    if ( cfg.readBoolEntry("Won", FALSE) ) {
	newGame();
	return;
    }
    // Create Cards, but don't shuffle or deal them yet
    createDeck();

    // Move the cards to their piles (deal them to their previous places)
    beginDealing();

    highestZ = 1;
    int k;
    for ( k = 0; k < 4; k++) {
	QString pile;
	pile.sprintf( "FreeCellPile%i", k );
	readPile( cfg, freecellPiles[k], pile, highestZ );
    }

    for ( k = 0; k < 4; k++) {
	QString pile;
	pile.sprintf( "DiscardPile%i", k );
	readPile( cfg, discardPiles[k], pile, highestZ );
    }

    for ( k = 0; k < 8; k++) {
	QString pile;
	pile.sprintf( "WorkingPile%i", k );
	readPile( cfg, workingPiles[k], pile, highestZ );
    }

    highestZ++;

    endDealing();
}


void FreecellCardGame::writeConfig( Config& cfg )
{
    cfg.setGroup("GameState");
    cfg.writeEntry("Won", haveWeWon() );
    int i;
    for ( i = 0; i < 4; i++ ) {
	QString pile;
	pile.sprintf( "FreeCellPile%i", i );
	freecellPiles[i]->writeConfig( cfg, pile );
    }
    for ( i = 0; i < 4; i++ ) {
	QString pile;
	pile.sprintf( "DiscardPile%i", i );
	discardPiles[i]->writeConfig( cfg, pile );
    }
    for ( i = 0; i < 8; i++ ) {
	QString pile;
	pile.sprintf( "WorkingPile%i", i );
	workingPiles[i]->writeConfig( cfg, pile );
    }
}


