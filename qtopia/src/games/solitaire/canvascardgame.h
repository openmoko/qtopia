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
#ifndef CANVAS_CARD_GAME_H
#define CANVAS_CARD_GAME_H

#include <qtopia/qpemenubar.h>
#include <qtopia/resource.h>
#include <qtopia/config.h>
#include <qmainwindow.h>
#include <qpainter.h>
#include <qcanvas.h>
#include <qwidget.h>
#include <stdlib.h>
#include <time.h>

#include "cardgame.h"
#include "canvasshapes.h"
#include "canvascard.h"
#include "canvascardpile.h"
#include "canvascardwindow.h"


class CanvasCardWindow;
class QCanvas;


class CanvasCardGame : public QObject, public CardGame
{
    Q_OBJECT
public:
friend class CanvasCardView;
    CanvasCardGame(QCanvas *c, QCanvasView *view, bool snap, bool casinoRules);
    virtual ~CanvasCardGame();

    virtual Card *newCard( Card::Value v, Card::Suit s, bool f ) {
	CanvasCard *card = new CanvasCard( v, s, f, canvas() );
	connect(card,SIGNAL(finished()),this,SLOT(doneMoving()));
	return card;
    }

    virtual void gameWon();

    enum Movement { CurrentCard, CurrentPile, Last };
    void resetCardSelection( Movement movement );
    CanvasCardPile *canvasCardPileFromInt( int pile );
    QRect selectionRect() {
	if ( state == SelectingPile ) {
	    QRect r(selectingCardRect.rect());
	    r.setTop(int(selectingCardRect.y())-3);
	    return r;
	}
	return selectingCardRect.rect();
    }

    virtual int pileForKey(int curPile, int key) { Q_UNUSED(key); return curPile; }
    virtual bool cardSelected(Card *card) { Q_UNUSED(card); return false; }
    virtual bool pileSelected(CardPile *pile) { Q_UNUSED(pile); return false; }

    void setSnap(bool b) { snapOn = b; }
    bool snap() { return snapOn; }

    void setCasinoRules(bool b) { casino = b; casinoRulesChanged(); }
    bool casinoRules() { return casino; }
    virtual void casinoRulesChanged() { }

    void setCardsDrawn(int n) { numberToDraw = n; }
    int cardsDrawn() { return numberToDraw; }

    void beginMovingCards(CanvasCard *card);
    void updateMovingCards(QPoint p);
    void cancelMovingCards();
    void endMovingCards();

public slots:
    void doneMoving();

signals:
    void moveFinished();

protected:
    void keyPressEvent(QKeyEvent *ke);
    void contentsMousePressEvent(QMouseEvent *e);
    void contentsMouseReleaseEvent(QMouseEvent *e);
    void contentsMouseDoubleClickEvent(QMouseEvent *e);
    void contentsMouseMoveEvent(QMouseEvent *e);

protected:
    // Mouse event state variables
    CanvasCard *moving;
    AnimatedCanvasCardPile alphaCardPile;
    QPoint cardOffset;

    // ###
    QCanvas *canvasPtr;
    QCanvasView *canvasView;
    QCanvas *canvas() { return canvasPtr; }

private:
    enum State { SelectingCard, SelectingPile };

    void setupSelectingPile();

    void updateSelectionRects();

    void updateCurSelectingCard();
    void updateCurSelectingPile();

    State state;
    int selectingCardI;
    int selectingPileI;
    Card *selectingCard;
    CanvasRoundRect selectingCardRect;
    CanvasRoundRect selectingCardRectGhost;

    bool casino;
    bool snapOn;
    int numberToDraw;
};


#endif

