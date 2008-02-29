/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "repeatentry.h"

//#include <qtopia/qcalendarwidget.h>

#include <qtopiaapplication.h>

#include <QSpinBox>
#include <QToolButton>
#include <QDateTime>
#include <QLabel>
#include <QRadioButton>
#include <QDesktopWidget>
#include <QStackedWidget>

/*#ifdef QTOPIA_PHONE
static const QTimeString::Length TIMESTRING_LENGTH = QTimeString::Medium;
#else*/
static const QTimeString::Length TIMESTRING_LENGTH = QTimeString::Long;
//#endif

// ====================================================================

RepeatEntry::RepeatEntry( QWidget *parent )
    : QDialog(parent), startMonday(Qtopia::weekStartsOnMonday())
{
    int approxSize = QFontMetrics(QFont()).width(" Wed ") * 7;
    QDesktopWidget *desktop = QApplication::desktop();
    if ( desktop->availableGeometry(desktop->screenNumber(this)).width() > approxSize )
        weekLabelLen = QTimeString::Medium;
    else
        weekLabelLen = QTimeString::Short;

    Ui::RepeatEntryBase::setupUi(this);

#ifdef Q_WS_QWS
    okButton->hide();
    cancelButton->hide();
    spacerItem->changeSize(0, 0);
#endif

    refreshLabels();

    setObjectName( "appointment-repeat" );
#ifdef QTOPIA_PHONE
    setWindowTitle(tr("Repeat"));
#else
    setWindowTitle(tr("Repeating Appointment"));
#endif
}

RepeatEntry::~RepeatEntry()
{
}

void RepeatEntry::accept()
{
    switch(repeatTypeSelect->currentIndex()) {
        case 0:
            mAppointment.setRepeatRule(QAppointment::Daily);
            break;
        case 1:
            mAppointment.setRepeatRule(QAppointment::Weekly);
            setAppointmentWeekFlags();
            break;
        case 2:
            if (dayOfMonth->isChecked())
                mAppointment.setRepeatRule(QAppointment::MonthlyDate);
            else if (weekDayOfMonth->isChecked())
                mAppointment.setRepeatRule(QAppointment::MonthlyDay);
            else
                mAppointment.setRepeatRule(QAppointment::MonthlyEndDay);
            break;
        case 3:
            mAppointment.setRepeatRule(QAppointment::Yearly);
            break;
    }
    mAppointment.setFrequency(frequency->value());

    QDialog::accept();
}

static inline void increment(Qt::DayOfWeek &day)
{
    switch(day)
    {
        case Qt::Monday:
            day = Qt::Tuesday;
            break;
        case Qt::Tuesday:
            day = Qt::Wednesday;
            break;
        case Qt::Wednesday:
            day = Qt::Thursday;
            break;
        case Qt::Thursday:
            day = Qt::Friday;
            break;
        case Qt::Friday:
            day = Qt::Saturday;
            break;
        case Qt::Saturday:
            day = Qt::Sunday;
            break;
        case Qt::Sunday:
            day = Qt::Monday;
            break;
    }
}

void RepeatEntry::refreshLabels()
{
    Qt::DayOfWeek day = startMonday ? Qt::Monday : Qt::Sunday;

    weekDayToggle[day] = weekDay1Toggle;
    weekDay1Toggle->setText(QTimeString::nameOfWeekDay(day, weekLabelLen));
    increment(day);

    weekDayToggle[day] = weekDay2Toggle;
    weekDay2Toggle->setText(QTimeString::nameOfWeekDay(day, weekLabelLen));
    increment(day);

    weekDayToggle[day] = weekDay3Toggle;
    weekDay3Toggle->setText(QTimeString::nameOfWeekDay(day, weekLabelLen));
    increment(day);

    weekDayToggle[day] = weekDay4Toggle;
    weekDay4Toggle->setText(QTimeString::nameOfWeekDay(day, weekLabelLen));
    increment(day);

    weekDayToggle[day] = weekDay5Toggle;
    weekDay5Toggle->setText(QTimeString::nameOfWeekDay(day, weekLabelLen));
    increment(day);

    weekDayToggle[day] = weekDay6Toggle;
    weekDay6Toggle->setText(QTimeString::nameOfWeekDay(day, weekLabelLen));
    increment(day);

    weekDayToggle[day] = weekDay7Toggle;
    weekDay7Toggle->setText(QTimeString::nameOfWeekDay(day, weekLabelLen));
}

