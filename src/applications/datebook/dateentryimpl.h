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
#ifndef DATEENTRY_H
#define DATEENTRY_H

#include "dateentry.h"

#include <qtopia/pim/event.h>

#include <qdatetime.h>

class MonthViewBase;

class DateEntry : public DateEntryBase
{
    Q_OBJECT

public:
    DateEntry( bool startOnMonday, const QDateTime &start,
	       const QDateTime &end,
	       QWidget* parent = 0, const char* name = 0 );
    DateEntry( bool startOnMonday, const PimEvent &event,
	       QWidget* parent = 0, const char* name = 0 );
    ~DateEntry();

    PimEvent event();
    void setAlarmEnabled( bool alarmPreset, int presetTime, PimEvent::SoundTypeChoice );

public slots:
    void endDateTimeChanged( const QDateTime & );
    void startDateTimeChanged( const QDateTime & );
    void slotRepeat();
    void setWeekStartsMonday( bool );
    void allDayToggled(bool);

    void turnOnAlarm();
    void checkAlarmSpin(int);

private slots:
    void setRepeatType(int);
    void setEndDate(const QDate &);

private:
    void init();
    void setDates( const QDateTime& s, const QDateTime& e );
    void setRepeatLabel();

    PimEvent mEvent;
    bool startWeekOnMonday;
};

#endif // DATEENTRY_H
