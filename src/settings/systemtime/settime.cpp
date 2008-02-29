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

#include "settime.h"

#include <qtopia/alarmserver.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/private/event.h>
#include <qtopia/datebookdb.h>
#include <qtopia/datebookmonth.h>
#include <qtopia/global.h>
#include <qtopia/resource.h>
#include <qtopia/timeconversion.h>
#include <qtopia/tzselect.h>
#include <qtopia/timestring.h>
#include <qtopia/qpedialog.h>
#if ( defined Q_WS_QWS || defined(_WS_QWS_) ) && !defined(QT_NO_COP)
#include <qtopia/qcopenvelope_qws.h>
#endif

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qtoolbutton.h>
#ifdef QWS
#include <qwindowsystem_qws.h>
#endif
#include <qcombobox.h>

#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>


SetDateTime::SetDateTime(QWidget *parent, const char *name, bool modal,  WFlags f )
    : QDialog( parent, name, modal, f )
{
    setCaption( tr("Set System Time") );

    QVBoxLayout *vb = new QVBoxLayout( this, 5 );

    QHBoxLayout *hb = new QHBoxLayout( vb, -1 );

    QLabel *lblZone = new QLabel( tr( "Time Zone" ), this );
    lblZone->setMaximumSize( lblZone->sizeHint() );
    hb->addWidget( lblZone );

    tz = new TimeZoneSelector( this );
    tz->setMinimumSize( tz->sizeHint() );
    hb->addWidget( tz );

    time = new SetTime( this );
    vb->addWidget( time );

    QHBoxLayout *db = new QHBoxLayout( vb );
    QLabel *dateLabel = new QLabel( tr("Date"), this );
    db->addWidget( dateLabel, 1 );
    date = new DateButton( TRUE, this );
    db->addWidget( date, 2 );


    QFrame *hline = new QFrame( this );
    hline->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    vb->addWidget( hline );

    Config config("qpe");
    config.setGroup( "Time" );

    QHBoxLayout *hb1 = new QHBoxLayout( vb );

    QLabel *l = new QLabel( tr("Time format"), this );
    //    l->setAlignment( AlignRight | AlignVCenter );
    hb1->addWidget( l, 1 );


    ampmCombo = new QComboBox( this );
    ampmCombo->insertItem( tr("24 hour"), 0 );
    ampmCombo->insertItem( tr("12 hour"), 1 );
    hb1->addWidget( ampmCombo, 2 );

    int show12hr = config.readBoolEntry("AMPM") ? 1 : 0;
    ampmCombo->setCurrentItem( show12hr );
    time->show12hourTime( show12hr );
    
    connect(ampmCombo, SIGNAL(activated(int)), 
	    time, SLOT(show12hourTime(int)));



    QHBoxLayout *hb2 = new QHBoxLayout( vb );
    l = new QLabel( tr("Weeks start on" ), this );
    //l->setAlignment( AlignRight | AlignVCenter );
    hb2->addWidget( l, 1 );

    weekStartCombo = new QComboBox( this );
    weekStartCombo->insertItem( tr("Sunday"), 0 );
    weekStartCombo->insertItem( tr("Monday"), 1 );

    hb2->addWidget( weekStartCombo, 2 );
    int startMonday =  config.readBoolEntry("MONDAY") ? 1 : 0;
    date->setWeekStartsMonday( startMonday );
    weekStartCombo->setCurrentItem( startMonday );
    
    connect( weekStartCombo, SIGNAL( activated(int)), 
	     date, SLOT(setWeekStartsMonday(int)));


    QHBoxLayout *hb3 = new QHBoxLayout( vb );
    l = new QLabel( tr("Date format" ), this );
    hb3->addWidget( l, 1 );
    dateFormatCombo = new QComboBox( this );
    hb3->addWidget( dateFormatCombo, 2 );
    
    config.setGroup( "Date" );
    DateFormat df(QChar(config.readEntry("Separator", "/")[0]),
	    (DateFormat::Order)config .readNumEntry("ShortOrder", DateFormat::DayMonthYear),
	    (DateFormat::Order)config.readNumEntry("LongOrder", DateFormat::DayMonthYear));

    int currentdf = 0;
    date_formats[0] = DateFormat('/', DateFormat::MonthDayYear);
    dateFormatCombo->insertItem( tr( date_formats[0].toNumberString()) );
    date_formats[1] = DateFormat('.', DateFormat::DayMonthYear);
    if (df == date_formats[1])
	currentdf = 1;
    dateFormatCombo->insertItem( tr( date_formats[1].toNumberString() ) );
    date_formats[2] = DateFormat('-', DateFormat::YearMonthDay, 
	    DateFormat::DayMonthYear);
    if (df == date_formats[2])
	currentdf = 2;
    dateFormatCombo->insertItem( tr( date_formats[2].toNumberString() ) ); //ISO8601
    date_formats[3] = DateFormat('/', DateFormat::DayMonthYear);
    if (df == date_formats[3])
	currentdf = 3;
    dateFormatCombo->insertItem( tr( date_formats[3].toNumberString() ) );

    dateFormatCombo->setCurrentItem( currentdf );
    date->setDateFormat( df );

    connect( dateFormatCombo, SIGNAL( activated(int)), 
	     this, SLOT(formatChanged(int)));


    vb->addStretch( 0 );

    QObject::connect( tz, SIGNAL( signalNewTz( const QString& ) ),
                      time, SLOT( slotTzChange( const QString& ) ) );
    QObject::connect( tz, SIGNAL( signalNewTz( const QString& ) ),
                      this, SLOT( tzChange( const QString& ) ) );

    dl = new QPEDialogListener(this);
}

