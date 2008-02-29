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
#include "canvascard.h"
#include "cardmetrics.h"
#include <qpe/resource.h>
#include <qpainter.h>
#include <qimage.h>
#include <qpaintdevice.h>
#include <math.h>

#if defined( QT_QWS_CASSIOPEIA )
#define SLOW_HARDWARE
#endif

// Seems to be fast enough to me even without Transformations in the library
//#if defined( QT_NO_TRANSFORMATIONS ) && defined( QT_QWS_IPAQ )
//#define SLOW_HARDWARE
//#endif

QPixmap *CreateScaledPixmap(QPixmap *srcPixmap, double scaleX, double scaleY)
{
#ifdef QT_NO_TRANSFORMATIONS
    int w = srcPixmap->width();
    int h = srcPixmap->height();
    int newW = (int)(w * scaleX);
    int newH = (int)(h * scaleY);
    QPixmap *dstPixmap = new QPixmap( newW, newH );
    // ### this is very poorly implemented and probably could be much faster
    for (int i = 0; i < newW; i++) {
	int srcX = w * i / newW;
	if (newH == h) {
	    // Optimise for scaling in the X-axis only
	    bitBlt( dstPixmap, i, 0, srcPixmap, srcX, 0, 1, h );
	} else {
	    for (int j = 0; j < newH; j++) {
		int srcY = h * j / newH;
		bitBlt( dstPixmap, i, j, srcPixmap, srcX, srcY, 1, 1 );
	    }
        }
    }
    return dstPixmap;
#else
    QWMatrix s;
    s.scale( scaleX, scaleY );
    return new QPixmap( srcPixmap->xForm( s ) );
#endif
}


CanvasCard::CanvasCard( eValue v, eSuit s, bool f, QCanvas *canvas ) :
	Card(v, s, f), QCanvasRectangle( 0, 0, 1, 1, canvas ), scaleX(1.0), scaleY(1.0)
{
    xOff = CardMetrics::width() / 2;
    yOff = CardMetrics::height() / 2;
    setSize( CardMetrics::width(), CardMetrics::height() );
    flipping = FALSE;
}


void CanvasCard::cardBackChanged()
{
    if ( !isFacing() )
	redraw();
}


