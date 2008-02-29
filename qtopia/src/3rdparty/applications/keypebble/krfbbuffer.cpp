/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
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
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include <assert.h>
#include <qpixmap.h>
#include <qbrush.h>
#include <qimage.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qevent.h>

#include "krfbdecoder.h"
#include "krfbbuffer.h"
#include "krfbserverinfo.h"

//
// Endian stuff
//
#ifndef KDE_USE_FINAL
const int endianTest = 1;
#endif

#define Swap16IfLE(s) \
    (*(char *)&endianTest ? ((((s) & 0xff) << 8) | (((s) >> 8) & 0xff)) : (s))

#define Swap32IfLE(l) \
    (*(char *)&endianTest ? ((((l) & 0xff000000) >> 24) | \
			     (((l) & 0x00ff0000) >> 8)  | \
			     (((l) & 0x0000ff00) << 8)  | \
			     (((l) & 0x000000ff) << 24))  : (l))

KRFBBuffer::KRFBBuffer( KRFBDecoder *decoder,
			QObject *parent, const char *name )
  : QObject( parent, name )
{
  assert( decoder );
  this->decoder = decoder;
  buffer = new QImage( 1, 1, 32 );
  scaler = 1;
}

KRFBBuffer::~KRFBBuffer()
{
  delete buffer;
}

void KRFBBuffer::paint( QPainter *p, int x, int y, int w, int h ) const
{
    if ( buffer->isNull() )
	return;
    if ( scaler == 1 ) {
	QRect dr( x, y, w, h );
	dr &= QRect( 0, 0, buffer->width(), buffer->height() );
	QImage sub( buffer->scanLine(dr.y())+dr.x()*4, dr.width(), dr.height(), 32, buffer->bytesPerLine(), 0, 0, QImage::IgnoreEndian );
	if ( !dr.isEmpty() )
	    p->drawImage( dr.x(), dr.y(), sub, 0, 0, dr.width(), dr.height() );
    } else {
	QRect dr( x, y, w, h );
	dr &= QRect( 0, 0, buffer->width()/scaler, buffer->height()/scaler );
	QRect sr( dr.x()*scaler, dr.y()*scaler, dr.width()*scaler, dr.height()*scaler );

	QImage sub( buffer->scanLine(sr.y())+sr.x()*4, sr.width(), sr.height(), 32, buffer->bytesPerLine(), 0, 0, QImage::IgnoreEndian );
	if ( scaler == 2 )
	    sub = fastScale( sub );
	else
	    sub = sub.smoothScale( dr.width(), dr.height() );
	if ( !dr.isEmpty() )
	    p->drawImage( dr.x(), dr.y(), sub, 0, 0, dr.width(), dr.height() );
    }
}

QImage KRFBBuffer::fastScale( QImage &si ) const
{
    QImage di( si.width()/scaler, si.height()/scaler, 32 );

    for ( int y = 0; y < di.height(); y++ ) {
	Q_UINT32 *dst = (Q_UINT32 *)di.scanLine( y );
	Q_UINT32 *src1 = (Q_UINT32 *)si.scanLine( y*scaler );
	Q_UINT32 *src2 = (Q_UINT32 *)si.scanLine( y*scaler+1 );
	for ( int x = 0; x < di.width(); x++ ) {
	    Q_UINT32 r = (*src1) & 0x00ff0000;
	    Q_UINT32 g = (*src1) & 0x0000ff00;
	    Q_UINT32 b = (*src1) & 0x0000ff;
	    r += (*(src1+1)) & 0x00ff0000;
	    g += (*(src1+1)) & 0x0000ff00;
	    b += (*(src1+1)) & 0x000000ff;
	    r += (*src2) & 0x00ff0000;
	    g += (*src2) & 0x0000ff00;
	    b += (*src2) & 0x000000ff;
	    r += (*(src2+1)) & 0x00ff0000;
	    g += (*(src2+1)) & 0x0000ff00;
	    b += (*(src2+1)) & 0x000000ff;
	    *dst = ((r>>2)&0x00ff0000) | ((g>>2)&0x0000ff00) | (b>>2);
	    dst++;
	    src1 += scaler;
	    src2 += scaler;
	}
    }

    return di;
}

void KRFBBuffer::resize( int w, int h )
{
    qWarning( "Resizing buffer" );

    delete buffer;
    buffer = new QImage( w, h, 32 );

    emit sizeChanged( w/scaler, h/scaler );
}

void KRFBBuffer::soundBell()
{
  emit bell();
}

void KRFBBuffer::setScaling( int s )
{
    scaler = s;
    emit sizeChanged( buffer->width()/scaler, buffer->height()/scaler );
}

void KRFBBuffer::mouseEvent( QMouseEvent *e )
{
  QMouseEvent te( e->type(), e->pos()*scaler, e->button(), e->state() );
  decoder->sendMouseEvent( &te );
}

void KRFBBuffer::keyPressEvent( QKeyEvent *e )
{
  decoder->sendKeyPressEvent( e );
}

void KRFBBuffer::keyReleaseEvent( QKeyEvent *e )
{
  decoder->sendKeyReleaseEvent( e );
}

