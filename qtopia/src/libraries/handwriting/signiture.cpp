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

#include "signiture.h"
#include "stroke.h"

#define QIMPEN_CORRELATION_POINTS   25
int arcTan( int dy, int dx )
{
    if ( dx == 0 ) {
        if ( dy >= 0 )
            return 64;
        else
            return 192;
    }

    if ( dy == 0 ) {
        if ( dx >= 0 )
            return 0;
        else
            return 128;
    }

    static int table[5][5] = {
        { 32, 19, 13, 10, 8  },
        { 45, 32, 24, 19, 16 },
        { 51, 40, 32, 26, 22 },
        { 54, 45, 37, 32, 27 },
        { 56, 49, 42, 37, 32 } };

    if ( dy > 0 ) {
        if ( dx > 0 )
            return table[dy-1][dx-1];
        else
            return 128 - table[dy-1][QABS(dx)-1];
    } else {
        if ( dx > 0 )
            return 256 - table[QABS(dy)-1][dx-1];
        else
            return 128 + table[QABS(dy)-1][QABS(dx)-1];
    }

    return 0;
}

QIMPenSigniture::QIMPenSigniture() : QArray<int>()
{
}

QIMPenSigniture::QIMPenSigniture(const QArray<int> &o)
    : QArray<int>(o)
{
}

QIMPenSigniture::QIMPenSigniture(const QIMPenSigniture &o)
    : QArray<int>(o)
{
}

void QIMPenSigniture::setStroke(const QIMPenStroke &stroke)
{
    resize(0);
    calcSigniture(stroke);
    if (count() != QIMPEN_CORRELATION_POINTS)
	scale( QIMPEN_CORRELATION_POINTS, loops() );
}

/*!
  Silly name.  Create an array from \a a that has \a e points extra at the start and
  end to enable a sliding correlation to be performed.
*/
QArray<int> QIMPenSigniture::createBase( const QArray<int> &a, int e )
{
    QArray<int> ra( a.count() + 2*e );
    int i;
    for ( i = 0; i < e; i++ ) {
        ra[i] = a[0];
        ra[(int)a.count() + e + i - 1] = a[(int)a.count() - 1];
    }
    for ( i = 0; i < (int)a.count(); i++ ) {
        ra[i+e] = a[i];
    }

    return ra;
}

/*!
  Scale the points in array \a s to \a count points.
  This is braindead at the moment (no smooth scaling) and fixing this is
  probably one of the simpler ways to improve performance.

  The \a t parameter is if it loops.
*/
void QIMPenSigniture::scale( unsigned int dcount, bool t )
{
    uint scount = count();

    int si = 0;
    if ( scount > dcount ) {
        int next = 0;
        for ( uint i = 0; i < dcount; i++ ) {
            next = (i+1) * scount / dcount;
            int maxval = 0;
            if ( t ) {
                for ( int j = si; j < next; j++ ) {
                    maxval = at(j) > maxval ? at(j) : maxval;
                }
            }
            int sum = 0;
            for ( int j = si; j < next; j++ ) {
                if ( t && maxval - at(j) > 128 )
                    sum += 256;
                sum += at(j);
            }
            at(int(i)) = sum / (next-si);
            if ( t && at(int(i)) > 256 )
                at(int(i)) %= 256;
            si = next;
        }
	resize(dcount);
    } else {
	resize(dcount);
	// could be better
        for ( int i = int(dcount)-1; i >= 0; i-- ) {
            si = i * scount / dcount;
            at(i)= at(si);
        }
    }
}


int QIMPenSigniture::calcError(const QIMPenSigniture &other) const
{
    if (slides()) {
	QArray<int> base = createBase( *this, 2 );
	int err = INT_MAX;
	for ( int i = 0; i < 4; i++ ) {
	    int e = calcError( base, other, i, loops() );
	    if ( e < err )
		err = e;
	}
	return err;
    } else {
	return calcError(*this, other, 0, loops());
    }
}
/*!
  Perform a correlation of the supplied arrays.  \a base should have
  win.count() + 2 * \a off points to enable sliding \a win over the
  \a base data.  If \a t is TRUE, the comparison takes into account
  the circular nature of the angular data.
  Returns the best (lowest error) match.
*/

