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

#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qtopia/contextbar.h>
#include <qmessagebox.h>
#include <stdlib.h>
#include "canvasshapes.h"
#include "canvascard.h"
#include "canvascardgame.h"
#include "canvascardpile.h"
#include "cardmetrics.h"
#include "canvasitemtimer.h"


CanvasCardGame::CanvasCardGame(QCanvas *c, QCanvasView *view, bool snap, bool casinoRules) :
	QObject(view),
	moving(0),
	alphaCardPile(c),
	cardOffset(0,0),
	canvasPtr(c),
	canvasView(view),
	state(SelectingCard),
	selectingCardRect(QPoint(0, 0), c),
	selectingCardRectGhost(QPoint(0, 0), c),
	casino(casinoRules),
	snapOn(snap),
	numberToDraw(1)
{
    selectingCardRect.setSelectionMode(CanvasRoundRect::enabledSelection);
    selectingCardRect.setZ(INT_MAX - 20);

    selectingCardRectGhost.setSelectionMode(CanvasRoundRect::partiallySelected);
    selectingCardRectGhost.setZ(INT_MAX - 21);

    alphaCardPile.hide();
    alphaCardPile.setZ(INT_MAX - 30);
    connect(&alphaCardPile,SIGNAL(finished()),this,SLOT(doneMoving()));

    selectingCard = 0;
    selectingCardI = 0;
    selectingPileI = -1;
}


CanvasCardGame::~CanvasCardGame()
{
}


void CanvasCardGame::gameWon()
{
    ((CanvasCardView *)canvasView)->setZoomViewEnabled(FALSE);
    ((CanvasCardView *)canvasView)->showWinText(TRUE);

    srand(time(NULL));
    
    QCanvasItemList list = canvas()->allItems();
    QCanvasItemList::Iterator it = list.begin();

    for (; it != list.end(); ++it) {
	if ( (*it)->rtti() == canvasCardId ) {
	    // disperse the cards everywhere
	    int x = (rand() % (canvas()->width() * 2)) - (canvas()->width()/2);
	    int y = canvas()->height() + (rand() % (canvas()->height() / 3));
	    ((CanvasCard *)*it)->moveTo(QPoint(x, y), 3000);
	}
    }
}


void CanvasCardGame::contentsMousePressEvent(QMouseEvent *e)
{
    if ( moving )
	return;

    QCanvasItemList l = canvas()->collisions( e->pos() );
    
    for (QCanvasItemList::Iterator it = l.begin(); it != l.end(); ++it) {
	if ( (*it)->rtti() == canvasCardId ) {
	    CanvasCard *card = (CanvasCard *)*it;

	    if ( card->isFlipping() )
		continue;
	
	    selectingCardI = find(card->cardPile()) - 1;
	    selectingCard = card;
	    state = SelectingPile;

	    if ( cardSelected(card) ) {
		alphaCardPile.hide();
		resetCardSelection(CurrentCard);
	    } else {
		beginMovingCards(card);
		cardOffset = e->pos() - card->pos();
		updateCurSelectingCard();
	    }
	    return;
	}
    }

    for (CardPile *p = first(); p; p = next()) {
	if (p->pileRect().contains(e->pos())) {
	    if (pileSelected(p)) {
		selectingCardI = find(p) - 1;
		selectingCard = 0;
		state = SelectingPile;
		resetCardSelection(CurrentCard);
		return;
	    }
	}
    }
}


// ### Should have some intelligent way to make double clicking on a
// card send it to the most appropriate pile
void CanvasCardGame::contentsMouseDoubleClickEvent(QMouseEvent *e)
{
    QCanvasItemList l = canvas()->collisions( e->pos() );
    for (QCanvasItemList::Iterator it = l.begin(); it != l.end(); ++it) {
	if ( (*it)->rtti() == canvasCardId ) {
//	    autoMove((Card *)*it);
	}
    }
}


void CanvasCardGame::contentsMouseMoveEvent(QMouseEvent *e)
{
    updateMovingCards(e->pos() - cardOffset);
}


void CanvasCardGame::contentsMouseReleaseEvent(QMouseEvent *)
{ 
    endMovingCards();
}


