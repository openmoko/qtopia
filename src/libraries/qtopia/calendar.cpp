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
#include "calendar.h"

#include <qdatetime.h>

/*! \class Calendar calendar.html

  \brief The Calendar class provides programmers with an easy to calculate
  and get information about dates, months and years.

  \ingroup qtopiaemb
*/

/*!
    Returns the name of the month for \a m.
    Equivalent to QDate::monthName().
*/
QString Calendar::nameOfMonth( int m )
{
    QDate d;
    return d.monthName( m );
}

/*!
    Returns the name of the day for \a d.
    Equivalent to QDate::dayName().
*/
QString Calendar::nameOfDay( int d )
{
    QDate dt;
    return dt.dayName( d );
}

/*! \obsolete */
QValueList<Calendar::Day> Calendar::daysOfMonth( int year, int month,
						 bool startWithMonday )
{
    QDate temp;
    temp.setYMD( year, month, 1 );
    int firstDay = temp.dayOfWeek();
    int i;
    QDate prev;
    QValueList<Day> days;    

    if ( startWithMonday )
	i = 1;
    else
	i = 0;
    
    if ( month > 1 )
	    prev.setYMD( year, month - 1, 1 );
    else
	prev.setYMD( year - 1, 12, 1 );	
    for ( ; i < firstDay; i++ ) {
	days.append( Day( prev.daysInMonth() - ( firstDay - i - 1 ),
			  Day::PrevMonth, FALSE ) );
    }
    for ( i = 1; i <= temp.daysInMonth(); i++ )
	days.append( Day( i, Day::ThisMonth, FALSE ) );
    i = 0;
    while ( days.count() < 6 * 7 )
	days.append( Day( ++i, Day::NextMonth, FALSE ) );

    return days;
}
