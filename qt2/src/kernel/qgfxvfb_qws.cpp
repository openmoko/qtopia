/*****************************************************************************
** $Id: qt/src/kernel/qgfxvfb_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of QGfxvfb (virtual frame buffer driver)
**
** Created : 940721
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

#include "qgfxraster_qws.h"

#ifndef QT_NO_QWS_VFB

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>

#include "qvfbhdr.h"

bool qvfbEnabled = FALSE;

// Unaccelerated screen/driver setup. Can be overridden by accelerated
// drivers

class QVFbScreen : public QScreen
{
public:
    QVFbScreen( int display_id );
    virtual ~QVFbScreen();
    virtual bool initDevice();
    virtual bool connect( const QString &displaySpec );
    virtual void disconnect();
    virtual int initCursor(void*, bool);
    virtual void shutdownDevice();
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
    virtual void save();
    virtual void restore();
    virtual void setMode(int nw,int nh,int nd);

    void setDirty( QRect r )
	{ hdr->dirty = true; hdr->update = hdr->update.unite( r ); }

    bool success;
    unsigned char *shmrgn;
    QVFbHeader *hdr;
};

static QVFbScreen *qvfb_screen = 0;

#ifndef QT_NO_QWS_CURSOR
class QVFbScreenCursor : public QScreenCursor
{
public:
    QVFbScreenCursor();

    virtual void set( const QImage &image, int hotx, int hoty );
    virtual void move( int x, int y );
};

QVFbScreenCursor::QVFbScreenCursor() : QScreenCursor()
{
}

void QVFbScreenCursor::set( const QImage &image, int hotx, int hoty )
{
    QWSDisplay::grab( TRUE );
    QRect r( data->x - hotx, data->y - hoty, image.width(), image.height() );
    qvfb_screen->setDirty( data->bound | r );
    QScreenCursor::set( image, hotx, hoty );
    QWSDisplay::ungrab();
}

void QVFbScreenCursor::move( int x, int y )
{
    QWSDisplay::grab( TRUE );
    QRect r( x - data->hotx, y - data->hoty, data->width, data->height );
    qvfb_screen->setDirty( r | data->bound );
    QScreenCursor::move( x, y );
    QWSDisplay::ungrab();
}
#endif


template <const int depth, const int type>
class QGfxVFb : public QGfxRaster<depth,type>
{
public:
    QGfxVFb(unsigned char *b,int w,int h);
    virtual ~QGfxVFb();

    virtual void drawPoint( int,int );
    virtual void drawPoints( const QPointArray &,int,int );
    virtual void drawLine( int,int,int,int );
    virtual void fillRect( int,int,int,int );
    virtual void drawPolyline( const QPointArray &,int,int );
    virtual void drawPolygon( const QPointArray &,bool,int,int );
    virtual void blt( int,int,int,int,int,int );
    virtual void scroll( int,int,int,int,int,int );
#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
    virtual void stretchBlt( int,int,int,int,int,int );
#endif
    virtual void tiledBlt( int,int,int,int );
};

template <const int depth, const int type>
QGfxVFb<depth,type>::QGfxVFb(unsigned char *b,int w,int h)
    : QGfxRaster<depth,type>( b, w, h )
{
}

template <const int depth, const int type>
QGfxVFb<depth,type>::~QGfxVFb()
{
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::drawPoint( int x, int y )
{
    QWSDisplay::grab( TRUE );
    if ( this->is_screen_gfx )
	qvfb_screen->setDirty( QRect( x+this->xoffs, y+this->yoffs, 1, 1 ) );
    QGfxRaster<depth,type>::drawPoint( x, y );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::drawPoints( const QPointArray &pa,int x,int y )
{
    QWSDisplay::grab( TRUE );
    if ( this->is_screen_gfx )
	qvfb_screen->setDirty( this->clipbounds );
    QGfxRaster<depth,type>::drawPoints( pa, x, y );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::drawLine( int x1,int y1,int x2,int y2 )
{
    QWSDisplay::grab( TRUE );
    if ( this->is_screen_gfx ) {
	QRect r;
	r.setCoords( x1+this->xoffs, y1+this->yoffs, x2+this->xoffs, y2+this->yoffs );
	qvfb_screen->setDirty( r.normalize() );
    }
    QGfxRaster<depth,type>::drawLine( x1, y1, x2, y2 );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::fillRect( int x,int y,int w,int h )
{
    QWSDisplay::grab( TRUE );
    if ( this->is_screen_gfx )
	qvfb_screen->setDirty( QRect( x+this->xoffs, y+this->yoffs, w, h ) );
    QGfxRaster<depth,type>::fillRect( x, y, w, h );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::drawPolyline( const QPointArray &pa,int x,int y )
{
    QWSDisplay::grab( TRUE );
    if ( this->is_screen_gfx )
	qvfb_screen->setDirty( this->clipbounds );
    QGfxRaster<depth,type>::drawPolyline( pa, x, y );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::drawPolygon( const QPointArray &pa,bool w,int x,int y )
{
    QWSDisplay::grab( TRUE );
    if ( this->is_screen_gfx )
	qvfb_screen->setDirty( this->clipbounds );
    QGfxRaster<depth,type>::drawPolygon( pa, w, x, y );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::blt( int x,int y,int w,int h, int sx, int sy )
{
    QWSDisplay::grab( TRUE );
    if ( this->is_screen_gfx )
	qvfb_screen->setDirty( QRect( x+this->xoffs, y+this->yoffs, w, h ) );
    QGfxRaster<depth,type>::blt( x, y, w, h, sx, sy );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::scroll( int x,int y,int w,int h,int sx,int sy )
{
    QWSDisplay::grab( TRUE );
    int dy = sy - y;
    int dx = sx - x;
    if ( this->is_screen_gfx )
	qvfb_screen->setDirty( QRect(QMIN(x,sx) + this->xoffs, QMIN(y,sy) + this->yoffs,
			   w+abs(dx), h+abs(dy)) );
    QGfxRaster<depth,type>::scroll( x, y, w, h, sx, sy );
    QWSDisplay::ungrab();
}

#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
template <const int depth, const int type>
void QGfxVFb<depth,type>::stretchBlt( int x,int y,int w,int h,int sx,int sy )
{
    QWSDisplay::grab( TRUE );
    if ( this->is_screen_gfx )
	qvfb_screen->setDirty( QRect( x + this->xoffs, y + this->yoffs, w, h) );
    QGfxRaster<depth,type>::stretchBlt( x, y, w, h, sx, sy );
    QWSDisplay::ungrab();
}
#endif

template <const int depth, const int type>
void QGfxVFb<depth,type>::tiledBlt( int x,int y,int w,int h )
{
    QWSDisplay::grab( TRUE );
    if ( this->is_screen_gfx )
	qvfb_screen->setDirty( QRect(x + this->xoffs, y + this->yoffs, w, h) );
    QGfxRaster<depth,type>::tiledBlt( x, y, w, h );
    QWSDisplay::ungrab();
}


/*
*/