void SetDateTime::accept()
{
    tz->setFocus();
    // really turn off the screensaver before doing anything
    {
	// Needs to be encased in { } so that it deconstructs and sends
#ifndef QT_NO_COP
	QCopEnvelope disableScreenSaver( "QPE/System", "setScreenSaverIntervals(int,int,int)" );
	disableScreenSaver << 0 << 0 << 0;
#endif
    }
    // Need to process the QCOP event generated above before proceeding
    qApp->processEvents();

    {
	Config config("qpe");
	config.setGroup( "Time" );
	config.writeEntry( "AMPM", ampmCombo->currentItem() );
	config.writeEntry( "MONDAY", weekStartCombo->currentItem() );
	config.setGroup( "Date" );
	DateFormat df = date_formats[dateFormatCombo->currentItem()];
	config.writeEntry( "Separator", QString(df.separator()));
	config.writeEntry( "ShortOrder", df.shortOrder());
	config.writeEntry( "LongOrder", df.longOrder());

	Config lconfig("locale");
	lconfig.setGroup( "Location" );
	lconfig.writeEntry( "Timezone", tz->currentZone() );
    }

    // before we progress further, set our TZ!
    setenv( "TZ", tz->currentZone(), 1 );
    // now set the time...
    QDateTime dt( date->date(), time->time() );

    if ( dt.isValid() ) {
	struct timeval myTv;
	myTv.tv_sec = TimeConversion::toUTC( dt );
	myTv.tv_usec = 0;

	if ( myTv.tv_sec != -1 )
	    ::settimeofday( &myTv, 0 );
	Global::writeHWClock();
	// since time has changed quickly load in the datebookdb
	// to allow the alarm server to get a better grip on itself
	// (example re-trigger alarms for when we travel back in time)
	DateBookDB db;
    } else {
	qWarning( "Invalid date/time" );
    }
    // set the timezone for everyone else...
#ifndef QT_NO_COP
    QCopEnvelope setTimeZone( "QPE/System", "timeChange(QString)" );
    setTimeZone << tz->currentZone();
#endif

    // AM/PM setting and notify time changed
#ifndef QT_NO_COP
    QCopEnvelope setClock( "QPE/System", "clockChange(bool)" );
    setClock << ampmCombo->currentItem();
#endif

    // Notify everyone what day we prefer to start the week on.
#ifndef QT_NO_COP
    QCopEnvelope setWeek( "QPE/System", "weekChange(bool)" );
    setWeek << weekStartCombo->currentItem();
#endif

    // Notify everyone what date format  to use 
#ifndef QT_NO_COP
    QCopEnvelope setDateFormat( "QPE/System", "setDateFormat(DateFormat)" );
    setDateFormat << date_formats[dateFormatCombo->currentItem()];
#endif

    // Restore screensaver
#ifndef QT_NO_COP
    QCopEnvelope enableScreenSaver( "QPE/System", "setScreenSaverIntervals(int,int,int)" );
    enableScreenSaver << -1 << -1 << -1;
#endif

    QDialog::accept();
}

void SetDateTime::done(int r)
{
    QDialog::done(r);
    close();
}

void SetDateTime::tzChange( const QString &tz )
{
    // set the TZ get the time and leave gracefully...
    QString strSave;
    strSave = getenv( "TZ" );
    setenv( "TZ", tz, 1 );

    QDate d = QDate::currentDate();
    // reset the time.
    if ( !strSave.isNull() ) {
	setenv( "TZ", strSave, 1 );
    }
    date->setDate( d );
}

void SetDateTime::formatChanged(int i)
{
    date->setDateFormat(date_formats[i]);
}

static const int ValueAM = 0;
static const int ValuePM = 1;



