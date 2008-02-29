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

#include <qpainter.h>
#include "cardmetrics.h"
#include "canvascardpile.h"
#include "canvasitemtimer.h"


void AnimatedCanvasCardPile::addCards( CanvasCard *card )
{
    reset();
    setPos(card->pos());
    savePos();

    int depth = 0;

    // Count the cards under the first card
    CanvasCard *tmpCard = card;
    while (tmpCard) {
	tmpCard->hide();
	tmpCard = (CanvasCard *)tmpCard->cardPile()->cardInfront(tmpCard);
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
	    card = (CanvasCard *)card->cardPile()->cardInfront(card);
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


void AnimatedCanvasCardPile::updatePosition( double percent )
{
    double percentLeft = 1.0 - percent;
    double x = dest.x() - percentLeft * delta.x();
    double y = dest.y() - percentLeft * delta.y();

    if ( percent == 1.0 ) {
	CanvasCard *item = firstCard;
	while (item) {
	    item->show();
	    item = (CanvasCard *)item->cardPile()->cardInfront(item);
	}
	hide();
	move( x, y );
    } else {
	move( x, y );
	hide();
	show();
    }
}


void AnimatedCanvasCardPile::animatedMove( QPoint p, int msecs )
{
    dest = p;
    delta = p - pos();
    start(msecs);
}


void AnimatedCanvasCardPile::draw( QPainter& p )
{
    p.drawImage( pos(), doubleBuffer );
}


