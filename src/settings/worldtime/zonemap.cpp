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

#include "sun.h"
#include "zonemap.h"

#include <qtopia/resource.h>
#include <qtopia/timestring.h>
#include <qtopia/timezone.h>
#include <qtopia/qpeapplication.h>

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
static const char strMAP[] = "simple_grid_400";

// the maximum distance we'll allow the pointer to be away from a city
// and still show the city's time
static const int iTHRESHOLD = 50000;

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
      drawableW( -1 ),
      drawableH( -1 ),
      bZoom( FALSE ),
      bIllum( TRUE ),
      citiesInit( FALSE )
{
    viewport()->setFocusPolicy( StrongFocus );

    // get the map loaded
    // just set the current image to point
    pixCurr = new QPixmap();

    QPixmap pixZoom = Resource::loadPixmap( "mag" );

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
    QObject::connect( cmdZoom, SIGNAL( toggled( bool ) ),
                      this, SLOT( slotZoom( bool ) ) );
    QObject::connect( &norm, SIGNAL( signalNewPoint( const QPoint& ) ),
                      this, SLOT( slotFindCity( const QPoint& ) ) );
    // update the sun's movement every 5 minutes
    tUpdate->start( 5 * 60 * 1000 );
    // May as well read in the timezone information too...

    cities.setAutoDelete(TRUE);

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

void ZoneMap::keyPressEvent( QKeyEvent *ke )
{
    switch ( ke->key() ) {
	case Key_Left:
	case Key_Right:
	case Key_Up:
	case Key_Down: {
		tHide->stop();
		if ( !m_cursor.isValid() )
		    slotFindCity( QPoint( contentsWidth(), contentsHeight() ) / 2 );
		TimeZone city = findCityNear( m_cursor, ke->key() );
		if ( city.isValid() ) {
		    m_cursor = city;
		    int tmpx, tmpy;
		    zoneToWin( m_cursor.lon(), m_cursor.lat(), tmpx, tmpy );
		    ensureVisible( tmpx, tmpy );
		    showCity( m_cursor );
		    tHide->start( 3000, true );
		}
	    }
	    break;

	case Key_Space:
	case Key_Enter:
	case Key_Return:
	    if ( m_cursor.isValid() ) {
		emit signalTz( m_cursor.id());
		tHide->start( 0, true );
	    }
	    break;
    }
}

const TimeZone ZoneMap::findCityNear( const TimeZone &city, int key )
{
    initCities();

    long ddist = LONG_MAX;
    QCString closestZone;
    for ( unsigned i = 0; i < cities.count(); i++ ) {
	CityPos *cp = cities[i];
	long dy = (cp->lat - city.lat())/100;
	long dx = (cp->lon - city.lon())/100;
	switch ( key ) {
	    case Key_Right:
	    case Key_Left:
		if ( key == Key_Left )
		    dx = -dx;
		if ( dx > 0 ) {
		    long dist = QABS(dy)*4 + dx;
		    if ( dist < ddist ) {
			ddist = dist;
			closestZone = cp->id;
		    }
		}
		break;
	    case Key_Down:
	    case Key_Up:
		if ( key == Key_Down )
		    dy = -dy;
		if ( dy > 0 ) {
		    long dist = QABS(dx)*4 + dy;
		    if ( dist < ddist ) {
			ddist = dist;
			closestZone = cp->id;
		    }
		}
		break;
	}
    }
    
    TimeZone nearCity( closestZone );
//    nearCity.dump();
    return nearCity;
}

void ZoneMap::slotFindCity( const QPoint &pos )
{
    initCities();

    // given coordinates on the screen find the closest city and display the
    // label close to it
    int tmpx, tmpy, x, y;

    if ( tHide->isActive() )
        tHide->stop();

    viewportToContents(pos.x(), pos.y(), tmpx, tmpy);
    winToZone( tmpx, tmpy, x, y );

    // Find city alogorithim: start out at an (near) infinite distance away and
    // then find the closest city, (similar to the Z-buffer technique, I guess)
    // the only problem is that this is all done with doubles, but I don't know
    // another way to do it at the moment.  Another problem is a linked list is
    // used obviously something indexed would help
    long lDistance;
    long lClosest = LONG_MAX;
    QCString closestZone;
    for ( unsigned i = 0; i < cities.count(); i++ ) {
	CityPos *cp = cities[i];
	// use the manhattenLength, a good enough of an appoximation here
	lDistance = QABS(y - cp->lat) + QABS(x - cp->lon);
	// first to zero wins!
	if ( lDistance < lClosest ) {
	    lClosest = lDistance;	    
	    closestZone = cp->id;
	}
    }

    // Okay, we found the closest city, but it might still be too far away.
    if ( lClosest <= iTHRESHOLD ) {
	m_cursor = TimeZone(closestZone);
	showCity( m_cursor );
    }
}

void ZoneMap::showCity( const TimeZone &city )
{
    if ( m_last == city )
	return;
    m_last = city;
    QDateTime cityTime = city.fromUtc(TimeZone::utcDateTime());

    lblCity->setText( city.city().replace( QRegExp("_"), " ") + "\n" +
		      TimeString::localHM( cityTime.time()));
    lblCity->setMinimumSize( lblCity->sizeHint() );
    lblCity->resize( QMAX(lblCity->sizeHint().width(),80), QMAX(lblCity->sizeHint().height(),40) );

    // Now decide where to move the label, x & y can be reused
    int tmpx, tmpy, x, y;
    zoneToWin( m_last.lon(), m_last.lat(), tmpx, tmpy );
    contentsToViewport(tmpx, tmpy, x, y);

    //
    // Put default position for the city label in the "above left"
    // area.  This avoids obscuring the popup for quite a bit of
    // the map.  Use the "below right" position for the border cases.
    //
    x -= iLABELOFFSET + lblCity->width();
    if (x < 0) {
	x += 2*iLABELOFFSET + lblCity->width();
    }
    y -= iLABELOFFSET + lblCity->height();
    if (y < 0) {
	y += 2*iLABELOFFSET + lblCity->height();
    }

    // draw in the city and the label
    if ( m_repaint.isValid()) {
	int repx,
	    repy;
	zoneToWin( m_repaint.lon(), m_repaint.lat(), repx, repy );
	updateContents( repx - iCITYOFFSET, repy - iCITYOFFSET,
			iCITYSIZE, iCITYSIZE );
    }
    updateContents( tmpx - iCITYOFFSET, tmpy - iCITYOFFSET, iCITYSIZE,
		    iCITYSIZE );
    m_repaint = m_last;

    lblCity->move( x, y );
    lblCity->show();
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
    // create a mask the functions from sun.h
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

    // get the position of the sun bassed on our current time...
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

    // Assume that the image is 32bpp as we should have converted previously to that...
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
}

void ZoneMap::drawCity( QPainter *p, const TimeZone &city )
{
    int x,
        y;

    p->setPen( red );
    zoneToWin( city.lon(), city.lat(), x, y );
    p->drawRect( x - iCITYOFFSET, y - iCITYOFFSET, iCITYSIZE, iCITYSIZE );
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
    bZoom = setZoom;
    if ( bZoom ) {
	makeMap( 2 * wImg , 2 * hImg );
	resizeContents( wImg, hImg );
    } else {
	makeMap( drawableW, drawableH );
	resizeContents( drawableW, drawableH );
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
    // but it is good to be complete
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
}
