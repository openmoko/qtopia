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
#include <stdlib.h>
#include <time.h>
#include "card.h"
#include "cardgame.h"


CardGame::CardGame(int numberOfJokers, int numberOfDecks) :
    jokers(numberOfJokers),
    decks(numberOfDecks),
    cards((52+jokers)*decks),
    cardsCreated(false)
{
    setAutoDelete(true);

    undealtPile = new CardPile(QPoint(-1, -1), "undealt");
    append( undealtPile );
    undealtPile->setAutoDelete( true );
}


CardGame::~CardGame()
{
}


CardPile *CardGame::closestPile(QPoint pnt, int maxDistance)
{
    int closestDistance = maxDistance * maxDistance * 2;
    CardPile *closestPile = NULL;
    
    for (CardPile *p = first(); p != NULL; p = next()) {
	if (p->isDropTarget()) {
	    int d = p->distanceFromNextPos(pnt);
	    if (d < closestDistance) {
		closestDistance = d;
		closestPile = p;		
	    }
	}
    }
    
    return closestPile;
}


void CardGame::beginDealing()
{
    for (CardPile *p = first(); p != NULL; p = next())
	p->beginDealing();
}


void CardGame::endDealing()
{
    for (CardPile *p = first(); p != NULL; p = next())
	p->endDealing();
}


void CardGame::readPiles(Config &cfg)
{
    for (CardPile *p = at(1); p != NULL; p = next())
	p->readConfig(cfg, undealtPile);
}


void CardGame::writePiles(Config &cfg)
{
    for (CardPile *p = at(1); p != NULL; p = next())
	p->writeConfig(cfg);
}


void CardGame::newGame()
{
    // Create Cards
    createCards();

    // Shuffle Cards
    shuffleCards();

    // Deal Cards
    beginDealing();
    deal();
    endDealing();
}


void CardGame::createCards()
{
    int i;
    if (!cardsCreated) {
        for (i = 0; i < 52; i++)
	    undealtPile->append( newCard( (Card::Value)((i % 13) + 1), (Card::Suit)((i / 13) + 1), FALSE ) );
        for (i = 0; i < jokers; i++)
	    undealtPile->append( newCard( Card::jokerVal, Card::jokerSuit, FALSE ) );
        cardsCreated = true;
    }
}


void CardGame::shuffleCards()
{
    srand(time(NULL));
    for (int i = 0; i < cards; i++) {
	int index = rand() % (cards - 1);
	undealtPile->insert(index, undealtPile->take(i));
	undealtPile->insert(i, undealtPile->take(index + 1));
    }
}


