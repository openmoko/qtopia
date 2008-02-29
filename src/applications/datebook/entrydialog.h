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
#ifndef ENTRYDIALOG_H
#define ENTRYDIALOG_H

#include <qtopia/pim/qappointment.h>

#include <QDateTime>
#include <QDialog>

class QTextEdit;
class QTabWidget;
class EntryDetails;
class QDLEditClient;
class QScrollArea;
class DateBookCategorySelector;
class RecurrenceDetails;
class ReminderPicker;
class QLabel;
class QLineEdit;
class QCheckBox;
class QDateEdit;
class QTimeEdit;
class QTimeZoneSelector;

class EntryDialog : public QDialog
{
    Q_OBJECT

public:
    EntryDialog( bool startOnMonday, const QAppointment &appointment, const QTime& defaultAllDayTime,
                 QWidget *parent = 0, Qt::WFlags f = 0 );
    ~EntryDialog();

    QAppointment appointment( const bool includeQdlLinks = true);

private slots:
    void updateStartDateTime();
    void updateStartTime();
    void updateEndDateTime();
    void updateEndTime();
    void setWeekStartsMonday( bool );
    void updateTimeUI();
    void initTab(int, QScrollArea *);

private:
    void init();
    void initEventDetails(QScrollArea *);
    void initRepeatDetails(QScrollArea *);
    void initNoteDetails(QScrollArea *);


    void setDates( const QDateTime& s, const QDateTime& e );
    void accept();

    QAppointment mAppointment;
    QAppointment mOrigAppointment;
    bool startWeekOnMonday;
    QTime allDayReminder;
    DateBookCategorySelector *comboCategory;
    RecurrenceDetails *recurDetails;
    ReminderPicker *reminderPicker;
    QTextEdit *editNote;
    QDLEditClient *editnoteQC;

    QLineEdit *mDescription, *mLocation;
    QCheckBox *checkAllDay;
    QDateEdit *startDate, *endDate;
    QTimeEdit *startTime, *endTime;
    QLabel *startTimeLabel, *endTimeLabel;
    QTimeZoneSelector *timezone;
};

#endif // ENTRYDIALOG_H
