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
#ifndef CARD_H
#define CARD_H


#include <qpoint.h>
#include "cardpile.h"


class Card {
public:
    enum Suit {
	jokerSuit = 0, clubs, spades, diamonds, hearts
    };

    enum Value {
	jokerVal = 0, ace, two, three, four, five,
	six, seven, eight, nine, ten, jack, queen, king
    };

    Card( Value v, Suit s, bool f );
    virtual ~Card();

    Value value() const { return val; }
    Suit suit() const { return cardSuit; }
    bool isRed() const { return ((suit() == diamonds) || (suit() == hearts)); }

    void setCardPile(CardPile *p) { pile = p; }
    CardPile *cardPile() const { return pile; }

    void setFacing(bool f) { faceUp = f; }
    bool isFacing() const { return faceUp; }

    virtual void setPos(QPoint p) = 0;
    virtual QPoint pos() const = 0;

    virtual void setHeight(int z) = 0;
    virtual int height() const = 0;
    
    virtual void setVisible(bool) = 0;
    virtual bool isVisible() const = 0;

    virtual void moveTo(QPoint p, int duration = 0) = 0;
    virtual void flipTo(QPoint p, int duration = 0) = 0;

    bool canBeMoved() { return cardPile()->isAllowedToBeMoved(this); }
    bool isSelectable() { return cardPile()->isSelectable(this); }
private:
    Value val;
    Suit cardSuit;
    bool faceUp;
    CardPile *pile;
};


#endif