void CanvasCard::draw(QPainter &painter)
{
    int ix = (int)x();
    int iy = (int)y();
    int w = CardMetrics::width();
    int h = CardMetrics::height();
    int rank = getValue();
    int suit = getSuit();

    QPainter *p = &painter;
    QPixmap *unscaledCard = NULL;

    if ((scaleX <= 0.98) || (scaleY <= 0.98)) 
    {
	p = new QPainter();
	unscaledCard = new QPixmap( w, h );
	p->begin(unscaledCard);
	ix = 0;
	iy = 0;
    }
	
    if ( isFacing() ) {

	// Joker
	if ( rank == jokerVal ) {
	    // Draw Joker Card


	} else {

	    p->setBrush( NoBrush );
QPen pn0;
	    if ( isRed() == TRUE )
{
		p->setPen( QColor( 0xFF, 0, 0 ) );
pn0=QColor( 0xFF, 0, 0 );
}
	    else
{
		p->setPen( QColor( 0, 0, 0 ) );
pn0=QColor( 0, 0, 0 );
}

QPen pn=p->pen();
	    int suitW = CardMetrics::suitWidth();
	    int suitH = CardMetrics::suitHeight();
	    int suitSmlW = CardMetrics::suitSmallWidth();
	    int suitSmlH = CardMetrics::suitSmallHeight();
	    int rankW = CardMetrics::rankWidth();
	    int rankH = CardMetrics::rankHeight();
	    int spadeW = CardMetrics::spadeWidth();
	    int spadeH = CardMetrics::spadeHeight();
	    int picturesW = CardMetrics::picturePixmap()->width() / 12;
	    int picturesH = CardMetrics::picturePixmap()->height();

/*
	if ( cardSizes == micro ) {
	    p->drawPixmap( ix +      0, iy +     0, *cardsFaces );
	    p->drawPixmap( ix +      3, iy +     3, *cardsChars, 5*(value-1), 0, 5, 6 );
	    p->drawPixmap( ix +     11, iy +     3, *cardsSuits, 5*(suit-1), 0, 5, 6 );
	    p->drawPixmap( ix +  w-3-5, iy + h-3-6, *cardsCharsUpsideDown, 5*(12-value+1), 0, 5, 6 );
	    p->drawPixmap( ix + w-11-5, iy + h-3-6, *cardsSuitsUpsideDown, 5*(3-suit+1), 0, 5, 6 );
	} else {
*/

	    int hcent = (w - suitW) / 2;
	    int vmarg = (h - picturesH * 2) / 2; 
	    int hmarg = (w - picturesW) / 2;

	    int r1x, r1y, r2x, r2y, s1x, s1y, s2x, s2y;

	    r1x = (hmarg - rankW) / 2;
	    r1y = r1x + 2;
	    r2x = w - r1x - rankW - 1;
	    r2y = h - r1y - rankH;

	    if ( CardMetrics::offsetDown() <= 15 ) {
		s2x = (hmarg - suitSmlW) / 2 + 1;
		s2y = r2y - 1;
		s1x = w - s2x - suitSmlW - 1;
		s1y = r1y;
		r2x += 1;
	    } else {
		s1x = (hmarg - suitSmlW) / 2;
		s1y = s1x + rankH + 4;
		s2x = w - s1x - suitSmlW - 1;
		s2y = h - s1y - suitSmlH;
	    }

p->setPen(pn0);
p->setPen(pn);
	    p->drawPixmap( ix +   0, iy +   0, *CardMetrics::facePixmap() );
p->setPen(pn0);
p->setPen(pn);
	    p->drawPixmap( ix + r1x, iy + r1y, *CardMetrics::rankPixmap(), rankW*(rank-1), 0, rankW, rankH );
p->setPen(pn0);
p->setPen(pn);
	    p->drawPixmap( ix + r2x, iy + r2y, *CardMetrics::rankIPixmap(), rankW*(12-rank+1), 0, rankW, rankH );
p->setPen(pn0);
p->setPen(pn);
	    p->drawPixmap( ix + s1x, iy + s1y, *CardMetrics::suitSmPixmap(), suitSmlW*(suit-1), 0, suitSmlW, suitSmlH );
p->setPen(pn0);
p->setPen(pn);
	    p->drawPixmap( ix + s2x, iy + s2y, *CardMetrics::suitSmIPixmap(), suitSmlW*(3-suit+1), 0, suitSmlW, suitSmlH );
p->setPen(pn0);
p->setPen(pn);

	    //	                 A 2 3 4 5 6 7 8 9 0 J Q K
	    int colTable[] = { 0,0,1,1,2,2,2,2,2,2,2,0,0,0 };
	    int rowTable[] = { 0,0,2,2,2,2,3,3,3,4,4,0,0,0 };
	    int col = colTable[rank], row = rowTable[rank];
	    if ( rank == 10 ) {
		int vspc2=(h-2*vmarg-4*suitH)/3;
		p->drawPixmap( ix + hcent, iy + vmarg+1*suitH+vspc2*1/2-suitH/2, *CardMetrics::suitPixmap(), suitW*(suit-1), 0, suitW, suitH );
		p->drawPixmap( ix + hcent, iy + vmarg+3*suitH+vspc2*5/2-suitH/2, *CardMetrics::suitIPixmap(), suitW*(3-suit+1), 0, suitW, suitH );
	    }
	    if ( rank == 7 || rank == 8 ) {
		int vspc1=(h-2*vmarg-3*suitH)/2;
		if ( rank==8 )
		    p->drawPixmap( ix + hcent, iy + vmarg+2*suitH+vspc1*3/2-suitH/2, *CardMetrics::suitIPixmap(), suitW*(3-suit+1), 0, suitW, suitH );
		p->drawPixmap( ix + hcent, iy + vmarg+1*suitH+vspc1*1/2-suitH/2, *CardMetrics::suitPixmap(), suitW*(suit-1), 0, suitW, suitH );
	    }

	    // Need special image file for Ace of Spades
	    if ( rank == 1 && suit == spades )
		p->drawPixmap( ix + (w-spadeW)/2, iy + (h-spadeH)/2, *CardMetrics::spadePixmap(), 0, 0, spadeW, spadeH );
	    else if ( rank == 1 || rank == 3 || rank == 5 || rank == 9 )
		p->drawPixmap( ix + hcent, iy + (h-suitH)/2, *CardMetrics::suitPixmap(), suitW*(suit-1), 0, suitW, suitH );

	    int vspc=(h-2*vmarg-row*suitH)/(row-1)+suitH;
	    for (int c=0; c<col; c++) {
		int xx=col==1 ? hcent : (c==0 ? hmarg : w-hmarg-suitW);
		for (int j=0; j<row; j++) {
		    if ( (j>=(row+1)/2) ) {
			p->drawPixmap( ix + xx, iy + 1 + vmarg+vspc*j, *CardMetrics::suitIPixmap(), suitW*(3-suit+1), 0, suitW, suitH );
		    } else {
			p->drawPixmap( ix + xx, iy + 0 + vmarg+vspc*j, *CardMetrics::suitPixmap(), suitW*(suit-1), 0, suitW, suitH );
		    }
		}
	    }

	    if ( rank > 10 ) {
		int i = 3 * (suit - 1) + (rank - 11);
		p->drawPixmap( ix + hmarg, iy + vmarg, *CardMetrics::picturePixmap(), picturesW*i, 0, picturesW, picturesH );
		p->drawPixmap( ix + hmarg, iy + vmarg + picturesH, *CardMetrics::pictureIPixmap(), picturesW*(11-i), 0, picturesW, picturesH );
	    }

	}


    } else {
	
	p->drawPixmap( ix, iy, *CardMetrics::backPixmap() );

    }

    if (p != &painter)
    {
	p->end();
	QPixmap *scaledCard = CreateScaledPixmap( unscaledCard, scaleX, scaleY );
	int xoff = scaledCard->width() / 2;
	int yoff = scaledCard->height() / 2;
	painter.drawPixmap( (int)x() + xOff - xoff, (int)y() + yOff - yoff, *scaledCard );
	delete p;
	delete unscaledCard;
	delete scaledCard;
    }
}


