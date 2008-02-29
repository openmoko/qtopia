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

#include "clock.h"
#include "stopwatch.h"
#include "alarm.h"
#include "analogclock.h"
#include "clockmain.h"

#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#include <qsettings.h>
#include <qtimestring.h>
#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif

#include <qtabwidget.h>
#include <qstackedwidget.h>
#include <qmessagebox.h>
#include <qmenu.h>

static void toggleScreenSaver( bool on )
{
    QtopiaApplication::setPowerConstraint(on ? QtopiaApplication::Enable : QtopiaApplication::DisableSuspend);
}

ClockMain::ClockMain(QWidget *parent, Qt::WFlags f)
    : QWidget(parent, f)
    , clockIndex(-1)
    , stopwatchIndex(-1)
    , alarmIndex(-1)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

#ifndef QTOPIA_PHONE
    tabWidget = new QTabWidget(this);
    layout->addWidget(tabWidget);
    clock = new Clock(tabWidget);
    stopWatch = new StopWatch(tabWidget);
    alarm = new Alarm(tabWidget);

    // Add each widget to the tabbed widget, and save the index for each so that
    // we can refer to it later.
    clockIndex = tabWidget->addTab(clock, tr("Clock"));
    stopwatchIndex = tabWidget->addTab(stopWatch, tr("Stop Watch"));
    alarmIndex = tabWidget->addTab(alarm, tr("Alarm"));

    connect(tabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(pageRaised(int)));
#else
    stack = new QStackedWidget(this);
    layout->addWidget(stack);
    clock = new Clock(stack);
    stopWatch = new StopWatch(stack);
    alarm = new Alarm(stack);

    // Add each widget to the stacked widget, and save the index for each so that
    // we can refer to it later.
    clockIndex = stack->addWidget(clock);
    stopwatchIndex = stack->addWidget(stopWatch);
    alarmIndex = stack->addWidget(alarm);
    connect(stack, SIGNAL(currentChanged(int)), this, SLOT(pageRaised(int)));

    contextMenu = QSoftMenuBar::menuFor(this);
    connect(contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(raisePage(QAction*)));
    actionClock = contextMenu->addAction(tr("Clock"));
    actionStopWatch = contextMenu->addAction(tr("Stop Watch"));
    actionAlarm = contextMenu->addAction(QIcon(":icon/alarmbell"), tr("Alarm"));
    stack->setCurrentIndex(clockIndex);

    QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);
#endif

    connect( qApp, SIGNAL(appMessage(const QString&,const QByteArray&)),
            this, SLOT(appMessage(const QString&,const QByteArray&)) );
    new ClockService(this);
    new AlarmService(this);

    setWindowTitle(tr("Clock"));
}

ClockMain::~ClockMain()
{
    toggleScreenSaver( true );
}

void ClockMain::showClock()
{
#ifndef QTOPIA_PHONE
    tabWidget->setCurrentIndex(clockIndex);
#else
    stack->setCurrentIndex(clockIndex);
#endif
}

void ClockMain::editAlarm()
{
#ifndef QTOPIA_PHONE
    tabWidget->setCurrentIndex(alarmIndex);
#else
    stack->setCurrentIndex(alarmIndex);
#endif
}

void ClockMain::setDailyEnabled( bool enable )
{
    alarm->setDailyEnabled( enable );
}

void ClockMain::appMessage( const QString &msg, const QByteArray &data )
{
    if ( msg == "alarm(QDateTime,int)" ) {
        QDataStream ds(data);
        QDateTime when;
        int t;
        ds >> when >> t;
        alarm->triggerAlarm(when, t);
    }
}

void ClockMain::raisePage(QAction *a)
{
#ifdef QTOPIA_PHONE
    int page = clockIndex;
    if (a == actionStopWatch)
        page = stopwatchIndex;
    else if (a == actionAlarm)
        page = alarmIndex;
    if (stack->currentWidget() != stack->widget(page))
        stack->setCurrentIndex(page);
#else
    Q_UNUSED(a);
#endif
}

void ClockMain::pageRaised(int idx)
{
    if (idx == clockIndex)
        setObjectName("clock"); // No tr
    else if (idx == alarmIndex) {
        setObjectName("alarms"); // No tr
    }
    else if (idx == stopwatchIndex)
        setObjectName("stopwatch"); // No tr
}

void ClockMain::closeEvent( QCloseEvent *e )
{
    if (!alarm->isValid()) {
        QMessageBox::warning(this, tr("Select Day"),
            tr("<qt>Daily alarm requires at least one day to be selected.</qt>"));
    } else {
        QWidget::closeEvent(e);
    }
}

/*!
    \service ClockService Clock
    \brief Provides the Qtopia Clock service.

    The \i Clock service enables applications to pop up the clock on
    the user's display by sending it a \c{showClock()} message.

    Client applications can request the \i Clock service with the
    following code:

    \code
    QtopiaServiceRequest req( "Clock", "showClock()" );
    req.send();
    \endcode

    \sa AlarmService
*/

/*!
    \internal
*/
ClockService::~ClockService()
{
}

/*!
    Instruct the \i Clock service to pop up the clock on the
    user's display.

    This slot corresponds to the QCop service message \c{Clock::showClock()}.
*/
void ClockService::showClock()
{
    parent->showClock();
}

/*!
    \service AlarmService Alarm
    \brief Provides the Qtopia Alarm service.

    The \i Alarm service enables applications to pop up a dialog on
    the user's display to edit the alarm time.

    Client applications can request the \i Alarm service with the
    following code:

    \code
    QtopiaServiceRequest req( "Alarm", "editAlarm()" );
    req.send();
    \endcode

    \sa ClockService
*/

/*!
    \internal
*/
AlarmService::~AlarmService()
{
}

/*!
    Instruct the \i Alarm service to pop up a dialog box that permits
    editing of the alarm.

    This slot corresponds to the QCop service message \c{Alarm::editAlarm()}.
*/
void AlarmService::editAlarm()
{
    parent->editAlarm();
}

/*!
    Instruct the \i Alarm service to enable or disable the daily alarm
    according to \a flag.

    This slot corresponds to the QCop service message
    \c{Alarm::setDailyEnabled(int)}.
*/
void AlarmService::setDailyEnabled( int flag )
{
    parent->setDailyEnabled( flag != 0 );
}
