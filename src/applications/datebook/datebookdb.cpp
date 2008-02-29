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

#include "datebookdb.h"
#include <qtopia/global.h>
#include <qtopia/pim/private/eventxmlio_p.h>
#include <qlist.h>

QTime profiler;
int profileMark;

DateBookTable::DateBookTable(QObject *parent, const char *name)
    : QObject(parent, name)
{
    dba = new EventXmlIO(EventIO::ReadWrite);
//    connect(dba, SIGNAL(datebookUpdated()), this, SIGNAL(datebookUpdated()));
}

DateBookTable::~DateBookTable()
{
    dba->saveData();
    delete dba;
}


QValueList<Occurrence> DateBookTable::getOccurrences( 
	const QDate &from, const QDate &to ) const
{
    return dba->getOccurrences(from, to);
}

Occurrence DateBookTable::getNextAlarm( const QDateTime &from, bool *ok) const
{
    QValueList<Occurrence> list = getOccurrences(from.date(), from.date());

    // need to find the one that matches the 'when' we got...
    QValueListIterator<Occurrence> it;

    for (it = list.begin(); it != list.end(); ++it ) {
	if ((*it).startInTZ() == from && (*it).event().hasAlarm()) {
	    // the right event.
	    if (ok)
		*ok = TRUE;
	    return *it;
	}
    }

    if (ok)
	*ok = FALSE;
    return Occurrence();
}

Occurrence DateBookTable::find(const QRegExp &r, int category, const QDate &dt,
	bool ascend, bool *resultFound) const
{
    //return dba->find(text, dt, cs, ascend, category);
    // for now, do this in the datebook, fix later
    const QList<PimEvent> &list = (QList<PimEvent> &)(dba->events());

    // need to find the one that matches the 'when' we got...
    QListIterator<PimEvent> it(list);

    PimEvent p;
    for (; it.current(); ++it ) {
	if (category == -1) {
	    if (it.current()->categories().count() != 0)
		continue;
	} else if (category != -2) {
	    if (!(it.current()->categories().contains(category)))
		continue;
	} 
	// either the category matched, or isn't important, continue.
	if (!it.current()->description().contains(r))
	    continue;

	// category matched, description matched.. no check occurence.
	bool ok;
	QDate next = it.current()->nextOccurrence(dt.addDays(-1), &ok);
	while (ok) {
	    Occurrence o(next, *(it.current()));
	    if (o.startInTZ().date() >= dt) {
		if (resultFound)
		    *resultFound = TRUE;
		return o;
	    }
	    next = it.current()->nextOccurrence(
		    next.addDays(it.current()->start().daysTo(it.current()->end()) + 1), &ok);
	}
	// didn't return, no valid occurence after dt.
	// descend not yet implemented.
    }
    // nothing found...
    if (resultFound)
	*resultFound = FALSE;
    return Occurrence();
}

Occurrence DateBookTable::find(const QUuid &u, const QDate &date, bool *ok) const
{
    const QList<PimEvent> &list = (QList<PimEvent> &)(dba->events());

    // need to find the one that matches the 'when' we got...
    QListIterator<PimEvent> it(list);

    PimEvent *p;
    for (; it.current(); ++it ) {
	p = *it;
	if (u == p->uid()) {

	    bool nextOk;
	    QDate res = p->nextOccurrence(date, &nextOk);
	    if (nextOk) {
		if (ok)
		    *ok = TRUE;
		return Occurrence(res, *p);
	    } else 
		break;
	}
    }

    // nothing found...
    if (ok)
	*ok = FALSE;
    return Occurrence();

}
void DateBookTable::addEvent(PimEvent &ev)
{
    dba->addEvent(ev);
}

void DateBookTable::removeEvent(const PimEvent &e)
{
    dba->removeEvent(e);
}

// replace event &uid with ev (but keep the uid);
void DateBookTable::updateEvent(const PimEvent &event)
{
    dba->updateEvent(event);
}

/*  Slot called when datebook is informed by some 1.5 app that it
    has modified the datebook.  The PIM lib would recognized this, but not
    until you tried reloading the data, hence the signal
*/
void DateBookTable::externalAccess()
{
    dba->ensureDataCurrent(TRUE);
    emit datebookUpdated();
}

void DateBookTable::flush()
{
    dba->saveData();
}

void DateBookTable::reload()
{
    dba->ensureDataCurrent(TRUE);
}
