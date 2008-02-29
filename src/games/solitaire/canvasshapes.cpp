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
#include <qpainter.h>
#include <qcanvas.h>
#include <qpixmap.h>
#include <qtopia/resource.h>
#include "canvasshapes.h"
#include "cardmetrics.h"


CanvasRoundRect::CanvasRoundRect(int x, int y, QCanvas *canvas) :
	QCanvasRectangle( x, y, CardMetrics::width(), CardMetrics::height(), canvas)
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
    p.drawRoundRect( (int)x(), (int)y(), CardMetrics::width(), CardMetrics::height() );
}


QPixmap *circlePix = NULL; 
QPixmap *crossPix = NULL; 

QPixmap *makeScaledPixmapOutOfBitmap( const char *file, int size ) {
    QBitmap tmp = Resource::loadBitmap( file );
    double scaleFactor = (double)size / tmp.width();
    QWMatrix m;
    m.scale( scaleFactor, scaleFactor );
    tmp = tmp.xForm( m );
    QPixmap *ret = new QPixmap( tmp );
    ret->setMask( tmp );
    return ret;
}


CanvasCircleOrCross::CanvasCircleOrCross(int x, int y, int radius, QCanvas *canvas) :
	QCanvasRectangle( x, y, radius, radius, canvas), circleShape(TRUE)
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

