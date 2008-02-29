/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef DATEBOOKSETTINGS_H
#define DATEBOOKSETTINGS_H

#include "ui_datebooksettingsbase_phone.h"

class DateBookSettings : public QDialog, public Ui::DateBookSettingsBase
{
    Q_OBJECT
public:
    DateBookSettings( bool whichClock, QWidget *parent = 0,
                      Qt::WFlags = 0 );
    virtual ~DateBookSettings();
    void setStartTime( int newStartViewTime );
    int startTime() const;
    void setAlarmPreset( bool bAlarm, int presetTime );
    bool alarmPreset() const;
    int presetTime() const;
    void setAlarmType( int alarmType );
    int alarmType() const;
    void setCompressDay( bool );
    bool compressDay() const;

    enum ViewType {DayView = 0, MonthView, WeekView};
    ViewType defaultView() const;
    void setDefaultView( ViewType viewType);

private slots:
    void slot12Hour( int );
    void slotChangeClock( bool );
    void enablePresetDetails(int);

private:
    void init();
    bool ampm;
    int oldtime;
};
#endif
