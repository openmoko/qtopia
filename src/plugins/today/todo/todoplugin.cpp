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

#include "todoplugin.h"
#include "todopluginoptions.h"

#include <qtopia/timestring.h>
#include <qtopia/config.h>
#include <qtopia/pim/todoaccess.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/services/services.h>
#include <qtopia/resource.h>

#include <qtl.h>

class shortTask
{
public:
    QString description;
    int	    priority;
    QDate   dueDate;
    bool    hasDueDate;
    QUuid   uuid;

    bool operator<(shortTask &t)  { return priority < t.priority; }
    bool operator>(shortTask &t)  { return priority > t.priority; }
    bool operator<=(shortTask &t) { return priority <= t.priority; }
};


class TodoPluginPrivate
{
public:
    TodoPluginPrivate()
    {
	days = 0;
	limit = 0;
	
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
	    limit = config.readNumEntry("limitcount", 3);	//+1 since 0 is not an option
	}
    }

    // max is currently unused.
    void getTaskList(uint /* max */)
    {
	tasks.clear();
	
	TodoIterator it(*todoAccess);
	for ( ; it.current(); ++it) {
	    PimTask task( *it.current() );
	    if ( !task.isCompleted() ) {
		shortTask   stask;

		stask.description = task.description();
		stask.priority = task.priority();
		stask.dueDate = task.dueDate();
		stask.hasDueDate = FALSE;
		stask.uuid = task.uid();

		switch(selection) {
		case 0:
		    tasks.append(stask);
		    break;

		case 1:
		    if ( task.hasDueDate() ) {
			QDate before = QDate::currentDate();
			before = before.addDays( days );

			//
			// Assign priority 1 if we have a due date that
			// satisfies our selection criteria.  This moves
			// it to the top of the list.
			//
			if ( task.dueDate() < before ) {
			    stask.hasDueDate = TRUE;
			    stask.priority = 1;
			    tasks.append( stask );
			}
		    }
		    break;

		default:
		    tasks.append( stask );
		    break;
		}
	    }
//	    if ( tasks.count() >= max )	//need count function in qtopiapim lib
//		break;
	}

	qHeapSort(tasks);
	
	if ( !limit )
	    limit = tasks.count();
	else if ( limit > tasks.count() )
	    limit = tasks.count();
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

QPixmap TodoPlugin::icon() const
{
    return Resource::loadPixmap("TodoList");
}

// lineHeight is currently unused.
QString TodoPlugin::html(uint charWidth, uint /* lineHeight */) const
{
    QString status;

    d->getTaskList( d->limit );

    if (d->tasks.count() > 0) {
	status = QString("You have %1 ").arg(d->tasks.count());
    } else {
	status = "You have no ";
    }

    if (d->selection == 0) {
	status += "open tasks ";
    } else if (d->selection == 1) {
	status += QString("task%1 ").arg(d->tasks.count() != 1 ? "s" : "");

	if (d->days == 1) {
	    status += "due today";
	} else {
	    status += "due the next " + QString::number(d->days) + " days";
	}
    }

    QString str;
    str = "<table> <tr> <td> <a href=\"raise:todolist\"><img src=\"TodoList\" alt=\"Todo\"></a> </td>";
    str += "<td> <b> " + tr(status) + " </b> </td> </tr> </table> ";

    
    if ( d->tasks.count() ) {
	str += " <table> <tr> ";
	for (uint i = 0; i < d->limit; i++ ) {
	    shortTask t = d->tasks[i];

	    QString when;
	    if ( t.hasDueDate ) {
		when = TimeString::longDateString( t.dueDate ); 
	    } else {
		when = QString("Priority %1").arg(t.priority);
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
	    if ( t.hasDueDate ) {
		str += "<td> " + tr(when) + " </td>";
	    } else {
		str += "<td> " + tr(when) +" </td>";
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
    QCopEnvelope e( Service::channel("todolist"), "showTask(QUuid)");
    e << d->tasks[ index.toInt() ].uuid;
}

void TodoPlugin::tasksUpdated()
{
    d->readConfig();  //we really only need the limit paramater
    emit reload();
}

