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

#include "clock.h"
#include "analogclock.h"

#include <qpe/qpeapplication.h>
#ifdef QWS
#include <qpe/qcopenvelope_qws.h>
#endif
#include <qpe/config.h>
#include <qpe/timestring.h>
#include <qpe/alarmserver.h>
#include <qpe/sound.h>
#include <qpe/resource.h>

#include <qlcdnumber.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qtabwidget.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <qlayout.h>

static const int sw_prec = 2;
static const int magic_daily = 2292922;
static const int magic_countdown = 2292923;

static void toggleScreenSaver( bool on )
{
#ifndef QT_NO_COP
    QCopEnvelope e("QPE/System", "setScreenSaverMode(int)" );
    e << (on ? QPEApplication::Enable: QPEApplication::DisableSuspend );
#endif
}

Clock::Clock( QWidget * parent, const char *, WFlags f )
    : ClockBase( parent, "clock", f )
{
    alarmDlg = 0;
    alarmDlgLabel = 0;

    Config config( "qpe" );
    config.setGroup("Time");
    ampm = config.readBoolEntry( "AMPM", TRUE );

    connect( tabs, SIGNAL(currentChanged(QWidget*)),
	     this, SLOT(tabChanged(QWidget*)) );

    analogClock->display( QTime::currentTime() );
    clockLcd->setNumDigits( 5 );
    clockLcd->setFixedWidth( clockLcd->sizeHint().width() );
    date->setText( TimeString::longDateString( QDate::currentDate() ) );
    if ( qApp->desktop()->width() < 200 )
	date->setFont( QFont("Helvetica", 14, QFont::Bold) );
    if ( qApp->desktop()->height() > 240 ) {
	clockLcd->setFixedHeight( 30 );
	stopwatchLcd->setFixedHeight( 30 );
    }

    connect( stopStart, SIGNAL(pressed()), SLOT(stopStartStopWatch()) );
    connect( reset, SIGNAL(pressed()), SLOT(resetStopWatch()) );

    t = new QTimer( this );
    connect( t, SIGNAL(timeout()), SLOT(updateClock()) );
    t->start( 1000 );

    alarmt = new QTimer( this );
    connect( alarmt, SIGNAL(timeout()), SLOT(alarmTimeout()) );

    connect( qApp, SIGNAL(timeChanged()), SLOT(updateClock()) );
    connect( qApp, SIGNAL(timeChanged()), SLOT(applyDailyAlarm()) );

    swatch_running = FALSE;
    swatch_totalms = 0;
    stopwatchLcd->setNumDigits( 8+1+sw_prec );
    stopwatchLcd->setFixedWidth( stopwatchLcd->sizeHint().width() );
    stopwatchLcd->display( "00:00:00.00" );

    connect( qApp, SIGNAL(clockChanged(bool)), this, SLOT(changeClock(bool)) );

    cdGroup->hide(); // XXX implement countdown timer.

    connect( dailyHour, SIGNAL(valueChanged(int)), this, SLOT(applyDailyAlarm()) );
    connect( dailyMinute, SIGNAL(valueChanged(int)), this, SLOT(setDailyMinute(int)) );
    connect( dailyAmPm, SIGNAL(activated(int)), this, SLOT(setDailyAmPm(int)) );
    cdLcd->display( "00:00" );

    Config cConfig( "Clock" );
    cConfig.setGroup( "Daily Alarm" );
    bool alarm = cConfig.readBoolEntry("Enabled", FALSE);
    dailyEnabled->setChecked( alarm );
    int m = cConfig.readNumEntry( "Minute", 0 );
    dailyMinute->setValue( m );
    int h = cConfig.readNumEntry( "Hour", 7 );
    if ( ampm ) {
	if (h > 12) {
	    h -= 12;
	    dailyAmPm->setCurrentItem( 1 );
	}
	if (h == 0) h = 12;
	dailyHour->setMinValue( 1 );
	dailyHour->setMaxValue( 12 );
    } else {
	dailyAmPm->hide();
    }
    dailyHour->setValue( h );

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    connect( qApp, SIGNAL(appMessage(const QCString&, const QByteArray&)),
	    this, SLOT(appMessage(const QCString&, const QByteArray&)) );
#endif

    QTimer::singleShot( 0, this, SLOT(updateClock()) );
}