int QIMPenSigniture::calcError( const QArray<int> &base,
                           const QArray<int> &win, int off, bool t )
{
    int err = 0;

    for ( unsigned i = 0; i < win.count(); i++ ) {
        int d = QABS( base[(int)i+off] - win[(int)i] );
        if ( t && d > 128 )
            d -= 256;
        err += QABS( d );
    }

    err /= win.count();

    return err;
}

TanSigniture::TanSigniture(const QIMPenStroke &stroke)
{
    setStroke(stroke);
}

TanSigniture::~TanSigniture() {}

void TanSigniture::calcSigniture(const QIMPenStroke &stroke)
{
    uint dist = 5; // number of points to include in calculation
    if ( stroke.chain().count() <= dist ) {
        resize(1);
        int dx = 0;
        int dy = 0;
        for ( uint j = 0; j < stroke.chain().count(); j++ ) {
            dx += stroke.chain()[j].dx;
            dy += stroke.chain()[j].dy;
        }
        at(0) = arcTan( dy, dx );
    } else {
        resize( (stroke.chain().count()-dist+1) / 2 );
        int idx = 0;
        for ( uint i = 0; i < (int)stroke.chain().count() - dist; i += 2 ) {
            int dx = 0;
            int dy = 0;
            for ( uint j = 0; j < dist; j++ ) {
                dx += stroke.chain()[i+j].dx;
                dy += stroke.chain()[i+j].dy;
            }
            at(idx++) = arcTan( dy, dx );
        }
    }
}

AngleSigniture::AngleSigniture(const QIMPenStroke &stroke)
{
    setStroke(stroke);
}

AngleSigniture::~AngleSigniture() {}

void AngleSigniture::calcSigniture(const QIMPenStroke &stroke)
{
    QPoint c = stroke.center();

    uint dist = 3; // number of points to include in calculation
    if ( stroke.chain().count() <= dist ) {
	resize(1);
	at(0) = 1;
    } else {
	resize( stroke.chain().count() );
	QPoint current(0, 0);
	int idx = 0;
	for ( uint i = 0; i < stroke.chain().count(); i++ ) {
	    int dx = c.x() - current.x();
	    int dy = c.y() - current.y();
	    int md = QMAX( QABS(dx), QABS(dy) );
	    if ( md > 5 ) {
		dx = dx * 5 / md;
		dy = dy * 5 / md;
	    }
	    at(idx++) = arcTan( dy, dx );
	    current += QPoint( stroke.chain()[i].dx, stroke.chain()[i].dy );
	}
    }
}

DistSigniture::DistSigniture(const QIMPenStroke &stroke)
{
    setStroke(stroke);
}

DistSigniture::~DistSigniture()
{
}

void DistSigniture::calcSigniture(const QIMPenStroke &stroke)
{
    resize( (stroke.chain().count()+1)/2 );
    QPoint c = stroke.center();
    QPoint pt( 0, 0 );

    int minval = INT_MAX;
    int maxval = 0;
    uint idx = 0, i;
    for ( i = 0; i < stroke.chain().count(); i += 2 ) {
        int dx = c.x() - pt.x();
        int dy = c.y() - pt.y();
        if ( dx == 0 && dy == 0 )
            at(idx) = 0;
        else
            at(idx) = dx*dx + dy*dy;

        if ( at(idx) > maxval )
            maxval = at(idx);
        if ( at(idx) < minval )
            minval = at(idx);
        pt.rx() += stroke.chain()[i].dx;
        pt.ry() += stroke.chain()[i].dy;
        idx++;
    }

    // normalise 0-255
    int div = maxval - minval;
    if ( div == 0 ) div = 1;
    for ( i = 0; i < count(); i++ ) {
        at(i) = (at(i) - minval ) * 255 / div;
    }

}

