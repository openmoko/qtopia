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

#ifndef REMINDERPICKER_H
#define REMINDERPICKER_H

#include <QDateTime>
#include <QWidget>
#include <QString>
#include <QList>

#include "qappointment.h"

class QFormLayout;
class QComboBox;
class QTimeEdit;
class QLabel;

class ReminderPicker : public QObject {
    Q_OBJECT;

    public:
        ReminderPicker(QObject *parent, QFormLayout *fl);

        QAppointment::AlarmFlags reminderType() const {return reminderFlag;}
        int reminderMinutes() const;

        void setDefaultAllDayReminderTime(const QTime& defaultAllDayTime);
        void setAllDay(bool allDay);
        void setReminderType(QAppointment::AlarmFlags type);
        void setReminderMinutes(int minutes);

        static QString formatReminder(bool allDay, QAppointment::AlarmFlags flag, int minutes);

    private slots:
        void reminderChanged(int index);
        void reminderDelayChanged(int index);
        void reminderTimeChanged(const QTime& time);
        void updateReminderMinutes();

    private:
        void initCB();
        void updateUI();
        void splitReminderMinutes(int& dayminutes, int &timeminutes);
        class ReminderEntry{
        public:
            int minutes;
            QString label;
            typedef enum {AllDay = 0x01, NotAllDay = 0x02, Both = 0x03} entry_type;
            entry_type type;

            ReminderEntry(entry_type allday, int m, const QString& l) : minutes(m), label(l),type(allday) {}
            ReminderEntry() : minutes(-1), type(Both) {}
            bool isOther() const {return (minutes == -1);}
        };

        static QList < ReminderEntry > reminderEntries;
        static bool listInited;
        bool mAllDay;
        bool mTimeSet;
        QTime mAllDayDefault;
        QComboBox *comboReminder;
        QComboBox *comboReminderDelay;
        QTimeEdit *timeEdit;
        QLabel *timeLabel;

        int dayminutes;
        int timeminutes;
        QAppointment::AlarmFlags reminderFlag;
};

#endif // REMINDERPICKER_H