SetTime::SetTime( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    use12hourTime = FALSE;

    QTime currTime = QTime::currentTime();
    hour = currTime.hour();
    minute = currTime.minute();

    QHBoxLayout *hb2 = new QHBoxLayout( this );
    hb2->setSpacing( 3 );

    QLabel *l = new QLabel( tr("Hour"), this );
    //    l->setAlignment( AlignRight | AlignVCenter );
    hb2->addWidget( l );

    sbHour = new QSpinBox( this );
    sbHour->setMinimumWidth( 30 );
    if(use12hourTime) {
	sbHour->setMinValue(1);
	sbHour->setMaxValue( 12 );
	int show_hour = hour;
	if (hour > 12)
	    show_hour -= 12;
	if (show_hour == 0)
	    show_hour = 12;

	sbHour->setValue( show_hour );
    } else {
	sbHour->setMinValue( 0 );
	sbHour->setMaxValue( 23 );
	sbHour->setValue( hour );
    }
    sbHour->setWrapping(TRUE);
    connect( sbHour, SIGNAL(valueChanged(int)), this, SLOT(hourChanged(int)) );
    hb2->addWidget( sbHour );

    hb2->addStretch( 1 );

    l = new QLabel( tr("Minute"), this );
    //l->setAlignment( AlignRight | AlignVCenter );
    hb2->addWidget( l );

    sbMin = new QSpinBox( this );
    sbMin->setMinValue( 0 );
    sbMin->setMaxValue( 59 );
    sbMin->setWrapping(TRUE);
    sbMin->setValue( minute );
    sbMin->setMinimumWidth( 30 );
    connect( sbMin, SIGNAL(valueChanged(int)), this, SLOT(minuteChanged(int)) );
    hb2->addWidget( sbMin );

    hb2->addStretch( 1 );

    ampm = new QComboBox( this );
    ampm->insertItem( tr("AM"), ValueAM );
    ampm->insertItem( tr("PM"), ValuePM );
    connect( ampm, SIGNAL(activated(int)), this, SLOT(checkedPM(int)) );
    hb2->addWidget( ampm );

    hb2->addStretch( 1 );

}

QTime SetTime::time() const
{
    return QTime( hour, minute, 0 );
}

void SetTime::hourChanged( int value )
{
    if(use12hourTime) {
	int realhour = value;
	if (realhour == 12)
	    realhour = 0;
	if (ampm->currentItem() == ValuePM )
	    realhour += 12;
	hour = realhour;
    } else
	hour = value;
}

void SetTime::minuteChanged( int value )
{
    sbMin->setPrefix( value <= 9 ? "0" : "" );
    minute = value;
}

void SetTime::show12hourTime( int on )
{
    use12hourTime = on;
    ampm->setEnabled(on);

    int show_hour = hour;
    if ( on ) {
	/* this might change the value of hour */
	sbHour->setMinValue(1);
	sbHour->setMaxValue( 12 );

	/* so use one we saved earlier */
	if (show_hour >= 12) {
	    show_hour -= 12;
	    ampm->setCurrentItem( ValuePM );
	} else {
	    ampm->setCurrentItem( ValueAM );
	}
	if (show_hour == 0)
	    show_hour = 12;

    } else {
	sbHour->setMinValue( 0 );
	sbHour->setMaxValue( 23 );
    }


    sbHour->setValue( show_hour );
}

void SetTime::checkedPM( int c )
{
    int show_hour = sbHour->value();
    if (show_hour == 12)
	show_hour = 0;

    if ( c == ValuePM )
	show_hour += 12;

    hour = show_hour;
}

void SetTime::slotTzChange( const QString &tz )
{
    // set the TZ get the time and leave gracefully...
    QString strSave;
    strSave = getenv( "TZ" );
    setenv( "TZ", tz, 1 );

    QTime t = QTime::currentTime();
    // reset the time.
    if ( !strSave.isNull() ) {
	setenv( "TZ", strSave, 1 );
    }

    // just set the spinboxes and let it propage through
    if(use12hourTime) {
	int show_hour = t.hour();
	if (t.hour() >= 12) {
	    show_hour -= 12;
	    ampm->setCurrentItem( ValuePM );
	} else {
	    ampm->setCurrentItem( ValueAM );
	}
	if (show_hour == 0)
	    show_hour = 12;
	sbHour->setValue( show_hour );
    } else {
	sbHour->setValue( t.hour() );
    }
    sbMin->setValue( t.minute() );
}



/*

SetDate::SetDate( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    QHBoxLayout *hb = new QHBoxLayout( this );
    dbm = new DateBookMonth( this );
    hb->addWidget( dbm );
}

QDate SetDate::date() const
{
    return dbm->selectedDate();
}

void SetDate::slotTzChange( const QString &tz )
{
    // set the TZ get the time and leave gracefully...
    QString strSave;
    strSave = getenv( "TZ" );
    setenv( "TZ", tz, 1 );

    QDate d = QDate::currentDate();
    // reset the time.
    if ( !strSave.isNull() ) {
	setenv( "TZ", strSave, 1 );
    }
    dbm->setDate( d.year(), d.month(), d.day() );
}

void SetDate::slotWeekChange( int startOnMonday )
{
    dbm->slotWeekChange( startOnMonday );
}
*/
