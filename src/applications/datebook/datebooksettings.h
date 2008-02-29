/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include <QDialog>
#include <QAppointment>

class QGroupBox;
class QSpinBox;
class QComboBox;
class ReminderPicker;

class DateBookSettings : public QDialog
{
    Q_OBJECT
public:
    DateBookSettings( bool whichClock, QWidget *parent = 0,
                      Qt::WFlags = 0 );
    virtual ~DateBookSettings();

    void setStartTime( int newStartViewTime );
    int startTime() const;

    void setPresetAlarm(QAppointment::AlarmFlags, int minutes);

    QAppointment::AlarmFlags alarmType() const;
    int alarmDelay() const;

    enum ViewType {DayView = 0, MonthView, WeekView};
    ViewType defaultView() const;
    void setDefaultView( ViewType viewType);

private slots:
    void slot12Hour( int );
    void slotChangeClock( bool );

protected:
    QGroupBox *fraView;
    QComboBox *cmbDefaultView;
    QSpinBox *spinStart;
    QGroupBox *fraAlarm;
    ReminderPicker *picker;

private:
    void init();
    bool ampm;
    int oldtime;
    QAppointment mAppt;
};
#endif