Clock::~Clock()
{
    toggleScreenSaver( true );
}

void Clock::updateClock()
{
    if ( tabs->currentPageIndex() == 0 ) {
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
	clockLcd->repaint( FALSE );
	analogClock->display( QTime::currentTime() );
	date->setText( TimeString::longDateString( QDate::currentDate() ) );
    } else if ( tabs->currentPageIndex() == 1 ) {
	QTime swatch_time;
	QString lcdtext;
	int totalms = swatch_totalms;
	if ( swatch_running )
	    totalms += swatch_start.elapsed();
	swatch_time = QTime(0,0,0).addMSecs(totalms);
	QString d = swatch_running ? QString("    ")
		    : QString::number(totalms%1000+1000);
	lcdtext = swatch_time.toString() + "." + d.right(3).left(sw_prec);
	stopwatchLcd->display( lcdtext );
	stopwatchLcd->repaint( FALSE );
	analogStopwatch->display( swatch_time );
    } else if ( tabs->currentPageIndex() == 2 ) {
	// Ensure they emit, even if edited by keyboard
	dailyHour->value();
	dailyMinute->value();
    }
}

void Clock::changeClock( bool a )
{
    if ( ampm != a ) {
	int minute = dailyMinute->value();
	int hour = dailyHour->value();
	if ( ampm ) {
	    if (hour == 12)
		hour = 0;
	    if (dailyAmPm->currentItem() == 1 )
		hour += 12;
	    dailyHour->setMinValue( 0 );
	    dailyHour->setMaxValue( 23 );
	    dailyAmPm->hide();
	} else {
	    if (hour > 12) {
		hour -= 12;
		dailyAmPm->setCurrentItem( 1 );
	    }
	    if (hour == 0) hour = 12;
	    dailyHour->setMinValue( 1 );
	    dailyHour->setMaxValue( 12 );
	    dailyAmPm->show();
	}
	dailyMinute->setValue( minute );
	dailyHour->setValue( hour );
    }
    ampm = a;
    updateClock();
}

void Clock::stopStartStopWatch()
{
    if ( swatch_running ) {
	swatch_totalms += swatch_start.elapsed();
	stopStart->setText( tr("Start") );
	t->stop();
	swatch_running = FALSE;
	toggleScreenSaver( TRUE );
	updateClock();
    } else {
	swatch_start.start();
	stopStart->setText( tr("Stop") );
        t->start( 1000 );
	swatch_running = TRUE;
	// disable screensaver while stop watch is running
	toggleScreenSaver( FALSE );
    }
    stopStart->setAccel( Key_Space );
}

void Clock::resetStopWatch()
{
    swatch_start.start();
    swatch_totalms = 0;
    updateClock();
}

void Clock::tabChanged( QWidget * )
{
    if ( tabs->currentPageIndex() == 0 ) {
	t->start(1000);
    } else if ( tabs->currentPageIndex() == 1 ) {
	if ( !swatch_running )
	    t->stop();
	stopStart->setAccel( Key_Space );
    } else if ( tabs->currentPageIndex() == 2 ) {
	t->start(1000);
    }
    updateClock();
}

void Clock::setDailyAmPm(int)
{
    applyDailyAlarm();
}

void Clock::setDailyMinute( int m )
{
    if ( m <= 9 )
	dailyMinute->setPrefix( "0" );
    else
	dailyMinute->setPrefix( "" );
    applyDailyAlarm();
}

