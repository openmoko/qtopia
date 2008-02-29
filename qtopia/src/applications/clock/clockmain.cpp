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

#include "clock.h"
#include "stopwatch.h"
#include "alarm.h"
#include "analogclock.h"
#include "clockmain.h"

#include <qtopia/qpeapplication.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/config.h>
#include <qtopia/timestring.h>
#include <qtopia/alarmserver.h>
#include <qtopia/sound.h>
#include <qtopia/resource.h>
#ifdef QTOPIA_PHONE
#include <qtopia/contextmenu.h>
#endif

#include <qtabwidget.h>
#include <qwidgetstack.h>
#include <qmessagebox.h>

static void toggleScreenSaver( bool on )
{
    QPEApplication::setTempScreenSaverMode(on ? QPEApplication::Enable : QPEApplication::DisableSuspend);  
}

ClockMain::ClockMain(QWidget *parent, const char *name, WFlags f)
    : QVBox(parent, name, f)
{
#ifndef QTOPIA_PHONE
    tabWidget = new QTabWidget(this);
    clock = new Clock(tabWidget);
    stopWatch = new StopWatch(tabWidget);
    alarm = new Alarm(tabWidget);

    tabWidget->addTab(clock, tr("Clock"));
    tabWidget->addTab(stopWatch, tr("Stop Watch"));
    tabWidget->addTab(alarm, tr("Alarm"));

    connect(tabWidget, SIGNAL(currentChanged(QWidget*)),
	    this, SLOT(pageRaised(QWidget*)));
#else
    stack = new QWidgetStack(this);
    clock = new Clock(stack);
    stopWatch = new StopWatch(stack);
    alarm = new Alarm(stack);

    stack->addWidget(clock, 0);
    stack->addWidget(stopWatch, 1);
    stack->addWidget(alarm, 2);
    connect(stack, SIGNAL(aboutToShow(QWidget*)),
	    this, SLOT(pageRaised(QWidget*)));

    contextMenu = new ContextMenu(this);
    connect(contextMenu, SIGNAL(activated(int)), this, SLOT(raisePage(int)));
    contextMenu->insertItem(tr("Clock"), 0);
    contextMenu->insertItem(tr("Stop Watch"), 1);
    contextMenu->insertItem(tr("Alarm"), 2);
    stack->raiseWidget(0);
#endif

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    connect( qApp, SIGNAL(appMessage(const QCString&,const QByteArray&)),
	    this, SLOT(appMessage(const QCString&,const QByteArray&)) );
#endif

    setCaption(tr("Clock"));
}

ClockMain::~ClockMain()
{
    toggleScreenSaver( true );
}

void ClockMain::appMessage( const QCString &msg, const QByteArray &data )
{
    if ( msg == "alarm(QDateTime,int)" ) {
	QDataStream ds(data,IO_ReadOnly);
	QDateTime when;
	int t;
	ds >> when >> t;
	alarm->triggerAlarm(when, t);
    } else if ( msg == "setDailyEnabled(int)" ) {
	QDataStream ds(data,IO_ReadOnly);
	int enableDaily;
	ds >> enableDaily;
	alarm->setDailyEnabled(enableDaily);
    } else if ( msg == "editAlarm()" || msg == "editDailyAlarm()" ) {
#ifndef QTOPIA_PHONE
	tabWidget->setCurrentPage(2);
#else
	stack->raiseWidget(2);
#endif
	QPEApplication::setKeepRunning();
    } else if (msg == "showClock()") {
	qDebug("showclock");
#ifndef QTOPIA_PHONE
	tabWidget->setCurrentPage(0);
#else
	stack->raiseWidget(0);
#endif
	QPEApplication::setKeepRunning();
    }
}

void ClockMain::raisePage(int p)
{
#ifdef QTOPIA_PHONE
    if (stack->visibleWidget() != stack->widget(p)) {
	stack->raiseWidget(p);
    }
#else
    Q_UNUSED(p);
#endif
}

void ClockMain::pageRaised(QWidget *w)
{
    if (w == clock)
	setName("clock"); // No tr
    else if (w == alarm)
	setName("alarm"); // No tr
    else if (w == stopWatch)
	setName("stopwatch"); // No tr
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

