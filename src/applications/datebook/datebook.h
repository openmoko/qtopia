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
#ifndef DATEBOOK_H
#define DATEBOOK_H

#include "datebookdb.h"

#include <qmainwindow.h>

class QAction;
class QWidgetStack;
class DayView;
class WeekView;
class MonthView;
class PimEvent;
class QDateTime;
class QDate;
class Ir;
class QMessageBox;

class DateBook : public QMainWindow
{
    Q_OBJECT

public:
    DateBook( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    ~DateBook();

signals:
    void newEvent();
    void signalNotFound();
    void signalWrapAround();

protected:
    QDate currentDate();
    void timerEvent( QTimerEvent *e );
    void closeEvent( QCloseEvent *e );

public slots:
    void flush();
    void reload();

    // some setting or env changed, data hasn't but how its displayed may have.
    void updateAlarms();
    void refreshWidgets();

private slots:
    void fileNew();
    void showSettings();
    void slotToday();	// view today
    void changeClock();
    void changeWeek( bool newDay );
    void appMessage(const QCString& msg, const QByteArray& data);
    // handle key events in the day view...
    void newEvent( const QString &str );
    void slotFind();
    void slotDoFind( const QString &, const QDate &, bool, bool, int );

    void viewDay();
    void viewWeek();
    void viewMonth();

    void showDay( int y, int m, int d );
    void showDay( const QDate & );

    void editOccurrence( const Occurrence &e );
    void removeOccurrence( const Occurrence &e );

    void editCurrentEvent();
    void removeCurrentEvent();

    void updateIcons();
    void setDocument( const QString & );
    void beamEvent( const PimEvent &e );

    void beamCurrentEvent();
    void beamDone( Ir *ir );

    void checkToday();

private:
    bool newEvent(const QDateTime& dstart,const QDateTime& dend,const QString& description,const QString& notes);
    void nextView();
    void viewToday();
    void viewDay(const QDate& dt);
    void viewWeek(const QDate& dt);
    void viewMonth(const QDate& dt);
    void addEvent( const PimEvent &e );
    void initDay();
    void initWeek();
    void initMonth();
    void initExceptionMb();
    void loadSettings();
    void saveSettings();
    bool receiveFile( const QString &filename );

    bool eventSelected() const;
    PimEvent currentEvent() const;
    Occurrence currentOccurrence() const;

private:
    DateBookTable *db;
    QWidgetStack *views;
    DayView *dayView;
    WeekView *weekView;
    MonthView *monthView;
    QAction *dayAction, *weekAction, *monthAction;
    QAction *editAction, *removeAction, *beamAction;
    bool aPreset;    // have everything set to alarm?
    int presetTime;  // the standard time for the alarm
    int startTime;
    bool ampm;
    bool onMonday;

    bool checkSyncing();
    bool syncing;
    bool inSearch;
    QDate lastToday; // last date that was the selected as 'Today'
    QTimer *midnightTimer;

    QString checkEvent(const PimEvent &);

    QMessageBox *exceptionMb;
    QDateTime lastcall;
};

#endif
