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

#include "settime.h"

#include <qtopia/alarmserver.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/private/event.h>
#include <qtopia/datetimeedit.h>
#include <qtopia/global.h>
#include <qtopia/resource.h>
#include <qtopia/timeconversion.h>
#include <qtopia/tzselect.h>
#include <qtopia/timestring.h>
#include <qtopia/qpedialog.h>
#if ( defined Q_WS_QWS || defined(_WS_QWS_) ) && !defined(QT_NO_COP)
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/qprocess.h>
#include <qtopia/process.h>
#endif
#ifdef QTOPIA_PHONE
#include <qtabwidget.h>
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
#include <unistd.h>


SetDateTime::SetDateTime(QWidget *parent, const char *name, bool modal,  WFlags f )
    : QDialog( parent, name, modal, f ), tzEditable(TRUE), tzLabel(0)
{
    setCaption( tr("Date/Time") );

    QVBoxLayout *vb = new QVBoxLayout( this, 4 );
#ifdef QTOPIA_PHONE
    QTabWidget *tb = new QTabWidget(this);
    vb->addWidget(tb);

    timePage = new QWidget(tb);
    QVBoxLayout *timeLayout = new QVBoxLayout( timePage, 4 );
    tb->addTab(timePage, tr("Time"));

    QWidget *formatPage = new QWidget(tb);
    QVBoxLayout *formatLayout = new QVBoxLayout( formatPage, 4 );
    tb->addTab(formatPage, tr("Format"));
#else
    timePage = this;
    QWidget *formatPage = this;
    QVBoxLayout *timeLayout = vb;
    QVBoxLayout *formatLayout = vb;
#endif

    tzLayout = new QHBoxLayout( timeLayout, -1 );

#ifdef QTOPIA_PHONE
    QLabel *lblZone = new QLabel( tr( "T.Z." ), timePage );
#else
    QLabel *lblZone = new QLabel( tr( "Time Zone" ), timePage );
#endif
    lblZone->setMaximumSize( lblZone->sizeHint() );
    tzLayout->addWidget( lblZone );

    tz = new TimeZoneSelector( timePage );
    tz->setMinimumSize( tz->sizeHint() );
    tzLayout->addWidget( tz );

    time = new SetTime( timePage );
    timeLayout->addWidget( time );

    QHBoxLayout *db = new QHBoxLayout( timeLayout );
    QLabel *dateLabel = new QLabel( tr("Date"), timePage );
    db->addWidget( dateLabel, 1 );
    date = new QPEDateEdit( QDate::currentDate(), timePage, 0, qApp->desktop()->width() > 200, FALSE );
    connect(date, SIGNAL(valueChanged(const QDate&)),
	    this, SLOT(dateChange(const QDate&)) );
    db->addWidget( date, 2 );

#ifndef QTOPIA_PHONE
    if (qApp->desktop()->height() >= 220) {
	QFrame *hline = new QFrame( timePage );
	hline->setFrameStyle( QFrame::HLine | QFrame::Sunken );
	timeLayout->addWidget( hline );
    }
#endif

    Config config("qpe");
    config.setGroup( "Time" );

    QHBoxLayout *hb1 = new QHBoxLayout( formatLayout );

    QLabel *l = new QLabel( tr("Time format"), formatPage );
    //    l->setAlignment( AlignRight | AlignVCenter );
    hb1->addWidget( l, 1 );


    ampmCombo = new QComboBox( formatPage );
    ampmCombo->insertItem( tr("24 hour"), 0 );
    ampmCombo->insertItem( tr("12 hour"), 1 );
    hb1->addWidget( ampmCombo, 2 );

    int show12hr = TimeString::currentAMPM() ? 1 : 0;
    ampmCombo->setCurrentItem( show12hr );
    time->show12hourTime( show12hr );

    connect(ampmCombo, SIGNAL(activated(int)),
	    time, SLOT(show12hourTime(int)));

    QHBoxLayout *hb2 = new QHBoxLayout( formatLayout );
    l = new QLabel( tr("Week starts" ), formatPage );
    //l->setAlignment( AlignRight | AlignVCenter );
    hb2->addWidget( l, 1 );

    weekStartCombo = new QComboBox( formatPage );
    weekStartCombo->insertItem( tr("Sunday"), 0 );
    weekStartCombo->insertItem( tr("Monday"), 1 );

    hb2->addWidget( weekStartCombo, 2 );
    int startMonday = Global::weekStartsOnMonday() ? 1 : 0;
    date->setWeekStartsMonday( startMonday );
    weekStartCombo->setCurrentItem( startMonday );

    connect( weekStartCombo, SIGNAL( activated(int)),
	     this, SLOT(weekStartChanged(int)));


    QHBoxLayout *hb3 = new QHBoxLayout( formatLayout );
    l = new QLabel( tr("Date format" ), formatPage );
    hb3->addWidget( l, 1 );
    dateFormatCombo = new QComboBox( formatPage );
    hb3->addWidget( dateFormatCombo, 2 );

    DateFormat df = TimeString::currentDateFormat();
    int currentdf = 0;

    date_formats = TimeString::formatOptions();
    dateFormatCombo->insertItem( date_formats[0].toNumberString() );
    if (df == date_formats[1])
	currentdf = 1;
    dateFormatCombo->insertItem( date_formats[1].toNumberString() );
    if (df == date_formats[2])
	currentdf = 2;
    dateFormatCombo->insertItem( date_formats[2].toNumberString() ); //ISO8601
    if (df == date_formats[3])
	currentdf = 3;
    dateFormatCombo->insertItem( date_formats[3].toNumberString() );

    dateFormatCombo->setCurrentItem( currentdf );
    date->setDateFormat( df );

    connect( dateFormatCombo, SIGNAL( activated(int)),
	     this, SLOT(formatChanged(int)));

#ifdef QTOPIA_PHONE
    timeLayout->addStretch( 0 );
    formatLayout->addStretch( 0 );
#else
    vb->addStretch( 0 );
#endif

    QObject::connect( tz, SIGNAL( signalNewTz(const QString&) ),
                      time, SLOT( slotTzChange(const QString&) ) );
    QObject::connect( tz, SIGNAL( signalNewTz(const QString&) ),
                      this, SLOT( tzChange(const QString&) ) );

    dl = new QPEDialogListener(this);
    dateChanged = FALSE;
    tzChanged = FALSE;

    //
    // Purge daily timer.  Avoids race between server triggering
    // daily alarm and the daily alarm getting removed then added
    // by the clock on receipt of timeChange().
    //
    AlarmServer::deleteAlarm(QDateTime(), "QPE/Application/clock",
	"alarm(QDateTime,int)", -1);

#ifdef QTOPIA_PHONE
    contextMenu = new ContextMenu( this );
#endif        
    connect( qApp, SIGNAL(appMessage(const QCString&, const QByteArray&)),
	    this, SLOT(appMessage(const QCString&, const QByteArray&)) );
}

