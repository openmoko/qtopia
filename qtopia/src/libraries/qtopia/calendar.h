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
    static QValueList<QDate> datesOfMonth( int year, int month, bool startWithMonday = FALSE ); // libqtopia

    static QDate dateAtIndex( int year, int month, int index, 
	    bool startWithMonday = FALSE); // libqtopia

    static QDate dateAtCoord( int year, int month, int row, int col,
	    bool startWithMonday = FALSE); // libqtopia

    static void coordForDate( int year, int month, const QDate &date, 
	    int &row, int &col,
	    bool startWithMonday = FALSE); // libqtopia

    static int indexForDate( int year, int month, const QDate &date,
	    bool startWithMonday = FALSE); // libqtopia

    static int weekInMonth(const QDate &d); // libqtopia

    static QDate addMonths(int nmonths, const QDate &orig); // libqtopia
    static QDate addYears(int nyears, const QDate &orig); // libqtopia
};

#endif
