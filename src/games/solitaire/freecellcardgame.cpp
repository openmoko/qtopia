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
#include "freecellcardgame.h"
#include "cardmetrics.h"


int numberOfFreeCells = 4;


FreecellCardGame::FreecellCardGame(QCanvas *c, QCanvasView *view, bool snap, bool casinoRules) :
    CanvasCardGame(c, view, snap, casinoRules)
{
    numberOfFreeCells = 4;

    int spaceBetweenPiles = CardMetrics::interCardGap() + CardMetrics::width(); // 21 + 7 * cardSizes;
    int xOrigin = CardMetrics::xOffset() - spaceBetweenPiles / 2 + 1;
    int spacing = 0;
    spaceBetweenPiles--;

    int i;
    QString pile;
    for ( i = 0; i < 4; i++) {
	QPoint p(xOrigin + i * spaceBetweenPiles, 10);
	pile.sprintf("FreeCellPile%i", i);
	freecellPiles[i] = new FreecellFreecellPile(p, pile, canvas());
	append(freecellPiles[i]);
    }
    for ( i = 0; i < 4; i++) {
	QPoint p(xOrigin + spacing + 6 + (i + 4) * spaceBetweenPiles, 10);
	pile.sprintf("DiscardPile%i", i);
	discardPiles[i] = new FreecellDiscardPile(p, pile, canvas());
	append(discardPiles[i]);
    }
    for ( i = 0; i < 8; i++) {
	QPoint p(xOrigin + spacing + 3 + i * spaceBetweenPiles, 15 + CardMetrics::height());
	pile.sprintf("WorkingPile%i", i);
	workingPiles[i] = new FreecellWorkingPile(p, pile, canvas());
	append(workingPiles[i]);
    }
}


void FreecellCardGame::deal(void)
{
    for (int i = 0; i < 52; i++) {
	Card *card = undealtPile->at(i);//cards[i];
	card->setFacing(true);
	workingPiles[i%8]->addCardToTop( card );
	card->setVisible(true);
    }
}


bool FreecellCardGame::cardSelected(Card *c)
{
    return !c->canBeMoved();
}


int FreecellCardGame::pileForKey(int curPile, int key)
{
    if ( curPile >= 0 && curPile < 16 && key >= 0 && key < 8 ) {
	switch (key) {
	    case 0: case 4: // left
		return (curPile == 0) ? 15 : curPile-1;
	    case 1: case 5: // right
		return (curPile == 15) ? 0 : curPile+1;
	    case 2: case 6: // up
		return (curPile < 8) ? ((curPile == 7) ? 8 : curPile+9) : curPile-8;
	    case 3: case 7: // down
		return (curPile > 7) ? ((curPile == 8) ? 7 : curPile-9) : curPile+8;
	}
    }
    return 0;
}


void FreecellCardGame::readConfig( Config& cfg )
{
    cfg.setGroup("GameState");
    if ( cfg.readBoolEntry("Won", FALSE) ) {
	newGame();
	return;
    }

    // Create Cards, but don't shuffle or deal them yet
    createCards();

    // Move the cards to their piles (deal them to their previous places)
    beginDealing();
    readPiles(cfg);
    endDealing();
}


void FreecellCardGame::writeConfig( Config& cfg )
{
    cfg.setGroup("GameState");
    cfg.writeEntry("Won", haveWeWon() );
    writePiles(cfg);
}


