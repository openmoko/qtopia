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

#include "cardpile.h"
#include "card.h"

#include <qtopia/config.h>
#include <qpoint.h>

#include <qlist.h>


CardPile::CardPile(QPoint p, QString n)
    : dealing(FALSE), target(TRUE), pileName(n)
{
    setPos(p);
    setNextPos(p);
}


static int lengthSquared(const QPoint &p)
{
    return p.x()*p.x()+p.y()*p.y();
}


int CardPile::distanceFromPile(QPoint p) const
{
    return lengthSquared(pos()-p);
}


int CardPile::distanceFromNextPos(QPoint p) const
{
    return lengthSquared(nextPos()-p);
}


Card *CardPile::cardInfront(Card *c) 
{
    CardPile *p = c->cardPile();
    if (p) {
	p->at(p->find(c));
	return p->next();
    } else {
	return NULL;
    }
}


Card *CardPile::cardBehind(Card *c) 
{
    CardPile *p = c->cardPile();
    if (p) {
	p->at(p->find(c));
	return p->prev();
    } else {
	return NULL;
    }
}


bool CardPile::kingOnTop() const
{
    Card *top = cardOnTop();
    return top && top->value() == Card::king;
}


bool CardPile::addCardToTop(Card *c)
{
    if (dealing || isAllowedOnTop(c)) {
	append((const Card *)c);
	cardAddedToTop(c);
	c->setCardPile(this);
	c->setPos(cardPos(c));
	c->setHeight(count());
	return TRUE;
    }
    return FALSE;
}


bool CardPile::addCardToBottom(Card *c)
{
    if (dealing || isAllowedOnBottom(c)) {
	prepend((const Card *)c);
	cardAddedToBottom(c);
	c->setCardPile(this);
	int i = 0;
	Card *card = getFirst();
	while ( card ) {
	    card->setPos(cardPos(card));
	    card->setHeight(i++);
	    card = next();
	}
	return TRUE;
    }
    return FALSE;
}


bool CardPile::removeCard(Card *c)
{
    if (dealing || isAllowedToBeMoved(c)) {
	take(find(c));
	cardRemoved(c);
	return TRUE;
    }
    return FALSE;
}


void CardPile::readConfig(Config& cfg, CardPile *undealtCards)
{
    cfg.setGroup(pileName);
    int numberOfCards = cfg.readNumEntry("NumberOfCards", 0);
    for ( int i = 0; i < numberOfCards; i++ ) {
	Card *card = undealtCards->at(cfg.readNumEntry(QString("Card%1").arg(i)));
	card->setFacing(cfg.readBoolEntry(QString("CardFacing%1").arg(i)));
	addCardToTop(card);
	card->setVisible(true);
    }
}


void CardPile::writeConfig(Config& cfg)
{
    int numberOfCards = 0;
    cfg.setGroup(pileName);
    Card *card = cardOnBottom();
    while ( card ) {
	QString cardStr;
	cardStr.sprintf( "%i", numberOfCards );
	int val  = (int)card->value() - 1 + ( (int)card->suit() - 1 ) * 13;
	cfg.writeEntry( "Card" + cardStr, val );
	cfg.writeEntry( "CardFacing" + cardStr, card->isFacing() );
	card = cardInfront( card );
	numberOfCards++;
    }
    cfg.writeEntry("NumberOfCards", numberOfCards);
}


