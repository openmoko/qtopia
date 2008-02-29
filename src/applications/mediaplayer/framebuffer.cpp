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
#include <qpixmap.h>
#include "framebuffer.h"


FrameBuffer::FrameBuffer() :x(0), y(0), w(0), h(0), bits(0), lineStep(0)
{
    jt = new uchar*[1];
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
        int width, int height, bool rotateDirection )
{
    int dstXAdd = 0;
    int dstYAdd = 0;
    int dstXOfs = 0;
    int dstYOfs = 0;
    int srcYAdd = srcBytesPerLine - width * sizeof(T);

    if ( rotateDirection ) {
            dstXOfs = 0;
            dstYOfs = width - 1;
            dstXAdd = -dstBytesPerLine;
            dstYAdd = 1 * sizeof(T) + width * dstBytesPerLine;
    } else {
            dstXOfs = height - 1;
            dstYOfs = 0;
            dstXAdd = dstBytesPerLine;
            dstYAdd = -1 * sizeof(T) - width * dstBytesPerLine;
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


void FrameBuffer::rotateToBuffer( FrameBuffer *dst, int dstX, int dstY, int srcX, int srcY, int w, int h, bool direction )
{
    uchar *srcPtr = jt[srcY] + QPixmap::defaultDepth() * srcX / 8;
    uchar *dstPtr = dst->jt[dstY] + QPixmap::defaultDepth() * dstX / 8;

    // Optimized image rotation code for nice bit depths
    switch ( QPixmap::defaultDepth() ) {
	case 16:
	    rotateLoopTemplate<ushort>( srcPtr, lineStep, dstPtr, dst->lineStep, w, h, direction );
	    break;
	case 32:
	    rotateLoopTemplate<uint>( srcPtr, lineStep, dstPtr, dst->lineStep, w, h, direction );
	    break;
    }
}

