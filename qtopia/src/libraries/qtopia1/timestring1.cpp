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

#include <qtopia/timestring.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>

#include <time.h>

// TRICKY: this is an sufficient extract from the Qtopia 1.5.0 TimeString code

class TimeStringFormat1 : public QObject
{
    Q_OBJECT
public:
    static DateFormat currentFormat()
    {
	if ( !self )
	    self  = new TimeStringFormat1;
	return self->format;
    }
    static TimeStringFormat1* theInstance();
    void emitFormatChanged( DateFormat f );

private slots:
    void formatChanged( DateFormat f )
    {
	format = f;
    }
private:
    static TimeStringFormat1 *self;
    DateFormat format;

    TimeStringFormat1()
	: QObject( qApp )
    {
	Config config("qpe");
	config.setGroup( "Date" );
	format = ::DateFormat(QChar(config.readEntry("Separator", "/")[0]),
		(::DateFormat::Order)config .readNumEntry("ShortOrder", ::DateFormat::DayMonthYear),
		(::DateFormat::Order)config.readNumEntry("LongOrder", ::DateFormat::DayMonthYear));

	connect( qApp, SIGNAL( dateFormatChanged(DateFormat) ),
		 this, SLOT( formatChanged(DateFormat) ) );
    }
};


TimeStringFormat1* TimeStringFormat1::theInstance()
{
    return self;
}
void TimeStringFormat1::emitFormatChanged( DateFormat f )
{
    emit formatChanged(f);
}

TimeStringFormat1 *TimeStringFormat1::self = 0;
///////////////////////////

class LocalTimeFormat : public QObject
{
    Q_OBJECT
public:
    static LocalTimeFormat *self();

    static bool currentAMPM()
    {
	return self()->ampm;
    }

signals:
    void changed();

private slots:
    void formatChanged( DateFormat f )
    {
	// We cannot rely on signal order, so force other change now.
	if ( !(f == TimeStringFormat1::currentFormat()) )
	    TimeStringFormat1::theInstance()->emitFormatChanged(f);

	emit changed();
    }

    void clockChanged( bool ap )
    {
	ampm = ap;
	emit changed();
    }

private:
    bool ampm;

    LocalTimeFormat()
	: QObject( qApp )
    {
	Config config("qpe");
	config.setGroup( "Time" );
	ampm = config.readBoolEntry("AMPM",FALSE);

	connect( qApp, SIGNAL( dateFormatChanged(DateFormat) ),
		 this, SLOT( formatChanged(DateFormat) ) );
	connect( qApp, SIGNAL(clockChanged(bool)),
		 this, SLOT(clockChanged(bool)) );
    }
};

LocalTimeFormat *LocalTimeFormat::self()
{
    static LocalTimeFormat *inst=new LocalTimeFormat;
    return inst;
}

/*!
  Call \a member() in \a obj when the time and/or date format changes.

  You should connect to whatever methods retrieve TimeString values
  if you want those displays to be always up-to-date.

  First availability: Qtopia 1.6
*/
void TimeString::connectChange(QObject* obj,const char* member)
{
    QObject::connect(LocalTimeFormat::self(),SIGNAL(changed()),obj,member);
}

/*!
  Disconnects the \a member and \a obj set in connect.

  This is rarely need since it will be automatically disconnected
  if \a obj is deleted.

  First availability: Qtopia 1.6
*/
void TimeString::disconnectChange(QObject* obj,const char* member)
{
    QObject::disconnect(LocalTimeFormat::self(),SIGNAL(changed()),obj,member);
}


