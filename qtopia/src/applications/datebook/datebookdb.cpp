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

#include "datebookdb.h"
#include <qtopia/global.h>
#include <qtopia/pim/private/eventxmlio_p.h>
#include <qlist.h>
#include <qmessagebox.h>

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
    if ( !dba->saveData() )
	QMessageBox::information( 0, tr( "Calendar" ),
		tr("<qt>Device full.  Some changes may not be saved.</qt>"));

    delete dba;
}


QValueList<Occurrence> DateBookTable::getOccurrences( 
	const QDate &from, const QDate &to ) const
{
    return dba->getOccurrencesInCurrentTZ(from, to);
}

QValueList<Occurrence> DateBookTable::getNextAlarm( const QDateTime &when, int warn) const
{
    QDateTime from = when.addSecs(60*warn);
    QValueList<Occurrence> list = getOccurrences(from.date(), from.date());
    QValueList<Occurrence> res;

    // need to find the one that matches the 'when' we got...
    QValueListIterator<Occurrence> it;

    for (it = list.begin(); it != list.end(); ++it ) {
	if ((*it).startInCurrentTZ() == from && (*it).event().hasAlarm()
		&& (*it).event().alarmDelay() == warn ) {
	    // the right event.
	    // put the audiable ones first (sort of like priority)
	    if ((*it).event().alarmSound() == PimEvent::Loud)
		res.prepend(*it);
	    else 
		res.append(*it);
	}
    }

    return res;
}

void DateBookTable::updateAlarm(const PimEvent &e) 
{
  dba->addEventAlarm((const PrEvent &)e);
}

void DateBookTable::updateAlarms()
{
    const QList<PimEvent> &list = (QList<PimEvent> &)(dba->events());

    // need to find the one that matches the 'when' we got...
    QListIterator<PimEvent> it(list);

    PimEvent p;
    for (; it.current(); ++it ) {
	updateAlarm(*(it.current()));
    }
}

Occurrence DateBookTable::find(const QRegExp &r, int category, const QDate &dt,
	bool, bool *success) const
{
    //return dba->find(text, dt, cs, ascend, category);
    // for now, do this in the datebook, fix later
    const QList<PimEvent> &list = (QList<PimEvent> &)(dba->events());

    // need to find the one that matches the 'when' we got...
    QListIterator<PimEvent> it(list);

    PimEvent p;
    Occurrence bestMatch;
    bool resultFound = FALSE;
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

	// category matched, description matched.. now check occurence.
	bool ok;
	QDate next = it.current()->nextOccurrence(dt.addDays(-1), &ok);
	while (ok) {
	    Occurrence o(next, *(it.current()));
	    if (o.startInCurrentTZ().date() >= dt) {
		if (!resultFound || o.startInCurrentTZ() < bestMatch.startInCurrentTZ())  {
		    resultFound = TRUE;
		    bestMatch = o;
		}
		break;
	    }
	    next = it.current()->nextOccurrence(
		    next.addDays(it.current()->start().daysTo(it.current()->end()) + 1), &ok);
	}
	// didn't return, no valid occurence after dt.
	// descend not yet implemented.
    }
    if (success)
	*success = resultFound;
    if (resultFound)
	return bestMatch;
    else
	return Occurrence();
}

PimEvent DateBookTable::find(const QUuid &u, bool *ok) const
{
    const QList<PimEvent> &list = (QList<PimEvent> &)(dba->events());

    QListIterator<PimEvent> it(list);

    PimEvent *p;
    for (; it.current(); ++it ) {
	p = *it;
	if (u == p->uid()) {
	    if (ok)
		*ok = TRUE;
	    return *p;
	}
    }

    if (ok)
	*ok = FALSE;
    return PimEvent();
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
QUuid DateBookTable::addEvent(const PimEvent &ev)
{
    return dba->addEvent(ev);
}

void DateBookTable::addException(const QDate &d, const PimEvent &parent)
{
    dba->addException(d, parent);
}

QUuid DateBookTable::addException(const QDate &d, const PimEvent &parent, const PimEvent &ev)
{
    return dba->addException(d, parent, ev);
}

void DateBookTable::removeExceptions(const PimEvent &e)
{
    // and also e's children
    const QValueList<QUuid> &list = ((PrEvent &)e).childUids();
    QValueList<QUuid>::ConstIterator it = list.begin();
    for (;it != list.end(); ++it) {
	qDebug("remi %ld", (*it).data1);
	//PimEvent p = find(((PrEvent &)e).parentUid());
	PimEvent p = find(*it);
	dba->removeEvent(p);
    }
}

void DateBookTable::removeEvent(const PimEvent &e)
{
    dba->removeEvent(e);
    if (e.isException()) {
	PimEvent p = find(e.seriesUid());
	p.removeException(e.uid());
	dba->updateEvent(e);
    } else if (e.hasExceptions()) {
	removeExceptions(e);
    }
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
