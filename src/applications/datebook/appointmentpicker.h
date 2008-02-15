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
#ifndef APPOINTMENTPICKER_H
#define APPOINTMENTPICKER_H

#include <qdialog.h>
#include <qdatetime.h>
#include <QAppointmentModel>

class QStackedWidget;
class DayView;
class MonthView;
class QAppointment;
class DateBook;


class AppointmentPicker : public QDialog
{
    Q_OBJECT

public:
    AppointmentPicker( DateBook *db, QSet<QPimSource> sources, QWidget *parent = 0, Qt::WFlags f = 0 );
    ~AppointmentPicker();

    bool appointmentSelected() const;
    QAppointment currentAppointment() const;
    QDate currentDate();

private slots:
    void nextView();
    void viewDay(const QDate& dt);
    void viewMonthAgain();

private:
    void viewToday();
    void viewDay();
    void viewMonth();
    void viewMonth(const QDate& dt);
    void initDay();
    void initMonth();

    QStackedWidget *views;
    DayView *dayView;
    MonthView *monthView;
    QDate lastToday; // last date that was the selected as 'Today'
    DateBook *datebook;
    QSet<QPimSource> mSources;

};



#endif
