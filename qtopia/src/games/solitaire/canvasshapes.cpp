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
#include <qcanvas.h>
#include <qpixmap.h>
#include <qtopia/resource.h>
#include "canvasshapes.h"
#include "cardmetrics.h"


CanvasRoundRect::CanvasRoundRect(QPoint p, QCanvas *canvas) :
	QCanvasRectangle( p.x(), p.y(), CardMetrics::width(), CardMetrics::height(), canvas),
	mode( notSelected )
{
    setZ(0);
    show();
}


void CanvasRoundRect::redraw()
{
    hide();
    show();
}

void CanvasRoundRect::drawShape(QPainter &p)
{
    switch ( mode ) {
	case disabledSelection:
	    p.setPen( Qt::red );
#ifdef QTOPIA_PHONE
	    p.drawRect( (int)x()+1, (int)y()+1, width()-2, height()-2 );
#else
	    p.drawRoundRect( (int)x()+1, (int)y()+1, width()-2, height()-2 );
#endif
	    p.setPen( QColor( 255, 128, 128 ) );
	    break;
	case enabledSelection:
	    p.setPen( Qt::green );
#ifdef QTOPIA_PHONE
	    p.drawRect( (int)x()+1, (int)y()+1, width()-2, height()-2 );
#else
	    p.drawRoundRect( (int)x()+1, (int)y()+1, width()-2, height()-2 );
#endif
	    p.setPen( QColor( 128, 255, 128 ) );
	    break;
	case partiallySelected:
#ifdef QTOPIA_PHONE
{
	    int w = width(), h = height();
	    QImage img(w, h, 32);
	    if ( img.depth() == 32 ) {
		img.setAlphaBuffer(true);
		QRgb alphaYellow = qRgba(128,255,0,90);
		for ( int j = 2; j < h-2; j++ ) {
		    QRgb *linePtr = (QRgb*)img.scanLine(j);
		    linePtr[0] = alphaYellow;
		    linePtr[1] = alphaYellow;
		    for ( int i = 2; i < w-2; i++ ) 
			linePtr[i] = qRgba(0,0,0,0);
		    linePtr[w-2] = alphaYellow;
		    linePtr[w-1] = alphaYellow;
		}
		for ( int i = 0; i < w; i++ ) {
		    ((QRgb*)img.scanLine(0))[i] = alphaYellow;
		    ((QRgb*)img.scanLine(1))[i] = alphaYellow;
		    ((QRgb*)img.scanLine(h-1))[i] = alphaYellow;
		    ((QRgb*)img.scanLine(h-2))[i] = alphaYellow;
		}
		p.drawImage((int)x(), (int)y(), img);
		return;
	    }
}
#endif
	    p.setPen( Qt::yellow );
	    break;
	default:
	    p.setPen( Qt::black );
	    break;
    }
#ifdef QTOPIA_PHONE
    p.drawRect( (int)x(), (int)y(), width(), height() );
#else
    p.drawRoundRect( (int)x(), (int)y(), width(), height() );
#endif
}


QPixmap *circlePix = NULL; 
QPixmap *crossPix = NULL; 

QPixmap *makeScaledPixmapOutOfBitmap( const char *file, int size )
{
    QBitmap tmp = Resource::loadBitmap( file );
    double scaleFactor = (double)size / tmp.width();
    QWMatrix m;
    m.scale( scaleFactor, scaleFactor );
    tmp = tmp.xForm( m );
    QPixmap *ret = new QPixmap( tmp );
    ret->setMask( tmp );
    return ret;
}


CanvasCircleOrCross::CanvasCircleOrCross(QPoint p, int radius, QCanvas *canvas) :
	QCanvasRectangle( p.x(), p.y(), radius, radius, canvas), circleShape(TRUE)
{
    setZ(0);
    if ( circlePix ) 
	delete circlePix;
    circlePix = makeScaledPixmapOutOfBitmap( "cards/circle", radius );
    if ( crossPix ) 
	delete crossPix;
    crossPix = makeScaledPixmapOutOfBitmap( "cards/cross", radius );
    show();
}


void CanvasCircleOrCross::redraw()
{
    hide();
    show();
}


void CanvasCircleOrCross::setCircle()
{
    circleShape = TRUE;
    redraw();
}


void CanvasCircleOrCross::setCross()
{
    circleShape = FALSE;
    redraw();
}


void CanvasCircleOrCross::drawShape(QPainter &p)
{
    int x1 = (int)x();
    int y1 = (int)y();
    int r = width();
    int lineWidth = r / 6;

    if (circleShape == TRUE) {
	// Green circle
	if ( circlePix ) {
	    p.setPen( QPen( QColor(0x10, 0xE0, 0x10) ) );
	    p.setBrush( Qt::NoBrush );
	    p.drawPixmap( x1, y1, *circlePix );
	} else {
	    // fall back for when there is no image
	    p.setPen( QPen( QColor(0x10, 0xE0, 0x10), lineWidth ) );
	    p.drawEllipse( x1 + lineWidth, y1 + lineWidth, r - lineWidth - 1, r - lineWidth - 1 );
	}
    } else {
	// Red cross
	if ( crossPix ) {
	    p.setPen( QPen( QColor(0xE0, 0x10, 0x10) ) );
	    p.setBrush( Qt::NoBrush );
	    p.drawPixmap( x1, y1, *crossPix );
	} else {
	    // fall back for when there is no image
	    p.setPen( QPen( QColor(0xE0, 0x10, 0x10), lineWidth ) );
	    p.drawLine( x1 + lineWidth, y1 + lineWidth, x1 + r - lineWidth, y1 + r - lineWidth);
	    p.drawLine( x1 + r - lineWidth, y1 + lineWidth, x1 + lineWidth, y1 + r - lineWidth);
	}
    }
}