/*!
  Returns \a hour as a string, in either 12 hour (if \a ampm is TRUE) or
  24 hour (if \a ampm is FALSE) format. 

  If  \a hour is greater than 23 or less then 0 then hour will default to 0 

  First availability: Qtopia 1.6
*/
QString TimeString::hourString( int hour, bool ampm )
{
    QString s;
    if ( (hour < 0) || (hour > 23))
	hour = 0;
	
    if ( ampm ) {
	if ( hour == 0 ) {
	    s = LocalTimeFormat::tr("%1am").arg(12);
	} else if ( hour == 12 ) {
	    s = LocalTimeFormat::tr("%1pm").arg(hour);
	} else if ( hour > 12 ) {
	    s = LocalTimeFormat::tr("%1pm").arg(hour-12);
	} else {
	    s = LocalTimeFormat::tr("%1am").arg(hour);
	}
    } else {
	if ( hour < 10 )
	    s = LocalTimeFormat::tr("0%1","0..9 hour of day").arg(hour);
	else
	    s = LocalTimeFormat::tr("%1","10..24 hour of day").arg(hour);
    }
    return s;
}

struct TimeFormatDef {
    const char* format;
    bool alt;
};

/*!
  Returns \a hour as a string.

  The format, including order depends on the user's settings.

  First availability: Qtopia 1.6
*/
QString TimeString::localH( int hour )
{
    return hourString( hour, currentAMPM() );
}

/*!
  Returns time \a t as a string,
  showing hours and minutes.

  The format, including order depends on the user's settings.

  First availability: Qtopia 1.6
*/
QString TimeString::localHM( const QTime &t )
{ return timeString( t, currentAMPM(), FALSE ); }

/*!
  Returns time \a t as a string,
  showing hours, minutes, and seconds.

  The format, including order depends on the user's settings.

  First availability: Qtopia 1.6
*/
QString TimeString::localHMS( const QTime &t )
{ return timeString( t, currentAMPM(), TRUE ); }

static QString shortDayName( int weekday )
{
    switch ( weekday ) {
	case 1: return qApp->translate("QDate", "M", "Single character representing Monday");
	case 2: return qApp->translate("QDate", "T", "Single character representing Tuesday");
	case 3: return qApp->translate("QDate", "W", "Single character representing Wednesday");
	case 4: return qApp->translate("QDate", "T", "Single character representing Thursday");
	case 5: return qApp->translate("QDate", "F", "Single character representing Friday");
	case 6: return qApp->translate("QDate", "S", "Single character representing Saturday");
	case 7: return qApp->translate("QDate", "S", "Single character representing Sunday");
	default: return QString();
    }
}

static const char* unTranslatedFullDayNames[] = {
    QT_TRANSLATE_NOOP( "QDate", "Monday" ),
    QT_TRANSLATE_NOOP( "QDate", "Tuesday" ),
    QT_TRANSLATE_NOOP( "QDate", "Wednesday" ),
    QT_TRANSLATE_NOOP( "QDate", "Thursday" ),
    QT_TRANSLATE_NOOP( "QDate", "Friday" ),
    QT_TRANSLATE_NOOP( "QDate", "Saturday" ),
    QT_TRANSLATE_NOOP( "QDate", "Sunday" )
};


