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
#ifndef FREECELL_CARD_GAME_H 
#define FREECELL_CARD_GAME_H 


#include "patiencecardgame.h"


extern int numberOfFreeCells;


class FreecellDiscardPile : public PatienceDiscardPile
{
public:
    FreecellDiscardPile(QPoint p, QString name, QCanvas *canvas) :
    	PatienceDiscardPile(p, name, canvas) { }

};


class FreecellWorkingPile : public PatienceWorkingPile
{
public:
    FreecellWorkingPile(QPoint p, QString name, QCanvas *canvas) :
    	PatienceWorkingPile(p, name, canvas) { }

     virtual bool isAllowedOnTop(Card *card) {
	if ( cardOnBottom() == NULL ) {
	    int numberOfCardsBeingMoved = 0;
	    Card *tempCard = card;

	    while ((tempCard != NULL)) {
		numberOfCardsBeingMoved++;
		tempCard = cardInfront(tempCard); 
	    }
       
	    if (numberOfCardsBeingMoved > numberOfFreeCells)
		return FALSE;
	}

	if ( card->isFacing() &&
	      cardOnTop() == NULL )
	    return TRUE;
        return PatienceWorkingPile::isAllowedOnTop( card );	
    }
    
    virtual bool isAllowedToBeMoved(Card *card) {
	int nextExpectedValue = (int)card->value();
	bool nextExpectedColor = card->isRed();
	int numberOfCardsBeingMoved = 0;
	
	while ((card != NULL)) {
	    numberOfCardsBeingMoved++;
	    if ( (int)card->value() != nextExpectedValue )
		return FALSE;
	    if ( card->isRed() != nextExpectedColor )
		return FALSE;
	    nextExpectedValue--;;
	    nextExpectedColor = !nextExpectedColor;
	    card = cardInfront(card); 
	}
       
	if (numberOfCardsBeingMoved <= (numberOfFreeCells + 1))
	    return TRUE;
	
	return FALSE;
    }
    virtual void cardRemoved(Card *card) {
	if ( !isDealing() &&  !cardOnTop() )
	    numberOfFreeCells++;
	PatienceWorkingPile::cardRemoved( card );
    }
    virtual void cardAddedToTop(Card *card) {
	if ( !isDealing() && cardOnBottom() == card )
	    numberOfFreeCells--;
	PatienceWorkingPile::cardAddedToTop( card );
    }
};


class FreecellFreecellPile : public CardPile, public CanvasRoundRect
{
public:
    FreecellFreecellPile(QPoint p, QString name, QCanvas *canvas)
        : CardPile(p, name), CanvasRoundRect(p, canvas) { }
    virtual bool isAllowedOnTop(Card *card) {
	if ( ( cardOnTop() == NULL ) && ( card->cardPile()->cardInfront(card) == NULL  ) )
	    return TRUE;
        return FALSE;	
    }
    virtual bool isAllowedToBeMoved(Card *card) {
	Q_UNUSED(card);
    	return TRUE;
    }
    virtual bool isSelectable(Card *card) {
	Q_UNUSED(card);
	return true;
    }
    virtual void cardAddedToTop(Card *card) {
	Q_UNUSED(card);
	numberOfFreeCells--;
    }
    virtual void cardRemoved(Card *card) {
	Q_UNUSED(card);
	numberOfFreeCells++;
    }
};


class FreecellCardGame : public CanvasCardGame
{
    Q_OBJECT
public:
    FreecellCardGame(QCanvas *c, QCanvasView *view, bool snap, bool casinoRules);
    virtual QString gameName() const { return QObject::tr("Freecell"); }
    virtual void deal(void);
    virtual bool haveWeWon() const { 
	return ( discardPiles[0]->kingOnTop() &&
		 discardPiles[1]->kingOnTop() &&
		 discardPiles[2]->kingOnTop() &&
		 discardPiles[3]->kingOnTop() );
    }
    virtual int pileForKey(int curPile, int key);
    virtual bool pileSelected(CardPile *pile) { Q_UNUSED(pile); return false; }
    virtual bool cardSelected(Card *card);
    void readConfig( Config& cfg );
    void writeConfig( Config& cfg );
    bool snapOn;
private:
    FreecellFreecellPile *freecellPiles[8];
    FreecellWorkingPile *workingPiles[8];
    FreecellDiscardPile *discardPiles[4];
};


#endif