void SetDateTime::setTimezoneEditable(bool tze)
{
    if (tze == tzEditable)
	return;
    if (tze) {
	tz->show();
	delete tzLabel;
	tzLabel = 0;
    } else {
	tz->hide();
        QStringList tzNameParts = QStringList::split('/', tz->currentZone());
        QString translatedTzName;
        for (QStringList::Iterator it = tzNameParts.begin(); it != tzNameParts.end(); ++it) {
            translatedTzName += qApp->translate("TimeZone", *it); //no tr
            translatedTzName += '/'; 
        }
        if (!translatedTzName.isEmpty())
            translatedTzName = translatedTzName.left(translatedTzName.length()-1);
	tzLabel = new QLabel(translatedTzName, timePage);
	tzLayout->addWidget(tzLabel);
    }
    tze = tzEditable;
}

void SetDateTime::appMessage( const QCString &msg, const QByteArray &)
{
    if( msg == "editTime()" ) {
	QPEApplication::setKeepRunning();
	time->setFocus();
    } else if( msg == "editDate()" ) {
	QPEApplication::setKeepRunning();
	date->setFocus();
    } else if( msg == "editAlarm()" ) {
	//TODO
	qDebug("SetDateTime::editAlarm() - Not yet implemented!");
    }
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

    bool ampmChange = FALSE;
    bool monSunChange = FALSE;

    {
	Config config("qpe");
	config.setGroup( "Time" );
	int show12hr = config.readBoolEntry("AMPM") ? 1 : 0;
	if ( show12hr != ampmCombo->currentItem() ) {
	    config.writeEntry( "AMPM", ampmCombo->currentItem() );
	    ampmChange = TRUE;
	}
	int startMonday =  config.readBoolEntry("MONDAY") ? 1 : 0;
	if ( startMonday != weekStartCombo->currentItem() ) {
	    config.writeEntry( "MONDAY", weekStartCombo->currentItem() );
	    monSunChange = TRUE;
	}
	config.setGroup( "Date" );
	DateFormat df = date_formats[dateFormatCombo->currentItem()];
	config.writeEntry( "Separator", QString(df.separator()));
	config.writeEntry( "ShortOrder", df.shortOrder());
	config.writeEntry( "LongOrder", df.longOrder());

	Config lconfig("locale");
	lconfig.setGroup( "Location" );
	lconfig.writeEntry( "Timezone", tz->currentZone() );
    }

    if ( time->changed() || dateChanged || tzChanged ) {
	// before we progress further, set our TZ!
	setenv( "TZ", tz->currentZone(), 1 );
#if ( defined Q_WS_QWS )
        // Change timezone of system!
	if(remove("/etc/localtime") == -1)
	  qWarning("Error removing old /etc/localtime, doesn't exist or is read-only");
	QString     filename = "/usr/share/zoneinfo/" + tz->currentZone();
        if(link(filename.local8Bit(),"/etc/localtime") == 0) 
	  qDebug("Updating /etc/localtime OK!, TZ with %s", tz->currentZone().latin1());
	else
	  qWarning("Updating /etc/localtime FAILED!, TZ with %s", tz->currentZone().latin1());
#endif
	// now set the time...
	QDateTime dt( date->date(), time->time() );

	if ( dt.isValid() ) {
	    struct timeval myTv;
	    myTv.tv_sec = TimeConversion::toUTC( dt );
	    myTv.tv_usec = 0;

	    if ( myTv.tv_sec != -1 )
		::settimeofday( &myTv, 0 );
	    Global::writeHWClock();
	    // Should leave updating alarms to datebook, rather than screw it up
	    // via duplicated functionality.
	    // DateBookDB is flawed, it should not be used anywhere.
	} else {
	    qWarning( "Invalid date/time" );
	}
	// set the timezone for everyone else...
#ifndef QT_NO_COP
	QCopEnvelope setTimeZone( "QPE/System", "timeChange(QString)" );
	setTimeZone << tz->currentZone();
#endif
    }

    // AM/PM setting and notify time changed
#ifndef QT_NO_COP
    if ( ampmChange ) {
	QCopEnvelope setClock( "QPE/System", "clockChange(bool)" );
	setClock << ampmCombo->currentItem();
    }
#endif

    // Notify everyone what day we prefer to start the week on.
#ifndef QT_NO_COP
    if ( monSunChange ) {
	QCopEnvelope setWeek( "QPE/System", "weekChange(bool)" );
	setWeek << weekStartCombo->currentItem();
    }
#endif

    // Notify everyone what date format to use
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
    // set the TZ, get the time and leave gracefully...
    QString strSave;
    strSave = getenv( "TZ" );
    setenv( "TZ", tz, 1 );

    QDate d = QDate::currentDate();
    // reset the time.
    if ( !strSave.isNull() ) {
	setenv( "TZ", strSave, 1 );
    }
    date->setDate( d );
    tzChanged = TRUE;
}