static const double flipLift = 1.2;


void CanvasCard::flipTo(int x2, int y2, int steps)
{
    flipSteps = steps;

#ifdef SLOW_HARDWARE
    move(x2,y2);
    Card::flipTo(x2,y2,steps);
#else
    int x1 = (int)x();
    int y1 = (int)y();
    double dx = x2 - x1;
    double dy = y2 - y1;

    flipping = TRUE;
    destX = x2;
    destY = y2;
    animSteps = flipSteps;
    setVelocity(dx/animSteps, dy/animSteps-flipLift);
    setAnimated(TRUE);
#endif
}


void CanvasCard::advance(int stage)
{
    if ( stage==1 ) {
	if ( animSteps-- <= 0 ) {
	    scaleX = 1.0;
	    scaleY = 1.0;
	    flipping = FALSE;
	    setVelocity(0,0);
	    setAnimated(FALSE);
	    move(destX,destY); // exact
	} else {
	    if ( flipping ) {
		if ( animSteps > flipSteps / 2 ) {
		    // animSteps = flipSteps .. flipSteps/2 (flip up) -> 1..0
		    scaleX = ((double)animSteps/flipSteps-0.5)*2;
		} else {
		    // animSteps = flipSteps/2 .. 0 (flip down) -> 0..1
		    scaleX = 1-((double)animSteps/flipSteps)*2;
		}
		if ( animSteps == flipSteps / 2-1 ) {
		    setYVelocity(yVelocity()+flipLift*2);
		    setFace( !isFacing() );
		}
	    }
	}
    }
    QCanvasRectangle::advance(stage);
}


void CanvasCard::animatedMove(int x2, int y2, int steps)
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

