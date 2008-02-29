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

#include "calibrate.h"

#include <qtopia/resource.h>

#include <qapplication.h>

#if defined(Q_WS_QWS) || defined(_WS_QWS_)

#include <qpainter.h>
#include <qfile.h>
#include <qtimer.h>
#include <qmessagebox.h>
#include <qwindowsystem_qws.h>
#include <qgfx_qws.h>
#include <stdlib.h>


Calibrate::Calibrate(QWidget* parent, const char * name, WFlags wf) :
    QDialog( parent, name, TRUE, wf | WStyle_Customize | WStyle_StaysOnTop )
{
    init();

    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(timeout()) );

    setFocusPolicy( StrongFocus );
    setFocus();
}

Calibrate::~Calibrate()
{
    store();
}

void
Calibrate::init(void)
{
    showCross = TRUE;
    pressed = FALSE;
    anygood = FALSE;
    const int offset = 30;
    QRect desk = qApp->desktop()->geometry();
    setGeometry( 0, 0, desk.width(), desk.height() );
    if ( desk.height() < 250 ) {
	int w = desk.height()/3;
	logo.convertFromImage(Resource::loadImage("qtlogo").smoothScale(w,w));
    } else {
	logo = Resource::loadPixmap( "qtlogo" );
    }
    cd.screenPoints[QWSPointerCalibrationData::TopLeft] = QPoint( offset, offset );
    cd.screenPoints[QWSPointerCalibrationData::BottomLeft] = QPoint( offset, qt_screen->deviceHeight() - offset );
    cd.screenPoints[QWSPointerCalibrationData::BottomRight] = QPoint( qt_screen->deviceWidth() - offset, qt_screen->deviceHeight() - offset );
    cd.screenPoints[QWSPointerCalibrationData::TopRight] = QPoint( qt_screen->deviceWidth() - offset, offset );
    cd.screenPoints[QWSPointerCalibrationData::Center] = QPoint( qt_screen->deviceWidth()/2, qt_screen->deviceHeight()/2 );
    goodcd = cd;
    reset();
}

void Calibrate::show()
{
    if ( !isVisible() && QWSServer::mouseHandler() ) {
	init();
	anygood = QFile::exists("/etc/pointercal");
	QWSServer::mouseHandler()->getCalibration(&goodcd);
	QWSServer::mouseHandler()->clearCalibration();
    }
    QDialog::show();
    setActiveWindow();
    QTimer::singleShot(0, this, SLOT(doGrab()) );
}

void Calibrate::store()
{
    if ( QWSServer::mouseHandler() && anygood )
	QWSServer::mouseHandler()->calibrate( &goodcd );
    reset();
}

void Calibrate::hide()
{
    if ( isVisible() )
	store();
    QDialog::hide();
}

void Calibrate::reset()
{
    penPos = QPoint();
    location = QWSPointerCalibrationData::TopLeft;
    crossPos = fromDevice( cd.screenPoints[location] );
}

QPoint Calibrate::fromDevice( const QPoint &p )
{
    return qt_screen->mapFromDevice( p,
		QSize(qt_screen->deviceWidth(), qt_screen->deviceHeight()) );
}

bool Calibrate::sanityCheck()
{
    QPoint tl = cd.devPoints[QWSPointerCalibrationData::TopLeft];
    QPoint tr = cd.devPoints[QWSPointerCalibrationData::TopRight];
    QPoint bl = cd.devPoints[QWSPointerCalibrationData::BottomLeft];
    QPoint br = cd.devPoints[QWSPointerCalibrationData::BottomRight];

    int vl = QABS( tl.y() - bl.y() );
    int vr = QABS( tr.y() - br.y() );
    int diff = QABS( vl - vr );
    int avg = ( vl + vr ) / 2;
    if ( diff > avg / 20 ) // 5% leeway
	return FALSE;

    int ht = QABS( tl.x() - tr.x() );
    int hb = QABS( br.x() - bl.x() );
    diff = QABS( ht - hb );
    avg = ( ht + hb ) / 2;
    if ( diff > avg / 20 ) // 5% leeway
	return FALSE;

    return TRUE;
}

bool Calibrate::checkTouch()
{
    QString proto = getenv( "QWS_MOUSE_PROTO" );
    bool touch = proto.contains("tpanel", FALSE);
    if ( !touch ) {
	QWSMouseHandler *h = QWSServer::mouseHandler();
	if ( h && h->inherits("QCalibratedMouseHandler") )
	    touch = TRUE;
    }
    if ( !touch ) {
	bool grab = QWidget::mouseGrabber() == this;
	releaseMouse();
	QMessageBox::warning( this, tr("Calibrate"), 
	    tr("<qt>Calibration may only be performed on the touch screen.") );
	if ( grab )
	    grabMouse();
    }

    return touch;
}

