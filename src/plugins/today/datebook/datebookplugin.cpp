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

#include "datebookplugin.h"
#include "datebookpluginoptions.h"

#include <qtopia/timestring.h>
#include <qtopia/config.h>
#include <qtopia/pim/datebookaccess.h>
#include <qtopia/pim/event.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/services.h>
#include <qtopia/quuid.h>
#include <qtopia/resource.h>

#include <qtl.h>

class ShortDateEvent
{
public:
    ShortDateEvent(void): repeating(FALSE) {}

    QString	description;
    QDateTime	start;
    QDateTime	end;
    QUuid	uid;
    bool	repeating;
    bool	allday;

    bool	operator<(ShortDateEvent &s) { return start < s.start; }
    bool	operator>(ShortDateEvent &s) { return start > s.start; }
    bool	operator<=(ShortDateEvent &s) { return start <= s.start; }
};

class DatebookPluginPrivate
{
public:
    DatebookPluginPrivate()
    {
	datebookAccess = new DateBookAccess();
	days = 1;
	limit = 0;
	readConfig();
    }
    
    ~DatebookPluginPrivate()
    {
	delete datebookAccess;
    }
    
    void readConfig()
    {
	Config config("datebookplugin");
	config.setGroup("view");
	days = config.readNumEntry("days", 1);
	limit = config.readNumEntry("limit", 3);
    }

    // max is currently unused.
    void getTaskList(uint /* max */)
    {
	events.clear();
	
	DateBookIterator it(*datebookAccess);
	QDate date = QDate::currentDate();
	QDate before = date;
	before = before.addDays( days );

	for ( ; it.current(); ++it) {
	    PimEvent event( *it.current() );

	    //
	    // Show events that occur between today and however many
	    // days in the future is requested (comes from setup).
	    // Take special note of recurring events.
	    //
	    if ((event.endInCurrentTZ().date() >= date && event.startInCurrentTZ().date() < before) ||
		    (event.hasRepeat() && event.nextOccurrence(date) < before)){
		ShortDateEvent	e;

		e.description = event.description();
		e.repeating = event.hasRepeat();
		e.start = event.startInCurrentTZ();
		e.end = event.endInCurrentTZ();
		e.uid = event.uid();
		e.allday = event.isAllDay();

		if (!e.repeating) {
		    events.append( e );
		} else {
		    //
		    // The first occurrence of this repeating event may have
		    // happened in the past.  If the event happens today, use
		    // the time of the event from the first occurrence.
		    //
		    if (event.nextOccurrence(date) == date) {
			e.start = date;
			e.start.setTime(event.startInCurrentTZ().time());
			events.append(e);
		    }

		    //
		    // Handle the rest of the recurrences.  Don't have
		    // to worry about the time, as we only show the day
		    // (if the event's not happening today).  Start checking
		    // occurrences from tomorrow, as we've already done
		    // today.  Check for post-today events to avoid wrapping.
		    //
		    QDate today = date.addDays(1);
		    bool post = TRUE;
		    while (today < before && post) {
			if (event.nextOccurrence(today, &post) < before) {
			    if (post) {
				e.start = event.nextOccurrence(today);
				events.append(e);
			    }
			}

			today = event.nextOccurrence(today, &post).addDays(1);
		    }
		}
	    }
	}

	qHeapSort(events);

	if ( !limit )
	    limit = events.count();
	else if ( limit > events.count() )
	    limit = events.count();
    }

    DateBookAccess *datebookAccess;
    QValueList<ShortDateEvent> events;
    int days;
    uint limit;
};

DatebookPlugin::DatebookPlugin(QObject *parent, const char *name)
	: QObject(parent, name)
{
    d = new DatebookPluginPrivate();
    connect( d->datebookAccess, SIGNAL( dateBookUpdated() ), this, SLOT( datebookUpdated() ) );
}

DatebookPlugin::~DatebookPlugin()
{
    delete d;
}

QString DatebookPlugin::name() const
{
    return tr("Calendar");
}

QPixmap DatebookPlugin::icon() const
{
    return Resource::loadPixmap("datebook/DateBook");
}