void RepeatEntry::setAppointmentWeekFlags()
{
    if (repeatTypeSelect->currentIndex() != 1)
        return;

    int i;
    Qt::DayOfWeek day = startMonday ? Qt::Monday: Qt::Sunday;
    for (i = 0; i < 7; i++) {
        QToolButton *tb = weekDayToggle[day];
        mAppointment.setRepeatOnWeekDay(day, tb->isChecked());

        increment(day);
    }
}

void RepeatEntry::setWeekDayToggles()
{
    bool sm = Qtopia::weekStartsOnMonday();
    if (sm != startMonday) {
        startMonday = sm;
        refreshLabels();
    }

    int i;
    Qt::DayOfWeek day = startMonday ? Qt::Monday: Qt::Sunday;
    for (i = 0; i < 7; i++) {
        QToolButton *tb = weekDayToggle[day];

        if (mAppointment.start().date().dayOfWeek() == day) {
            tb->setEnabled(false);
            tb->setChecked(true);
        } else {
            tb->setEnabled(true);
            tb->setChecked(mAppointment.repeatOnWeekDay(day));
        }

        increment(day);
    }
}

QString RepeatEntry::trSmallOrdinal(int n) const
{
    if ( n == 1 ) return tr("first", "eg. first Friday of month");
    else if ( n == 2 ) return tr("second", "eg. second Friday of month");
    else if ( n == 3 ) return tr("third", "eg. third Friday of month");
    else if ( n == 4 ) return tr("fourth", "eg. fourth Friday of month");
    else if ( n == 5 ) return tr("fifth", "eg. fifth Friday of month");
    else return QString::number(n);
}

void RepeatEntry::setAppointment(const QAppointment &appointment)
{
    mAppointment = appointment;
    if (appointment.repeatRule() == QAppointment::NoRepeat)
        frequency->setValue(1);
    else
        frequency->setValue(appointment.frequency());
    stackedWidget->setCurrentIndex(frequency->value());

    setWeekDayToggles();

    switch(appointment.repeatRule()) {
        default:
        case QAppointment::Daily:
            repeatTypeSelect->setCurrentIndex(0);
            stackedWidget->setCurrentIndex(0);
            dayOfMonth->setChecked(true);
            break;
        case QAppointment::Weekly:
            repeatTypeSelect->setCurrentIndex(1);
            stackedWidget->setCurrentIndex(1);
            dayOfMonth->setChecked(true);
            break;
        case QAppointment::MonthlyDate:
            repeatTypeSelect->setCurrentIndex(2);
            stackedWidget->setCurrentIndex(2);
            dayOfMonth->setChecked(true);
            break;
        case QAppointment::MonthlyDay:
            repeatTypeSelect->setCurrentIndex(2);
            stackedWidget->setCurrentIndex(2);
            weekDayOfMonth->setChecked(true);
            break;
        case QAppointment::MonthlyEndDay:
            repeatTypeSelect->setCurrentIndex(2);
            stackedWidget->setCurrentIndex(2);
            lastWeekDayOfMonth->setChecked(true);
            break;
        case QAppointment::Yearly:
            repeatTypeSelect->setCurrentIndex(3);
            stackedWidget->setCurrentIndex(3);
            dayOfMonth->setChecked(true);
            break;
    }

    //  Update the 'monthy' radio button text

    QDate date = mAppointment.start().date();

    dayOfMonth->setText(tr("Day %1 of the month", "eg. Day 3 of the month").arg(date.day()));

    weekDayOfMonth->setText(tr("The %1 %2", "eg. The second Monday (of the month)")
        .arg(trSmallOrdinal(((date.day() - 1) / 7) + 1))
        .arg(QTimeString::localDayOfWeek(date, TIMESTRING_LENGTH)));

    lastWeekDayOfMonth->setText(tr("The %1 %2\nfrom the end",
        "eg. The third Tuesday from the end (of the month)")
        .arg(trSmallOrdinal(((date.daysInMonth() - date.day()) / 7) + 1))
        .arg(QTimeString::localDayOfWeek(date, TIMESTRING_LENGTH)));
}

QAppointment RepeatEntry::appointment() const
{
    return mAppointment;
}

