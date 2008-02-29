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
#include <qpalette.h>
#include "imageutil.h"


void changeTextColor( QWidget *w )
{
    QPalette p = w->palette();
    p.setBrush( QColorGroup::Background, QColor( 167, 212, 167 ) );
    p.setBrush( QColorGroup::Base, QColor( 167, 212, 167 ) );
    w->setPalette( p );
}



QPixmap combineImageWithBackground( const QImage &img, const QPixmap &bg, const QPoint &offset ) {
    QPixmap pix( img.width(), img.height() );
    QPainter p( &pix );
    p.drawTiledPixmap( pix.rect(), bg, offset );
    p.drawImage( 0, 0, img );
    return pix;
}


QPixmap maskPixToMask( const QPixmap &pix, const QBitmap &mask )
{
    QPixmap pixmap;
    pixmap = pix;
    pixmap.setMask( mask );
    return pixmap;
}


void scaleSize( int &reqW, int &reqH, int imgW, int imgH, ScaleMode mode )
{
    if ( mode == ScaleFree )
        return;
    int t1 = imgW * reqH;
    int t2 = reqW * imgH;
    if ( (mode == ScaleMin && (t1 > t2)) || (mode == ScaleMax && (t1 < t2)) )
        reqH = imgW ? t2 / imgW : 0;
    else
        reqW = imgH ? t1 / imgH : 0;
}


QImage fastScale( const QString &file, int w, int h )
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
	for ( int i = 0, x = 0; i < w; i++, x+=dx ) {
	    *dstLine++ = srcLine[x>>16];
//	    outImg.setPixel( i, j, img.pixel( x >> 16, y >> 16 ) );
	}
    }

    return outImg;
}
