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
#include <qpixmap.h>
#include "framebuffer.h"


/*!
  \class FrameBuffer framebuffer.h
  \brief This class is specific to the mediaplayer apps.

    It is currently private to those apps, and its API,
    and therefore binary compatibilty may change in later
    versions.

    This class provides a way to abstract drawing in to
    either a QImage's bits, or directly to the screen from
    the bits returned by QDirectPainter. It also handles
    rotation of buffers allowing for example rotation from
    a QImage direct to the screen if a FrameBuffer object
    has been contructed around both. This class is specific
    for the VideoOutput class for allowing it to decode
    video frames either directly to the screen when it can
    without rotation or clipping, and when it can't to
    be able to rotate efficently, or fall back to painting
    a QImage with a QPainter in the worst case.
*/


FrameBuffer::FrameBuffer() : jt(0), x(0), y(0), w(0), h(0), bits(0), lineStep(0)
{
}


FrameBuffer::~FrameBuffer()
{
    delete []jt;
}


void FrameBuffer::create( uchar *b, int ls, int x1, int y1, int width, int height )
{
    if ( bits == b && ls == lineStep && x == x1 && y == y1 && w == width && h == height )
	return;

    delete []jt;
    jt = new uchar*[height];

    x = x1;
    y = y1;
    bits = b;
    lineStep = ls;
    w = width;
    h = height;
    uchar *fp = bits + lineStep * y + QPixmap::defaultDepth() * x / 8;

    for ( int i = 0; i < h; i++, fp += lineStep ) 
	jt[i] = fp;
}


uchar **FrameBuffer::jumpTable()
{
    return jt;
}


int FrameBuffer::width()
{
    return w;
}


int FrameBuffer::height()
{
    return h;
}


template<class T>
static inline void rotateLoopTemplate(
        uchar *src, int srcBytesPerLine, uchar *dst, int dstBytesPerLine,
        int width, int height, int quadrants )
{
    int dstXAdd = 0;
    int dstYAdd = 0;
    int dstXOfs = 0;
    int dstYOfs = 0;
    int srcYAdd = srcBytesPerLine - width * sizeof(T);

    switch (quadrants) {
	case 1:
            dstXOfs = 0;
            dstYOfs = width - 1;
            dstXAdd = -dstBytesPerLine;
            dstYAdd = 1 * sizeof(T) + width * dstBytesPerLine;
	    break;
	case 2:
            dstXOfs = width - 1;
            dstYOfs = height - 1;
            dstXAdd = -sizeof(T);
            dstYAdd = -dstBytesPerLine + width * sizeof(T);
	    break;
	case 3:
            dstXOfs = height - 1;
            dstYOfs = 0;
            dstXAdd = dstBytesPerLine;
            dstYAdd = -1 * sizeof(T) - width * dstBytesPerLine;
	    break;
	default:
            dstXOfs = 0;
            dstYOfs = 0;
            dstXAdd = sizeof(T);
            dstYAdd = dstBytesPerLine - width * sizeof(T);
    }

    T *dstPtr = (T *)(dst + dstYOfs * dstBytesPerLine) + dstXOfs;
    T *srcPtr = (T *)src;
    for ( int y = 0; y < height; y++ ) {
        for ( int x = 0; x < width; x++ ) {
            *dstPtr = *srcPtr++;
            dstPtr = (T *)((uchar*)dstPtr + dstXAdd); // add dstXAdd number of bytes
        }
        srcPtr = (T *)((uchar*)srcPtr + srcYAdd); // add srcYAdd number of bytes
        dstPtr = (T *)((uchar*)dstPtr + dstYAdd); // add dstYAdd number of bytes
    }
}


void FrameBuffer::rotateToBuffer( FrameBuffer *dst, int dstX, int dstY, int srcX, int srcY, int w, int h, int quadrants )
{
    int dd = QPixmap::defaultDepth();
    uchar *srcPtr = jt[srcY] + dd * srcX / 8;
    uchar *dstPtr = dst->jt[dstY] + dd * dstX / 8;

    // Optimized image rotation code for nice bit depths
    switch ( dd ) {
	case 16:
	    rotateLoopTemplate<ushort>( srcPtr, lineStep, dstPtr, dst->lineStep, w, h, quadrants );
	    break;
	case 8:
	case 32:
	    rotateLoopTemplate<uint>( srcPtr, lineStep, dstPtr, dst->lineStep, w, h, quadrants );
	    break;
    }
}

