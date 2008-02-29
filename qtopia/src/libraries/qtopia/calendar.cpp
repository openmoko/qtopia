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
#include "calendar.h"

#include <qdatetime.h>
#ifdef QTOPIA_DESKTOP
#include <qtopia/qpeapplication.h>
#endif

/*! \class Calendar calendar.html

  \brief The Calendar class provides programmers with an easy to calculate
  and get information about dates, months and years.

  \ingroup qtopiaemb
*/

#ifdef QTOPIA_DESKTOP
//translations in qt.qm
static const char* unTranslatedMediumDayNames[] = {
    "Mon" , "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};

static const char* unTranslatedMediumMonthNames[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dez"
};
#endif
/*!
    Returns the name of the month for \a m.
    Equivalent to QDate::monthName().
*/
QString Calendar::nameOfMonth( int m )
{
    QDate d;
#ifdef QTOPIA_DESKTOP
    if (qApp)
        return qApp->translate( "QDate", unTranslatedMediumMonthNames[m-1] );
#endif
    return d.monthName( m );
}


/*!
    Returns the name of the day for \a d.
    Equivalent to QDate::dayName().
*/
QString Calendar::nameOfDay( int d )
{
    QDate dt;
#ifdef QTOPIA_DESKTOP
    if (qApp)
        return qApp->translate( "QDate", unTranslatedMediumDayNames[d-1] );
#endif
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
