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

#include "sun.h"
#include "zonemap.h"

#include <qtopia/resource.h>
#include <qtopia/timestring.h>
#include <qtopia/timezone.h>
#include <qtopia/qpeapplication.h>
#ifdef QTOPIA_PHONE
# include <qtopia/contextbar.h>
#include <qtopia/global.h>
#endif

#include <qdatetime.h>
#include <qfile.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlist.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qtoolbutton.h>
#include <qwhatsthis.h>

#include <limits.h>

// the map file...
#ifndef Q_OS_WIN32
static const char strZONEINFO[] = "/usr/share/zoneinfo/zone.tab";
#else
static const char strZONEINFO[] = "./etc/zoneinfo/zone.tab";
#endif
static const char strMAP[] = "worldtime/simple_grid_400";

// The label offset (how far away from pointer)
static const int iLABELOFFSET = 8;

// the size of the dot to draw, and where to start it
static const int iCITYSIZE = 3;
const int iCITYOFFSET = 2;

// the darkening function
static inline void darken( QImage *pImage, int start, int stop, int row );
static void dayNight( QImage *pImage );

ZoneMap::ZoneMap( QWidget *parent, const char* name )
    : QScrollView( parent, name ),
      ox( 0 ),
      oy( 0 ),
      minMovement( 1 ),
      maxMovement( 4 ),
      drawableW( -1 ),
      drawableH( -1 ),
      bZoom( FALSE ),
      bIllum( TRUE ),
      m_cursor_x( -1 ),
      m_cursor_y( -1 ),
      citiesInit( FALSE ),
      accelHori(0),
      accelVert(0)
{
#ifdef QTOPIA_PHONE
    setMargin(2);
#endif
    setFocusPolicy( StrongFocus );

    cursorTimer = new QTimer(this);
    connect(cursorTimer, SIGNAL(timeout()), this, SLOT(cursorTimeout()));

    // get the map loaded
    // just set the current image to point
    pixCurr = new QPixmap();

    QPixmap pixZoom = Resource::loadPixmap( "worldtime/mag" );

    cmdZoom = new QToolButton( this );
    cmdZoom->setPixmap( pixZoom );
    cmdZoom->setToggleButton( true );

    cmdZoom->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0,
                            (QSizePolicy::SizeType)0,
                            cmdZoom->sizePolicy().hasHeightForWidth() ) );
    cmdZoom->setMaximumSize( cmdZoom->sizeHint() );
    // probably don't need this, but just in case...
    cmdZoom->move( width() - cmdZoom->width(), height() - cmdZoom->height() );
    QWhatsThis::add( cmdZoom, tr("Click to zoom the map in/out.") );


    lblCity = new QLabel( tr( "CITY" ), this );
    lblCity->setMinimumSize( lblCity->sizeHint() );
    lblCity->setFrameStyle( QFrame::Plain | QFrame::Box );
    lblCity->setBackgroundColor( yellow );
    lblCity->setFont( QApplication::font() );
    lblCity->hide();

    // A timer to make sure the label gets hidden
    tHide = new QTimer( this );
    QObject::connect( tHide, SIGNAL( timeout() ),
                      lblCity, SLOT( hide() ) );
    QObject::connect( tHide, SIGNAL( timeout() ),
                      this, SLOT( slotRedraw() ) );
    QTimer *tUpdate = new QTimer( this );
    QObject::connect( tUpdate, SIGNAL( timeout() ),
                      this, SLOT( slotUpdate() ) );
    QObject::connect( qApp, SIGNAL( timeChanged() ),
                      this, SLOT( slotUpdate() ) );
    QObject::connect( cmdZoom, SIGNAL( toggled(bool) ),
                      this, SLOT( slotZoom(bool) ) );
    QObject::connect( &norm, SIGNAL( signalNewPoint(const QPoint&) ),
                      this, SLOT( slotFindCity(const QPoint&) ) );
    // update the sun's movement every 5 minutes
    tUpdate->start( 5 * 60 * 1000 );
    // May as well read in the timezone information too...

    cities.setAutoDelete(TRUE);

#ifdef QTOPIA_PHONE
    ContextBar::setLabel(this, Key_Select, ContextBar::Select);
#endif

    QTimer::singleShot( 0, this, SLOT(initCities()) );
}

ZoneMap::~ZoneMap()
{
    delete pixCurr;
}

