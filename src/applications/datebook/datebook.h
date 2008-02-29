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
#ifndef DATEBOOK_H
#define DATEBOOK_H

#include <qtopia/pim/qappointmentmodel.h>
#include <qtopia/pim/qappointment.h>

#include "exceptiondialog.h"
#include "dayview.h"
#include "monthview.h"
#include "appointmentdetails.h"

#if !QTOPIA_PHONE && QTOPIA4_TODO
# include "weekview.h"
#endif

#if !defined(QTOPIA_DESKTOP)
# include "datebookgui.h"
#else
# include "datebookgui_qd.h"
#endif

#include <QStackedWidget>
#include <QAction>
#include <QDateTime>
#include <QMessageBox>
#include <QtopiaAbstractService>
#include <QDSData>
#include <QStack>

class QAction;
class QStackedWidget;
class DayView;
class WeekView;
class MonthView;
class QAppointment;
class QDateTime;
class QMessageBox;
class ExceptionDialog;
class AppointmentDetails;
class QDLClient;
class QDSActionRequest;
class EntryDialog;

class DateBook : public DateBookGui
{
    friend class AppointmentPicker;
    friend class CalendarService;
    Q_OBJECT

public:
    DateBook( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~DateBook();

public slots:
    void flush();
    void reload();

    // some setting or env changed, data hasn't but how its displayed may have.
    void updateAlarms();

    void selectToday();     // Select today's date without changing view
    void viewToday();       // View today's date in day view
    void viewDay();         // View currently selected date in day view
    void viewDay(const QDate& d);
    void viewWeek();
    void viewWeek(const QDate& d);
    void viewMonth();
    void viewMonth(const QDate& d);
    void nextView();

    void unfoldAllDay();
    void foldAllDay();

    void checkToday();

    void showSettings();

    void changeClock();
    void changeWeek(bool newDay);

    void appMessage(const QString& msg, const QByteArray& data);

    void newAppointment();
    bool newAppointment(const QString &str);
    bool newAppointment(const QDateTime &dstart, const QDateTime &dend, const QString &description, const QString &notes);

    void addAppointment(const QAppointment &e);

    void editCurrentOccurrence();
    QOccurrence editOccurrence(const QOccurrence &o);
    QOccurrence editOccurrence(const QOccurrence &o, bool preview);

    void removeCurrentOccurrence();
    void removeOccurrence(const QOccurrence &o);
    void removeOccurrencesBefore();
    void removeOccurrencesBefore(const QDate &date, bool prompt = true);

    void beamCurrentAppointment();
    void beamAppointment(const QAppointment &a);
    void setDocument(const QString &filename);

    void showAppointmentDetails();
    void showAppointmentDetails(const QOccurrence &o);
    void hideAppointmentDetails();

    void showAccountSettings();

    void qdlActivateLink( const QDSActionRequest& request );
    void qdlRequestLinks( const QDSActionRequest& request );

    /*void find();
    void doFind(const QString &, const QDate &, Qt::CaseSensitivity, bool, const QCategoryFilter &);*/

    void updateIcons();

signals:
    void searchNotFound();
    void searchWrapAround();

protected:
    void timerEvent(QTimerEvent *e);
    void closeEvent(QCloseEvent *e);

    void init();
    void initDayView();
    void initMonthView();
    void initAppointmentDetails();
#if !QTOPIA_PHONE && QTOPIA4_TODO
    void initWeekView();
#endif

    void initExceptionDialog();
    int askException(const QString &action);

    void loadSettings();
    void saveSettings();

    bool receiveFile(const QString &filename);

    bool occurrenceSelected() const;
    QAppointment currentAppointment() const;
    QOccurrence currentOccurrence() const;

    void raiseView(QWidget *widget);

    QDate currentDate();
    bool checkSyncing();
    QString validateAppointment(const QAppointment &a);

private:
    QDSData appointmentQDLLink( QAppointment& appointment );
    void removeAppointmentQDLLink( QAppointment& appointment );

    QAppointmentModel *model;

    DayView *dayView;
    MonthView *monthView;
    AppointmentDetails *appointmentDetails;
    QStackedWidget *viewStack;
#if !QTOPIA_PHONE && QTOPIA4_TODO
    WeekView *weekView;
#endif
    EntryDialog* editorView;

    ExceptionDialog *exceptionDialog;

    // Configuration values
    bool aPreset;    // have everything set to alarm?
    int presetTime;  // the standard time for the alarm
    int startTime;
    bool ampm;
    bool onMonday;
    bool compressDay;

    bool syncing;
    bool inSearch;
    bool closeAfterView;
    QDate lastToday; // last date that was the selected as 'Today'
    QTimer *midnightTimer;
    QTimer *updateIconsTimer;

    QStack<QOccurrence> prevOccurrences;

    QString beamfile;
};

class CalendarService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class DateBook;
private:
    CalendarService( DateBook *parent )
        : QtopiaAbstractService( "Calendar", parent )
        { datebook = parent; publishAll(); }

public:
    ~CalendarService();

public slots:
    void newAppointment();
    void newAppointment( const QDateTime& start, const QDateTime& end,
                         const QString& description, const QString& notes );
    void addAppointment( const QAppointment& appointment );
    void updateAppointment( const QAppointment& appointment );
    void removeAppointment( const QAppointment& appointment );
    void raiseToday();
    void nextView();
    void showAppointment( const QUniqueId& uid );
    void showAppointment( const QUniqueId& uid, const QDate& date );
    void cleanByDate( const QDate& date );
    void activateLink( const QDSActionRequest& request );
    void requestLinks( const QDSActionRequest& request );

private:
    DateBook *datebook;
};

#endif
