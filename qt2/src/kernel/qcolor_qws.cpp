/****************************************************************************
** $Id: qt/src/kernel/qcolor_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of QColor class for FB
**
** Created : 991026
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qcolor.h"
#include "string.h"
#include "qpaintdevice.h"
#include "qapplication.h"
#include "qgfx_qws.h"
#include "qwsdisplay_qws.h"

extern uint qt_get_rgb_val( const char *name );

/*****************************************************************************
  QColor static member functions
 *****************************************************************************/

int QColor::maxColors()
{
    return 256;
}

int QColor::numBitPlanes()
{
    return 32;
}

void QColor::initialize()
{
    if ( color_init )				// already initialized
	return;
    color_init = TRUE;
}

void QColor::cleanup()
{
    if ( !color_init )
	return;
    color_init = FALSE;
}

#if 0
// Return a value for how close a is to b
// Lower is better
static inline int match(QRgb a,QRgb b)
{
    int ret;

#ifndef QT_NO_QWS_DEPTH_8
    int h1,s1,v1;
    int h2,s2,v2;
    /*
    QColor tmp1(a);
    QColor tmp2(b);
    tmp1.hsv(&h1,&s1,&v1);
    tmp2.hsv(&h2,&s2,&v2);
    */
    h1=qRed(a);
    s1=qGreen(a);
    v1=qBlue(a);
    h2=qRed(b);
    s2=qGreen(b);
    v2=qBlue(b);
    ret=abs(h1-h2);
    ret+=abs(s1-s2);
    ret+=abs(v1-v2);
#else
    ret=abs(qGray(a)-qGray(b));
#endif

    return ret;
}

inline unsigned int closestMatch(int r,int g,int b)
{
    QRgb * clut=qt_screen->clut();
    int clutcols=qt_screen->numCols();
    if ( r>255 || g>255 || b>255 || r<0 || g<0 || b<0 )
	abort();

    QRgb tomatch=qRgb(r,g,b);
    int loopc;
    unsigned int hold=0xfffff;
    unsigned int tmp;
    int pos=0;
    for(loopc=0;loopc<clutcols;loopc++) {
	tmp=match(clut[loopc],tomatch);
	if(tmp<hold) {
	    hold=tmp;
	    pos=loopc;
	}
    }
    return pos;
}
#endif


/*****************************************************************************
  QColor member functions
 *****************************************************************************/

uint QColor::alloc()
{
    rgbVal &= RGB_MASK;

// These macros mazimize optimizations even on dumb compilers.

#define GET \
    const int r = qRed(rgbVal);\
    const int g = qGreen(rgbVal);\
    const int b = qBlue(rgbVal);

    switch (qt_screen->depth()) {
      case 1: {
	GET
	return pix = qGray(r,g,b) < 128 ? 1 : 0;
#if !defined( QT_NO_IMAGE_16_BIT ) || !defined( QT_NO_QWS_DEPTH_16 )
      } case 16: {
	return pix = qt_convRgbTo16(rgbVal);
#endif	
      } case 24:
        case 32: {
	GET
	const int red_shift = 16;
	const int green_shift = 8;
	const int red_mask   = 0xff0000;
	const int green_mask = 0x00ff00;
	const int blue_mask  = 0x0000ff;
	const int tg = g << green_shift;
#ifndef QT_NO_QWS_DEPTH_32_BGR
	if ( qt_screen->pixelType() == QGfx::BGRPixel ) {
	    const int tb = b << red_shift;
	    pix = (r & blue_mask) | (tg & green_mask) | (tb & red_mask);
	} else 
#endif
	{
	    const int tr = r << red_shift;
	    pix = (b & blue_mask) | (tg & green_mask) | (tr & red_mask);
	}
	return pix |= 0xff000000;
     } default: {
	GET
#ifndef QT_NO_QWS_DEPTH_8GRAYSCALE
	return pix=qGray(r,g,b);
#else
	return pix=qt_screen->alloc(r,g,b);
#endif
      }
    }
}

void QColor::setSystemNamedColor( const QString& name )
{
    if ( !color_init ) {
#if defined(CHECK_STATE)
	qWarning( "QColor::setSystemNamedColor: Cannot perform this operation "
		 "because QApplication does not exist" );
#endif
	alloc();				// makes the color black
	return;
    }
    rgbVal = qt_get_rgb_val( name.latin1() );
    if ( lazy_alloc ) {
	rgbVal |= RGB_DIRTY;			// alloc later
	pix = 0;
    } else {
	alloc();				// alloc now
    }
}

int QColor::enterAllocContext()
{
    return 0;
}


void QColor::leaveAllocContext()
{
}


int QColor::currentAllocContext()
{
    return 0;
}


void QColor::destroyAllocContext( int )
{
}