void Calibrate::moveCrosshair( QPoint pt )
{
/*
    QPainter p( this );
    p.drawPixmap( crossPos.x()-8, crossPos.y()-8, saveUnder );
    saveUnder = QPixmap::grabWindow( winId(), pt.x()-8, pt.y()-8, 16, 16 );
    p.drawRect( pt.x()-1, pt.y()-8, 2, 7 );
    p.drawRect( pt.x()-1, pt.y()+1, 2, 7 );
    p.drawRect( pt.x()-8, pt.y()-1, 7, 2 );
    p.drawRect( pt.x()+1, pt.y()-1, 7, 2 );
*/
    showCross = FALSE;
    repaint( crossPos.x()-8, crossPos.y()-8, 16, 16 );
    showCross = TRUE;
    crossPos = pt;
    repaint( crossPos.x()-8, crossPos.y()-8, 16, 16 );
}

void Calibrate::paintEvent( QPaintEvent * )
{
    QPainter p( this );

    int y;

    if ( !logo.isNull() ) {
	y = height() / 2 - logo.height() - 15;
	p.drawPixmap( (width() - logo.width())/2, y, logo );
    }

    y = height() / 2 + 15;

    p.drawText( 0, y+height()/8, width(), height() - y, AlignHCenter,
	tr("Touch the crosshairs firmly and\n"
            "accurately to calibrate your screen.") );

    QFont f = p.font(); f.setBold(TRUE);
    p.setFont( f );
    p.drawText( 0, y, width(), height() - y, AlignHCenter|WordBreak, 
	    tr("Welcome to Qtopia") );

/*
    saveUnder = QPixmap::grabWindow( winId(), crossPos.x()-8, crossPos.y()-8,
				     16, 16 );
    moveCrosshair( crossPos );
*/
    if ( showCross ) {
	p.drawRect( crossPos.x()-1, crossPos.y()-8, 2, 7 );
	p.drawRect( crossPos.x()-1, crossPos.y()+1, 2, 7 );
	p.drawRect( crossPos.x()-8, crossPos.y()-1, 7, 2 );
	p.drawRect( crossPos.x()+1, crossPos.y()-1, 7, 2 );
    }
}

void Calibrate::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_Escape )
	QDialog::keyPressEvent( e );
}

void Calibrate::keyReleaseEvent( QKeyEvent *e )
{
    if ( e->key() == Key_Escape )
	QDialog::keyReleaseEvent( e );
}

void Calibrate::mousePressEvent( QMouseEvent *e )
{
    pressed = TRUE;
    // map to device coordinates
    QPoint devPos = qt_screen->mapToDevice( e->pos(),
			QSize(qt_screen->width(), qt_screen->height()) );
    if ( penPos.isNull() )
	penPos = devPos;
    else
	penPos = QPoint( (penPos.x() + devPos.x())/2,
			 (penPos.y() + devPos.y())/2 );
}

void Calibrate::mouseMoveEvent( QMouseEvent *e )
{
    if ( !pressed )
	return;
    // map to device coordinates
    QPoint devPos = qt_screen->mapToDevice( e->pos(),
			QSize(qt_screen->width(), qt_screen->height()) );
    if ( penPos.isNull() )
	penPos = devPos;
    else
	penPos = QPoint( (penPos.x() + devPos.x())/2,
			 (penPos.y() + devPos.y())/2 );
}

void Calibrate::mouseReleaseEvent( QMouseEvent * )
{
    if ( !pressed )
	return;
    pressed = FALSE;
    if ( timer->isActive() )
	return;

    if ( !checkTouch() ) {
	hide();
	reject();
	return;
    }

    bool doMove = TRUE;

    cd.devPoints[location] = penPos;
    if ( location < QWSPointerCalibrationData::LastLocation ) {
	location = (QWSPointerCalibrationData::Location)((int)location + 1);
    } else {
	if ( sanityCheck() ) {
	    reset();
	    anygood = TRUE;
	    goodcd = cd;
	    hide();
	    emit accept();
	    doMove = FALSE;
	} else {
	    location = QWSPointerCalibrationData::TopLeft;
	}
    }
	    
    if ( doMove ) {
	QPoint target = fromDevice( cd.screenPoints[location] );
	dx = (target.x() - crossPos.x())/10;
	dy = (target.y() - crossPos.y())/10;
	timer->start( 30 );
    }
}

void Calibrate::timeout()
{
    QPoint target = fromDevice( cd.screenPoints[location] );

    bool doneX = FALSE;
    bool doneY = FALSE;
    QPoint newPos( crossPos.x() + dx, crossPos.y() + dy );

    if ( QABS(crossPos.x() - target.x()) <= QABS(dx) ) {
	newPos.setX( target.x() );
	doneX = TRUE;
    }

    if ( QABS(crossPos.y() - target.y()) <= QABS(dy) ) {
	newPos.setY(target.y());
	doneY = TRUE;
    }

    if ( doneX && doneY ) {
	penPos = QPoint();
	timer->stop();
    }

    moveCrosshair( newPos );
}

void Calibrate::doGrab()
{
    if ( !QWidget::mouseGrabber() ) {
	if ( !checkTouch() )
	    reject();
	else
	    grabMouse();
    } else {
	QTimer::singleShot( 50, this, SLOT(doGrab()) );
    }
}

#endif // _WS_QWS_
