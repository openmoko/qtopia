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
    if (( mode == ScaleMin && (t1 > t2) ) || ( mode == ScaleMax && (t1 < t2) ))
        reqH = t2 / imgW;
    else
        reqW = t1 / imgH;
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
