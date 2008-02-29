/****************************************************************************
** $Id: qt/src/kernel/qcursor_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of QCursor class for FB
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

#include "qcursor.h"

#ifndef QT_NO_CURSOR
#include "qbitmap.h"
#include "qimage.h"
#include "qapplication.h"
#include "qdatastream.h"
#include "qwsdisplay_qws.h"

static int nextCursorId = BitmapCursor + 1;

/*****************************************************************************
  Internal QCursorData class
 *****************************************************************************/

struct QCursorData : public QShared
{
    QCursorData( int s = 0, int i = 0 );
   ~QCursorData();
    int	      cshape;
    int       id;
    QBitmap  *bm, *bmm;
    short     hx, hy;
};

QCursorData::QCursorData( int s, int i )
{
    cshape = s;
    id = i;
    bm = bmm = 0;
    hx = hy  = 0;
}

QCursorData::~QCursorData()
{
    if ( bm )
	delete bm;
    if ( bmm )
	delete bmm;
}


/*****************************************************************************
  Global cursors
 *****************************************************************************/

static const int cursors = 15;
static QCursor cursorTable[cursors];

static const int arrowCursorIdx = 0;

QT_STATIC_CONST_IMPL QCursor & Qt::arrowCursor = cursorTable[0];
QT_STATIC_CONST_IMPL QCursor & Qt::upArrowCursor = cursorTable[1];
QT_STATIC_CONST_IMPL QCursor & Qt::crossCursor = cursorTable[2];
QT_STATIC_CONST_IMPL QCursor & Qt::waitCursor = cursorTable[3];
QT_STATIC_CONST_IMPL QCursor & Qt::ibeamCursor = cursorTable[4];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeVerCursor = cursorTable[5];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeHorCursor = cursorTable[6];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeBDiagCursor = cursorTable[7];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeFDiagCursor = cursorTable[8];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeAllCursor = cursorTable[9];
QT_STATIC_CONST_IMPL QCursor & Qt::blankCursor = cursorTable[10];
QT_STATIC_CONST_IMPL QCursor & Qt::splitHCursor = cursorTable[11];
QT_STATIC_CONST_IMPL QCursor & Qt::splitVCursor = cursorTable[12];
QT_STATIC_CONST_IMPL QCursor & Qt::pointingHandCursor = cursorTable[13];
QT_STATIC_CONST_IMPL QCursor & Qt::forbiddenCursor = cursorTable[14];


QCursor *QCursor::find_cur( int shape )		// find predefined cursor
{
    return (uint)shape <= LastCursor ? &cursorTable[shape] : 0;
}


static bool initialized = FALSE;

void QCursor::cleanup()
{
    int shape;
    for( shape = 0; shape < cursors; shape++ ) {
	delete cursorTable[shape].data;
	cursorTable[shape].data = 0;
    }
    initialized = FALSE;
}


void QCursor::initialize()
{
    int shape;
    for( shape = 0; shape < cursors; shape++ )
	cursorTable[shape].data = new QCursorData( shape, shape );
    initialized = TRUE;
    qAddPostRoutine( cleanup );
}

HANDLE QCursor::handle() const
{
    return (HANDLE)data->id;
}


QCursor::QCursor()
{
    if ( !initialized ) {
	if ( !qApp || qApp->startingUp() ) {
	    data = 0;
	    return;
	}
	initialize();
    }
    QCursor* c = &cursorTable[arrowCursorIdx];
    c->data->ref();
    data = c->data;
}



QCursor::QCursor( int shape )
{
    if ( !initialized )
	initialize();
    QCursor *c = find_cur( shape );
    if ( !c )					// not found
	c = &cursorTable[arrowCursorIdx];	//   then use arrowCursor
    c->data->ref();
    data = c->data;
}


void QCursor::setBitmap( const QBitmap &bitmap, const QBitmap &mask,
			 int hotX, int hotY )
{
    if ( !initialized )
	initialize();
    if ( bitmap.depth() != 1 || mask.depth() != 1 ||
	 bitmap.size() != mask.size() ) {
#if defined(CHECK_NULL)
	qWarning( "QCursor: Cannot create bitmap cursor; invalid bitmap(s)" );
#endif
	QCursor *c = &cursorTable[arrowCursorIdx];
	c->data->ref();
	data = c->data;
	return;
    }
    data = new QCursorData;
    CHECK_PTR( data );
    data->bm  = new QBitmap( bitmap );
    data->bmm = new QBitmap( mask );
    data->cshape = BitmapCursor;
    data->id = nextCursorId++;
    data->hx = hotX >= 0 ? hotX : bitmap.width()/2;
    data->hy = hotY >= 0 ? hotY : bitmap.height()/2;

    QPaintDevice::qwsDisplay()->defineCursor(data->id, *data->bm,
					    *data->bmm, data->hx, data->hy);
}

QCursor::QCursor( const QCursor &c )
{
    if ( !initialized )
	initialize();
    data = c.data;				// shallow copy
    data->ref();
}

QCursor::~QCursor()
{
    if ( data && data->deref() )
	delete data;
}


QCursor &QCursor::operator=( const QCursor &c )
{
    if ( !initialized )
	initialize();
    c.data->ref();				// avoid c = c
    if ( data->deref() )
	delete data;
    data = c.data;
    return *this;
}


int QCursor::shape() const
{
    if ( !initialized )
	initialize();
    return data->cshape;
}

void QCursor::setShape( int shape )
{
    if ( !initialized )
	initialize();
    QCursor *c = find_cur( shape );		// find one of the global ones
    if ( !c )					// not found
	c = &cursorTable[arrowCursorIdx];	//   then use arrowCursor
    c->data->ref();
    if ( data->deref() )			// make shallow copy
	delete data;
    data = c->data;
}


const QBitmap *QCursor::bitmap() const
{
    if ( !initialized )
	initialize();
    return data->bm;
}

const QBitmap *QCursor::mask() const
{
    if ( !initialized )
	initialize();
    return data->bmm;
}

QPoint QCursor::hotSpot() const
{
    if ( !initialized )
	initialize();
    return QPoint( data->hx, data->hy );
}

void QCursor::update() const
{
    if ( !initialized )
	initialize();
    register QCursorData *d = data;		// cheat const!

    if ( d->cshape == BitmapCursor ) {
	// XXX
	return;
    }
    if ( d->cshape >= SizeVerCursor && d->cshape < SizeAllCursor ||
	 d->cshape == BlankCursor ) {
	//	int i = (d->cshape - SizeVerCursor)*2;
	// XXX data: cursor_bits16[i], 16,16
	// XXX mask: cursor_bits16[i+1], 16,16
	return;
    }
    if ( d->cshape >= SplitVCursor && d->cshape <= PointingHandCursor ) {
	//int i = (d->cshape - SplitVCursor)*2;
	// XXX data: cursor_bits32[i], 32, 32
	// XXX mask: cursor_bits32[i+1], 32, 32
	//int hs = d->cshape != PointingHandCursor? 16 : 0;
	// XXX ...
	return;
    }

    // XXX standard shapes?
}

#endif //QT_NO_CURSOR



extern int *qt_last_x,*qt_last_y;

QPoint QCursor::pos()
{
    // This doesn't know about hotspots yet so we disable it
    //qt_accel_update_cursor();
    if ( qt_last_x )
	return QPoint( *qt_last_x,*qt_last_y );
    else
	return QPoint();
}

void QCursor::setPos( int x, int y )
{
    // Avoid potential loop
    if (pos() == QPoint(x,y))
	return;
    QPaintDevice::qwsDisplay()->moveCursor(x, y);
}

