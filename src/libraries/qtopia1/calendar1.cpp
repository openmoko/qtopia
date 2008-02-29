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
#include <qtopia/calendar.h>
#include <qapplication.h>



const char* const longWeekNames[7] =
{
    QT_TRANSLATE_NOOP("LongWeekName", "Monday"),
    QT_TRANSLATE_NOOP("LongWeekName", "Tuesday"),
    QT_TRANSLATE_NOOP("LongWeekName", "Wednesday"),
    QT_TRANSLATE_NOOP("LongWeekName", "Thursday"),
    QT_TRANSLATE_NOOP("LongWeekName", "Friday"),
    QT_TRANSLATE_NOOP("LongWeekName", "Saturday"),
    QT_TRANSLATE_NOOP("LongWeekName", "Sunday")
};


QString Calendar::nameOfDay( int d, bool lname )
{
    //return QDate::shortDayName(d); // 3.0
    if (d < 1 || d > 7)
	return QString::null;

    if (lname)
	return qApp->translate("LongWeekName", longWeekNames[d-1]);

    QDate td;
    return td.dayName(d);
}

QString Calendar::nameOfDay( const QDate &td, bool lname )
{
    //return QDate::shortDayName(d); // 3.0
    if (lname)
	return qApp->translate("LongWeekName", longWeekNames[td.dayOfWeek()-1]);

    return td.dayName(td.dayOfWeek());
}

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

QDate Calendar::dateAtCoord( int year, int month, 
	int row, int col, bool startWithMonday)
{
    int pos = row*7 + col;
    return dateAtIndex(year, month, pos, startWithMonday);
}

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


const char* const ordinal[54] = 
{
    QT_TRANSLATE_NOOP("Ordinal", "first"),
    QT_TRANSLATE_NOOP("Ordinal", "second"),
    QT_TRANSLATE_NOOP("Ordinal", "third"),
    QT_TRANSLATE_NOOP("Ordinal", "fourth"),
    QT_TRANSLATE_NOOP("Ordinal", "fifth"),
    QT_TRANSLATE_NOOP("Ordinal", "sixth"),
    QT_TRANSLATE_NOOP("Ordinal", "seventh"),
    QT_TRANSLATE_NOOP("Ordinal", "eighth"),
    QT_TRANSLATE_NOOP("Ordinal", "ninth"),
    QT_TRANSLATE_NOOP("Ordinal", "tenth"),
    QT_TRANSLATE_NOOP("Ordinal", "eleventh"),
    QT_TRANSLATE_NOOP("Ordinal", "twelfth"),
    QT_TRANSLATE_NOOP("Ordinal", "thirteenth"),
    QT_TRANSLATE_NOOP("Ordinal", "fourteenth"),
    QT_TRANSLATE_NOOP("Ordinal", "fifteenth"),
    QT_TRANSLATE_NOOP("Ordinal", "sixteenth"),
    QT_TRANSLATE_NOOP("Ordinal", "seventeenth"),
    QT_TRANSLATE_NOOP("Ordinal", "eighteenth"),
    QT_TRANSLATE_NOOP("Ordinal", "nineteenth"),
    QT_TRANSLATE_NOOP("Ordinal", "twentieth"),
    QT_TRANSLATE_NOOP("Ordinal", "twenty-first"),
    QT_TRANSLATE_NOOP("Ordinal", "twenty-second"),
    QT_TRANSLATE_NOOP("Ordinal", "twenty-third"),
    QT_TRANSLATE_NOOP("Ordinal", "twenty-fourth"),
    QT_TRANSLATE_NOOP("Ordinal", "twenty-fifth"),
    QT_TRANSLATE_NOOP("Ordinal", "twenty-sixth"),
    QT_TRANSLATE_NOOP("Ordinal", "twenty-seventh"),
    QT_TRANSLATE_NOOP("Ordinal", "twenty-eighth"),
    QT_TRANSLATE_NOOP("Ordinal", "twenty-ninth"),
    QT_TRANSLATE_NOOP("Ordinal", "thirtieth"),
    QT_TRANSLATE_NOOP("Ordinal", "thirty-first"),
    QT_TRANSLATE_NOOP("Ordinal", "thirty-second"),
    QT_TRANSLATE_NOOP("Ordinal", "thirty-third"),
    QT_TRANSLATE_NOOP("Ordinal", "thirty-forth"),
    QT_TRANSLATE_NOOP("Ordinal", "thirty-fifth"),
    QT_TRANSLATE_NOOP("Ordinal", "thirty-sixth"),
    QT_TRANSLATE_NOOP("Ordinal", "thirty-seventh"),
    QT_TRANSLATE_NOOP("Ordinal", "thirty-eighth"),
    QT_TRANSLATE_NOOP("Ordinal", "thirty-ninth"),
    QT_TRANSLATE_NOOP("Ordinal", "fortieth"),
    QT_TRANSLATE_NOOP("Ordinal", "forty-first"),
    QT_TRANSLATE_NOOP("Ordinal", "forty-second"),
    QT_TRANSLATE_NOOP("Ordinal", "forty-third"),
    QT_TRANSLATE_NOOP("Ordinal", "forty-forth"),
    QT_TRANSLATE_NOOP("Ordinal", "forty-fifth"),
    QT_TRANSLATE_NOOP("Ordinal", "forty-sixth"),
    QT_TRANSLATE_NOOP("Ordinal", "forty-seventh"),
    QT_TRANSLATE_NOOP("Ordinal", "forty-eighth"),
    QT_TRANSLATE_NOOP("Ordinal", "forty-ninth"),
    QT_TRANSLATE_NOOP("Ordinal", "fiftieth"),
    QT_TRANSLATE_NOOP("Ordinal", "fifty-first"),
    QT_TRANSLATE_NOOP("Ordinal", "fifty-second"),
    QT_TRANSLATE_NOOP("Ordinal", "fifty-third"),
    QT_TRANSLATE_NOOP("Ordinal", "fifty-forth")
};

/* Handles up to 54th */
QString Calendar::ordinalNumber(int num)
{
    if (num < 1 || num > 54)
	return QString::null;

    return qApp->translate("Ordinal", ordinal[num-1]);
}

int Calendar::weekInMonth(const QDate &dt)
{
    if (dt.day() == 1)
	return 1;
    return ((dt.day() - 1) / 7) + 1;
}

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
