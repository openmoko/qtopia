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

#include "analogclock.h"

#include <qlayout.h>
#include <qpainter.h>
#include <qimage.h>
#include <qtopia/global.h>
#include <qtopia/resource.h>

#include <math.h>

const double deg2rad = 0.017453292519943295769;	// pi/180

static QRect expand(const QRect &r, int e=1)
{
    QRect er(r);
    er.rLeft() -= e;
    er.rTop() -= e;
    er.rRight() += e;
    er.rBottom() += e;

    return er;
}

AnalogClock::AnalogClock( QWidget *parent, const char *name )
    : QFrame( parent, name ), isEvent(TRUE)
{
    setMinimumSize(50,50);
}

QSizePolicy AnalogClock::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

void AnalogClock::setFace(const QString &f)
{
    face = f;
}

void AnalogClock::drawContents( QPainter *pp )
{
#if !defined(NO_DEBUG)
    static bool first = true;
    if ( first ) {
	QTOPIA_PROFILE("first paint event");
	first = false;
    }
#endif

    QRect r = contentsRect();

    int size = QMIN(r.width(), r.height());
    QRect pr(0, 0, size, size);
    if (buf.width() != size || buf.height() != size) {
	buf.resize(size, size);
	QPainter p(&buf);
	if (!face.isEmpty()) {
	    QImage img = Resource::loadImage(face);
	    if (!img.isNull()) {
		img = img.smoothScale(size, size);
		facePm.convertFromImage(img);
	    }
	}
	p.fillRect(0,0,size,size,palette().normal().brush(QColorGroup::Background));
	if (!facePm.isNull())
	    p.drawPixmap(0,0,facePm);
    }

    QPainter bp(&buf);

    QPoint center( size / 2, size / 2 );

    const int w_tick = pr.width()/300+1;
    const int w_sec = pr.width()/400+1;
    const int w_hour = pr.width()/80+1;

    QPoint l1( pr.x() + pr.width() / 2, pr.y() + 2 );
    QPoint l2( pr.x() + pr.width() / 2, pr.y() + 8 );

    QPoint h1( pr.x() + pr.width() / 2, pr.y() + pr.height() / 4 );
    QPoint h2( pr.x() + pr.width() / 2, pr.y() + pr.height() / 2 );

    QPoint m1( pr.x() + pr.width() / 2, pr.y() + pr.height() / 9 );
    QPoint m2( pr.x() + pr.width() / 2, pr.y() + pr.height() / 2 );

    QPoint s1( pr.x() + pr.width() / 2, pr.y() + 8 );
    QPoint s2( pr.x() + pr.width() / 2, pr.y() + pr.height() / 2 );

    QColor color(black);
    QTime time = currTime;
    QPoint offs((r.width()-size)/2, (r.height()-size)/2);

    if ( isEvent || prevTime.minute() != currTime.minute()
	    || prevTime.hour() != currTime.hour()
	    || QABS(prevTime.secsTo(currTime)) > 1 ) {
	bp.setBrushOrigin(pp->brushOrigin() - offs);
	bp.fillRect( 0, 0, buf.width(), buf.height(), palette().normal().brush(QColorGroup::Background));
	if (!facePm.isNull())
	    bp.drawPixmap(0,0,facePm);

	// draw ticks
	if (facePm.isNull()) {
	    bp.setPen( QPen( color, w_tick ) );
	    for ( int i = 0; i < 12; i++ )
		bp.drawLine( rotate( center, l1, i * 30 ), rotate( center, l2, i * 30 ) );
	}

	// draw hour pointer
	h1 = rotate( center, h1, 30 * ( time.hour() % 12 ) + time.minute() / 2 );
	h2 = rotate( center, h2, 30 * ( time.hour() % 12 ) + time.minute() / 2 );
	bp.setPen( color );
	bp.setBrush( color );
	drawHand( &bp, h1, h2 );

	// draw minute pointer
	m1 = rotate( center, m1, time.minute() * 6 );
	m2 = rotate( center, m2, time.minute() * 6 );
	bp.setPen( color );
	bp.setBrush( color );
	drawHand( &bp, m1, m2 );

	changed = QRect(0, 0, buf.width(), buf.height());
    }

    prevTime = currTime;

    if (!isEvent) {
	changed.translate(offs.x(), offs.y());
	pp->setClipRegion(changed);
    }
    pp->drawPixmap(offs.x(), offs.y(), buf);
    changed = QRegion();
    pp->setClipping(FALSE);

    // draw second pointer
    s1 = rotate( center, s1, time.second() * 6 );
    s2 = rotate( center, s2, time.second() * 6 );
    QRect sr = QRect(s1, s2).normalize();
    pp->setPen( QPen( color, w_sec ) );
    QImage img(sr.width(), sr.height(), 32);
    img.fill(0x00000000);
    img.setAlphaBuffer(TRUE);
    s1.rx() -= sr.x();
    s1.ry() -= sr.y();
    s2.rx() -= sr.x();
    s2.ry() -= sr.y();
    aaLine(img, s1.x(), s1.y(), s2.x(), s2.y(), black);
    pp->drawImage(sr.x()+offs.x(), sr.y()+offs.y(), img);
//    pp->drawLine( s1, s2 );

    // cap
    pp->setBrush(color);
    pp->drawEllipse( center.x()-w_hour/2+offs.x(), center.y()-w_hour/2+offs.y(), w_hour, w_hour );

    changed = expand(sr);
    isEvent = TRUE;
}

// Dijkstra's bisection algorithm to find the square root as an integer.