QVFbScreen::QVFbScreen( int display_id ) : QScreen( display_id )
{
    qvfb_screen = this;
    shmrgn = 0;
    hdr = 0;
    data = 0;
}

QVFbScreen::~QVFbScreen()
{
}


static int QVFb_dummy;

bool QVFbScreen::connect( const QString & )
{
    optype = &QVFb_dummy;
    key_t key = ftok( QString(QT_VFB_MOUSE_PIPE).arg(displayId).latin1(), 'b' );

    if ( key == -1 )
	return FALSE;

    int shmId = shmget( key, 0, 0 );
    if ( shmId != -1 )
	shmrgn = (unsigned char *)shmat( shmId, 0, 0 );
    else
	return FALSE;

    if ( (int)shmrgn == -1 || shmrgn == 0 )
	return FALSE;

    hdr = (QVFbHeader *) shmrgn;
    data = shmrgn + hdr->dataoffset;

    dw = w = hdr->width;
    dh = h = hdr->height;
    d = hdr->depth;
    lstep = hdr->linestep;

#ifdef DEBUG
    qDebug( "Connected to VFB server: %d x %d x %d", w, h, d );
#endif

// 18-bpp-support
    if ( (d == 18) || (d == 19) )
	d = 24;
// End of 18-bpp-support
    size = lstep * h;
    mapsize = size;
    screencols = hdr->numcols;
    memcpy( screenclut, hdr->clut, sizeof( QRgb ) * screencols );

    qvfbEnabled = TRUE;
    return TRUE;
}

void QVFbScreen::disconnect()
{
    if ( (int)shmrgn != -1 && shmrgn ) {
	shmdt((char*)shmrgn );
	shmrgn = 0;
    }
}

