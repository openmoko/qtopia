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
		 this, SLOT( formatChanged( DateFormat ) ) );
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
		 this, SLOT( formatChanged( DateFormat ) ) );
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
*/
void TimeString::connectChange(QObject* obj,const char* member)
{
    QObject::connect(LocalTimeFormat::self(),SIGNAL(changed()),obj,member);
}

/*!
  Disconnects the \a member and \a obj set in connect.

  This is rarely need since it will be automatically disconnected
  if \a obj is deleted.
*/
void TimeString::disconnectChange(QObject* obj,const char* member)
{
    QObject::disconnect(LocalTimeFormat::self(),SIGNAL(changed()),obj,member);
}


QString TimeString::hourString( int hour, bool ampm )
{
    QString s;
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

QString TimeString::localH( int hour )
{
    return hourString( hour, currentAMPM() );
}

QString TimeString::localHM( const QTime &t )
{ return timeString( t, currentAMPM(), FALSE ); }

QString TimeString::localHMS( const QTime &t )
{ return timeString( t, currentAMPM(), TRUE ); }

static QString dayname(int weekday, bool lng)
{
    char buffer[255];
    struct tm tt;
    memset( &tt, 0, sizeof( tm ) );
    tt.tm_wday = ( weekday == 7 ) ? 0 : weekday;
    if ( strftime( buffer, sizeof( buffer ), lng ? "%A" : "%a", &tt ) )
	return QString::fromLocal8Bit( buffer );
    else
	return QDate().dayName(weekday);
}


QString TimeString::localHMDayOfWeek( const QDateTime &t )
{
    // just create a shorter time String
    QString strTime = TimeStringFormat1::tr( "%1 %2", "1=Monday 2=12:45" )
	.arg(::dayname(t.date().dayOfWeek(), FALSE))
	.arg(timeString( t.time(), currentAMPM(), FALSE ));
    return strTime;
}

QString TimeString::localHMSDayOfWeek( const QDateTime &t )
{
    // just create a shorter time String
    QString strTime = TimeStringFormat1::tr( "%1 %2", "1=Monday 2=12:45" )
	.arg(::dayname(t.date().dayOfWeek(), FALSE))
	.arg(timeString( t.time(), currentAMPM(), TRUE ));
    return strTime;
}

QString TimeString::localYMDHMS( const QDateTime &t, Length )
{
    return dateString( t, currentAMPM(), TRUE, currentDateFormat() );
}

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

QString TimeString::localYMD( const QDate &dt, Length len )
{
    if ( len == Short ) return shortDate(dt);
    if ( len == Medium ) return dateString(dt);
    return longDateString(dt);
}

QString TimeString::localDayOfWeek( const QDate& dt, Length len )
{
    DateFormat dfo = currentDateFormat();
    DateFormat df(dfo.separator(),DateFormat::Order(0),DateFormat::Order(0));
    int v=DateFormat::showWeekDay;
    if ( len==Long )
	v |= DateFormat::longWord;
    return df.wordDate(dt,v);
}

QString TimeString::localDayOfWeek( int day1to7, Length len )
{
    return dayname(day1to7,len==Long);
}

bool TimeString::currentAMPM()
{ return LocalTimeFormat::currentAMPM(); }

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