void ZoneMap::viewportMousePressEvent( QMouseEvent* event )
{
    // add the mouse event into the normalizer, and get the average,
    // pass it along
    slotRedraw();
    norm.start();
    norm.addEvent( event->pos() );
}

void ZoneMap::viewportMouseMoveEvent( QMouseEvent* event )
{
    if ((event->x() >= 0 && event->x() < viewport()->width()) &&
	    (event->y() >= 0 && event->y() < viewport()->height())) {
	norm.addEvent( event->pos() );
    }
}

void ZoneMap::viewportMouseReleaseEvent( QMouseEvent* )
{
    // get the averaged points in case a timeout hasn't occurred,
    // more for "mouse clicks"
    norm.stop();
    if (m_last.isValid()) {
	emit signalTz( m_last.id() );
	m_last = TimeZone();
    }
    tHide->start( 2000, true );
}

void ZoneMap::beginEditing()
{
#ifdef QTOPIA_PHONE
    if ( Global::mousePreferred() || !isModalEditing() ) {
	if( !Global::mousePreferred() )
	    setModalEditing( TRUE );
#endif
	tHide->stop();
	if (m_cursor.isValid())
	    showCity(m_cursor);
	else 
	    slotFindCity( QPoint( contentsWidth(), contentsHeight() ) / 2 );
#ifdef QTOPIA_PHONE
    }
#endif
}

int ZoneMap::heightForWidth(int w) const
{
    float scale = .5;
    return int(float(w) * scale);
}

void ZoneMap::keyPressEvent( QKeyEvent *ke )
{
#ifdef QTOPIA_PHONE
    if ( /* style().guiStyle() == KeyPadStyle */ TRUE )  {
	switch( ke->key() ) {
	    case Key_Select:
		if( !Global::mousePreferred() ) {
		    if ( !isModalEditing() ) {
			beginEditing();
			return;
		    }
		}
		break;
	    case Key_No:
	    case Key_Back:
		if ( !Global::mousePreferred() && isModalEditing() ) {
		    setModalEditing( FALSE );
		    tHide->start( 0, true );
		} else {
		    ke->ignore();
		}
		return;
		break;
	    default:
		if( !Global::mousePreferred() ) {
		    if ( !isModalEditing() ) {
			ke->ignore();
			return;
		    }
		}
	}
    }
#endif
    switch ( ke->key() ) {
#ifdef TEST_ACCESS_TO_CITIES
	case Key_T:
	    testAccess();
	    break;
#endif
	case Key_Left:
	    if (!ke->isAutoRepeat() && accelHori == 0) {
		accelHori = -1;
		cursorTimer->changeInterval(500);
		updateCursor();
	    }
	    break;
	case Key_Right:
	    if (!ke->isAutoRepeat() && accelHori == 0) {
		accelHori = 1;
		cursorTimer->changeInterval(500);
		updateCursor();
	    }
	    break;
	case Key_Up:
	    if (!ke->isAutoRepeat() && accelVert == 0) {
		accelVert = -1;
		cursorTimer->changeInterval(500);
		updateCursor();
	    }
	    break;
	case Key_Down:
	    if (!ke->isAutoRepeat() && accelVert == 0) {
		accelVert = 1;
		cursorTimer->changeInterval(500);
		updateCursor();
	    }
	    break;
	case Key_Space:
	case Key_Enter:
	case Key_Return:
#ifdef QTOPIA_PHONE
	case Key_Select:
#endif

#ifdef QTOPIA_PHONE
	    if( !Global::mousePreferred() )
#endif
	    if ( m_cursor.isValid() ) {
		emit signalTz( m_cursor.id());
		tHide->start( 0, true );
#ifdef QTOPIA_PHONE
		setModalEditing( FALSE );
#endif
	    }
	    break;
	default:
	    QScrollView::keyPressEvent(ke);
    }
}

void ZoneMap::keyReleaseEvent( QKeyEvent *ke )
{
    switch(ke->key()) {
	case Key_Up:
	case Key_Down:
	    if (!ke->isAutoRepeat() && accelVert != 0) {
		accelVert = 0;
		if (accelHori == 0)
		    cursorTimer->stop();
	    }
	    break;
	case Key_Left:
	case Key_Right:
	    if (!ke->isAutoRepeat() && accelHori != 0) {
		accelHori = 0;
		if (accelVert == 0)
		    cursorTimer->stop();
	    }
	    break;
	default:
	    QScrollView::keyReleaseEvent(ke);
    }
}

