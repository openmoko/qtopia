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

#include "clock.h"
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
#ifdef QTOPIA_PHONE
# include <qtopia/contextbar.h>
#endif

#include <qlcdnumber.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qmessagebox.h>

Clock::Clock( QWidget * parent, const char *, WFlags f )
    : ClockBase( parent, "clock", f ) // No tr
{
    Config config( "qpe" );
    config.setGroup("Time");
    ampm = config.readBoolEntry( "AMPM" );

    if (!ampm)
	clockAmPm->hide();

    analogClock->display( QTime::currentTime() );
    analogClock->setFace("background");
    clockLcd->setNumDigits( 5 );
    clockLcd->setFixedWidth( clockLcd->sizeHint().width() );
#ifdef QTOPIA_PHONE
    ContextBar::setLabel(clockLcd, Key_Select, ContextBar::NoLabel);
    clockLcd->setFocusPolicy(StrongFocus);
#endif
    date->setText("");
    if ( qApp->desktop()->width() < 200 )
	date->setFont( QFont(date->font().family(), 14, QFont::Bold) );
    if ( qApp->desktop()->height() > 240 )
	clockLcd->setFixedHeight( 30 );

    t = new QTimer( this );
    connect( t, SIGNAL(timeout()), SLOT(updateClock()) );
    t->start( 1000 );

    connect( qApp, SIGNAL(timeChanged()), SLOT(updateClock()) );
    connect( qApp, SIGNAL(clockChanged(bool)), this, SLOT(changeClock(bool)) );

    QTimer::singleShot( 0, this, SLOT(updateClock()) );
}

Clock::~Clock()
{
}

void Clock::updateClock()
{
    QTime tm = QDateTime::currentDateTime().time();
    QString s;
    if ( ampm ) {
	int hour = tm.hour();
	if (hour == 0) 
	    hour = 12;
	if (hour > 12)
	    hour -= 12;
	s.sprintf( "%2d%c%02d", hour, ':', tm.minute() );
	clockAmPm->setText( (tm.hour() >= 12) ? "PM" : "AM" );
	clockAmPm->show();
    } else {
	s.sprintf( "%2d%c%02d", tm.hour(), ':', tm.minute() );
	clockAmPm->hide();
    }
    clockLcd->display( s );
    if (isVisible())
	clockLcd->repaint( FALSE );
    analogClock->display( QTime::currentTime() );
    if (width() < 220) {
	QString text = TimeString::localDayOfWeek(QDate::currentDate());
	text += " " + TimeString::localYMD( QDate::currentDate());
	date->setText( text );
    } else {
	date->setText( TimeString::localYMD( QDate::currentDate(), TimeString::Long));
    }
}

void Clock::changeClock( bool a )
{
    ampm = a;
    updateClock();
}


