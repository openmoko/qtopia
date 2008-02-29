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
#ifndef CARD_GAME_H
#define CARD_GAME_H


#include <qtopia/config.h>
#include <qpoint.h>
#include <qlist.h>
#include "card.h"
#include "cardpile.h"


class CardGame : public QList<CardPile>
{
public:
    CardGame(int numOfJokers = 0, int numOfDecks = 1);
    virtual ~CardGame();

    virtual QString gameName() const = 0;

    virtual void newGame();
    void createCards();
    virtual Card *newCard( Card::Value v, Card::Suit s, bool f ) = 0;
    void shuffleCards();
    void beginDealing();
    virtual void deal() { }
    void endDealing();

    virtual void readConfig(Config& cfg) { Q_UNUSED(cfg); }
    virtual void writeConfig(Config& cfg) { Q_UNUSED(cfg); }

    virtual void gameWon() = 0;
    virtual bool haveWeWon() const { return FALSE; }

    void readPiles(Config &cfg);
    void writePiles(Config &cfg);

    CardPile *closestPile(QPoint p, int maxDistance);

protected:
    CardPile *undealtPile;

private:
    int jokers;
    int decks;
    int cards;
    bool cardsCreated;
};


#endif

