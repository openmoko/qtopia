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

#include "timestring.h"
#include <qobject.h>
#include <qdatetime.h>
#include <qapplication.h> 
#include "config.h"

#include <time.h>


class TimeStringFormat : public QObject
{
    Q_OBJECT
public:
    static DateFormat currentFormat()
    {
	if ( !self )
	    self  = new TimeStringFormat;
	return self->format;
    }
private slots:
    void formatChanged( DateFormat f )
    {
	format = f;
    }
private:
    static TimeStringFormat *self;
    DateFormat format;

    TimeStringFormat()
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

TimeStringFormat *TimeStringFormat::self = 0;

QString DateFormat::toNumberString() const
{
    QString buf = "";
    // for each part of the order
    for (int i = 0; i < 3; i++) {
	// switch on the relavent 3 bits.
	switch((_shortOrder >> (i * 3)) & 0x0007) { 
	    case 0x0001:
		buf += TimeStringFormat::tr( "D", "day in month" );
		break;
	    case 0x0002:
		buf += TimeStringFormat::tr( "M" );
		break;
	    case 0x0004:
		buf += TimeStringFormat::tr( "Y" );
		break;
	}
	if (i < 2)
	    buf += _shortSeparator;
    }
    return buf;
}

QString DateFormat::toWordString() const
{
    QString buf = "";
    // for each part of the order
    for (int i = 0; i < 3; i++) {
	// switch on the relavent 3 bits.
	switch((_longOrder >> (i * 3)) & 0x0007) { 
	    case 0x0001:
		buf += TimeStringFormat::tr( "day", "in month" );
		if (i < 2) { 
		    if ((_shortOrder << ((i+1) * 3)) & 0x0007)
			buf += ", ";
		    else 
			buf += " ";
		}
		break;
	    case 0x0002:
		buf += TimeStringFormat::tr( "month" );
		if (i < 2) 
		    buf += " ";
		break;
	    case 0x0004:
		buf += TimeStringFormat::tr( "year" );
		if (i < 2) 
		    buf += ", ";
		break;
	}
    }
    return buf;
}

QString DateFormat::numberDate(const QDate &d, int v) const
{
    QString buf = "";

    int pad = 2;

    // for each part of the order
    for (int i = 0; i < 3; i++) {
	// switch on the relavent 3 bits.
	switch((_shortOrder >> (i * 3)) & 0x0007) { 
	    case 0x0001:
	      if (pad==2) buf += QString().sprintf("%02d",d.day());
	      else buf += QString().sprintf("%d",d.day());
		break;
	    case 0x0002:
	      if (i==0) { // no padding with only MM/DD/YY format
		pad=0;
	      }
	      if (pad==2) buf += QString().sprintf("%02d",d.month());
	      else buf += QString().sprintf("%d",d.month());
		break;
	    case 0x0004:
		{
		    int year = d.year();
		    if (!(v & longNumber))
			year = year % 100;
		    buf += QString().sprintf("%02d",year);
		}
		break;
	}
	if (i < 2)
	    buf += _shortSeparator;
    }
    return buf;
}

static QString dayname(const QDate& d, bool lng)
{
    char buffer[255];
    struct tm tt;
    int weekday = d.dayOfWeek();
    memset( &tt, 0, sizeof( tm ) );
    tt.tm_wday = ( weekday == 7 ) ? 0 : weekday;
    if ( strftime( buffer, sizeof( buffer ), lng ? "%A" : "%a", &tt ) )
	return QString::fromLocal8Bit( buffer );
    else
	return d.dayName(d.dayOfWeek());
}

QString DateFormat::wordDate(const QDate &d, int v) const
{
    // for each part of the order
    QString weekDay;
    if (v & showWeekDay)
	weekDay = ::dayname(d,(v & longWord));

    QString date="";
    QString sep="";
    for (int i = 0; i < 3; i++) {
	// switch on the relavent 3 bits.
	int field = (_longOrder >> (i * 3)) & 0x0007;
        if ( field && !date.isEmpty() )
	    date += sep;
	switch (field) {
	    case 0x0001: // Day
	        if (i==1) {
		    date += QString().sprintf("%02d",d.day());
		    sep = TimeStringFormat::tr(", ","day-date separator");
	        } else {
		    date += QString().sprintf("%2d",d.day());
		    if (separator()=='.') // 2002/1/11 
		        sep = ".";
		    else 
		        sep = "";
	            sep += TimeStringFormat::tr(" ","day-date separator");
		}
		break;
	    case 0x0002: // Month
		{
		    QString monthName;
		    char buffer[255];
		    tm tt;
		    memset( &tt, 0, sizeof( tm ) );
		    tt.tm_mon = d.month() - 1;
		    if ( strftime( buffer, sizeof( buffer ), (v & longWord) ? "%B" : "%b", &tt ) )
			monthName = QString::fromLocal8Bit( buffer );
		    else
			monthName = d.monthName(d.month());
		    date += monthName;
		}
		sep = TimeStringFormat::tr(" ","month-date separator");
		break;
	    case 0x0004: // Year
		{
		    int year = d.year();
		    if (!(v & longNumber))
			year = year % 100;

		    if (year < 10)
			date += "0";

		    date += QString::number(year);
		}
		sep = TimeStringFormat::tr(", ","year-date seperator");
		break;
	}
    }

    QString r = "";
    if ( weekDay.isEmpty() )
	r = date;
    else if ((_longOrder & 0x0007) == 0x0002)
	r = TimeStringFormat::tr("%1 %2","1=Monday 2=January 12").arg(weekDay).arg(date);
    else if ( _longOrder )
	r = TimeStringFormat::tr("%1, %2","1=Monday 2=12 January").arg(weekDay).arg(date);
    else
	r = weekDay;
    return r;
}

#ifndef QT_NO_DATASTREAM
void DateFormat::save(QDataStream &d) const
{
    d << _shortSeparator.unicode();
    uint v= _shortOrder;
    d << v;
    v = _longOrder;
    d << v;
}

void DateFormat::load(QDataStream &d) 
{
    ushort value;
    d >> value;
    _shortSeparator = QChar(value);
    uint v = 0;
    d >> v;
    _shortOrder = (Order)v;
    v = 0;
    d >> v;
    _longOrder = (Order)v;
}

QDataStream &operator<<(QDataStream &s, const DateFormat&df)
{
        df.save(s);
	    return s;
}
QDataStream &operator>>(QDataStream &s, DateFormat&df)
{
        df.load(s);
	    return s;
}
#endif

QString TimeString::shortDate( const QDate &d, DateFormat dtf )
{
    return dtf.wordDate(d);
}

QString TimeString::dateString( const QDate &d, DateFormat dtf )
{
    return dtf.wordDate(d, DateFormat::longNumber);
}


QString TimeString::longDateString( const QDate &d, DateFormat dtf )
{
    return dtf.wordDate(d, DateFormat::showWeekDay | DateFormat::longNumber 
	    | DateFormat::longWord);
}

DateFormat TimeString::currentDateFormat()
{
    return TimeStringFormat::currentFormat();
}


QString TimeString::dateString( const QDateTime &dt, bool ampm, bool seconds, DateFormat dtf )
{
    const QDate& d = dt.date();
    const QTime& t = dt.time();

    // based on QDateTime::toString()
    QString buf = timeString(t,ampm,seconds);
    buf += " ";
    buf += longDateString( d, dtf );

    return buf;
}

QString TimeString::timeString( const QTime &t, bool ampm, bool seconds )
{
    if ( !ampm ) {
	if ( seconds )
	    return t.toString();
	QString r = QString::number(t.hour());
	if ( t.hour() < 10 ) r.prepend( "0" );
	r.append( ":" );
	if ( t.minute() < 10 ) r.append( "0" );
	r.append(QString::number(t.minute()));
	return r;
    }
    // ### else the hard case that should disappear in Qt 3.0
    QString argString = seconds ? "%4:%5:%6 %7" : "%4:%5 %7";
    int hour = t.hour();
    QString strMin = QString::number( t.minute() );
    QString strSec = QString::number( t.second() );
    if ( hour > 12 )
	argString = argString.arg( hour - 12, 2 );
    else {
        if ( hour == 0 )
	    argString = argString.arg( 12 );
        else
	    argString = argString.arg( hour, 2 );
    }
    if ( t.minute() < 10 )
	strMin.prepend( "0" );
    if ( t.second() < 10 )
	strSec.prepend( "0" );
    argString = argString.arg( strMin );
    if ( seconds )
	argString = argString.arg( strSec );
    if ( hour >= 12 )
	argString = argString.arg( TimeStringFormat::tr("PM") );
    else
	argString = argString.arg( TimeStringFormat::tr("AM") );
    return argString;
}

QString TimeString::shortTime( bool ampm, bool seconds )
{
    // just create a shorter time String
    QDateTime dtTmp = QDateTime::currentDateTime();
    QString strTime = TimeStringFormat::tr( "%1 %2", "1=Monday 2=12:45" )
	.arg(::dayname(dtTmp.date(),FALSE))
	.arg(timeString( dtTmp.time(), ampm, seconds ));
    return strTime;
}

QString TimeString::dateString( const QDateTime &t, bool ampm )
{
    return dateString(t,ampm,FALSE);
}

QString TimeString::timeString( const QTime &t, bool ampm)
{
    return timeString(t,ampm,FALSE);
}

QString TimeString::shortTime( bool ampm )
{
    return shortTime(ampm,FALSE);
}

QString TimeString::numberDateString( const QDate &d, DateFormat dtf )
{
  return dtf.numberDate(d);
}
QString TimeString::longNumberDateString( const QDate &d, DateFormat dtf )
{
  return dtf.numberDate(d,DateFormat::longNumber);
}

#include "timestring.moc"