bool QVFbScreen::initDevice()
{
    optype = &QVFb_dummy;
    if(d==8) {
	screencols=256;
#ifndef QT_NO_QWS_DEPTH_8GRAYSCALE
	// Build greyscale palette
	for(int loopc=0;loopc<256;loopc++) {
	    screenclut[loopc]=qRgb(loopc,loopc,loopc);
	}
#else
	// 6x6x6 216 color cube
	int idx = 0;
	for( int ir = 0x0; ir <= 0xff; ir+=0x33 ) {
	    for( int ig = 0x0; ig <= 0xff; ig+=0x33 ) {
		for( int ib = 0x0; ib <= 0xff; ib+=0x33 ) {
		    screenclut[idx]=qRgb( ir, ig, ib );
		    idx++;
		}
	    }
	}
	screencols=idx;
#endif
	memcpy( hdr->clut, screenclut, sizeof( QRgb ) * screencols );
	hdr->numcols = screencols;
    } else if ( d == 4 ) {
	int val = 0;
	for ( int idx = 0; idx < 16; idx++, val += 17 ) {
	    screenclut[idx] = qRgb( val, val, val );
	}
	screencols = 16;
	memcpy( hdr->clut, screenclut, sizeof( QRgb ) * screencols );
	hdr->numcols = screencols;
    } else if ( d == 1 ) {
	screencols = 2;
	screenclut[1] = qRgb( 0xff, 0xff, 0xff );
	screenclut[0] = qRgb( 0, 0, 0 );
	memcpy( hdr->clut, screenclut, sizeof( QRgb ) * screencols );
	hdr->numcols = screencols;
    }

    return true;
}

void QVFbScreen::shutdownDevice()
{
}

int QVFbScreen::initCursor(void* e, bool init)
{
#ifndef QT_NO_QWS_CURSOR
    qt_sw_cursor=true;
    // ### until QLumpManager works Ok with multiple connected clients,
    // we steal a chunk of shared memory
    SWCursorData *data = (SWCursorData *)e - 1;
    qt_screencursor=new QVFbScreenCursor();
    qt_screencursor->init( data, init );
    return sizeof(SWCursorData);
#else
    Q_UNUSED(e);
    Q_UNUSED(init);
    return 0;
#endif
}

void QVFbScreen::setMode(int ,int ,int)
{
}

// save the state of the graphics card
// This is needed so that e.g. we can restore the palette when switching
// between linux virtual consoles.
void QVFbScreen::save()
{
    // nothing to do.
}

// restore the state of the graphics card.
void QVFbScreen::restore()
{
}

QGfx * QVFbScreen::createGfx(unsigned char * bytes,int w,int h,int d, int linestep)
{
    QGfx* ret = 0;
    if(d==1) {
	if ( bytes == qt_screen->base() )
	    ret = new QGfxVFb<1,0>(bytes,w,h);
	else
	    ret = new QGfxRaster<1,0>(bytes,w,h);
#ifndef QT_NO_QWS_DEPTH_4
    } else if (d==4) {
	if ( bytes == qt_screen->base() )
	    ret = new QGfxVFb<4,0>(bytes,w,h);
	else
	    ret = new QGfxRaster<4,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_16
    } else if(d==16) {
	if ( bytes == qt_screen->base() )
	    ret = new QGfxVFb<16,0>(bytes,w,h);
	else
	    ret = new QGfxRaster<16,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_8
    } else if (d==8) {
	if ( bytes == qt_screen->base() )
	    ret = new QGfxVFb<8,0>(bytes,w,h);
	else
	    ret = new QGfxRaster<8,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_8GRAYSCALE
    } else if (d==8) {
	if ( bytes == qt_screen->base() )
	    ret = new QGfxVFb<8,0>(bytes,w,h);
	else
	    ret = new QGfxRaster<8,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_32
    } else if (d==32) {
	if ( bytes == qt_screen->base() )
	    ret = new QGfxVFb<32,0>(bytes,w,h);
	else
	    ret = new QGfxRaster<32,0>(bytes,w,h);
#endif
// 18-bpp-support
#ifndef QT_NO_QWS_DEPTH_24
    } else if (d==24) {
	if ( bytes == qt_screen->base() )
	    ret = new QGfxVFb<24,0>(bytes,w,h);
	else
	    ret = new QGfxRaster<24,0>(bytes,w,h);
#endif
// 18-bpp-support
    } else {
	qFatal("Can't drive depth %d",d);
    }
    ret->setLineStep(linestep);
    return ret;
}

extern "C" QScreen * qt_get_screen_qvfb( int display_id )
{
    return new QVFbScreen( display_id );
}

#endif

