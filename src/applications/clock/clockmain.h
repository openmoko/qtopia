/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef CLOCKMAINWINDOW_H
#define CLOCKMAINWINDOW_H

#include <qwidget.h>
#include <qtopiaabstractservice.h>

class Clock;
class StopWatch;
class Alarm;
class QTabWidget;
class QMenu;
class QAction;

class ClockMain : public QWidget
{
    Q_OBJECT
public:
    ClockMain( QWidget *parent=0, Qt::WFlags fl=0 );
    ~ClockMain();

private slots:
    void appMessage(const QString& msg, const QByteArray& data);

public slots:
    void showClock();
    void editAlarm();
    void setDailyEnabled( bool enable );
    void setTime();

protected:
    void closeEvent(QCloseEvent *);

private:
    QTabWidget *tabWidget;

    Clock *clock;
    StopWatch *stopWatch;
    Alarm *alarm;
    QMenu *contextMenu;

    int clockIndex;
    int stopwatchIndex;
    int alarmIndex;
};

class ClockService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class ClockMain;
private:
    ClockService( ClockMain *parent )
        : QtopiaAbstractService( "Clock", parent )
        { this->parent = parent; publishAll(); }

public:
    ~ClockService();

public slots:
    void showClock();

private:
    ClockMain *parent;
};

class AlarmService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class ClockMain;
private:
    AlarmService( ClockMain *parent )
        : QtopiaAbstractService( "Alarm", parent )
        { this->parent = parent; publishAll(); }

public:
    ~AlarmService();

public slots:
    void editAlarm();
    void setDailyEnabled( int flag );

private:
    ClockMain *parent;
};

#endif