void SetDateTime::formatChanged(int i)
{
    date->setDateFormat(date_formats[i]);
}

void SetDateTime::weekStartChanged(int s)
{
    date->setWeekStartsMonday(s==1);
}

void SetDateTime::dateChange( const QDate & )
{
    dateChanged = TRUE;
}

//===========================================================================

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

    QLabel *l = new QLabel( tr("Time"), this );
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

    l = new QLabel( tr(":"), this );
    //l->setAlignment( AlignRight | AlignVCenter );
    hb2->addWidget( l );

    sbMin = new QSpinBox( this );
    sbMin->setMinValue( 0 );
    sbMin->setMaxValue( 59 );
    sbMin->setWrapping(TRUE);
    sbMin->setValue( minute );
    minuteChanged(minute);
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

    userChanged = FALSE;
}

QTime SetTime::time() const
{
    return QTime( hour, minute, 0 );
}

void SetTime::focusInEvent( QFocusEvent *e )
{
    QWidget::focusInEvent( e );
    sbHour->setFocus();
#ifdef QTOPIA_PHONE
    if( !Global::mousePreferred() ) {
	if( sbHour->isModalEditing() )
	    sbHour->setModalEditing( TRUE );
    }
#endif
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

    userChanged = TRUE;
}

void SetTime::minuteChanged( int value )
{
    sbMin->setPrefix( value <= 9 ? "0" : "" );
    minute = value;
    userChanged = TRUE;
}

void SetTime::show12hourTime( int on )
{
    bool uc = userChanged;
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
    userChanged = uc;
}

void SetTime::checkedPM( int c )
{
    int show_hour = sbHour->value();
    if (show_hour == 12)
	show_hour = 0;

    if ( c == ValuePM )
	show_hour += 12;

    hour = show_hour;
    userChanged = TRUE;
}

void SetTime::slotTzChange( const QString &tz )
{
    // set the TZ, get the time and leave gracefully...
    QString strSave;
    strSave = getenv( "TZ" );
    setenv( "TZ", tz, 1 );

    QTime t = QTime::currentTime();
    // reset the time.
    if ( !strSave.isNull() ) {
	setenv( "TZ", strSave, 1 );
    }

    // just set the spinboxes and let it propagate through
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
    userChanged = TRUE;
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
