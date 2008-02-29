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

#include "todoplugin.h"
#include "todopluginoptions.h"

#include <qtopia/timestring.h>
#include <qtopia/config.h>
#include <qtopia/pim/todoaccess.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/services.h>
#include <qtopia/resource.h>

#include <qtl.h>

class shortTask
{
public:
    QString description;
    int	    priority;
    QDate   dueDate;
    bool    dueSoon;
    QUuid   uuid;

    int operator<(const shortTask &t) const
    { return cmp(t) < 0; }
    int operator>(const shortTask &t) const
    { return cmp(t) > 0; }
    int operator<=(const shortTask &t) const
    { return cmp(t) <= 0; }

    int cmp(const shortTask &t) const
    {
	int r = 0;
	if ( dueSoon ) {
	    if ( t.dueSoon )
		r = t.dueDate.daysTo(dueDate);
	    else
		r = +1;
	} else if ( t.dueSoon )
	    r = -1;

	if ( !r ) {
	    r = priority - t.priority;
	    if ( !r ) {
		r = t.dueDate.daysTo(dueDate);
		if ( !r )
		    r = description.compare(t.description);
		if ( !r ) {
		    if ( uuid < t.uuid )
			r = -1;
		    else if ( uuid > t.uuid )
			r = +1;
		}
	    }
	}

	return r;
    }
};


class TodoPluginPrivate
{
public:
    TodoPluginPrivate()
    {
	days = 0;
	todoAccess = new TodoAccess();
	readConfig();
    }
    
    ~TodoPluginPrivate()
    {
	delete todoAccess;
    }
    
    void readConfig()
    {
	Config config("todoplugin");
	config.setGroup("view");
	selection = config.readNumEntry("selection", 0);
	days = config.readNumEntry("days", 1);

	if ( config.readBoolEntry("limit", TRUE) ) {
	    limit = config.readNumEntry("limitcount", 3);
	} else {
	    limit = 0;
	}
    }

    void getTaskList()
    {
	tasks.clear();
	
	TodoIterator it(*todoAccess);

	QDate before = QDate::currentDate();
	before = before.addDays( days );
	QDate later(9999,1,1);

	for ( ; it.current(); ++it) {
	    PimTask task( *it.current() );
	    if ( !task.isCompleted() ) {
		shortTask   stask;

		stask.description = task.description();
		stask.priority = task.priority();
		stask.dueDate = task.hasDueDate()
		    ? task.dueDate() : later;
		stask.uuid = task.uid();
		stask.dueSoon = FALSE;
		if ( selection==1 ) {
		    if ( task.hasDueDate() && task.dueDate() < before ) {
			stask.dueSoon = TRUE;
			tasks.append( stask );
		    }
		} else {
		    tasks.append( stask );
		}
	    }
	}

	qHeapSort(tasks);
    }

    TodoAccess *todoAccess;
    QValueList<shortTask> tasks;
    int selection, days;
    uint limit;
};

TodoPlugin::TodoPlugin(QObject *parent, const char *name)
    : QObject(parent, name)
{
    d = new TodoPluginPrivate();

    connect( d->todoAccess, SIGNAL(todoListUpdated() ),
	    this, SLOT(tasksUpdated() ) );
}

TodoPlugin::~TodoPlugin()
{
    delete d;
}

QString TodoPlugin::name() const
{
    return tr("Todo List");
}

QPixmap TodoPlugin::icon() const
{
    return Resource::loadPixmap("todolist/TodoList");
}

// lineHeight is currently unused.
QString TodoPlugin::html(uint charWidth, uint /* lineHeight */) const
{
    QString status;

    d->getTaskList();

    // We try to be logical with the substrings, to allow
    // translation. If we have failed we will need to verbosify.
    QString open,due;
    if ( d->selection == 0 ) {
	open = tr("open","inserted into: You have 'open' tasks");
	due = "";
    } else {
	open = "";
	if ( d->days == 1 ) {
	    due = tr("due today", "eg you have open tasks 'due today'");
	} else {
	    due = tr("due in the next %1 days", "eg you have 5 open tasks due in the next 5 days").arg(d->days);
	}
    }

    status = d->tasks.count() == 0
	    ? tr("You have no %1 tasks %2","eg. 1=\"open\", 2=\"due today\"").
		arg(open).arg(due)
	    : (d->tasks.count() == 1
		? tr("You have %1 %2 task %3","eg. 1 open")
		: tr("You have %1 %2 tasks %3","eg. 2 open"))
		.arg(d->tasks.count()).arg(open).arg(due);

    QString str;
    str = "<table> <tr> <td> <a href=\"raise:todolist\"><img src=\"todolist/TodoList\" alt=\"Todo\"></a> </td>";
    str += "<td> <b> " + status + " </b> </td> </tr> </table> ";

    if ( d->tasks.count() ) {
	str += " <table> <tr> ";
	for (uint i = 0; (!d->limit || i < d->limit) && i < d->tasks.count(); i++ ) {
	    shortTask t = d->tasks[i];

	    QString when;
	    if ( t.dueSoon ) {
		when = TimeString::localYMD( t.dueDate ); 
	    } else {
		when = tr("Priority %1").arg(t.priority);
	    }
	    
	    QString desc = t.description;
	    uint trunc = charWidth - when.length();
	    if ( trunc <= 0 ) {
	    	desc = "...";
	    } else {
		if ( desc.length() > trunc ) {
		    desc.truncate(trunc);
		    desc += "...";
		}
	    }
	    
	    str += "<td> <a href=\"qcop:" + name() + QString(":%1\">").arg(i) + desc + "</a> </td>";
	    if ( t.dueSoon ) {
		str += "<td> " + when + " </td>";
	    } else {
		str += "<td> " + when +" </td>";
	    }

	    str += " </tr> <tr> ";
	}
	str += " </tr> </table>";
    }

    return str;
}

QWidget* TodoPlugin::widget(QWidget *parent)
{
    return new TodoPluginOptions(parent, "Todo plugin");
}

void TodoPlugin::accepted(QWidget *w) const
{
    ( (TodoPluginOptions *) w)->writeConfig();
    d->readConfig();
}

void TodoPlugin::itemSelected(const QString &index) const
{
    QCopEnvelope e( Service::channel("Tasks"), "showTask(QUuid)");
    e << d->tasks[ index.toInt() ].uuid;
}

void TodoPlugin::tasksUpdated()
{
    d->readConfig();  //we really only need the limit paramater
    emit reload();
}

