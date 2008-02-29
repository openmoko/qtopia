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
#ifndef PATIENCE_CARD_GAME_H
#define PATIENCE_CARD_GAME_H 


#include <qpopupmenu.h>
#include <qmainwindow.h>
#include <qintdict.h>
#include <qcanvas.h>
#include "canvascardgame.h"
#include "canvascardpile.h"
#include "cardmetrics.h"


class PatienceFaceDownDeck : public CanvasCardPile
{
public:
    PatienceFaceDownDeck(QPoint p, QString name, QCanvas *canvas)
        : CanvasCardPile(p, name, canvas) { }
    virtual bool isAllowedOnTop(Card *card) {
	Q_UNUSED(card);
	// Need to check it is from the faceUpDealingPile
	return TRUE;
    }
    virtual bool isAllowedToBeMoved(Card *card) {
	Q_UNUSED(card);
	//if ( ( !card->isFacing() ) && ( card == cardOnTop() ) )
	if ( card == cardOnTop() )
	    return TRUE;
        return FALSE;	
    }
    virtual bool isSelectable(Card *card) {
	return card->isFacing();
    }
};


class PatienceFaceUpDeck : public CanvasCardPile
{
public:
    PatienceFaceUpDeck(QPoint p, QString name, QCanvas *canvas)
        : CanvasCardPile(p, name, canvas) { }
    virtual bool isAllowedOnTop(Card *card) {
	Q_UNUSED(card);
	// Need to check it is from the faceDownDealingPile
	return TRUE;
    }
    virtual bool isAllowedToBeMoved(Card *card) {
	Q_UNUSED(card);
	//if ( ( card->isFacing() ) && ( card == cardOnTop() ) )
	if ( card == cardOnTop() )
	    return TRUE;
        return FALSE;	
    }
    virtual bool isSelectable(Card *card) {
	return card == cardOnTop();
    }
};


class PatienceDiscardPile : public CanvasCardPile
{
public:
    PatienceDiscardPile(QPoint p, QString name, QCanvas *canvas)
        : CanvasCardPile(p, name, canvas) { }
    virtual bool isAllowedOnTop(Card *card) {
	if ( card->isFacing() && ( card->cardPile()->cardInfront(card) == NULL ) &&
	   ( ( ( cardOnTop() == NULL ) && ( card->value() == Card::ace ) ) ||
	     ( ( cardOnTop() != NULL ) &&
//#define CHEAT_TEST_MODE
#ifndef CHEAT_TEST_MODE
	       ( (int)card->value() == (int)cardOnTop()->value() + 1 ) &&
#endif
	       ( card->suit() == cardOnTop()->suit() ) ) ) )
	    return TRUE;
        return FALSE;	
    }
    virtual bool isAllowedToBeMoved(Card *card) {
	if ( card->isFacing() && ( card == cardOnTop() ) )
	    return TRUE;
        return FALSE;	
    }
    virtual bool isSelectable(Card *card) {
	return card == cardOnTop();
    }
    virtual void drawShape(QPainter &p) {
	int ix = (int)x();
	int iy = (int)y();
	int w = width();
	int h = height();
	int rankW = CardMetrics::rankWidth();
	int rankH = CardMetrics::rankHeight();
	int picturesW = CardMetrics::picturePixmap()->width() / 12;
	int picturesH = CardMetrics::picturePixmap()->height();
	int vmarg = (h - picturesH * 2) / 2; 
	int hmarg = (w - picturesW) / 2;
	int r1x = (hmarg - rankW) / 2;
	Q_UNUSED(vmarg);
#ifndef QTOPIA_PHONE
	int r1y = r1x + 2;
#else
	int r1y = r1x + 1;
#endif
	p.drawPixmap( ix + r1x, iy + r1y, *CardMetrics::rankPixmap(), 0, 0, rankW, rankH );
#ifndef QTOPIA_PHONE
	int r2x = w - r1x - rankW;
	int r2y = h - r1y - rankH;
	p.drawPixmap( ix + r2x, iy + r2y, *CardMetrics::rankIPixmap(), rankW*12, 0, rankW, rankH );
#endif
	CanvasRoundRect::drawShape(p);
    }
};


