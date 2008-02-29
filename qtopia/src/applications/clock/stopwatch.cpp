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

#include "stopwatch.h"
#include "analogclock.h"

#include <qtopia/qpeapplication.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/config.h>
#include <qtopia/timestring.h>
#include <qtopia/alarmserver.h>
#include <qtopia/sound.h>
#include <qtopia/resource.h>
#include <qtopia/global.h>

#include <qlcdnumber.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qpainter.h>
#include <qtabwidget.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qlineedit.h>

static const int sw_prec = 2;
static const int magic_daily = 2292922;
static const int magic_countdown = 2292923;

static void toggleScreenSaver( bool on )
{
    QPEApplication::setTempScreenSaverMode(on ? QPEApplication::Enable : QPEApplication::DisableSuspend);  
}

StopWatch::StopWatch( QWidget * parent, const char *, WFlags f )
    : StopWatchBase( parent, "clock", f ), swatch_splitms(99) // No tr
{
    swLayout = 0;

    analogStopwatch = new AnalogClock( swFrame );
    stopwatchLcd = new QLCDNumber( swFrame );
    stopwatchLcd->setFrameStyle( QFrame::NoFrame );
    stopwatchLcd->setSegmentStyle( QLCDNumber::Flat );
    stopwatchLcd->setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred) );

    if ( qApp->desktop()->height() > 240 )
	stopwatchLcd->setFixedHeight( 30 );

    connect( stopStart, SIGNAL(pressed()), SLOT(stopStartStopWatch()) );
    connect( reset, SIGNAL(pressed()), SLOT(resetStopWatch()) );

#ifdef QTOPIA_PHONE
    if (!Global::mousePreferred()) {
	stopStart->setText("* " + stopStart->text());
	stopStart->setAccel(Key_Asterisk);
	reset->setText(reset->text()+" #");
	reset->setAccel(Key_NumberSign);
	stopStart->setFocusPolicy(NoFocus);
	reset->setFocusPolicy(NoFocus);
    }
#endif

    t = new QTimer( this );
    connect( t, SIGNAL(timeout()), SLOT(updateClock()) );

    connect( qApp, SIGNAL(timeChanged()), SLOT(updateClock()) );

    swatch_running = FALSE;
    swatch_totalms = 0;
    swatch_currLap = 0;
    swatch_dispLap = 0;
    stopwatchLcd->setNumDigits( 8+1+sw_prec );

    QVBoxLayout *lvb = new QVBoxLayout( lapFrame );

    nextLapBtn = new QToolButton( UpArrow, lapFrame );
    connect( nextLapBtn, SIGNAL(clicked()), this, SLOT(nextLap()) );
    nextLapBtn->setAccel( Key_Up );
    lvb->addWidget( nextLapBtn );

    prevLapBtn = new QToolButton( DownArrow, lapFrame );
    connect( prevLapBtn, SIGNAL(clicked()), this, SLOT(prevLap()) );
    prevLapBtn->setAccel( Key_Down );
    prevLapBtn->setMinimumWidth( 15 );
    lvb->addWidget( prevLapBtn );

    prevLapBtn->setEnabled( FALSE );
    nextLapBtn->setEnabled( FALSE );

#ifdef QTOPIA_PHONE
    nextLapBtn->setFocusPolicy ( NoFocus );
//    prevLapBtn->setFocusPolicy ( NoFocus );
#endif

    reset->setEnabled( FALSE );

    lapLcd->setNumDigits( 8+1+sw_prec );
    splitLcd->setNumDigits( 8+1+sw_prec );

    lapNumLcd->display( 1 );

    lapTimer = new QTimer( this );
    connect( lapTimer, SIGNAL(timeout()), this, SLOT(lapTimeout()) );

    for (uint s = 0; s < swatch_splitms.count(); s++ )
	swatch_splitms[(int)s] = 0;

    connect( qApp, SIGNAL(clockChanged(bool)), this, SLOT(changeClock(bool)) );

    swFrame->installEventFilter( this );
    updateClock();
}

StopWatch::~StopWatch()
{
    toggleScreenSaver( true );
}

void StopWatch::updateClock()
{
    int totalms = swatch_totalms;
    if ( swatch_running )
	totalms += swatch_start.elapsed();
    setSwatchLcd( stopwatchLcd, totalms, !swatch_running );
    QTime swatch_time = QTime(0,0,0).addMSecs(totalms);
    analogStopwatch->display( swatch_time );
    if ( swatch_dispLap == swatch_currLap ) {
	swatch_splitms[swatch_currLap] = swatch_totalms;
	if ( swatch_running )
	    swatch_splitms[swatch_currLap] += swatch_start.elapsed();
	updateLap();
    }
}

void StopWatch::changeClock( bool )
{
    updateClock();
}

