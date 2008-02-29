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
#ifndef CALENDAR_H
#define CALENDAR_H

#include <qtopia/qpeglobal.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qdatetime.h>

class  QTOPIA_EXPORT Calendar
{
public:
    // obsolete
    struct Day
    {
	enum Type { PrevMonth, ThisMonth, NextMonth };
	
	Day() : date( 0 ), type( ThisMonth ), holiday( FALSE ) {}
	Day( int d, Type t, bool h ) : date( d ), type( t ), holiday( h ) {}
	
	int date;
	Type type;
	bool holiday;
    };
    // obsolete
    static QValueList<Day> daysOfMonth( int year, int month, bool startWithMonday = FALSE );

    static QString nameOfMonth( int m );
    static QString nameOfDay( int d );

    // New functions
    static QValueList<QDate> datesOfMonth( int year, int month, bool startWithMonday = FALSE );

    static QDate dateAtIndex( int year, int month, int index, 
	    bool startWithMonday = FALSE);

    static QDate dateAtCoord( int year, int month, int row, int col,
	    bool startWithMonday = FALSE);

    static void coordForDate( int year, int month, const QDate &date, 
	    int &row, int &col,
	    bool startWithMonday = FALSE);

    static int indexForDate( int year, int month, const QDate &date,
	    bool startWithMonday = FALSE);

    static int weekInMonth(const QDate &d);

    static QDate addMonths(int nmonths, const QDate &orig);
    static QDate addYears(int nyears, const QDate &orig);
};

#endif
