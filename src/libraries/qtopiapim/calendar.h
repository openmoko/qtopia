/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef PIMCALENDAR_H
#define PIMCALENDAR_H

#include <qtopia/qpeglobal.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qdatetime.h>


class QTOPIA_EXPORT PimCalendar
{
public:
    static QString nameOfMonth( int m );
    static QString nameOfDay( int d, bool lname = FALSE );
    static QString nameOfDay( const QDate &d, bool lname = FALSE );
    static QValueList<QDate> daysOfMonth( int year, int month, bool startWithMonday = FALSE );

    static QDate dateAtPosition( int year, int month, bool startWithMonday, 
	    int row, int col);
    static void positionOfDate( int year, int month, bool startWithMonday, 
	    const QDate &date, int &row, int &col);
    static int positionOfDate( int year, int month, bool startWithMonday, 
	    const QDate &date);
    static int weekOfDate(const QDate &d);

    static QString ordinalNumber(int);

    static QDate addMonths(int nmonths, const QDate &orig);
    static QDate addYears(int nyears, const QDate &orig);
};

#endif