// lineHeight is currently unused.
QString DatebookPlugin::html(uint charWidth, uint /* lineHeight */) const
{
    QString status;

    uint eventCount = d->limit;
    d->getTaskList(eventCount);

    if ( d->events.count() == 1 ) {
	status = tr("You have 1 event");
    } else if ( d->events.count() > 1 ) {
	status = tr("You have %1 events").arg( d->events.count() );
    } else {
	status = tr("You have no events");
    }

    if ( d->days == 1 ) {
	status = tr("%1 today","eg. You have 1 event").arg(status);
    } else {
	status = tr("%1 over the next %2 days","eg. You have 5 events").arg(status).arg( d->days);
    }

    QString str;
    str = "<table> <tr> <td> <a href=\"raise:datebook\"><img src=\"datebook/DateBook\" alt=\"Calendar\"></a> </td>";
    str += "<td> <b> " + status + " </b> </td> </table>";

    if ( d->limit ) {
	str += " <table> ";
	for (uint i = 0; i < d->limit; i++ ) {
	    ShortDateEvent t = d->events[i];

	    QString when;
	    QString startTime = TimeString::localHM(t.start.time());
	    QString endTime = TimeString::localHM(t.end.time());
	    QDate today = QDate::currentDate();
	    QDate tomorrow = today.addDays(1);

	    if ( t.start.date() == QDate::currentDate() ) {
		if (t.allday) {
		    when = tr("All day");
		} else {
		    // use different strings to keep things translatable
		    if ( t.end.date() == today )
			when = tr("%1 to %2").arg(startTime).arg(endTime);
		    else if ( t.end.date() == tomorrow )
			when = tr("%1 to tomorrow, %2").arg(startTime).arg(endTime);
		    else {
			// within the next week, use the day name
			if (t.end.date() < today.addDays(6))
			    when = tr("%1 to %2, %3","eg 12pm to mon, 1pm").arg(startTime).arg(t.end.date().dayName(t.end.date().dayOfWeek())).arg(endTime);
			else {
			    if (today.year() != t.end.date().year())
				when = tr("%1 to %2 %3 %4, %5","eg 12pm to 1 feb 2015, 1pm").arg(startTime).arg(t.end.date().day()).arg(t.end.date().monthName(t.end.date().month())).arg(t.end.date().year()).arg(endTime);
			    else
				when = tr("%1 to %2 %3, %4","eg 12pm to 1 feb, 1pm").arg(startTime).arg(t.end.date().day()).arg(t.end.date().monthName(t.end.date().month())).arg(endTime);
			}
		    }
		}
	    } else if (t.repeating) {
		when = TimeString::localYMD(t.start.date());
	    } else if ( t.end.date() == QDate::currentDate() ) {
		when = tr("Ending %1").arg(endTime);
	    // this case doesnt seem be included
	    } else if ( t.start.date() > QDate::currentDate() ) {
		when = TimeString::localYMD( t.start.date() );
	    } else {	// Date within bounds
		when = tr("Finishes %1","time").arg(TimeString::localYMD( t.end.date() ));
	    }

	    QString desc = t.description;
	    int trunc = charWidth - when.length();
	    if ( trunc <= 0 ) {
	    	desc = "...";
	    } else {
		if ( desc.length() > uint(trunc) ) {
		    desc.truncate(trunc);
		    desc += "...";
		}
	    }
	    
	    str += "<tr> <td> <a href=\"qcop:" +
		    name() + QString(":%1\">").arg(i) + desc + "</a> "
		    "</td> <td> " + when + " </td> </tr> <tr> ";
	}
	str += " </tr> </table>";
    }

    return str;
}

QWidget* DatebookPlugin::widget(QWidget *parent)
{
    return new DatebookPluginOptions(parent, "Datebook plugin");
}

void DatebookPlugin::accepted(QWidget *w) const
{
    ( (DatebookPluginOptions *) w)->writeConfig();
    d->readConfig();
}

void DatebookPlugin::itemSelected(const QString &index) const
{
    QCopEnvelope e( Service::channel("Calendar"), "showEvent(QUuid,QDate)");
    e << d->events[ index.toInt() ].uid;
    e << d->events[ index.toInt() ].start.date();
}

void DatebookPlugin::datebookUpdated()
{
    d->readConfig();	//refresh limit
    emit reload();
}

