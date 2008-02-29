/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include <math.h>

#include "calibrate.h"

#if defined(Q_WS_QWS)

#include <QScreen>
#include <QPainter>
#include <QDesktopWidget>
#include <QApplication>
#include <QMenu>
#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif
#include <qtopialog.h>

#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>

Calibrate::Calibrate(QWidget* parent, Qt::WFlags f)
    : QDialog( parent, f )
{
    setObjectName("_fullscreen_");
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    setWindowState(Qt::WindowFullScreen);
    activateWindow();
    raise();
    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(timeout()) );

    QMenu *contextMenu = QSoftMenuBar::menuFor(this);
    connect (contextMenu,SIGNAL(triggered(QAction*)),this,SLOT(menuTriggered(QAction*)));
    
    qLog(Input)<<"Starting Calibrate!";
}

Calibrate::~Calibrate()
{
    store();
}

void Calibrate::showEvent(QShowEvent *e )
{
    qLog(Input) << "Calibrate::showEvent()";
    showCross = true;
    pressed = false;
    anygood = false;
    const int offset = 30;

    QDesktopWidget *desktop = QApplication::desktop();
    QRect desk = desktop->screenGeometry(desktop->primaryScreen());
    setGeometry(desk);
    logo = QPixmap(":image/qpelogo");

    cd.screenPoints[QWSPointerCalibrationData::TopLeft] = QPoint( offset, offset );
    cd.screenPoints[QWSPointerCalibrationData::BottomLeft] = QPoint( offset, qt_screen->deviceHeight() - offset );
    cd.screenPoints[QWSPointerCalibrationData::BottomRight] = QPoint( qt_screen->deviceWidth() - offset, qt_screen->deviceHeight() - offset );
    cd.screenPoints[QWSPointerCalibrationData::TopRight] = QPoint( qt_screen->deviceWidth() - offset, offset );
    cd.screenPoints[QWSPointerCalibrationData::Center] = QPoint( qt_screen->deviceWidth()/2, qt_screen->deviceHeight()/2 );
    goodcd = cd;
    reset();

    if ( QWSServer::mouseHandler() ) {
      QString calFile = qgetenv("POINTERCAL_FILE");
      if (calFile.isEmpty())
        calFile = "/etc/pointercal";

      qLog(Input) << "Using calibration file " << calFile;
      anygood = QFile::exists(calFile);
      QWSServer::mouseHandler()->getCalibration(&goodcd);
      QWSServer::mouseHandler()->clearCalibration();
    }
    QDialog::showEvent(e);
    setVisible(true);
    setFocusPolicy( Qt::StrongFocus );
    setFocus();
    showFullScreen();
    QTimer::singleShot(0, this, SLOT(doGrab()) );
}

void Calibrate::store()
{
    if ( QWSServer::mouseHandler() && anygood ) {
      qLog(Input) << "Store calibration data to file";
      QWSServer::mouseHandler()->calibrate( &goodcd );
    }
}

void Calibrate::hideEvent(QHideEvent *e )
{
    qLog(Input) << "Calibrate::hideEvent()";
    store();
    reset();
    QDialog::hide();
    QDialog::reject();
    QDialog::hideEvent(e);
}

void Calibrate::reset()
{
    qLog(Input) << "Calibrate::reset()";
    penPos = QPoint();
    location = QWSPointerCalibrationData::TopLeft;
    crossPos = fromDevice( cd.screenPoints[location] );
    releaseMouse();
    releaseKeyboard();
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

    int p1 = (br.x() - tl.x()) * (br.x() - tl.x()) +
             (br.y() - tl.y()) * (br.y() - tl.y());

    int p2 = (bl.x() - tr.x()) * (bl.x() - tr.x()) +
             (bl.y() - tr.y()) * (bl.y() - tr.y());

    int d1 = (int)sqrt(p1);
    int d2 = (int)sqrt(p2);

    int avg = abs(d1 + d2) / 2;
    int tol= (int)avg/50; // tolerance of 2% in calibration points
    // Decrease 50 to increase percentage tolerance

    qLog(Input)<<"sanityCheck() d1="<<d1<<", d2="<<d2<<", avg="<<avg<<", tol="<<tol;
    if( (d1 >= avg-tol) && (d1 <= avg+tol) ) {
      if( (d2 >= avg-tol) && (d2 <= avg+tol) )
        return true;
    }
    return false;
}

