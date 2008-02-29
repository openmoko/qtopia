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

#include <QAppointment>
#include <QComboBox>
#include <QList>
#include <QLabel>
#include <QGroupBox>

#include <QFormLayout>
#include <QVBoxLayout>
#include "reminderpicker.h"

#include <QDebug>

#include <QDialog>
#include <QSpinBox>
#include <QtopiaApplication>
#include <QTimeEdit>

class OtherReminderDialog : public QDialog
{
    Q_OBJECT;
public:
    OtherReminderDialog(bool allDay, int minutes, QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~OtherReminderDialog();
    void setReminder(bool allDay, int minutes);
    int reminder();

protected:
    QLabel *minutesL;
    QLabel *hoursL;
    QSpinBox *minutesSB;
    QSpinBox *hoursSB;
    QSpinBox *daysSB;
    QSpinBox *weeksSB;
};


OtherReminderDialog::OtherReminderDialog(bool allDay, int min, QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    QVBoxLayout *vl = new QVBoxLayout();

    QGroupBox *gb = new QGroupBox(tr("Reminder:"));
    QFormLayout *fl = new QFormLayout();
    minutesSB = new QSpinBox();
    hoursSB = new QSpinBox();
    daysSB = new QSpinBox();
    weeksSB = new QSpinBox();

    minutesL = new QLabel(tr("Minutes:"));
    hoursL = new QLabel(tr("Hours:"));
    minutesL->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    hoursL->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    minutesL->setBuddy(minutesSB);
    hoursL->setBuddy(hoursSB);
    fl->addRow(minutesL, minutesSB);
    fl->addRow(hoursL, hoursSB);
    fl->addRow(tr("Days:"), daysSB);
    fl->addRow(tr("Weeks:"), weeksSB);

    setWindowTitle(tr("Other reminder"));
    gb->setLayout(fl);
    vl->addWidget(gb);
    vl->addSpacing(40);
    vl->addStretch();
    setLayout(vl);

    setReminder(allDay, min);
}

OtherReminderDialog::~OtherReminderDialog()
{
}

void OtherReminderDialog::setReminder(bool allDay, int minutes)
{
    if (allDay) {
        minutesSB->hide();
        minutesL->hide();
        hoursSB->hide();
        hoursL->hide();
    } else {
        minutesSB->show();
        minutesL->show();
        hoursSB->show();
        minutesL->show();
    }

    int weeks = minutes / (7 * 24 * 60);
    int days = (minutes - (weeks * 7 * 24 * 60)) / (24 * 60);
    int hours = (minutes - (((weeks * 7) + days) * 24 * 60)) / 60;

    minutesSB->setValue(minutes % 60);
    hoursSB->setValue(hours);
    daysSB->setValue(days);
    weeksSB->setValue(weeks);
}

int OtherReminderDialog::reminder()
{
    int ret = 0;

    ret += minutesSB->value();
    ret += hoursSB->value() * 60;
    ret += daysSB->value() * (24 * 60);
    ret += weeksSB->value() * (7 * 24 * 60);

    return ret;
}

QList<ReminderPicker::ReminderEntry> ReminderPicker::reminderEntries;
bool ReminderPicker::listInited;

ReminderPicker::ReminderPicker(QObject *parent, QFormLayout *fl)
    : QObject(parent), mAllDay(false), mTimeSet(false), mAllDayDefault(8,0)
{
    if (!listInited) {
        listInited = true;
        reminderEntries.append(ReminderEntry(ReminderEntry::NotAllDay, 0, tr("At the event time")));
        reminderEntries.append(ReminderEntry(ReminderEntry::AllDay, 0, tr("On the day")));
        reminderEntries.append(ReminderEntry(ReminderEntry::NotAllDay, 5, tr("5 minutes before")));
//        reminderEntries.append(ReminderEntry(ReminderEntry::NotAllDay, 10, tr("10 minutes before")));
        reminderEntries.append(ReminderEntry(ReminderEntry::NotAllDay, 15, tr("15 minutes before")));
        reminderEntries.append(ReminderEntry(ReminderEntry::NotAllDay, 30, tr("30 minutes before")));
        reminderEntries.append(ReminderEntry(ReminderEntry::NotAllDay, 60, tr("1 hour before")));
        reminderEntries.append(ReminderEntry(ReminderEntry::NotAllDay, 120, tr("2 hours before")));
        reminderEntries.append(ReminderEntry(ReminderEntry::Both, 1 * 24 * 60, tr("1 day before")));
        reminderEntries.append(ReminderEntry(ReminderEntry::Both, 2 * 24 * 60, tr("2 days before")));
        reminderEntries.append(ReminderEntry(ReminderEntry::Both, 3 * 24 * 60, tr("3 days before")));
        reminderEntries.append(ReminderEntry(ReminderEntry::Both, 7 * 24 * 60, tr("1 week before")));
        reminderEntries.append(ReminderEntry(ReminderEntry::Both, 14 * 24 * 60, tr("2 weeks before")));
// Unwanted complexity at the moment:
//        reminderEntries.append(ReminderEntry(ReminderEntry::Both, -1, tr("Other...")));
    }
    timeminutes = 0;
    dayminutes = 0;

    comboReminder = new QComboBox();
    comboReminderDelay = new QComboBox();

    comboReminder->clear();
    comboReminder->addItem(tr("None"));
    comboReminder->addItem(tr("Audible"));
    comboReminder->addItem(tr("Silent"));

    timeEdit = new QTimeEdit();
    timeLabel = new QLabel();

    timeLabel->setText(tr("At", "At: 9:00am"));
    timeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    timeLabel->setBuddy(timeEdit);
    initCB();

    fl->addRow(tr("Reminder"), comboReminder);
    fl->addRow(timeLabel, timeEdit);
    fl->addRow(comboReminderDelay);

    connect( comboReminder, SIGNAL(activated(int)), this, SLOT(reminderChanged(int)) );
    connect( comboReminderDelay, SIGNAL(activated(int)), this, SLOT(reminderDelayChanged(int)) );
    connect( timeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(reminderTimeChanged(QTime)));
}

void ReminderPicker::initCB()
{
    int idx = 0;
    comboReminderDelay->clear();
    ReminderEntry::entry_type t = mAllDay ? ReminderEntry::AllDay : ReminderEntry::NotAllDay;
    foreach (ReminderEntry re, reminderEntries) {
        if (re.type & t)
            comboReminderDelay->addItem(re.label, idx);
        idx++;
    }
}

void ReminderPicker::reminderChanged(int index)
{
    if (index == 0) { // None
        reminderFlag = QAppointment::NoAlarm;
        comboReminderDelay->setEnabled(false);
        timeEdit->setEnabled(false);
        timeLabel->setEnabled(false);
    } else {
        reminderFlag = (index == 1) ? QAppointment::Audible : QAppointment::Visible;
        comboReminderDelay->setEnabled(true);
        timeLabel->setEnabled(true);
        timeEdit->setEnabled(true);
    }
}

void ReminderPicker::reminderDelayChanged(int index)
{
    ReminderEntry re = reminderEntries.value(comboReminderDelay->itemData(index).toInt());

    if (re.isOther()) {
        OtherReminderDialog d(mAllDay, dayminutes);

        if (QtopiaApplication::execDialog(&d)) {
            // save the delay
            dayminutes = d.reminder();
        }
    } else {
        dayminutes = re.minutes;
    }
}

void ReminderPicker::reminderTimeChanged(const QTime&)
{
    mTimeSet = true;
    updateReminderMinutes();
}

void ReminderPicker::updateReminderMinutes()
{
    ReminderEntry re = reminderEntries.value(comboReminderDelay->itemData(comboReminderDelay->currentIndex()).toInt());
    timeminutes = (timeEdit->time().hour() * 60) + timeEdit->time().minute();
    dayminutes = re.minutes;
}

void ReminderPicker::setDefaultAllDayReminderTime(const QTime& defTime)
{
    mAllDayDefault = defTime;

    if (!mTimeSet) {
        timeEdit->setTime(defTime);
        updateReminderMinutes();
    }
}

void ReminderPicker::setAllDay(bool allDay)
{
    if (allDay != mAllDay) {
        mAllDay = allDay;
        initCB();
    }

    // Clear this.
    dayminutes = 0;

    // And maybe this, if the user hasn't set it
    if (!mTimeSet) {
        timeEdit->setTime(mAllDayDefault);
        updateReminderMinutes();
    }
    updateUI();
}

void ReminderPicker::setReminderType(QAppointment::AlarmFlags flag)
{
    reminderFlag = flag;
    updateUI();
}

int ReminderPicker::reminderMinutes() const {
    if (mAllDay) {
        return dayminutes - timeminutes;
    }
    return dayminutes;
}

void ReminderPicker::setReminderMinutes(int minutes)
{
    updateUI();

    // Also set the time edit UI (for an all day event)
    if (mAllDay) {
        // Basically any reminders on the day will be a negative
        // minutes value (since all day events are considered to
        // start at 12:00am).
        if (minutes <= 0) {
            timeminutes = -minutes;
            minutes = 0;
        } else {
            timeminutes = minutes % (24 * 60);
            if (timeminutes != 0)
                timeminutes = (24*60) - timeminutes;
            minutes += timeminutes;
        }
        dayminutes = minutes - timeminutes;
    } else
        dayminutes = minutes;

    timeEdit->setTime(QTime(timeminutes / 60, timeminutes %60));
}

void ReminderPicker::updateUI()
{
    comboReminder->blockSignals(true);
    comboReminderDelay->blockSignals(true);
    timeEdit->blockSignals(true);

    // now the alarm details
    int idx = 0;
    bool found = false;

    ReminderEntry::entry_type t = mAllDay ? ReminderEntry::AllDay : ReminderEntry::NotAllDay;
    foreach(ReminderEntry re, reminderEntries) {
        if (re.type & t) {
            if (re.minutes == dayminutes) {
                comboReminderDelay->setCurrentIndex(idx);
                found = true;
                break;
            }
            idx++;
        }
    }

    // idx is either after "Other" or at the found value
    if (!found) {
        comboReminderDelay->setCurrentIndex(idx - 1);
    }

    if (reminderFlag != QAppointment::NoAlarm) {
        timeLabel->setEnabled(true);
        timeEdit->setEnabled(true);
        comboReminderDelay->setEnabled(true);
        if (reminderFlag == QAppointment::Audible)
            comboReminder->setCurrentIndex(1);
        else
            comboReminder->setCurrentIndex(2);
    } else {
        timeEdit->setEnabled(false);
        timeLabel->setEnabled(false);
        comboReminderDelay->setEnabled(false);
        comboReminder->setCurrentIndex(0);
    }

    if (mAllDay) {
        timeEdit->show();
        timeLabel->show();
    } else {
        timeEdit->hide();
        timeLabel->hide();
    }

    timeEdit->blockSignals(false);
    comboReminder->blockSignals(false);
    comboReminderDelay->blockSignals(false);
}

QString ReminderPicker::formatReminder(bool allDay, QAppointment::AlarmFlags flag, int prelay)
{
    int todminutes = 0;

    if (allDay) {
        if (prelay <= 0) {
            todminutes = -prelay;
            prelay = 0;
        } else {
            todminutes = prelay % (24 * 60);
            if (todminutes != 0)
                todminutes = (24*60) - todminutes;
            prelay += todminutes;
        }
    }

    int days = prelay / (24 * 60);
    int hours = (prelay / 60) % 24 ;
    int minutes = prelay % 60;

    // Try to make the time simpler a little
    // with hours & minutes, except leave
    // 0,15,30,45
    if (minutes !=0 && minutes != 15 && minutes != 30 && minutes !=45 && hours <=2) {
        minutes += hours * 60;
        hours = 0;
    }

    QString remstr;
    QString typestr = (flag == QAppointment::Visible ?
                    tr("Silent") :
                    tr("Audible", "eg. 5 minutes before (Audible)"));

    // XXX this probably needs to be split up further for better translations
    if (!allDay) {
        if (prelay > 0) {
            if (days != 0) {
                remstr = tr("%n days", "3 days", days);
            }
            if (hours != 0) {
                if (!remstr.isEmpty())
                    remstr += ", ";
                remstr += tr("%n hours", "3 hours", hours);
            }
            if (minutes != 0) {
                if (!remstr.isEmpty())
                    remstr += ", ";
                remstr += tr("%n minutes", "15 minutes", minutes);
            }

            return tr("%1 before (%2)", "e.g. 5 days, 7 hours before (Audible)").arg(remstr).arg(typestr);
        } else {
            return tr("At the event time");
        }
    } else {
        QTime t(todminutes / 60, todminutes %60, 0);
        QString tstr = QTimeString::localHM(t);

        if (days == 0) {
            // A reminder on the day
            return tr("%1 on the day (%2)", "9:00 on the day(Audible)").arg(tstr).arg(typestr);
        } else {
            return tr("%1 before, at %2 (%3)", "3 days before, at 9:00 (Audible)").arg(tr("%n days", "3 days", days)).arg(tstr).arg(typestr);
        }
    }
}

#include "reminderpicker.moc"