static uint int_sqrt(uint n)
{
    if ( n >= UINT_MAX>>2 ) // n must be in the range 0...UINT_MAX/2-1
	return 2*int_sqrt( n/4 );
    uint h, p= 0, q= 1, r= n;
    while ( q <= n )
	q <<= 2;
    while ( q != 1 ) {
	q >>= 2;
	h= p + q;
	p >>= 1;
	if ( r >= h ) {
	    p += q;
	    r -= h;
	}
    }
    return p;
}

void AnalogClock::drawHand( QPainter *p, QPoint p1, QPoint p2 )
{
    int hw = 7;
    if ( contentsRect().height() < 100 )
	hw = 5;

    int dx = p2.x() - p1.x();
    int dy = p2.y() - p1.y();
    int w = dx*dx+dy*dy;
    int ix,iy;
    w = int_sqrt(w*256);
    iy = w ? (hw * dy * 16)/ w : dy ? 0 : hw;
    ix = w ? (hw * dx * 16)/ w : dx ? 0 : hw;

    // rounding dependent on sign
    int nix, niy;
    if ( ix < 0 ) {
	nix = ix/2;
	ix = (ix-1)/2;
    } else {
	nix = (ix+1)/2;
	ix = ix/2;
    }
    if ( iy < 0 ) {
	niy = iy/2;
	iy = (iy-1)/2;
    } else {
	niy = (iy+1)/2;
	iy = iy/2;
    }

    QPointArray pa(4);
    pa[0] = p1;
    pa[1] = QPoint( p2.x()+iy, p2.y()-nix );
    pa[2] = QPoint( p2.x()-niy, p2.y()+ix );
    pa[3] = p1;

    p->drawPolygon( pa );

    QRect br = pa.boundingRect();
    QImage img(br.width(), br.height(), 32);
    img.fill(0x00000000);
    img.setAlphaBuffer(TRUE);
    pa.translate(-br.x(), -br.y());
    aaLine(img, pa[1].x(), pa[1].y(), pa[0].x(), pa[0].y(), black);
    aaLine(img, pa[2].x(), pa[2].y(), pa[0].x(), pa[0].y(), black);
    p->drawImage(br.x(), br.y(),img);
}

void AnalogClock::display( const QTime& t )
{
    currTime = t;
    if( isVisible() ) isEvent = FALSE;
    repaint( false );
}

QPoint AnalogClock::rotate( QPoint c, QPoint p, int a )
{
    double angle = deg2rad * ( - a + 180 );
    double nx = c.x() - ( p.x() - c.x() ) * cos( angle ) -
		( p.y() - c.y() ) * sin( angle );
    double ny = c.y() - ( p.y() - c.y() ) * cos( angle ) +
		( p.x() - c.x() ) * sin( angle );
    return QPoint( int(nx), int(ny) );
}

const int aaWeight = 300;

// poor man's antialiased line
void AnalogClock::aaLine(QImage &img, int x1, int y1, int x2, int y2, const QColor &col)
{
    if(x1>x2) {
	int x3;
	int y3;
	x3=x2;
	y3=y2;
	x2=x1;
	y2=y1;
	x1=x3;
	y1=y3;
    }

    int dx=x2-x1;
    int dy=y2-y1;

    int ax=QABS(dx)*2;
    int ay=QABS(dy)*2;
    int sy=dy>0 ? 1 : -1;
    int x=x1;
    int y=y1;

    int d;
    QRgb rgb = col.rgb() & 0x00FFFFFF;

    if (ax > ay) {
	d=ay-(ax >> 1);
	for(;;) {
	    img.setPixel(x, y, rgb | 0xFF000000);
	    int dd = d + ax/2 - ay;
	    int v = aaWeight*dd*dd/(ax*ax);
//	    qDebug("drawPix A: %d, %d - ax: %d, ay: %d %d, %d", x, y, ax, ay, dd, v);
	    if (y > 0 && y < img.height()-1) {
		if (dd < 0) {
		    int av = (img.pixel(x, y-sy) & 0xFF000000) >> 24;
		    av += v;
		    av = QMIN(av, 255);
		    img.setPixel(x, y-sy, rgb | (av << 24) );
		} else {
		    int av = (img.pixel(x, y+sy) & 0xFF000000) >> 24;
		    av += v;
		    av = QMIN(av, 255);
		    img.setPixel(x, y+sy, rgb | (av << 24) );
		}
	    }
	    if (x==x2)
		return;
	    if(d>=0) {
		y+=sy;
		d-=ax;
	    }
	    x++;
	    d+=ay;
	}
    } else {
	d=ax-(ay >> 1);
	for(;;) {
	    img.setPixel(x, y, col.rgb() | 0xFF000000);
	    int dd = d + ay/2 - ax;
	    int v = aaWeight*dd*dd/(ay*ay);
//	    qDebug("drawPix B: %d, %d - ax: %d, ay: %d %d, %d", x, y, ax, ay, dd, v);
	    if (x > 0 && x < img.width()-1) {
		if (dd < 0) {
		    int av = (img.pixel(x-1, y) & 0xFF000000) >> 24;
		    av += v;
		    av = QMIN(av, 255);
		    img.setPixel(x-1, y, rgb | (av << 24));
		} else {
		    int av = (img.pixel(x+1, y) & 0xFF000000) >> 24;
		    av += v;
		    av = QMIN(av, 255);
		    img.setPixel(x+1, y, rgb | (av << 24));
		}
	    }
	    if(y==y2)
		return;
	    if(d>=0) {
		x++;
		d-=ay;
	    }
	    y+=sy;
	    d+=ax;
	}
    }
}

