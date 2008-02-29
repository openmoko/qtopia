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
#include <qtopia/calendar.h>
#include <qapplication.h>

/*!
  Returns a list of 42 dates (6 weeks) that cover the given \a month and
  \a year.

  The list starts on the Monday on or before the 1st of the month if 
  \a startWithMonday is TRUE, otherwise the list starts on the Sunday on 
  or before the 1st of the month.

  First availability: Qtopia 1.6
*/
QValueList<QDate> Calendar::datesOfMonth( int year, int month,
						 bool startWithMonday )
{
    QDate thisMonth(year, month, 1);
    QDate currentDate = addMonths(-1, thisMonth);
    int firstDay = thisMonth.dayOfWeek();

    QValueList<QDate> days;    

    if ( startWithMonday )
	firstDay--;
    if (firstDay == 7)
	firstDay = 0;
    // firstDay is now index of first day of speced month.
    
    currentDate.setYMD(currentDate.year(), currentDate.month(),
	    currentDate.daysInMonth() - firstDay);
    // last day if not to be shown, day before etc if one to be shown...

    for (int i = 0; i < 6 * 7; i++) {
	currentDate = currentDate.addDays(1);
	days.append(currentDate);
    }

    return days;
}

/*!
    Using the calender page given by datesOfMonth() will return the 
    date at the position \a row, \a col, for the given \a month and \a year.

  The page starts on the Monday on or before the 1st of the month if 
  \a startWithMonday is TRUE, otherwise the page starts on the Sunday on 
  or before the 1st of the month.

  First availability: Qtopia 1.6
*/
QDate Calendar::dateAtCoord( int year, int month, 
	int row, int col, bool startWithMonday)
{
    int pos = row*7 + col;
    return dateAtIndex(year, month, pos, startWithMonday);
}

/*!
    Using the calender page given by datesOfMonth() will return the 
    date at the position \a index for the given \a month and \a year.

  The page starts on the Monday on or before the 1st of the month if 
  \a startWithMonday is TRUE, otherwise the page starts on the Sunday on 
  or before the 1st of the month.

  First availability: Qtopia 1.6
*/
QDate Calendar::dateAtIndex( int year, int month, int index,
	bool startWithMonday)
{
    QDate thisMonth(year, month, 1);
    QDate currentDate = addMonths(-1, thisMonth);
    int firstDay = thisMonth.dayOfWeek();

    if ( startWithMonday )
	firstDay--;
    if (firstDay == 7)
	firstDay = 0;
    // firstDay is now index of first day of speced month.

    currentDate.setYMD(currentDate.year(), currentDate.month(),
	    currentDate.daysInMonth() - firstDay);
    // last day if not to be shown, day before etc if one to be shown...


    return currentDate.addDays(index + 1);
}

/*!
  Returns the index for \a date on the calendar page given by datesOfMonth()
  for the given \a year and \a month.

  The page starts on the Monday on or before the 1st of the month if 
  \a startWithMonday is TRUE, otherwise the page starts on the Sunday on 
  or before the 1st of the month.

  First availability: Qtopia 1.6
*/
int Calendar::indexForDate( int year, int month, const QDate &date, 
	bool startWithMonday)
{    
    QDate thisMonth(year, month, 1);
    QDate currentDate = addMonths(-1, thisMonth);
    int firstDay = thisMonth.dayOfWeek();

    if ( startWithMonday )
	firstDay--;
    if (firstDay == 7)
	firstDay = 0;
    // firstDay is now index of first day of speced month.

    currentDate.setYMD(currentDate.year(), currentDate.month(),
	    currentDate.daysInMonth() - firstDay);
    // last day if not to be shown, day before etc if one to be shown...

    int index = currentDate.daysTo(date) - 1;
    if (index > 41) return -1;
    if (index < 0) return -1;
    return index;
}

/*!
  Sets the \a row and \a col to the position for
  \a date on the calendar page given by datesOfMonth()
  for the given \a year and \a month.

  The page starts on the Monday on or before the 1st of the month if 
  \a startWithMonday is TRUE, otherwise the page starts on the Sunday on 
  or before the 1st of the month.

  First availability: Qtopia 1.6
*/
void Calendar::coordForDate( int year, int month,
	const QDate &date, int &row, int &col,
	bool startWithMonday)
{
    int offset = indexForDate(year, month, date, startWithMonday);

    if (offset < 0) {
	row = col = -1;
    } else {
	row = offset / 7;
	col = offset % 7;
    }
}

/*!
  Returns the week number the date \a dt is in the month for that date.
  For instance the first Wednesday of the month is in the first week the second
  Wednesday in the month is in the second week and so on.

  First availability: Qtopia 1.6
*/
int Calendar::weekInMonth(const QDate &dt)
{
    if (dt.day() == 1)
	return 1;
    return ((dt.day() - 1) / 7) + 1;
}

/*!
  Returns a date \a nmonths later than the date \a orig (or earlier if \a nmonths is negative).

  Rounds to the last day of month if the day of the calculated date would be
  invalid.

  First availability: Qtopia 1.6
*/
QDate Calendar::addMonths(int nmonths, const QDate &orig)
{
    int y, m, d;
    y = orig.year();
    m = orig.month();
    d = orig.day();

    while ( nmonths != 0 ) {
	if ( nmonths < 0 && nmonths + 12 <= 0 ) {
	    y--;
	    nmonths+=12;
	} else if ( nmonths < 0 ) {
	    m+= nmonths;
	    nmonths = 0;
	    if ( m <= 0 ) {
		--y;
		m+=12;
	    }
	} else if ( nmonths - 12 >= 0 ) {
	    y++;
	    nmonths-=12;
	} else if ( m == 12 ) {
	    y++;
	    m = 0;
	} else {
	    m+= nmonths;
	    nmonths = 0;
	    if ( m > 12 ) {
		++y;
		m -= 12;
	    }
	}
    }

    QDate tmp(y,m,1);

    if( d > tmp.daysInMonth() )
	d = tmp.daysInMonth();

    QDate date(y, m, d);
    return date;
}

/*!
  Returns a date \a nyears later than the date \a orig (or earlier if \a nyears is negative).

  First availability: Qtopia 1.6
*/
QDate Calendar::addYears( int nyears , const QDate &orig)
{
    int y, m, d;
    y = orig.year();
    m = orig.month();
    d = orig.day();

    y += nyears;
    QDate date(y, m, d);
    return date;
}
