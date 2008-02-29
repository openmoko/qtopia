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

#include "cardgame.h"
#include "canvasshapes.h"
#include "canvascard.h"
#include "canvascardgame.h"
#include "cardmetrics.h"

#include <qpe/resource.h>
#include <qpe/config.h>

#include <qmainwindow.h>
#include <qpe/qpemenubar.h>
#include <qpainter.h>

#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <math.h>


extern int highestZ;


class CanvasCardPile : public QCanvasRectangle
{
public:
    CanvasCardPile( CanvasCardGame *ccg, QCanvas *canvas ) : QCanvasRectangle( canvas ), parent( ccg ) { 
	pile = new QPixmap( 0, 0 );
	firstCard = NULL;
    }

    void addCards( CanvasCard *card );
    void advance(int stage);
    void animatedMove() { animatedMove(savedX, savedY); }
    void savePos(void) { savedX = (int)x(); savedY = (int)y(); }
    void animatedMove(int x2, int y2, int steps = 7 );

protected:
    virtual void draw( QPainter& p );

private:
    CanvasCardGame *parent;
    QPixmap *pile;
    QImage doubleBuffer;
    CanvasCard *firstCard;
    int destX, destY;
    int savedX, savedY;
    int animSteps;
};


void CanvasCardPile::addCards( CanvasCard *card )
{
    int depth = 0;

    // Count the cards under the first card
    CanvasCard *tmpCard = card;
    while (tmpCard) {
	tmpCard->hide();
	tmpCard = (CanvasCard *)tmpCard->getCardPile()->cardInfront(tmpCard);
	depth++;
    }

    firstCard = card;
    int height = CardMetrics::height() + (depth - 1) * CardMetrics::offsetDown();
    int width  = CardMetrics::width();

    setSize( width, height );

    QImage blank(width,height,32);
    blank.setAlphaBuffer(TRUE);
    for ( int i = 0; i < height; i++ ) {
	QRgb *linePtr = (QRgb*)blank.scanLine( i );
	for ( int j = 0; j < width; j++ ) {
	    linePtr[j]=qRgba(0,0,0,1);
	}
    }
    pile->convertFromImage(blank);

    {
	QPainter p( pile );
	p.translate( -card->x(), -card->y() );

	while (card) {
	    card->draw( p );
	    card = (CanvasCard *)card->getCardPile()->cardInfront(card);
	}
    }

    doubleBuffer = pile->convertToImage().convertDepth( 32 );

    if ( doubleBuffer.depth() == 32 ) { // 32-bit depth support, we can do alpha effects
	doubleBuffer.setAlphaBuffer( TRUE );
	for ( int i = 0; i < height; i++ ) {
	    QRgb *linePtr = (QRgb*)doubleBuffer.scanLine( i );
	    int alpha = 255 - i * 220 / height;
	    for ( int j = 0; j < width; j++ ) {
		const QRgb p = linePtr[j];
		if ( qAlpha(p) > alpha )
		    linePtr[j] = qRgba(qRed(p),qGreen(p),qBlue(p),alpha);
	    }
	}
    }
}


void CanvasCardPile::advance(int stage)
{
    if ( stage==1 ) {
	if ( animSteps-- <= 0 ) {
	    CanvasCard *item = firstCard;
	    while (item) {
		item->show();
		item = (CanvasCard *)item->getCardPile()->cardInfront(item);
	    }
	    setVelocity(0,0);
	    setAnimated(FALSE);
	    parent->cancelMoving();
	    hide();
	    move(destX,destY); // exact
	}
    }
    QCanvasRectangle::advance(stage);
}


void CanvasCardPile::animatedMove(int x2, int y2, int steps = 7 )
{
    destX = x2;
    destY = y2;

    double x1 = x(), y1 = y(), dx = x2 - x1, dy = y2 - y1;

    // Ensure a good speed
    while ( fabs(dx/steps)+fabs(dy/steps) < 5.0 && steps > 4 )
	steps--;

    setAnimated(TRUE);
    setVelocity(dx/steps, dy/steps);

    animSteps = steps;
}


void CanvasCardPile::draw( QPainter& p )
{
    p.drawImage( (int)x(), (int)y(), doubleBuffer );
}


CanvasCardGame::~CanvasCardGame() {
    // the deletion stuff should be fixed now and only deletes
    // items created by this CardGame. I haven't verified there are zero
    // memory leaks yet
    if ( alphaCardPile )
	delete alphaCardPile;
}


void CanvasCardGame::gameWon() {
	
    srand(time(NULL));
    
    QCanvasItemList list = canvas()->allItems();
    QCanvasItemList::Iterator it = list.begin();
 
    for (; it != list.end(); ++it) {
	if ( (*it)->rtti() == canvasCardId ) {
	    // disperse the cards everywhere
	    int x = 300 - rand() % 1000;
	    int y = 300 + rand() % 200;
	    ((CanvasCard *)*it)->animatedMove( x, y, 50 );
	}
    }
}


