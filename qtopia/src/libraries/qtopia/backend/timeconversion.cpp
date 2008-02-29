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

#include <qglobal.h>
#include <qtopia/timeconversion.h>
#include <qregexp.h>
#include <stdlib.h>

/*!
  \obsolete
  \sa TimeString
*/
QString TimeConversion::toString( const QDate &d )
{
    if ( !d.isValid() )
	return QString::null;
    QString r = QString::number( d.day() ) + "." +
		QString::number( d.month() ) + "." +
		QString::number( d.year() );
    return r;
}

/*!
  \obsolete
  \sa TimeString
*/
QDate TimeConversion::fromString( const QString &datestr )
{
    int monthPos = datestr.find('.');
    int yearPos = datestr.find('.', monthPos+1 );
    if ( monthPos == -1 || yearPos == -1 )
	return QDate();
    int d = datestr.left( monthPos ).toInt();
    int m = datestr.mid( monthPos+1, yearPos - monthPos - 1 ).toInt();
    int y = datestr.mid( yearPos+1 ).toInt();
    return QDate( y,m,d );
}

/*!
  Converts local-time \a dt to a Unix time_t.
*/
time_t TimeConversion::toUTC( const QDateTime& dt )
{
    time_t tmp;
    struct tm *lt;

#if defined(Q_WS_WIN32) || defined (Q_OS_WIN64)
    _tzset();
#else
    tzset();
#endif

    // get a tm structure from the system to get the correct tz_name
    tmp = time( 0 );
    lt = localtime( &tmp );

    lt->tm_sec = dt.time().second();
    lt->tm_min = dt.time().minute();
    lt->tm_hour = dt.time().hour();
    lt->tm_mday = dt.date().day();
    lt->tm_mon = dt.date().month() - 1; // 0-11 instead of 1-12
    lt->tm_year = dt.date().year() - 1900; // year - 1900
    //lt->tm_wday = dt.date().dayOfWeek(); ignored anyway
    //lt->tm_yday = dt.date().dayOfYear(); ignored anyway
    lt->tm_wday = -1;
    lt->tm_yday = -1;
    // tm_isdst negative -> mktime will find out about DST
    lt->tm_isdst = -1;
    // keep tm_zone and tm_gmtoff
    tmp = mktime( lt );
    return tmp;
}

/*!
  Converts Unix time_t \a time to a local-time QDateTime.
*/
QDateTime TimeConversion::fromUTC( time_t time )
{
    struct tm *lt;

#if defined(Q_WS_WIN32) || defined (Q_OS_WIN64)
    _tzset();
#else
    tzset();
#endif
    lt = localtime( &time );
    QDateTime dt;
    dt.setDate( QDate( lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday ) );
    dt.setTime( QTime( lt->tm_hour, lt->tm_min, lt->tm_sec ) );
    return dt;
}


/*!
  Returns the number of real seconds between \a from and \a to,
  accounting for any summertime/wintertime changes.
*/
int TimeConversion::secsTo( const QDateTime &from, const QDateTime &to )
{
    return toUTC( to ) - toUTC( from );
}

/*!
  Converts local-time \a d, at time 0:00, to an ISO8601 string, specified
  with a UTC timezone. You may prefer to use toISO8601(d,FALSE).
*/
QCString TimeConversion::toISO8601( const QDate &d )
{
    time_t tmp = toUTC( d );
    struct tm *utc = gmtime( &tmp );

    QCString str;
    str.sprintf("%04d%02d%02d", (utc->tm_year + 1900), utc->tm_mon+1, utc->tm_mday );
    return str;
}

/*!
  Converts local-time \a dt to an ISO8601 string, specified
  with a UTC timezone.
*/
QCString TimeConversion::toISO8601( const QDateTime &dt )
{
    time_t tmp = toUTC( dt );
    struct tm *utc = gmtime( &tmp );

    QCString str;
    str.sprintf("%04d%02d%02dT%02d%02d%02dZ",
		(utc->tm_year + 1900), utc->tm_mon+1, utc->tm_mday,
		utc->tm_hour, utc->tm_min, utc->tm_sec );
    return str;
}

/*!
  Converts the ISO8601 time string \a s to a local-time QDateTime.
*/
QDateTime TimeConversion::fromISO8601( const QCString &s )
{

#if defined(Q_WS_WIN32) || defined (Q_OS_WIN64)
    _tzset();
#else
    tzset();
#endif

    struct tm *thetime = new tm;

    QCString str = s.copy();
    str.replace(QRegExp("-"), "" );
    str.replace(QRegExp(":"), "" );
    str.stripWhiteSpace();
    str = str.lower();

    int i = str.find( "t" );
    QCString date;
    QCString timestr;
    if ( i != -1 ) {
	date = str.left( i );
	timestr = str.mid( i+1 );
    } else {
	date = str;
    }

//     qDebug("--- parsing ISO time---");
    thetime->tm_year = 100;
    thetime->tm_mon = 0;
    thetime->tm_mday = 0;
    thetime->tm_hour = 0;
    thetime->tm_min = 0;
    thetime->tm_sec = 0;

//     qDebug("date = %s", date.data() );

    switch( date.length() ) {
	case 8:
	    thetime->tm_mday = date.right( 2 ).toInt();
	case 6:
	    thetime->tm_mon = date.mid( 4, 2 ).toInt() - 1;
	case 4:
	    thetime->tm_year = date.left( 4 ).toInt();
	    thetime->tm_year -= 1900;
	    break;
	default:
	    break;
    }

    int tzoff = 0;
    bool inLocalTime = FALSE;
    if ( timestr.find( 'z', 0, TRUE ) == (int)timestr.length() - 1 && !timestr.isEmpty())
	// UTC
	timestr = timestr.left( timestr.length() -1 );
    else {
	int plus = timestr.find( "+" );
	int minus = timestr.find( "-" );
	if ( plus != -1 || minus != -1 && !timestr.isEmpty()) {
	    // have a timezone offset
	    plus = (plus != -1) ? plus : minus;
	    QCString off = timestr.mid( plus );
	    timestr = timestr.left( plus );

	    int tzoffhour = 0;
	    int tzoffmin = 0;
	    switch( off.length() ) {
		case 5:
		    tzoffmin = off.mid(3).toInt();
		case 3:
		    tzoffhour = off.left(3).toInt();
		default:
		    break;
	    }
	    tzoff = 3600*tzoffhour + 60*tzoffmin;
	} else
	    inLocalTime = TRUE;
    }

    // get the time:
    switch( timestr.length() ) {
	case 6:
	    thetime->tm_sec = timestr.mid( 4 ).toInt();
	case 4:
	    thetime->tm_min = timestr.mid( 2, 2 ).toInt();
	case 2:
	    thetime->tm_hour = timestr.left( 2 ).toInt();
	default:
	    break;
    }

    int tzloc = 0;
    time_t tmp = time( 0 );
    if ( !inLocalTime ) {
	// have to get the offset between gmt and local time
	struct tm *lt = localtime( &tmp );
	tzloc = mktime( lt );
	struct tm *ut = gmtime( &tmp );
	tzloc -= mktime( ut );
    }
//     qDebug("time: %d %d %d, tzloc=%d, tzoff=%d", thetime->tm_hour, thetime->tm_min, thetime->tm_sec,
//  	   tzloc, tzoff );

    tmp = mktime( thetime );
    tmp += tzloc + tzoff;

    delete thetime;

    return fromUTC( tmp );
}

