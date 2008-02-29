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

#include "cardgame.h"
#include "canvascard.h"
#include "cardmetrics.h"
#include "canvasitemtimer.h"
#include <qtopia/resource.h>
#include <qpainter.h>
#include <qimage.h>
#include <qapplication.h>
#include <qpaintdevice.h>
#include <math.h>


/*
static QImage fastScale( const QString &file, int w, int h )
{
    QImage img( file );
    QImage srcImg = img.convertDepth( 32 );

    int iw = srcImg.width();
    int ih = srcImg.height();
    int dx = iw * 65535 / w;
    int dy = ih * 65535 / h;

    QImage outImg( w, h, 32 );
    outImg.setAlphaBuffer( srcImg.hasAlphaBuffer() );
    uint **dst = (uint**)outImg.jumpTable();
    uint **src = (uint**)srcImg.jumpTable();

    for ( int j = 0, y = 0; j < h; j++, y+=dy ) {
	uint *dstLine = dst[j];
	uint *srcLine = src[y>>16];
	for ( int i = 0, x = 0; i < w; i++, x+=dx ) 
	    *dstLine++ = srcLine[x>>16];
    }

    return outImg;
}
*/


static QPixmap *CreateScaledPixmap(QPixmap *srcPixmap, double scaleX, double scaleY)
{
#ifdef QT_NO_TRANSFORMATIONS
    int w = srcPixmap->width();
    int h = srcPixmap->height();
    int newW = (int)(w * scaleX);
    int newH = (int)(h * scaleY);
    QPixmap *dstPixmap = new QPixmap( newW, newH );
    // return dstPixmap->convertFromImage( srcPixmap->convertToImage().smoothScale( newW, newH ) );
    if ( newH == 0 || newW == 0 )
	return dstPixmap;
    QPainter p( dstPixmap );
    // ### this is very poorly implemented and probably could be much faster
    for (int i = 0; i < newW; i++) {
       int srcX = w * i / newW;
       if (newH == h) {
           // Optimise for scaling in the X-axis only
           p.drawPixmap( i, 0, *srcPixmap, srcX, 0, 1, h );
       } else {
           for (int j = 0; j < newH; j++) {
               int srcY = h * j / newH;
               p.drawPixmap( i, j, *srcPixmap, srcX, srcY, 1, 1 );
           }
        }
    }
    p.end();
    return dstPixmap;
#else
    QWMatrix s;
    s.scale( scaleX, scaleY );
    return new QPixmap( srcPixmap->xForm( s ) );
#endif
}


CanvasCard::CanvasCard( Value v, Suit s, bool f, QCanvas *canvas ) :
	Card(v, s, f), AnimatedItem(), QCanvasRectangle(0, 0, 1, 1, canvas), scaleX(1.0), scaleY(1.0)
{
    int w = CardMetrics::width();
    int h = CardMetrics::height();
    offset = QPoint(w/2, h/2);
    setSize(w, h);
    flipping = FALSE;
    haveCache = FALSE;
}


void CanvasCard::cardBackChanged()
{
    if ( !isFacing() && isVisible() ) {
	hide();
	show();
    }
}