/* accelerating cursor movement */
void ZoneMap::cursorTimeout() {
    if (accelHori < 0 && accelHori > -5)
	accelHori = -5;
    else if (accelHori > 0 && accelHori < 5)
	accelHori = 5;
    if (accelVert < 0 && accelVert > -5)
	accelVert = -5;
    else if (accelVert > 0 && accelVert < 5)
	accelVert = 5;
    updateCursor();
    cursorTimer->changeInterval(100);
}

// how many multiples of 100 ms to hold down before movement rate changes
static const char accel_rate = 5;
void ZoneMap::updateCursor()
{
    // accellerate timer after first one..
    uint mx, my;
    uint habs = QABS(accelHori);
    uint vabs = QABS(accelVert);
    mx = minMovement << (habs / accel_rate); // min movement doubles.
    my = minMovement << (vabs / accel_rate);
    if (mx < maxMovement) {
	if (accelHori < 0)
	    accelHori--;
	else if (accelHori > 0)
	    accelHori++;
    } else {
	mx = maxMovement;
    }
    if (my < maxMovement) {
	if (accelVert < 0)
	    accelVert--;
	else if (accelVert > 0)
	    accelVert++;
    } else {
	my = maxMovement;
    }

    if (m_cursor_x != -1 && m_cursor_y != -1) {
	int nx = m_cursor_x;
	int ny = m_cursor_y;

	// horizontal movement
	if (accelHori < 0) {
	    nx -= mx;
	    if (nx < 0)
		nx = 0;
	} else if (accelHori > 0) {
	    nx += mx;
	    if (nx > contentsWidth())
		nx = contentsWidth();
	}
	// vertical movement
	if (accelVert < 0) {
	    ny -= my;
	    if (ny < 0) 
		ny = 0; 
	} else if (accelVert > 0) {
	    ny += my;
	    if (ny > contentsHeight())
		ny = contentsHeight();
	}

	setCursorPoint(nx, ny);
	ensureVisible(nx, ny);
    }
}

QRect expandTo(const QRect &in, const QPoint &p) {
    if (in.isValid()) {
	QRect r(in);
	if (p.x() < r.left())
	    r.setLeft(p.x());
	else if (p.x() > r.right())
	    r.setRight(p.x());
	if (p.y() < r.top())
	    r.setTop(p.y());
	else if (p.y() > r.bottom())
	    r.setBottom(p.y());
	return r;
    } else {
	return QRect(p.x(), p.y(), 1, 1);
    }
}

void ZoneMap::setCursorPoint(int ox, int oy)
{
    if (ox != m_cursor_x || oy != m_cursor_y) {
	// Old Location Window Coords
	int olwx, olwy;
	zoneToWin( m_cursor.lon(), m_cursor.lat(), olwx, olwy);
	// New Location Window Coords
	int nlwx, nlwy;

	QRect bounds(m_cursor_x, m_cursor_y, 1,1);
	bounds = expandTo(bounds, QPoint(ox, oy));
	bounds = expandTo(bounds, QPoint(olwx, olwy));

	m_cursor_x = ox;
	m_cursor_y = oy;

	// zone coords x and y.
	int zx, zy;
	winToZone( m_cursor_x, m_cursor_y, zx, zy);

	long lDistance;
	long lClosest = LONG_MAX;
	TimeZone closestZone;
	QPoint lPoint;
	for ( unsigned i = 0; i < cities.count(); i++ ) {
	    CityPos *cp = cities[i];
	    // use the manhattenLength, a good enough of an appoximation here
	    lDistance = QABS(zy - cp->lat) + QABS(zx - cp->lon);
	    // first to zero wins!
	    if ( lDistance < lClosest ) {
		lClosest = lDistance;	    
		lPoint = QPoint(cp->lat, cp->lon);
		closestZone = TimeZone(cp->id);
	    }
	}


	if (m_cursor != closestZone) {
	    m_cursor = closestZone;
	    zoneToWin( m_cursor.lon(), m_cursor.lat(), nlwx, nlwy);
	    bounds = expandTo(bounds, QPoint(nlwx, nlwy));
	} else {
	    nlwx = olwx;
	    nlwy = olwy;
	}

	// m_curosr
	m_last = m_cursor;
	QDateTime cityTime = m_cursor.fromUtc(TimeZone::utcDateTime());

	lblCity->setText( m_cursor.city().replace( QRegExp("_"), " " ) + "\n" +
		TimeString::localHM( cityTime.time()));
	lblCity->setMinimumSize( lblCity->sizeHint() );
	lblCity->resize( QMAX(lblCity->sizeHint().width(),80), QMAX(lblCity->sizeHint().height(),40) );

	// Now decide where to move the label, x & y can be reused
	int x, y;
	contentsToViewport(nlwx, nlwy, x, y);

	//
	// Put default position for the city label in the "above left"
	// area.  This avoids obscuring the popup for quite a bit of
	// the map.  Use the "below right" position for the border cases.
	//
	x -= iLABELOFFSET + lblCity->width();
	if (x < 0) {
	    // right
	    x += 2*iLABELOFFSET + lblCity->width();
	    // still keep on screen, over red dot if need be.
	    if ((x+lblCity->width() > width()))
		x -= x+lblCity->width() - width();
	}
	y -= iLABELOFFSET + lblCity->height();
	if (y < 0) {
	    // below
	    y += 2*iLABELOFFSET + lblCity->height();
	    // still keep on screen, over red dot if need be.
	    if ((y+lblCity->height() > height()))
		y -= y+lblCity->height() - height();
	}

	// draw in the city and the label
	if ( m_repaint.isValid()) {
	    int repx, repy;
	    zoneToWin( m_repaint.lon(), m_repaint.lat(), repx, repy );
	    bounds = expandTo(bounds, QPoint(repx, repy));
	}
	m_repaint = m_last;

	lblCity->move( x, y );
	lblCity->show();

	// m_curosr
	repaintContents( bounds.x()-(1+iCITYOFFSET), bounds.y()-(1+iCITYOFFSET), bounds.width()+2+iCITYSIZE, bounds.height()+2+iCITYSIZE);
    }
}