void KRFBBuffer::copyRect( int srcX, int srcY,
                           int destX, int destY, int w, int h )
{
    if ( srcY > destY ) {
	for ( int j = 0; j < h; j++ ) {
	    uchar *sp = buffer->scanLine( j+srcY ) + srcX*4;
	    uchar *dp = buffer->scanLine( j+destY ) + destX*4;
	    memmove( dp, sp, w*4 );
	}
    } else {
	for ( int j = h-1; j >= 0; j-- ) {
	    uchar *sp = buffer->scanLine( j+srcY ) + srcX*4;
	    uchar *dp = buffer->scanLine( j+destY ) + destX*4;
	    memmove( dp, sp, w*4 );
	}
    }

    updateDone( destX, destY, w, h );
}

void KRFBBuffer::drawRawRectChunk( void *data,
				   int x, int y, int w, int h )
{
    int redMax = Swap16IfLE( decoder->format->redMax );
    int greenMax = Swap16IfLE( decoder->format->greenMax );
    int blueMax = Swap16IfLE( decoder->format->blueMax );

    if ( decoder->format->bpp == 8 ) {
	uchar *d = (unsigned char *) data;

	uint r,g,b;

	for ( int j = 0; j < h; j++ ) {
	    uint *p = ( uint * ) (buffer->scanLine( j+y ) + x*4 );
	    for ( int i = 0; i < w ; i++, p++ ) {
		uchar sp = d[ j * w + i ];
		r = sp;
		r = r >> decoder->format->redShift;
		r = r & redMax;

		g = sp;
		g = g >> decoder->format->greenShift;
		g = g & greenMax;

		b = sp;
		b = b >> decoder->format->blueShift;
		b = b & blueMax;

		r = ( r * 255 ) / redMax;
		g = ( g * 255 ) / greenMax;
		b = ( b * 255 ) / blueMax;

		*p = qRgb( r,g,b );
	    }
	}
    } else if ( decoder->format->bpp == 16 ) {
	ushort *d = (ushort *) data;

	ulong r,g,b;

	for ( int j = 0; j < h; j++ ) {
	    uint *p = ( uint * ) (buffer->scanLine( j+y ) + x*4 );
	    for ( int i = 0; i < w ; i++, p++ ) {
		ushort pixel = d[ j * w + i ];
//		pixel = Swap16IfLE( pixel );

		r = pixel;
		r = r >> decoder->format->redShift;
		r = r & redMax;

		g = pixel;
		g = g >> decoder->format->greenShift;
		g = g & greenMax;

		b = pixel;
		b = b >> decoder->format->blueShift;
		b = b & blueMax;

		r = ( r * 255 ) / redMax;
		g = ( g * 255 ) / greenMax;
		b = ( b * 255 ) / blueMax;

		*p = qRgb( r,g,b );
	    }
	}
    } else if ( decoder->format->bpp == 32 ) {
	ulong *d = (ulong *) data;

	ulong r,g,b;

	for ( int j = 0; j < h; j++ ) {
	    uint *p = ( uint * ) (buffer->scanLine( j+y ) + x*4 );
	    for ( int i = 0; i < w ; i++, p++ ) {
		ulong pixel = d[ j * w + i ];
		pixel = Swap32IfLE( pixel );

		r = pixel;
		r = r >> decoder->format->redShift;
		r = r & redMax;

		g = pixel;
		g = g >> decoder->format->greenShift;
		g = g & greenMax;

		b = pixel;
		b = b >> decoder->format->blueShift;
		b = b & blueMax;

		r = ( r * 255 ) / redMax;
		g = ( g * 255 ) / greenMax;
		b = ( b * 255 ) / blueMax;

		*p = qRgb( r,g,b );
	    }
	}
    }
    else {
	// just clear it
    }

    updateDone( x, y, w, h );
}

void KRFBBuffer::fillRect( int x, int y, int w, int h, unsigned long p )
{
    int redMax = Swap16IfLE( decoder->format->redMax );
    int greenMax = Swap16IfLE( decoder->format->greenMax );
    int blueMax = Swap16IfLE( decoder->format->blueMax );

    if ( decoder->format->bpp == 8 || decoder->format->bpp == 16 ) {
	uint r = p >> decoder->format->redShift;
	r = r & redMax;
	r = ( r * 255 ) / redMax;

	uint g = p >> decoder->format->greenShift;
	g = g & greenMax;
	g = ( g * 255 ) / greenMax;

	uint b = p >> decoder->format->blueShift;
	b = b & blueMax;
	b = ( b * 255 ) / blueMax;

	ulong pixel = qRgb( r,g,b );
	for ( int j = 0; j < h; j++ ) {
	    uint *p = ( uint * ) (buffer->scanLine( j+y ) + x*4 );
	    for ( int i = 0; i < w ; i++, p++ ) {
		*p = pixel;
	    }
	}
    } else if ( decoder->format->bpp == 32 ) {
	qDebug( "Implement 32bpp fillRect" );
    }
}

void KRFBBuffer::updateDone( int x, int y, int w, int h )
{
    if ( scaler != 1 ) {
	int x2 = (x+w+scaler-1)/scaler;
	int y2 = (y+h+scaler-1)/scaler;
	x /= scaler;
	y /= scaler;
	w = x2-x;
	h = y2-y;
    }

    emit updated( x, y, w, h );
}