void CanvasCard::draw(QPainter &painter)
{
    int ix = (int)x();
    int iy = (int)y();
    int w = CardMetrics::width();
    int h = CardMetrics::height();
    int rank = value();
    int isuit = suit();

    QPainter *p = &painter;
    QPixmap *drawPixmap = 0;
	
    if ( isFacing()  ) {
    
	if ( !haveCache ) {

	    haveCache = TRUE;
	    cachedPixmap = *CardMetrics::facePixmap();
	    cachedPixmap.setOptimization( QPixmap::BestOptim );
	    p = new QPainter();
	    p->begin( &cachedPixmap );
	    ix = 0;
	    iy = 0;

	    // Joker
	    if ( rank == jokerVal ) {
		// Draw Joker Card


	    } else {

		p->setBrush( NoBrush );
		if ( isRed() == TRUE )
		    p->setPen( QColor( 0xFF, 0, 0 ) );
		else
		    p->setPen( QColor( 0, 0, 0 ) );

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

		int hcent = (w - suitW) / 2;
		int vmarg = (h - picturesH * 2) / 2; 
		int hmarg = (w - picturesW) / 2;

		int r1x, r1y, r2x, r2y, s1x, s1y, s2x, s2y;

		r1x = (hmarg - rankW) / 2;
#ifndef QTOPIA_PHONE
		r1y = r1x + 2;
#else
		r1y = r1x + 1;
#endif
		r2x = w - r1x - rankW - 1;
		r2y = h - r1y - rankH;

		if ( CardMetrics::offsetDown() <= 15 || rank==1 ) {
		    s2x = (hmarg - suitSmlW) / 2 + 1;
		    s2y = r2y - 1;
#ifndef QTOPIA_PHONE
		    s1x = w - s2x - suitSmlW - 1;
#else
		    s1x = w - s2x - suitSmlW;
#endif
		    s1y = r1y;
		    r2x += 1;
		} else {
		    s1x = (hmarg - suitSmlW) / 2;
		    s1y = s1x + rankH + 4;
		    s2x = w - s1x - suitSmlW - 1;
		    s2y = h - s1y - suitSmlH;
		}

		p->drawPixmap( ix +   0, iy +   0, *CardMetrics::facePixmap() );
		p->drawPixmap( ix + r1x, iy + r1y, *CardMetrics::rankPixmap(), rankW*(rank-1), 0, rankW, rankH );
		p->drawPixmap( ix + s1x, iy + s1y, *CardMetrics::suitSmPixmap(), suitSmlW*(isuit-1), 0, suitSmlW, suitSmlH );
#ifndef QTOPIA_PHONE
		p->drawPixmap( ix + r2x, iy + r2y, *CardMetrics::rankIPixmap(), rankW*(12-rank+1), 0, rankW, rankH );
		p->drawPixmap( ix + s2x, iy + s2y, *CardMetrics::suitSmIPixmap(), suitSmlW*(3-isuit+1), 0, suitSmlW, suitSmlH );
#else
		int rankBigW = CardMetrics::rankBigPixmap()->width() / 13;
		int rankBigH = CardMetrics::rankBigPixmap()->height();
		p->drawPixmap( ix + (w-rankBigW)/2, iy + r1y + rankH + 2, *CardMetrics::rankBigPixmap(), rankBigW*(rank-1), 0, rankBigW, rankBigH );
#endif

		//                 A 2 3 4 5 6 7 8 9 0 J Q K
		int colTable[] = { 0,0,1,1,2,2,2,2,2,2,2,0,0,0 };
		int rowTable[] = { 0,0,2,2,2,2,3,3,3,4,4,0,0,0 };
		int col = colTable[rank], row = rowTable[rank];
		if ( rank == 10 ) {
		    int vspc2=(h-2*vmarg-4*suitH)/3;
		    p->drawPixmap( ix + hcent, iy + vmarg+1*suitH+vspc2*1/2-suitH/2, *CardMetrics::suitPixmap(), suitW*(isuit-1), 0, suitW, suitH );
#ifndef QTOPIA_PHONE
		    p->drawPixmap( ix + hcent, iy + vmarg+3*suitH+vspc2*5/2-suitH/2, *CardMetrics::suitIPixmap(), suitW*(3-isuit+1), 0, suitW, suitH );
#endif
		}
		if ( rank == 7 || rank == 8 ) {
		    int vspc1=(h-2*vmarg-3*suitH)/2;
#ifndef QTOPIA_PHONE
		    if ( rank==8 )
			p->drawPixmap( ix + hcent, iy + vmarg+2*suitH+vspc1*3/2-suitH/2, *CardMetrics::suitIPixmap(), suitW*(3-isuit+1), 0, suitW, suitH );
#endif
		    p->drawPixmap( ix + hcent, iy + vmarg+1*suitH+vspc1*1/2-suitH/2, *CardMetrics::suitPixmap(), suitW*(isuit-1), 0, suitW, suitH );
		}

		// Need special image file for Ace of Spades
		if ( rank == 1 && isuit == spades )
		    p->drawPixmap( ix + (w-spadeW)/2, iy + (h-spadeH)/2, *CardMetrics::spadePixmap(), 0, 0, spadeW, spadeH );
		else if ( rank == 1 || rank == 3 || rank == 5 || rank == 9 )
		    p->drawPixmap( ix + hcent, iy + (h-suitH)/2, *CardMetrics::suitPixmap(), suitW*(isuit-1), 0, suitW, suitH );

		int vspc=(h-2*vmarg-row*suitH)/(row-1)+suitH;
		for (int c=0; c<col; c++) {
		    int xx=col==1 ? hcent : (c==0 ? hmarg : w-hmarg-suitW);
		    for (int j=0; j<row; j++) {
			if ( (j>=(row+1)/2) ) {
#ifndef QTOPIA_PHONE
			    p->drawPixmap( ix + xx, iy + 1 + vmarg+vspc*j, *CardMetrics::suitIPixmap(), suitW*(3-isuit+1), 0, suitW, suitH );
#endif
			} else {
			    p->drawPixmap( ix + xx, iy + 0 + vmarg+vspc*j, *CardMetrics::suitPixmap(), suitW*(isuit-1), 0, suitW, suitH );
			}
		    }
		}

		if ( rank > 10 ) {
		    int i = 3 * (isuit - 1) + (rank - 11);
		    p->drawPixmap( ix + hmarg, iy + vmarg, *CardMetrics::picturePixmap(), picturesW*i, 0, picturesW, picturesH );
#ifndef QTOPIA_PHONE
		    p->drawPixmap( ix + hmarg, iy + vmarg + picturesH, *CardMetrics::pictureIPixmap(), picturesW*(11-i), 0, picturesW, picturesH );
#endif
		}

	    }

	    p->end();

	}

	drawPixmap = &cachedPixmap;

    } else {

	if ( !cachedPixmap.isNull() )
	    cachedPixmap = QPixmap();
	haveCache = FALSE;

	drawPixmap = CardMetrics::backPixmap();

    }

    if ((scaleX <= 0.98) || (scaleY <= 0.98)) {
	QPixmap *scaledCard = CreateScaledPixmap( drawPixmap, scaleX, scaleY );
	QPoint mid = QPoint(scaledCard->width(),scaledCard->height()) / 2;
	painter.drawPixmap(pos() + offset - mid, *scaledCard);
	delete scaledCard;
    } else {
	painter.drawPixmap( (int)x(), (int)y(), *drawPixmap );
    }
}


void CanvasCard::moveTo(QPoint p, int msecs)
{
    flipping = false;
    animatedMove(p, msecs);
}


void CanvasCard::flipTo(QPoint p, int msecs)
{
    flipping = true;
    origFace = isFacing();
    origHeight = height();
    animatedMove(p, msecs);
}


void CanvasCard::updatePosition( double percent )
{
    double percentLeft = 1.0 - percent;
    double x = dest.x() - percentLeft * delta.x();
    double y = dest.y() - percentLeft * delta.y();

    if ( flipping ) {
	if ( percent > 0.5 ) {
	    scaleX = (percent - 0.5) * 2;
	    setFacing( !origFace );
	    setHeight(origHeight);
	} else {
	    scaleX = (0.5 - percent) * 2;
	    setFacing( origFace );
	    setHeight(INT_MAX-int(percentLeft*10));
	}
	y -= (1.0 - scaleX) * 8;
    }

    if ( percent == 1.0 ) {
	flipping = FALSE;
	scaleX = 1.0;
	scaleY = 1.0;
    }

    setPos(QPoint(int(x), int(y)));
}


void CanvasCard::animatedMove(QPoint p, int msecs)
{
    dest = p;
    delta = dest - pos();
    start(msecs);
}