void ZoneMap::slotFindCity( const QPoint &pos )
{
    initCities();

    int tmpx, tmpy;
    viewportToContents(pos.x(), pos.y(), tmpx, tmpy);

    setCursorPoint(tmpx, tmpy);
    if(bZoom)
    {
      ensureVisible(pos.x()*2,pos.y()*2);
    }
    else
      ensureVisible(pos.x(), pos.y());
}

void ZoneMap::showCity( const TimeZone &city )
{
    // use set cursor point to erase old point if need be.
    int mx, my;
    zoneToWin(city.lon(), city.lat(), mx, my);
    setCursorPoint(mx, my);
}

void ZoneMap::resizeEvent( QResizeEvent *e )
{
    //
    // Disable zooming when resizing.
    //
    if (bZoom) {
	bZoom = FALSE;
	cmdZoom->setOn(FALSE);
    }

    // keep the zoom button down in the corner
    QSize _size = e->size();
    cmdZoom->move( _size.width() - cmdZoom->width(),
                   _size.height() - cmdZoom->height() );
    if ( !bZoom ) {
	drawableW = width() - 2 * frameWidth();
	drawableH = height() - 2 * frameWidth();
	makeMap( drawableW, drawableH );
	resizeContents( drawableW, drawableH );
    } else {
	resizeContents( wImg, hImg );
    }
}

void ZoneMap::drawCities( QPainter *p )
{
    int x,y;
    // draw in the cities
    // for testing only as when you put it
    // on the small screen it looks awful and not to mention useless
    p->setPen( red );
    TimeZone curZone;
    const char *zoneID;
    QStrListIterator it( TimeZone::ids() );
    for (; it.current(); ++it) {
	zoneID = it.current();   
	curZone = TimeZone( zoneID );
	zoneToWin( curZone.lat(), curZone.lon(), x, y );
	if ( x > wImg )
	    x = x - wImg;
	p->drawRect( x - iCITYOFFSET, y - iCITYOFFSET, iCITYSIZE, iCITYSIZE);
    }
}

static void dayNight(QImage *pImage)
{
    // create a mask that functions from sun.h
    double dJulian,
           dSunRad,
           dSunDecl,
           dSunRadius,
           dSunLong;
    int wImage = pImage->width(),
        hImage = pImage->height(),
        iStart,
        iStop,
        iMid,
        relw,
        i;
    short * wtab = new short [ wImage ];
    time_t tCurrent;
    struct tm *pTm;

    // get the position of the sun based on our current time...
    tCurrent = time( NULL );
    pTm = gmtime( &tCurrent );
    dJulian = jtime( pTm );
    sunpos( dJulian, 0, &dSunRad, &dSunDecl, &dSunRadius, &dSunLong );

    // now get the projected illumination
    projillum( wtab, wImage, hImage, dSunDecl );
    relw = wImage - int( wImage * 0.0275 );

    // draw the map, keeping in mind that we may go too far off the map...
    iMid = ( relw * ( 24*60 - pTm->tm_hour * 60 - pTm->tm_min ) ) / ( 24*60 );

    for ( i = 0; i < hImage; i++ ) {
	if ( wtab[i] > 0 ) {
	    iStart = iMid - wtab[i];
	    iStop = iMid + wtab[i];
	    if ( iStart < 0 ) {
		darken( pImage, iStop, wImage + iStart, i );
	    } else if ( iStop > wImage ) {
		darken( pImage, iStop - wImage, iStart, i );
	    } else {
		darken( pImage, 0, iStart, i );
		darken( pImage, iStop, wImage, i );
	    }
	} else {
	    darken( pImage, 0, wImage, i );
	}
    }
    delete [] wtab;
}

