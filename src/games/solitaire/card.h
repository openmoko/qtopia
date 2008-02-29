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
#ifndef CARD_H
#define CARD_H


#include <qpoint.h>


class CardPile;


enum eSuit {
    jokerSuit = 0, clubs, spades, diamonds, hearts
};


enum eValue {
    jokerVal = 0, ace, two, three, four, five,
    six, seven, eight, nine, ten, jack, queen, king
};


class Card {
public:
    Card( eValue v, eSuit s, bool f );
    virtual ~Card();

    eValue getValue() { return val; }
    eSuit getSuit() { return suit; }

    void setCardPile(CardPile *p) { cardPile = p; }
    CardPile *getCardPile() { return cardPile; }

    void setFace(bool f) { faceUp = f; }
    bool isFacing() { return faceUp; }

    bool isShowing() { return showing; }
    bool isRed() { return ((suit == diamonds) || (suit == hearts)); }

    int getX(void) { return ix; }
    int getY(void) { return iy; }
    int getZ(void) { return iz; }

private:
    eValue val;
    eSuit suit;
    bool faceUp;
    bool showing;
    int ix, iy, iz;
    CardPile *cardPile;
};


#endif

