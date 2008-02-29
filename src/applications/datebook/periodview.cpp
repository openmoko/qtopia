/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "periodview.h"

#include <qtopia/pim/qappointmentmodel.h>

PeriodView::PeriodView( QOccurrenceModel *datedb, bool stm, QWidget *parent )
    : QWidget(parent), db(datedb), bOnMonday(stm)
{
    sHour = 8;
    cDate = QDate::currentDate();
}

bool PeriodView::hasSelection() const
{
    return false;
}

QAppointment PeriodView::currentAppointment() const
{
    return QAppointment();
}

void PeriodView::selectDate(const QDate &d)
{
    cDate = d;
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

