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
#ifndef DBMONTH
#define DBMONTH

#include <qtopia/pim/event.h>
#include <qtopia/datepicker.h>
#include "datebookdb.h"

#include <qdatetime.h>

class DayPaintCache
{
public:
    DayPaintCache() : nAllDay(FALSE), rAllDay(FALSE), tAllDay(FALSE) {}

    QValueList<int> nLine;
    QValueList<int> rLine;
    QValueList<int> tLine;

    bool nAllDay;
    bool rAllDay;
    bool tAllDay;
};

class MonthView : public QPEDatePicker
{
    Q_OBJECT

public:
    MonthView( DateBookTable *, QWidget *parent = 0, const char *name = 0 );
    ~MonthView();

public slots:
    void updateOccurrences();
    void getEventsForMonth(int y, int m);

    void setDate(const QDate &);

protected slots:
    void paintDayBackground(const QDate &, QPainter *, const QRect &,
	    const QColorGroup &);

private:
    void resizeEvent(QResizeEvent *e);
    QValueList<Occurrence> daysEvents;

    DateBookTable *mDb;

    //QVector<DayPaintCache> paintCache;
    QMap<QDate, DayPaintCache*> paintCache;
    int line_height;
};

#endif