void Calibrate::moveCrosshair( QPoint pt )
{
    showCross = false;
    repaint( crossPos.x()-8, crossPos.y()-8, 17, 17 );
    showCross = true;
    crossPos = pt;
    repaint( crossPos.x()-8, crossPos.y()-8, 17, 17 );
}

void Calibrate::paintEvent( QPaintEvent * )
{
    QPainter p( this );

    int y = height()/4;

    if ( !logo.isNull() ) {
      y = height() / 2 - (logo.height()+p.font().pixelSize()) - 15;
      p.drawPixmap( (width() - logo.width())/2, y, logo );
      y += logo.height()+5;
    }

    QFont f = p.font(); f.setBold(true);
    p.setFont( f );
    p.drawText( 0, y, width(), height() - y, Qt::AlignHCenter|Qt::TextWordWrap,
        tr("Welcome to Qtopia") );

    y = height() / 2 + 20;

    f.setBold(false);
    p.setFont( f );
    QString rt(tr("Touch the crosshairs firmly and accurately to calibrate your screen."));
    p.drawText( 20, y, width()-40, height()-y, Qt::AlignHCenter|Qt::TextWordWrap, rt);
    if ( showCross ) {
      p.drawRect( crossPos.x()-1, crossPos.y()-8, 2, 7 );
      p.drawRect( crossPos.x()-1, crossPos.y()+1, 2, 7 );
      p.drawRect( crossPos.x()-8, crossPos.y()-1, 7, 2 );
      p.drawRect( crossPos.x()+1, crossPos.y()-1, 7, 2 );
    }
}

void Calibrate::keyPressEvent( QKeyEvent *e )
{
    // Because we're a full-screen application, we don't want another
    // program to start (as it will be hidden behind us) or receive key
    // events, so we must handle all key events. The only key events
    // that are meaningful to us are those that will cause us to exit -
    // all others are accepted and not acted on, except for the Hangup key.
    // The Hangup key is special because we must exit and then pass it
    // on to the server (by pretending to ignore it), so that the server
    // can terminate all other applications too.

    if (( e->key() == Qt::Key_Escape ) || ( e->key() == Qt::Key_Back ) ||
        ( e->key() == Qt::Key_Hangup )) {
        reset();
        QDialog::hide();
    }

    if ( e->key() == Qt::Key_Hangup )
        e->ignore();
    else
        e->accept();
}

void Calibrate::keyReleaseEvent( QKeyEvent *e )
{
    // Accept all key releases to prevent them getting passed to anything
    // that is hidden behind us.
    e->accept();
}

void Calibrate::mousePressEvent( QMouseEvent *e )
{
    qLog(Input) << "Calibrate::mousePressEvent( QMouseEvent *e )";
    pressed = true;
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
    qLog(Input) << "Calibrate::mouseMoveEvent( QMouseEvent *e )";
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
    qLog(Input) << "Calibrate::mouseReleaseEvent( QMouseEvent *e )";
    if ( !pressed )
      return;
    pressed = false;
    if ( timer->isActive() )
      return;

    bool doMove = true;

    cd.devPoints[location] = penPos;
    if ( location < QWSPointerCalibrationData::LastLocation ) {
      location = (QWSPointerCalibrationData::Location)((int)location + 1);
    } else {
      if ( sanityCheck() ) {
        reset();
        anygood = true;
        goodcd = cd;
        hide();
        doMove = false;
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

    bool doneX = false;
    bool doneY = false;
    QPoint newPos( crossPos.x() + dx, crossPos.y() + dy );

    if ( abs(crossPos.x() - target.x()) <= abs(dx) ) {
      newPos.setX( target.x() );
      doneX = true;
    }

    if ( abs(crossPos.y() - target.y()) <= abs(dy) ) {
      newPos.setY(target.y());
      doneY = true;
    }

    if ( doneX && doneY ) {
      penPos = QPoint();
      timer->stop();
    }
    moveCrosshair( newPos );
}

void Calibrate::doGrab()
{
    qLog(Input) << "Calibrate::doGrab()";
/*
    if ( !QWidget::mouseGrabber() ) {
      grabMouse();
      keyboardGrabber();
    } else {
      QTimer::singleShot( 50, this, SLOT(doGrab()) );
    }
*/
    grabMouse();
    keyboardGrabber();
}

void Calibrate::menuTriggered(QAction *action)
{
    hide();
}

#endif // Q_WS_QWS
