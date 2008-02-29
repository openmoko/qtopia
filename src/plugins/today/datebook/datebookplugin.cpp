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

#include "datebookplugin.h"
#include "datebookpluginoptions.h"

#include <qtopia/timestring.h>
#include <qtopia/config.h>
#include <qtopia/pim/datebookaccess.h>
#include <qtopia/pim/event.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/services/services.h>
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
	    if ((event.end().date() >= date && event.start().date() < before) ||
		    (event.hasRepeat() && event.nextOccurrence(date) < before)){
		ShortDateEvent	e;

		e.description = event.description();
		e.repeating = event.hasRepeat();
		e.start = event.start();
		e.end = event.end();
		e.uid = event.uid();

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
			e.start.setTime(event.start().time());
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

QPixmap DatebookPlugin::icon() const
{
    return Resource::loadPixmap("DateBook");
}

// lineHeight is currently unused.
QString DatebookPlugin::html(uint charWidth, uint /* lineHeight */) const
{
    QString status;

    uint eventCount = d->limit;
    d->getTaskList(eventCount);

    if ( d->events.count() ) {
	status = QString("You have %1 event%2 ").
	    arg( d->events.count() ).
	    arg((d->events.count() == 1) ? "" : "s");
    } else {
	status = "You have no events ";
    }

    if ( d->days == 1 ) {
	status += " today";
    } else {
	status += QString(" over the next %1 days").arg( d->days);
    }

    QString str;
    str = "<table> <tr> <td> <a href=\"raise:datebook\"><img src=\"DateBook\" alt=\"DateBook\"></a> </td>";
    str += "<td> <b> " + tr(status) + " </b> </td> </table>";

    if ( d->limit ) {
	str += " <table> ";
	for (uint i = 0; i < d->limit; i++ ) {
	    ShortDateEvent t = d->events[i];

	    QString when;
	    if ( t.start.date() == QDate::currentDate() ) {
		when = "Starting " + TimeString::timeString( t.start.time() );
	    } else if (t.repeating) {
		when = TimeString::longDateString(t.start.date());
	    } else if ( t.end.date() == QDate::currentDate() ) {
		when = "Ending " + TimeString::timeString( t.end.time() );
	    } else if ( t.start.date() > QDate::currentDate() ) {
		when = TimeString::longDateString( t.start.date() );
	    } else {	// Date within bounds
		when = "Finishes " + TimeString::longDateString( t.end.date() );
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
		    "</td> <td> " + tr(when) + " </td> </tr> <tr> ";
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
    QCopEnvelope e( Service::channel("datebook"), "showEvent(QUuid,QDate)");
    e << d->events[ index.toInt() ].uid;
    e << d->events[ index.toInt() ].start.date();
}

void DatebookPlugin::datebookUpdated()
{
    d->readConfig();	//refresh limit
    emit reload();
}

