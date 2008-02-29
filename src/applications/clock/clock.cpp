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

#include "clock.h"

#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#include <qsettings.h>
#include <qtimestring.h>
#ifdef QTOPIA_PHONE
# include <qtopia/qsoftmenubar.h>
#endif

#include <qlcdnumber.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qmessagebox.h>
#include <QDesktopWidget>
#include <QAnalogClock>

Clock::Clock(QWidget *parent, Qt::WFlags f)
    : QWidget(parent, f)
{
    setupUi(this);
    QSettings config("Trolltech","qpe");
    config.beginGroup("Time");
    ampm = config.value( "AMPM" ).toBool();

    if (!ampm)
        clockAmPm->hide();

    analogClock->display( QTime::currentTime() );
    analogClock->setFace( QPixmap( ":image/background" ) );
    clockLcd->setNumDigits( 5 );
    clockLcd->setFixedWidth( clockLcd->sizeHint().width() );
#ifdef QTOPIA_PHONE
    QSoftMenuBar::setLabel(clockLcd, Qt::Key_Select, QSoftMenuBar::NoLabel);
#endif
    date->setText( QTimeString::localYMD( QDate::currentDate(), QTimeString::Long ) );

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
        clockLcd->repaint();
    analogClock->display( QTime::currentTime() );
    date->setText( QTimeString::localYMD( QDate::currentDate(), QTimeString::Long ) );
}

void Clock::changeClock( bool a )
{
    ampm = a;
    updateClock();
}


