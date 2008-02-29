/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qfile.h>
#include <QtAlgorithms>
// #include <qtl.h>
#include <math.h>
#include <limits.h>
#include <qdatastream.h>
#include "stroke.h"

#define QIMPEN_CORRELATION_POINTS   25

/*!
  \class QIMPenStroke
  \brief The QIMPenStroke class handles a single stroke.

  Can calculate closeness of match to
  another stroke.
  \ingroup qtopiahandwriting
*/

bool QIMPenStroke::useVertPos = true;

QIMPenStroke::QIMPenStroke() : cheight(75)
{
}

QIMPenStroke::QIMPenStroke( const QIMPenStroke &st )
    : startPoint( st.startPoint ), lastPoint( st.lastPoint ),
    links( st.links ), cheight(st.cheight)
{
}

QIMPenStroke &QIMPenStroke::operator=( const QIMPenStroke &s )
{
    clear();
    startPoint = s.startPoint;
    lastPoint = s.lastPoint;
    qCopy( s.links.begin(), s.links.end(), this->links.begin() );
    // links = s.links.copy();
    cheight = s.cheight;

    return *this;
}

void QIMPenStroke::clear()
{
    startPoint = QPoint(0,0);
    lastPoint = QPoint( 0, 0 );
    links.resize( 0 );
    tsig.resize( 0 );
    dsig.resize( 0 );
    asig.resize( 0 );
}

/*!
  Begin inputting a new stroke at position \a p.
*/
void QIMPenStroke::beginInput( QPoint p )
{
    clear();
    startPoint = p;
    bounding = QRect();
    internalAddPoint( p );
}

/*!
  Add a point \a p to the stroke's shape.
  Returns true if the point was successfully added.
*/
bool QIMPenStroke::addPoint( QPoint p )
{
    if ( links.count() > 500 ) // sanity check (that the user is sane).
        return false;

    int dx = p.x() - lastPoint.x();
    int dy = p.y() - lastPoint.y();
    if ( qAbs( dx ) > 1 || qAbs( dy ) > 1 ) {
        // The point is not adjacent to the previous point, so we fill
        // in with a straight line.  Some kind of non-linear
        // interpolation might be better.
        int x = lastPoint.x();
        int y = lastPoint.y();
        int ix = 1;
        int iy = 1;
        if ( dx < 0 ) {
            ix = -1;
            dx = -dx;
        }
        if ( dy < 0 ) {
            iy = -1;
            dy = -dy;
        }
        int d = 0;
        if ( dx < dy ) {
            d = dx;
            do {
                y += iy;
                d += dx;
                if ( d > dy ) {
                    x += ix;
                    d -= dy;
                }
                internalAddPoint( QPoint( x, y ) );
            } while ( y != p.y() );
        } else {
            d = dy;
            do {
                x += ix;
                d += dy;
                if ( d > dx ) {
                    y += iy;
                    d -= dx;
                }
                internalAddPoint( QPoint( x, y ) );
            } while ( x != p.x() );
        }
    } else {
        internalAddPoint( p );
    }

    return true;
}

/*!
  Finish inputting a stroke.
*/
void QIMPenStroke::endInput()
{
    if ( links.count() < 3 ) {
        QIMPenGlyphLink gl;
        links.resize(1);
        gl.dx = 1;
        gl.dy = 0;
        links[0] = gl;
    }
}