void CanvasCardGame::contentsMousePressEvent(QMouseEvent *e) {

    if ( moving )
	return;

    QCanvasItemList l = canvas()->collisions( e->pos() );
    
    for (QCanvasItemList::Iterator it = l.begin(); it != l.end(); ++it) {
	
	if ( (*it)->rtti() == canvasCardId ) {
	
	    moving = (CanvasCard *)*it;

	    if ( moving->animated() )
		return;
		
	    cardXOff = (int)(e->pos().x() - moving->x());
	    cardYOff = (int)(e->pos().y() - moving->y());
	    
	    if ( !mousePressCard( moving, e->pos() ) ) {
		CanvasCard *card = moving;

		if ( alphaCardPile )
		    delete alphaCardPile;

		alphaCardPile = new CanvasCardPile( this, canvas() );
		alphaCardPile->move( card->x(), card->y() );
		alphaCardPile->savePos();
		alphaCardPile->show();
		alphaCardPile->addCards( card );
		alphaCardPile->setZ( INT_MAX );

		moved = TRUE;		
	    } else {
		if ( alphaCardPile )
		    alphaCardPile->hide();
	    }
	    return;	
	}
    }
    
    mousePress( e->pos() );
}

/*
//
// Should have some intelligent way to make double clicking on a
// card send it to the most appropriate pile
//
void CanvasCardGame::contentsMouseDoubleClickEvent(QMouseEvent *e) {
    QCanvasItemList l = canvas()->collisions( e->pos() );
    for (QCanvasItemList::Iterator it = l.begin(); it != l.end(); ++it) {
	if ( (*it)->rtti() == canvasCardId ) {
	    CanvasCard *card = (CanvasCard *)*it;

	    if ( card->animated() )
		return;
	    
	    if ( card->getCardPile()->isAllowedToBeMoved(card) ) {
		if (card->getCardPile()->cardInfront(card) == NULL) {
		    CardPile *pile = first();
		    if (pile && pile->isAllowedOnTop(card)) {
		       // move card to this pile
		       return;
		    }
		}
	    }
	}
    }
}
*/

void CanvasCardGame::contentsMouseMoveEvent(QMouseEvent *e) {
    
    QPoint p = e->pos();
    
    if ( moving ) {

	moved = TRUE;

	if (moving->isFacing() != TRUE)
	    return;

	int tx = (int)p.x() - cardXOff; 
	int ty = (int)p.y() - cardYOff;

	if (snapOn == TRUE) {
	    CardPile *pile = closestPile( tx, ty, 50 );
	    if ( pile && pile->isAllowedOnTop( moving ) ) {
		    QPoint p = pile->getHypertheticalNextCardPos();
		    if ( alphaCardPile )
			alphaCardPile->move( p.x(), p.y() );
		    return;
	    }
	}

	if ( alphaCardPile )
	    alphaCardPile->move( tx, ty );
    }
    
}


void CanvasCardGame::contentsMouseReleaseEvent(QMouseEvent *e)
{ 
    QPoint p = e->pos();

    Q_UNUSED(p);

    if ( moving ) {
	
	CanvasCard *item = moving;

	if ( item->animated() )
	    return;
	
	if ( alphaCardPile )
	if ( moved ) {
	    
	    CardPile *pile = closestPile((int)alphaCardPile->x(), (int)alphaCardPile->y(), 30);

	    if (pile && pile->isAllowedOnTop(item)) {
		    CardPile *oldPile = item->getCardPile();
		    Card *c = NULL;
		    if ( oldPile != pile) {
			while ( item ) {
			    item->show();
			    if ( oldPile ) {
				c = oldPile->cardInfront(item);
				oldPile->removeCard(item);
			    }    
			    pile->addCardToTop(item);
			    item->setCardPile(pile);
			    //item->move( pile->getCardPos(item) );
			    QPoint p = pile->getCardPos(item);
			    item->setPos( p.x(), p.y(), highestZ );
			    highestZ++;

			    if (item->getValue() == king && haveWeWon()) {
				alphaCardPile->hide();
				gameWon();
				moving = NULL;
				return;
			    }
			    
			    if (oldPile) {
				item = (CanvasCard *)c;
			    } else {
				item = NULL;
			    }
			}	
			alphaCardPile->hide();
			moving = NULL;
			return;
		    }
	    }

	    alphaCardPile->animatedMove();
	}
    }

    moved = FALSE;
}


void CanvasCardGame::readPile( Config& cfg, CardPile *pile, QString name, int& highestZ )
{
    cfg.setGroup( name );
    int numberOfCards = cfg.readNumEntry("NumberOfCards", 0);
    Card *card = NULL;
    for ( int i = 0; i < numberOfCards; i++ ) {
	QString cardStr;
	cardStr.sprintf( "%i", i );
	int val = cfg.readNumEntry( "Card" + cardStr );
	bool facing = cfg.readBoolEntry( "CardFacing" + cardStr );
	card = cards[ val ];
	card->setFace(facing);
	pile->addCardToTop(card);
	card->setCardPile(pile);
	QPoint p = pile->getCardPos( card );
	((CanvasCard*)card)->setPos( p.x(), p.y(), highestZ );
	((CanvasCard*)card)->showCard();
	highestZ++;
    }
}


