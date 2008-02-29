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
#include "periodview.h"

#include "datebookdb.h"

PeriodView::PeriodView( DateBookTable *datedb, bool ct, bool stm, 
	    QWidget *parent, const char *name)
: QWidget(parent, name), db(datedb), cType(ct), bOnMonday(stm)
{
    sHour = 8;
    cDate = QDate::currentDate();
}

bool PeriodView::hasSelection() const
{
    return FALSE;
}

PimEvent PeriodView::currentEvent() const
{
    return PimEvent();
}

void PeriodView::selectDate(const QDate &d)
{
    cDate = d;
}

void PeriodView::setTwelveHour( bool ct )
{
    cType = ct;
}

void PeriodView::setStartOnMonday( bool b )
{
    bOnMonday = b;
}

void PeriodView::setDayStarts( int h )
{
    sHour = h;
}

void PeriodView::databaseUpdated()
{
}