void Clock::appMessage( const QCString &msg, const QByteArray &data )
{
    if ( msg == "alarm(QDateTime,int)" ) {
	QDataStream ds(data,IO_ReadOnly);
	QDateTime when;
	int t;
	ds >> when >> t;
	QTime theTime( when.time() );
	if ( t == magic_daily ) {
	    QString msg = tr("<b>Daily Alarm:</b><p>");
	    QString ts;
	    if ( ampm ) {
	    bool pm = FALSE;
		int h = theTime.hour();
		if (h > 12) {
		    h -= 12;
		    pm = TRUE;
		}
		if (h == 0) h = 12;
		ts.sprintf( "%02d:%02d %s", h, theTime.minute(), pm?"PM":"AM" );
	    } else {
		ts.sprintf( "%02d:%02d", theTime.hour(), theTime.minute() );
	    }
	    msg += ts;
	    Sound::soundAlarm();
	    alarmCount = 0;
	    alarmt->start( 5000 );
	    if ( !alarmDlg ) {
		alarmDlg = new QDialog( this, 0, TRUE );
		alarmDlg->setCaption( tr("Clock") );
		QVBoxLayout *vb = new QVBoxLayout( alarmDlg, 6 );
		QHBoxLayout *hb = new QHBoxLayout( vb );
		QLabel *l = new QLabel( alarmDlg );
		l->setPixmap( Resource::loadPixmap("alarmbell") );
		hb->addWidget(l);
		alarmDlgLabel = new QLabel( msg, alarmDlg );
		alarmDlgLabel->setAlignment( AlignCenter );
		hb->addWidget(alarmDlgLabel);
		QPushButton *cmdOk = new QPushButton( tr("OK"), alarmDlg );
		vb->addWidget(cmdOk);
		connect( cmdOk, SIGNAL(clicked()), alarmDlg, SLOT(accept()) );
	    } else {
		alarmDlgLabel->setText(msg);
	    }
	    // Set for tomorrow, so user wakes up every day, even if they
	    // don't confirm the dialog.
	    applyDailyAlarm();
	    if ( !alarmDlg->isVisible() ) {
		QPEApplication::execDialog(alarmDlg);
		alarmt->stop();
	    }
	} else if ( t == magic_countdown ) {
	    // countdown
	    Sound::soundAlarm();
	}
    } else if ( msg == "setDailyEnabled(int)" ) {
	QDataStream ds(data,IO_ReadOnly);
	int enableDaily;
	ds >> enableDaily;
	dailyEnabled->setChecked( enableDaily );
	applyDailyAlarm();
    }
}

void Clock::alarmTimeout()
{
    if ( alarmCount < 10 ) {
	Sound::soundAlarm();
	alarmCount++;
    } else {
	alarmCount = 0;
	alarmt->stop();
    }
}

QDateTime Clock::nextAlarm( int h, int m )
{
    QDateTime now = QDateTime::currentDateTime();
    QTime at( h, m );
    QDateTime when( now.date(), at );
    if ( when < now )
	when = when.addDays( 1 );

    return when;
}

void Clock::applyDailyAlarm()
{
    int minute = dailyMinute->value();
    int hour = dailyHour->value();
    if ( ampm ) {
	if (hour == 12)
	    hour = 0;
	if (dailyAmPm->currentItem() == 1 )
	    hour += 12;
    }

    Config config( "Clock" );
    config.setGroup( "Daily Alarm" );
    config.writeEntry( "Hour", hour );
    config.writeEntry( "Minute", minute );

    bool enableDaily = dailyEnabled->isChecked();
    config.writeEntry( "Enabled", enableDaily );

    QDateTime when = nextAlarm( hour, minute );
    AlarmServer::deleteAlarm(QDateTime(), "QPE/Application/clock",
	    "alarm(QDateTime,int)", magic_daily);
    if ( dailyEnabled->isChecked() ) {
	AlarmServer::addAlarm(when, "QPE/Application/clock",
			    "alarm(QDateTime,int)", magic_daily);
    }
}

void Clock::closeEvent( QCloseEvent *e )
{
    applyDailyAlarm();
    ClockBase::closeEvent(e);
}