static inline void darken( QImage *pImage, int start, int stop, int row )
{
    // Always clip stop parameter to ensure our preconditions
    if ( stop >= pImage->width() )
	stop =  pImage->width() - 1;

    // Assume that the image is 32bpp as we should have converted to that previously...
    QRgb *p = (QRgb *)pImage->scanLine( row );
    for ( int j = start; j <= stop; j++ ) {
	QRgb rgb = p[j];
	p[j] = qRgb( 2 * qRed( rgb ) / 3, 2 * qGreen( rgb ) / 3, 2 * qBlue( rgb ) / 3 );
    }
}

void ZoneMap::makeMap( int w, int h )
{
    QImage imgOrig = Resource::loadImage( strMAP );
    if ( imgOrig.isNull() ) {
	QMessageBox::warning( this,
	                      tr( "Couldn't Find Map" ),
	                      tr( "<p>Couldn't load map: %1, exiting")
                              .arg( strMAP ) );
	exit(-1);
    }

    // set up the color table for darkening...
    imgOrig = imgOrig.convertDepth( 32 );

    // else go one with making the map...
    if ( bIllum ) {
	// do a daylight mask
	dayNight(&imgOrig);
    }
    // redo the width and height
    wImg = w;
    hImg = h;
    ox = ( wImg / 2 ) - int( wImg * 0.0275 );
    oy = hImg / 2;
    pixCurr->convertFromImage( imgOrig.smoothScale(w, h),
                               QPixmap::ThresholdDither );

    // should also work out max accell, or accell rates for this zoom.
    maxMovement=wImg*90;
}

void ZoneMap::drawCity( QPainter *p, const TimeZone &city )
{
    int x, y;

    p->setPen( red );
    zoneToWin( city.lon(), city.lat(), x, y );
    if (m_cursor_x != -1 && m_cursor_y != -1) {
	p->drawLine( m_cursor_x, m_cursor_y, x, y);

    }
    p->drawRect( m_cursor_x - iCITYOFFSET, m_cursor_y - iCITYOFFSET, iCITYSIZE, iCITYSIZE );
}

void ZoneMap::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
    // if there is a need to resize, then do it...
    // get our drawable area
    drawableW = width() - 2 * frameWidth();
    drawableH = height() - 2 * frameWidth();

    int pixmapW = pixCurr->width(),
        pixmapH = pixCurr->height();
    if ( !bZoom &&  ( ( pixmapW != drawableW ) ||
                      ( pixmapH != drawableH) ) ) {
	makeMap( drawableW, drawableH );
    }

    // taken from the scrollview example...
    int rowheight = pixCurr->height();
    int toprow = cy / rowheight;
    int bottomrow = ( cy + ch + rowheight - 1 ) / rowheight;
    int colwidth = pixCurr->width();
    int leftcol= cx / colwidth;
    int rightcol= ( cx + cw + colwidth - 1 ) / colwidth;
    for ( int r = toprow; r <= bottomrow; r++ ) {
	int py = r * rowheight;
	for ( int c = leftcol; c <= rightcol; c++ ) {
	    int px = c * colwidth;
	    p->drawPixmap( px, py, *pixCurr );
	}
    }

    // Draw that city!
    if ( m_last.isValid() )
	drawCity( p, m_last );
}