void StopWatch::stopStartStopWatch()
{
    if ( swatch_running ) {
	swatch_totalms += swatch_start.elapsed();
	swatch_splitms[swatch_currLap] = swatch_totalms;
	stopStart->setText( tr("Start") );
	reset->setText( tr("Reset") );
#ifdef QTOPIA_PHONE
	if (!Global::mousePreferred()) {
	    stopStart->setText("* " + stopStart->text());
	    stopStart->setAccel(Key_Asterisk);
	    reset->setText(reset->text()+" #");
	    reset->setAccel(Key_NumberSign);
	}
#endif
	reset->setEnabled( TRUE );
	t->stop();
	swatch_running = FALSE;
	toggleScreenSaver( TRUE );
	updateClock();
    } else {
	swatch_start.start();
	stopStart->setText( tr("Stop") );
	reset->setText( tr("Lap/Split") );
#ifdef QTOPIA_PHONE
	if (!Global::mousePreferred()) {
	    stopStart->setText("* " + stopStart->text());
	    stopStart->setAccel(Key_Asterisk);
	    reset->setText(reset->text()+" #");
	    reset->setAccel(Key_NumberSign);
	}
#endif
	reset->setEnabled( swatch_currLap < 98 );
        t->start( 1000 );
	swatch_running = TRUE;
	// disable screensaver while stop watch is running
	toggleScreenSaver( FALSE );
    }
    swatch_dispLap = swatch_currLap;
    updateLap();
    prevLapBtn->setEnabled( swatch_dispLap );
    nextLapBtn->setEnabled( swatch_dispLap < swatch_currLap );
#ifndef QTOPIA_PHONE
    stopStart->setAccel( Key_Return );
#endif
}

void StopWatch::resetStopWatch()
{
    if ( swatch_running ) {
	swatch_splitms[swatch_currLap] = swatch_totalms+swatch_start.elapsed();
	swatch_dispLap = swatch_currLap;
	if ( swatch_currLap < 98 )  // allow up to 99 laps
	    swatch_currLap++;
	reset->setEnabled( swatch_currLap < 98 );
	updateLap();
	lapTimer->start( 2000, TRUE );
    } else {
	swatch_start.start();
	swatch_totalms = 0;
	swatch_currLap = 0;
	swatch_dispLap = 0;
	for ( uint i = 0; i < swatch_splitms.count(); i++ )
	    swatch_splitms[(int)i] = 0;
	updateLap();
	updateClock();
	reset->setText( tr("Lap/Split") );
	reset->setEnabled( FALSE );
    }
    prevLapBtn->setEnabled( swatch_dispLap );
    nextLapBtn->setEnabled( swatch_dispLap < swatch_currLap );
}

void StopWatch::prevLap()
{
    if ( swatch_dispLap > 0 ) {
	swatch_dispLap--;
	updateLap();
	prevLapBtn->setEnabled( swatch_dispLap );
	nextLapBtn->setEnabled( swatch_dispLap < swatch_currLap );
    }
}

void StopWatch::nextLap()
{
    if ( swatch_dispLap < swatch_currLap ) {
	swatch_dispLap++;
	updateLap();
	prevLapBtn->setEnabled( swatch_dispLap );
	nextLapBtn->setEnabled( swatch_dispLap < swatch_currLap );
    }
}

void StopWatch::lapTimeout()
{
    swatch_dispLap = swatch_currLap;
    updateLap();
    prevLapBtn->setEnabled( swatch_dispLap );
    nextLapBtn->setEnabled( swatch_dispLap < swatch_currLap );
}

void StopWatch::updateLap()
{
    if ( swatch_running && swatch_currLap == swatch_dispLap ) {
	swatch_splitms[swatch_currLap] = swatch_totalms;
	swatch_splitms[swatch_currLap] += swatch_start.elapsed();
    }
    int split = swatch_splitms[swatch_dispLap];
    int lap;
    if ( swatch_dispLap > 0 )
	lap = swatch_splitms[swatch_dispLap] - swatch_splitms[swatch_dispLap-1];
    else
	lap = swatch_splitms[swatch_dispLap];

    lapNumLcd->display( swatch_dispLap+1 );
    bool showMs = !swatch_running || swatch_dispLap!=swatch_currLap;
    setSwatchLcd( lapLcd, lap, showMs );
    setSwatchLcd( splitLcd, split, showMs );
}

void StopWatch::setSwatchLcd( QLCDNumber *lcd, int ms, bool showMs )
{
    QTime swatch_time = QTime(0,0,0).addMSecs(ms);
    QString d = showMs ? QString::number(ms%1000+1000) : QString("    ");
    QString lcdtext = swatch_time.toString() + "." + d.right(3).left(sw_prec);
    lcd->display( lcdtext );
    lcd->repaint( FALSE );
}

bool StopWatch::eventFilter( QObject *o, QEvent *e )
{
    if ( o == swFrame && e->type() == QEvent::Resize ) {
	QResizeEvent *re = (QResizeEvent *)e;
	delete swLayout;
	if ( re->size().height() < 80 || re->size().height()*3 < re->size().width() )
	    swLayout = new QHBoxLayout( swFrame );
	else
	    swLayout = new QVBoxLayout( swFrame );
	swLayout->addWidget( analogStopwatch );
	swLayout->addWidget( stopwatchLcd );
	swLayout->activate();
    }

    return FALSE;
}

void StopWatch::showEvent(QShowEvent *e)
{
    StopWatchBase::showEvent(e);
    updateClock();
}