void CanvasCardGame::beginMovingCards(CanvasCard *card)
{
    if ( moving )
	return;
    moving = card;
    alphaCardPile.addCards(moving);
    alphaCardPile.show();
    cardOffset = QPoint(0,0);
}


void CanvasCardGame::updateMovingCards(QPoint pnt)
{
    if ( !moving )
	return;
    if ( snapOn ) {
	CardPile *pile = closestPile(pnt, CardMetrics::width()/2);
	if ( pile && pile->isAllowedOnTop(moving) )
	    pnt = pile->hypertheticalNextCardPos();
    }
    alphaCardPile.setPos(pnt);
}


void CanvasCardGame::cancelMovingCards()
{
    alphaCardPile.animatedMove();
    resetCardSelection(Last);
}


void CanvasCardGame::endMovingCards()
{
    // Check it is okay to be here
    if ( !moving )
	return;

    CardPile *pile = closestPile(alphaCardPile.pos(), CardMetrics::width() / 2);
    CardPile *oldPile = moving->cardPile();

    // Will it be valid to move here or should we reject and reset this move
    if ( !pile || !pile->isAllowedOnTop(moving) || oldPile == pile ) {
	cancelMovingCards();
	return;
    }

    // If it is valid, start moving the cards to the new pile
    Card *item = moving;
    while ( item ) {
	Card *next = 0;
	item->setVisible(true);
	if ( oldPile ) {
	    next = oldPile->cardInfront(item);
	    oldPile->removeCard(item);
	}
	pile->addCardToTop(item);
	item = next;
    }

    selectingPileI = find(pile) - 1;

    // Finish up and check if this move causes us to win the game
    alphaCardPile.hide();
    moving = 0;
    if ( haveWeWon() )
	gameWon();

    resetCardSelection(CurrentPile);
}


void CanvasCardGame::doneMoving()
{
    moving = 0;
    updateSelectionRects();
    emit(moveFinished());
}


CanvasCardPile *CanvasCardGame::canvasCardPileFromInt( int pile )
{
    // pile+1 to take into account invisible dealing pile
    return (CanvasCardPile*)at(pile+1);
}


void CanvasCardGame::resetCardSelection( Movement movement )
{
#ifdef QTOPIA_PHONE
    ContextBar::setLabel(canvasView, Key_Back, ContextBar::Back, ContextBar::Modal);
#endif

    switch ( movement ) {
	case CurrentCard:
	    selectingCard = 0;
	    break;
	case CurrentPile:
	    selectingCard = 0;
	    selectingCardI = selectingPileI;
	    break;
	case Last:
	    selectingPileI = selectingCardI;
	    break;
    }
    updateCurSelectingCard();
    state = SelectingCard;
}


void CanvasCardGame::updateSelectionRects()
{
    switch ( state ) {
	case SelectingCard:
	    updateCurSelectingCard();
	    break;
	case SelectingPile:
	    updateCurSelectingPile();
	    break;
	default:
	    break;
    }
}


void CanvasCardGame::updateCurSelectingCard()
{
    int h = 0;
    if ( !selectingCard ) 
	selectingCard = canvasCardPileFromInt(selectingCardI)->cardOnTop();
    selectingCardRect.setSelectionMode(CanvasRoundRect::enabledSelection);
    if ( selectingCard ) {
	Card *prevCard2 = selectingCard;
	Card *nextCard2 = CardPile::cardInfront(prevCard2);
	while ( nextCard2 ) {
	    prevCard2 = nextCard2;
	    nextCard2 = CardPile::cardInfront(prevCard2);
	}
	h = prevCard2->pos().y() - selectingCard->pos().y();
	selectingCardRect.setPos(selectingCard->pos());
	if ( !selectingCard->canBeMoved() )
	    selectingCardRect.setSelectionMode(CanvasRoundRect::disabledSelection);
    } else {
	selectingCardRect.setPos(canvasCardPileFromInt(selectingCardI)->pos());
    }
    selectingCardRect.setSize(CardMetrics::width(),CardMetrics::height()+h);
    selectingCardRectGhost.move(selectingCardRect.x(),selectingCardRect.y());
    selectingCardRectGhost.setSize(CardMetrics::width(),CardMetrics::height()+h);
}


