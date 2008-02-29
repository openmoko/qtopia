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
    MonthView( DateBookTable *, QWidget *parent = 0, const char *name = 0, bool ac = FALSE );
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

    QVector<DayPaintCache> paintCache;
    int line_height;
};

#endif
