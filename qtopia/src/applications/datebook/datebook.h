/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
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

#if !defined(QTOPIA_DESKTOP)
#include "datebookgui.h"
#else
#include "datebookgui_qd.h"
#endif

class QAction;
class QWidgetStack;
class DayView;
class WeekView;
class MonthView;
class PimEvent;
class QDateTime;
class Ir;
class QMessageBox;
class ExceptionDialog;
class EventView;
class FindDialog;

class DateBook : public DateBookGui
{
    friend class EventPicker;

    Q_OBJECT

public:
#if !defined(QTOPIA_DESKTOP)
    DateBook( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
#else
    DateBook();
#endif
    ~DateBook();


signals:
    void eventsChanged();
    void findDone(int);

protected:
    QDate currentDate();
    void timerEvent( QTimerEvent *e );
    void closeEvent( QCloseEvent *e );
    void init();

public slots:
    void flush();
    void reload();

    // some setting or env changed, data hasn't but how its displayed may have.
    void updateAlarms();
    void refreshWidgets();

protected slots:
    void fileNew();
    void showSettings();
    void slotToday();	// view today
    void changeClock();
    void changeWeek( bool newDay );
    void appMessage(const QCString& msg, const QByteArray& data);
    // handle key events in the day view...
    bool newEvent( const QString &str );
    void slotFind();
    void slotDoFind( const QString &, const QDate &, bool, bool, int );

    void viewDay();
    void viewDay(const QDate& dt);
    void viewWeek();
    void viewMonth();
    void nextView();

    void editOccurrence( const Occurrence &e );
    void editOccurrence( const Occurrence &e, bool preview );
    void removeOccurrence( const Occurrence &e );

    void editCurrentEvent();
    void removeCurrentEvent();

    void updateIcons();
    void setDocument( const QString & );
    void beamEvent( const PimEvent &e );

    void beamCurrentEvent();
    void beamDone( Ir *ir );

    void checkToday();

    void showEventDetails();
    void hideEventDetails();

    void slotPurge();

protected:
    bool newEvent(const QDateTime& dstart,const QDateTime& dend,const QString& description,const QString& notes);
    void viewToday();
    void viewWeek(const QDate& dt);
    void viewMonth(const QDate& dt);
    void addEvent( const PimEvent &e );
    void initDay();
    void initWeek();
    void initMonth();
    void initEvent();
    void initExceptionMb();
    virtual void loadSettings();
    void saveSettings();
    bool receiveFile( const QString &filename );

    bool eventSelected() const;
    PimEvent currentEvent() const;
    Occurrence currentOccurrence() const;

    void raiseWidget( QWidget *widget );

    void purgeEvents( const QDate &date, bool prompt = TRUE );
    QValueList<PimEvent> purge_getEvents( const QDateTime &from, const QDate &date );

    DateBookTable *db;
    QWidgetStack *views;
    DayView *dayView;
    WeekView *weekView;
    MonthView *monthView;
    EventView *eventView;
    bool aPreset;    // have everything set to alarm?
    int presetTime;  // the standard time for the alarm
    int startTime;
    bool ampm;
    bool onMonday;
    bool compressDay;

    bool checkSyncing();
    bool syncing;
    bool inSearch;
    QDate lastToday; // last date that was the selected as 'Today'
    QTimer *midnightTimer;

    QString checkEvent(const PimEvent &);

    ExceptionDialog *exceptionMb;
    QDateTime lastcall;
#ifdef Q_WS_QWS
    QString beamfile;
#endif

    FindDialog *findDialog;
};


#endif