void CanvasCardGame::updateCurSelectingPile()
{
    QRect r = canvasCardPileFromInt(selectingPileI)->boundingRect();
    int y = r.height() - CardMetrics::height();
    int x = r.width() - CardMetrics::width();

    if ( selectingCardI == selectingPileI ) {
	y += CardMetrics::height();
	y -= alphaCardPile.height() + CardMetrics::offsetDown();
    }
    selectingCardRect.setPos(QPoint(r.x()+x, r.y()+y));
    updateMovingCards(r.topLeft()+QPoint(x,y));
}


void CanvasCardGame::setupSelectingPile()
{
    if ( moving )
	return;
    if ( !pileSelected(canvasCardPileFromInt(selectingCardI)) ) {
#ifdef QTOPIA_PHONE
        ContextBar::setLabel(canvasView, Key_Back, ContextBar::Cancel, ContextBar::Modal);
#endif
	state = SelectingPile;

	if ( selectingCard && selectingCard->canBeMoved() ) {
	    beginMovingCards((CanvasCard*)selectingCard);
	    selectingPileI = selectingCardI;
	    return;
	}
    }
    resetCardSelection(CurrentCard);
}


void CanvasCardGame::keyPressEvent(QKeyEvent *ke)
{
    switch ( state ) {
	case SelectingCard:
	    switch ( ke->key() ) {
		case Key_Left:
		    selectingCard = 0;
		    selectingCardI = pileForKey(selectingCardI,0);
		    break;
		case Key_Right:
		    selectingCard = 0;
		    selectingCardI = pileForKey(selectingCardI,1);
		    break;
		case Key_Up:
		    if ( selectingCard ) 
			selectingCard = CardPile::cardBehind(selectingCard);
		    if ( !selectingCard || !selectingCard->isSelectable() ) {
			selectingCard = 0;
			selectingCardI = pileForKey(selectingCardI,2);
		    }
		    break;
		case Key_Down:
		    if ( selectingCard ) 
			selectingCard = CardPile::cardInfront(selectingCard);
		    if ( !selectingCard || !selectingCard->isSelectable() ) {
			selectingCard = 0;
			selectingCardI = pileForKey(selectingCardI,3);
		    }
		    if ( !selectingCard ) {
			selectingCard = canvasCardPileFromInt(selectingCardI)->cardOnTop();
			Card *next = selectingCard;
			while ( next && next->isSelectable() ) {
			    selectingCard = next;
			    next = CardPile::cardBehind(next);
			}
		    }
		    break;
#ifdef QTOPIA_PHONE
		case Key_Back:
		case Key_No:
#endif
		case Key_Escape:
		    selectingCard = 0;
#ifdef QTOPIA_PHONE
		    // Causes app to be closed in phone mode
		    if( !Global::mousePreferred() )
			canvasView->setModalEditing(false);
		    else
			ke->ignore();
#endif
		    return;
#ifdef QTOPIA_PHONE
		case Key_Select:
#endif
		case Key_Space:
		case Key_Enter:
		    setupSelectingPile();
		    return;
	    }
	    updateCurSelectingCard();
	    return;
	case SelectingPile:
	    switch ( ke->key() ) {
		case Key_Left:
		    selectingPileI = pileForKey(selectingPileI,4);
		    break;
		case Key_Right:
		    selectingPileI = pileForKey(selectingPileI,5);
		    break;
		case Key_Up:
		    selectingPileI = pileForKey(selectingPileI,6);
		    break;
		case Key_Down:
		    selectingPileI = pileForKey(selectingPileI,7);
		    break;
#ifdef QTOPIA_PHONE
		case Key_Back:
		case Key_No:
#endif
		case Key_Escape:
		    cancelMovingCards();
		    return;
#ifdef QTOPIA_PHONE
		case Key_Select:
#endif
		case Key_Space:
		case Key_Enter:
		    // Move moving pile to drop location or back to start loc.
		    endMovingCards();
		    return;
	    }
	    updateCurSelectingPile();
	    return;
    }
}