void ZoneMap::slotZoom( bool setZoom )
{
    if (setZoom == bZoom)
	return;

    bZoom = setZoom;
    int cx, cy;
    winToZone(m_cursor_x, m_cursor_y, cx, cy);
    if ( bZoom ) {
	makeMap( 2 * wImg , 2 * hImg );
	resizeContents( wImg, hImg );
    } else {
	makeMap( drawableW, drawableH );
	resizeContents( drawableW, drawableH );
    }
    if (m_cursor_x != -1 && m_cursor_y != -1) {
        zoneToWin(cx, cy, cx, cy);
        setCursorPoint(cx, cy);

        ensureVisible(cx, cy);
        int lx, ly;
        zoneToWin( m_cursor.lon(), m_cursor.lat(), lx, ly );
        updateContents( QRect(QMIN(lx, cx)-(1+iCITYOFFSET), QMIN(ly, cy)-(1+iCITYOFFSET),
                    QABS(lx - cx)+2+iCITYSIZE, QABS(ly - cy)+2+iCITYSIZE));
    }
}

void ZoneMap::slotIllum( bool setIllum )
{
    bIllum = !setIllum;
    // make the map...
    makeMap( pixCurr->width(), pixCurr->height() );
    updateContents( 0, 0, wImg, hImg );
}

void ZoneMap::slotUpdate( void )
{
    // recalculate the light, most people will never see this,
    // but it is good to be thorough
    makeMap ( pixCurr->width(), pixCurr->height() );
    updateContents( contentsX(), contentsY(), drawableW, drawableH );
}

void ZoneMap::slotRedraw( void )
{
    // paint over that pesky city...
    int x, y;
    if ( m_repaint.isValid() ) {
	m_last = TimeZone();
	zoneToWin(m_repaint.lon(), m_repaint.lat(), x, y);
	updateContents( x - iCITYOFFSET, y - iCITYOFFSET, iCITYSIZE, iCITYSIZE);
        m_repaint = TimeZone();
    }
}

void ZoneMap::initCities()
{
    // Contructing TimeZone::TimeZone( city ) is hideously expensive -
    // preload the position of each city.
    if ( citiesInit )
	return;
    QStrList list = TimeZone::ids() ;
    QStrListIterator it( list );
    int count = 0;
    cities.resize( list.count() );
    for (; it.current(); ++it) {
	QCString zoneID = it.current();
	TimeZone curZone( zoneID );

	if ( !curZone.isValid() ){
	    qDebug("ZoneMap::slotFindCity  Invalid zoneID %s", zoneID.data() );
	    continue;
	}

	CityPos *cp = new CityPos;
	cp->lat = curZone.lat();
	cp->lon = curZone.lon();
	cp->id = zoneID.copy();

	cities.insert( count++, cp );
    }

    cities.resize(count);
    citiesInit = TRUE;

    /* should also set 
       min lat
       max lat
       min long
       max long
       and go through zone file for
       min distance between two cities, (halv it for default min movement)
    */
    ulong lDistance;
    ulong lClosest = ULONG_MAX;
    for ( uint i = 0; i < cities.count(); i++ ) {
	long latfrom = cities[i]->lat;
	long lonfrom = cities[i]->lon;
	for ( uint j = 0; j < cities.count(); j++ ) {
	    if (i != j) {
		long latto = cities[j]->lat;
		long lonto = cities[j]->lon;
		// use the manhattenLength, a good enough of an appoximation here
		lDistance = QABS(latfrom - latto) + QABS(lonfrom - lonto);
		// first to zero wins!
		if ( lDistance < lClosest ) {
		    lClosest = lDistance;	    
		}
	    }
	}
    }
}

#ifdef TEST_ACCESS_TO_CITIES
void ZoneMap::testAccess()
{
    qDebug("test access");
    initCities();
    for ( unsigned i = 0; i < cities.count(); i++ ) {
	CityPos *cp = cities[i];
	// check if pixel for this city and or pixels around this city can
	// be accessed.
	int lat, lon, x, y, cx, cy;
	lat = cp->lat;
	lon = cp->lon;
	zoneToWin(lon, lat, x, y);
	bool found = false;
	QCString id = cp->id;
	for (cx = x-1; cx <= x+1; ++cx) {
	    for (cy = y-1; cy <= y+1; ++cy) {
		setCursorPoint(cx,cy);
		if (m_cursor.id() == id) {
		    found = true;
		    break;
		}
	    }
	}
	if (!found) {
	    qDebug("couldn't find city %s", (const char *)id);
	    for (cx = x-1; cx <= x+1; ++cx) {
		for (cy = y-1; cy <= y+1; ++cy) {
		    setCursorPoint(cx,cy);
		    qDebug("\t%d:%d got %s instead", cx, cy, (const char *)m_cursor.id());
		}
	    }
	}
    }
}
#endif