/*!
  Return an indicator of the closeness of this stroke to \a pen.
  Lower value is better.
*/
unsigned int QIMPenStroke::match( QIMPenStroke *pen )
{
    double lratio;

    if ( links.count() > pen->links.count() )
        lratio = (links.count()+2) / (pen->links.count()+2);
    else
        lratio =  (pen->links.count()+2) / (links.count()+2);

    lratio -= 1.0;

    if ( lratio > 2.0 ) {
        return 400000;
    }

    createSignatures();
    pen->createSignatures();

    int ch = canvasHeight();
    int pch = pen->canvasHeight();

    int vdiff, evdiff;
    if (!useVertPos) {
        // if they are different, need a differnt way of measureing
        int h = boundingRect().height();
        int ph = pen->boundingRect().height();

        int sh = h * 75 / ch;
        int sph = ph * 75 / pch;

        if (((sh << 1) < sph && sph > 4)
                || ((sph << 1) < ph && ph > 4)) {
            return 400000;
        }
        vdiff = 0;
        evdiff = 0;
    } else {
        int sp = startPoint.y();
        int psp = pen->startPoint.y();
        vdiff = qAbs(sp - psp);

        // Insanely offset?
        if ( vdiff > 18 ) {
            return 400000;
        }

        vdiff -= 4;
        if ( vdiff < 0 )
            vdiff = 0;

        // Ending point offset
        int lp = lastPoint.y();
        int plp = pen->lastPoint.y();
        evdiff = qAbs(lp - plp);
        // Insanely offset?
        if ( evdiff > 20 ) {
            return 400000;
        }

        evdiff -= 5;
        if ( evdiff < 0 )
            evdiff = 0;
    }

    // do a correlation with the three available signatures.
    int err1 = INT_MAX;
    int err2 = INT_MAX;
    int err3 = INT_MAX;

    err1 = tsig.calcError(pen->tsig);
    if ( err1 > tsig.maxError() ) {  // no need for more matching
        return 400000;
    }

    // maybe a sliding window is worthwhile for these too.
    err2 = dsig.calcError( pen->dsig );
    if ( err2 > dsig.maxError() ) {
        return 400000;
    }

    err3 = asig.calcError( pen->asig );
    if ( err3 > asig.maxError() ) {
        return 400000;
    }

    // Some magic numbers here - the addition reduces the weighting of
    // the error and compensates for the different error scales.  I
    // consider the tangent signature to be the best indicator, so it
    // has the most weight.  This ain't rocket science.
    // Basically, these numbers are the tuning factors.
    unsigned int err = ( err1 + tsig.weight() )
            * ( err2 + dsig.weight() )
            * ( err3 + asig.weight() )
            + vdiff * 1000 + evdiff * 500
            + (unsigned int)(lratio * 5000.0);

    return err;
}

/*!
  Return the bounding rect of this stroke.
*/
QRect QIMPenStroke::boundingRect() const
{
    if ( !bounding.isValid() ) {
        int x = startPoint.x();
        int y = startPoint.y();
        bounding = QRect( x, y, 1, 1 );

        for ( int i = 0; i < (int)links.count(); i++ ) {
            x += links[i].dx;
            y += links[i].dy;
            if ( x < bounding.left() )
                bounding.setLeft( x );
            if ( x > bounding.right() )
                bounding.setRight( x );
            if ( y < bounding.top() )
                bounding.setTop( y );
            if ( y > bounding.bottom() )
                bounding.setBottom( y );
        }
    }

    return bounding;
}


/*!
  Creates signatures used in matching if not already created.
*/
void QIMPenStroke::createSignatures()
{
    if ( tsig.isEmpty() )
        tsig.setStroke(*this);
    if ( asig.isEmpty() )
        asig.setStroke(*this);
    if ( dsig.isEmpty() )
        dsig.setStroke(*this);
}

/*!
  Add another point \a p to the stroke's shape.
*/
void QIMPenStroke::internalAddPoint( QPoint p )
{
    if ( p == lastPoint )
        return;

    if ( !lastPoint.isNull() ) {
        QIMPenGlyphLink gl;
        gl.dx = p.x() - lastPoint.x();
        gl.dy = p.y() - lastPoint.y();

        /*ALARM BELLS*/
        links.resize( links.size() + 1 );   //### resize by 1 is bad
        links[(int)links.size() - 1] = gl;
    }

    lastPoint = p;
    bounding = QRect();
}

/*!
  Calculate the center of gravity of the stroke.
*/
QPoint QIMPenStroke::center() const
{
    QPoint pt( 0, 0 );
    int ax = 0;
    int ay = 0;

    for ( int i = 0; i < (int)links.count(); i++ ) {
        pt.rx() += links[i].dx;
        pt.ry() += links[i].dy;
        ax += pt.x();
        ay += pt.y();
    }

    ax /= (int)links.count();
    ay /= (int)links.count();

    return QPoint( ax, ay );
}
/*!
  Write the character's data to the stream.
*/
QDataStream &operator<< (QDataStream &s, const QIMPenStroke &ws)
{
    s << ws.startPoint;
    s << ws.links.count();
    for ( int i = 0; i < (int)ws.links.count(); i++ ) {
        s << (qint8)ws.links[i].dx;
        s << (qint8)ws.links[i].dy;
    }

    return s;
}

/*!
  Read the character's data from the stream.
*/
QDataStream &operator>> (QDataStream &s, QIMPenStroke &ws)
{
    qint8 i8;
    s >> ws.startPoint;
    ws.lastPoint = ws.startPoint;
    unsigned size;
    s >> size;
    ws.links.resize( size );
    for ( int i = 0; i < (int)size; i++ ) {
        s >> i8;
        ws.links[i].dx = i8;
        s >> i8;
        ws.links[i].dy = i8;
        ws.lastPoint += QPoint( ws.links[i].dx, ws.links[i].dy );
    }

    return s;
}