class PatienceWorkingPile : public CanvasCardPile
{
public:
    PatienceWorkingPile(QPoint p, QString name, QCanvas *canvas)
        : CanvasCardPile(p, name, canvas), top(p) { }
    virtual bool isAllowedOnTop(Card *card) {
	if ( card->isFacing() &&
	     ( ( ( cardOnTop() == NULL ) && (card->value() == Card::king) ) ||
	       ( ( cardOnTop() != NULL ) &&
#ifndef CHEAT_TEST_MODE
 	         ( (int)card->value() + 1 == (int)cardOnTop()->value() ) &&
#endif
	         ( card->isRed() != cardOnTop()->isRed() ) ) ) )
	    return TRUE;
        return FALSE;
    }
    virtual bool isAllowedToBeMoved(Card *card) {
	if ( card->isFacing() )
	    return TRUE;
        return FALSE;	
    }
    virtual bool isSelectable(Card *card) {
	return card->isFacing();
    }
    virtual void cardAddedToTop(Card *card) {
	Q_UNUSED(card);
	top = cardPos(NULL);
	setNextPos( top );
    }
    virtual void cardRemoved(Card *card) {
	Q_UNUSED(card);

	Card *newTopCard = cardOnTop();

	if ( !newTopCard ) {
	    top = pos();
	} else {
	    top = cardPos(NULL);
	    if ( newTopCard->isFacing() == FALSE ) {
		int offsetDown = CardMetrics::offsetDown();
		// correct the position taking in to account the card is not
    		// yet flipped, but will become flipped
		if ( CardMetrics::width() > 22 )
		    top = QPoint( top.x() - 1, top.y() - 3 );
		else
		    top = QPoint( top.x(), top.y() - 3 );
		newTopCard->flipTo(top, 400);
		top = QPoint( top.x(), top.y() + offsetDown );
	    }
	}
	setNextPos( top );
    }
    virtual QPoint cardPos(Card *c) const {
	int x = pos().x(), y = pos().y();
	Card *card = cardOnBottom();
	while ((card != c) && (card != NULL)) {
	    if (card->isFacing()) {
		int offsetDown = CardMetrics::offsetDown();
		y += offsetDown; 
	    } else {
		if ( CardMetrics::width() > 22 )
		    x += 1;
		y += 3;
	    }
	    card = cardInfront(card); 
	}
	return QPoint( x, y );
    }
    virtual QPoint hypertheticalNextCardPos(void) const {
	return top;
    }
private:
    QPoint top;

};


class PatienceCardGame : public CanvasCardGame
{
    Q_OBJECT
public:
    PatienceCardGame(QCanvas *c, QCanvasView *view, bool snap, bool casinoRules);
    virtual ~PatienceCardGame();
    virtual QString gameName() const { return QObject::tr("Patience"); }
    virtual void deal(void);
    virtual bool haveWeWon() const { 
	return ( discardPiles[0]->kingOnTop() &&
		 discardPiles[1]->kingOnTop() &&
		 discardPiles[2]->kingOnTop() &&
		 discardPiles[3]->kingOnTop() );
    }

    void flipOverCards();
    void turnOverDeck();

    virtual int pileForKey(int curPile, int key);
    virtual bool cardSelected(Card *card);
    virtual bool pileSelected(CardPile *pile);
    bool canTurnOverDeck(void) { return !casinoRules() || numberOfTimesThroughDeck < 3; }
    virtual void casinoRulesChanged() {
	setCircleCrossState();
    }
    void throughDeck(void) {
	numberOfTimesThroughDeck++;
	setCircleCrossState();
    }
    void setCircleCrossState() {
	if (canTurnOverDeck())
    	    circleCross->setCircle();
	else
    	    circleCross->setCross();
    }
    virtual void writeConfig( Config& cfg );
    virtual void readConfig( Config& cfg );
private:
    CanvasCircleOrCross *circleCross;
    CanvasRoundRect *rectangle;
    PatienceWorkingPile *workingPiles[7];
    PatienceDiscardPile *discardPiles[4];
    PatienceFaceDownDeck *faceDownDealingPile;
    PatienceFaceUpDeck *faceUpDealingPile;
    int numberOfTimesThroughDeck;
};


#endif