#ifdef QTOPIA_DESKTOP
//translations in qt.qm
static const char* unTranslatedMediumDayNames[] = {
    "Mon" , "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};
#endif

static QString dayname(int weekday, bool lng)
{
    if ( lng  && qApp )
        return qApp->translate("QDate", unTranslatedFullDayNames[weekday-1]);
    else { 
#ifdef QTOPIA_DESKTOP
        if (qApp)
            return qApp->translate( "QDate", unTranslatedMediumDayNames[weekday-1]);
#endif   
        return QDate().dayName(weekday);
    }
}


/*!
  Returns date/time \a t as a string,
  showing hours, minutes, and day of the week.

  The format, including order depends on the user's settings.

  First availability: Qtopia 1.6
*/
QString TimeString::localHMDayOfWeek( const QDateTime &t )
{
    // just create a shorter time String
    QString strTime = TimeStringFormat1::tr( "%1 %2", "1=Monday 2=12:45" )
	.arg(::dayname(t.date().dayOfWeek(), FALSE))
	.arg(timeString( t.time(), currentAMPM(), FALSE ));
    return strTime;
}

/*!
  Returns date/time \a t as a string,
  showing hours, minutes, seconds, and day of the week.

  The format, including order depends on the user's settings.

  First availability: Qtopia 1.6
*/
QString TimeString::localHMSDayOfWeek( const QDateTime &t )
{
    // just create a shorter time String
    QString strTime = TimeStringFormat1::tr( "%1 %2", "1=Monday 2=12:45" )
	.arg(::dayname(t.date().dayOfWeek(), FALSE))
	.arg(timeString( t.time(), currentAMPM(), TRUE ));
    return strTime;
}

/*!
  Returns date/time \a dt as a string,
  showing year, month, date, hours, minutes, and seconds.
  \a len determines the length of the resulting string.

  The format, including order depends on the user's settings.

  First availability: Qtopia 1.6
*/
QString TimeString::localYMDHMS( const QDateTime &dt, Length len )
{
    const QDate& d = dt.date();
    const QTime& t = dt.time();
    return LocalTimeFormat::tr("%1 %2","date,time").arg(localYMD(d,len)).arg(localHMS(t));
}

/*!
  Returns date \a dt as a string,
  showing month and date.
  \a len determines the length of the resulting string.

  The format, including order depends on the user's settings.

  First availability: Qtopia 1.6
*/
QString TimeString::localMD( const QDate &dt, Length len )
{
    DateFormat dfo = currentDateFormat();
    // drop years
    DateFormat df(dfo.separator(),
	DateFormat::Order(dfo.shortOrder()&0333),
	DateFormat::Order(dfo.longOrder()&0333));
    int v=0;
    if ( len==Long )
	v |= DateFormat::longNumber | DateFormat::longWord;
    return df.wordDate(dt,v);
}

/*!
  Returns date \a dt as a string,
  showing year, month, and date.
  \a len determines the length of the resulting string.

  The format, including order depends on the user's settings.

  First availability: Qtopia 1.6
*/
QString TimeString::localYMD( const QDate &dt, Length len )
{
    if ( len == Short ) return shortDate(dt);
    if ( len == Medium ) return dateString(dt);
    return longDateString(dt);
}

/*!
  Returns the day of week at date \a d.
  \a len determines the length of the resulting string.

  First availability: Qtopia 1.6
*/
QString TimeString::localDayOfWeek( const QDate& d, Length len )
{
    DateFormat dfo = currentDateFormat();
    DateFormat df(dfo.separator(),DateFormat::Order(0),DateFormat::Order(0));
    int v=DateFormat::showWeekDay;
    if ( len==Long )
	v |= DateFormat::longWord;
    if ( len == Short )
	return ::shortDayName( d.dayOfWeek() );
    else
	return df.wordDate(d,v);
}

/*!
  Returns the day of week \a day1to7, where
  1 is Monday.
  \a len determines the length of the resulting string.

  The format, including order depends on the user's settings.

  First availability: Qtopia 1.6
*/
QString TimeString::localDayOfWeek( int day1to7, Length len )
{
    if ( len == Short )
        return ::shortDayName( day1to7 );
    else
	return dayname(day1to7,len==Long);
}

/*!
  Returns the user's current preference for 12 hour time
  over 24 hour time.

  First availability: Qtopia 1.6
*/
bool TimeString::currentAMPM()
{ return LocalTimeFormat::currentAMPM(); }

/*!
  First availability: Qtopia 1.6
*/
QArray<DateFormat> TimeString::formatOptions()
{
    QArray<DateFormat> options(4);
    options[0] = DateFormat('/', DateFormat::MonthDayYear);
    options[1] = DateFormat('.', DateFormat::DayMonthYear);
    options[2] = DateFormat('-', DateFormat::YearMonthDay);
    options[3] = DateFormat('/', DateFormat::DayMonthYear);
    return options;
}

#include "timestring1.moc"
