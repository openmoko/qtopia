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

#include <qfile.h>
#include <qasciidict.h>
#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qtopia/stringutil.h>
#include <qtopia/quuid.h>
#include <qfileinfo.h>
#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/timeconversion.h>
#include <qtopia/alarmserver.h>
#include <qapplication.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include "eventxmlio_p.h"
#include "event.h"

#ifdef Q_OS_WIN32
#include <process.h>
#else
#include <unistd.h>
#endif


// from time_t to QDateTime, but no timezone conversion.
QDateTime asQDateTime( time_t time )
{
    struct tm *lt;
#if defined(Q_WS_WIN32) || defined (Q_OS_WIN64)
    _tzset();
#else
    tzset();
#endif
    lt = gmtime( &time );
    QDateTime dt(
	    QDate( lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday ),
	    QTime( lt->tm_hour, lt->tm_min, lt->tm_sec ) );
    return dt;
}

time_t asTimeT( const QDateTime& dt )
{
    time_t tmp;
    struct tm *lt;

#if defined(Q_WS_WIN32) || defined (Q_OS_WIN64)
    _tzset();
#else
    tzset();
#endif

    // get a tm structure from the system to get the correct tz_name
    tmp = time( 0 );
    lt = gmtime( &tmp );

    lt->tm_sec = dt.time().second();
    lt->tm_min = dt.time().minute();
    lt->tm_hour = dt.time().hour();
    lt->tm_mday = dt.date().day();
    lt->tm_mon = dt.date().month() - 1; // 0-11 instead of 1-12
    lt->tm_year = dt.date().year() - 1900; // year - 1900
    //lt->tm_wday = dt.date().dayOfWeek(); ignored anyway
    //lt->tm_yday = dt.date().dayOfYear(); ignored anyway
    lt->tm_wday = -1;
    lt->tm_yday = -1;
    // tm_isdst negative -> mktime will find out about DST
    lt->tm_isdst = -1;
    // keep tm_zone and tm_gmtoff
    tmp = mktime( lt );
    return tmp;
}

EventXmlIO::EventXmlIO(AccessMode m,
		     const QString &file,
		     const QString &journal )
    : EventIO(m),
      PimXmlIO(PimEvent::keyToIdentifierMap(), PimEvent::identifierToKeyMap() ),
      needsSave(FALSE)
{
    if ( file != QString::null )
	setDataFilename( file );
    else setDataFilename( Global::applicationFileName( "datebook", "datebook.xml" ) );
    if ( journal != QString::null )
	setJournalFilename( journal );
    else setJournalFilename( Global::journalFileName( ".caljournal" ) );

    m_PrEvents.setAutoDelete(TRUE);

    loadData();

    /*
        When Outlook creates an exception to a repeating event, it copies the Qtopia UID into
        the exception. Older versions of Qtopia Desktop did not generate a new UID for the
        exception resulting in more than one event with the same UID.

        If we read in an event and it's an exception with a parentUID the same as it's UID
        then give it a new UID (and update the parent).

        We only do this on Qtopia Desktop because if both the PDA and Qtopia Desktop did this
        at the same time it would get nasty (due to the changing UIDs).
    */
#ifdef QTOPIA_DESKTOP
    if ( m != ReadOnly ) {
        QValueList<QUuid> parentUids;
        QValueList<PrEvent*> modified;
        PrEvent *cursor;

        // First go through the list and give the exceptions new UIDs
        for ( m_PrEvents.first(); (cursor = m_PrEvents.current()); m_PrEvents.next() ) {
            QUuid uid = cursor->uid();
            if ( cursor->isException() && uid == cursor->seriesUid() ) {
                if ( !parentUids.contains(uid) )
                    parentUids << uid;
                cursor->setUid( PimXmlIO::uuidFromInt( PimXmlIO::generateUid() ) );
                modified << cursor;
            }
        }

        // Then update the parents
        for ( m_PrEvents.first(); (cursor = m_PrEvents.current()); m_PrEvents.next() ) {
            QUuid uid = cursor->uid();
            if ( !parentUids.contains(uid) )
                continue;

            QValueList<QUuid> kids = cursor->childUids();
            while ( kids.contains(uid) ) {
                kids.remove(uid);
                cursor->removeChildUid(uid);
            }
            for ( QValueList<PrEvent*>::Iterator it = modified.begin(); it != modified.end(); it++ ) {
                PrEvent *ev = (*it);
                QUuid kuid = ev->uid();
                if ( ev->isException() && ev->seriesUid() == uid && !kids.contains(kuid) ) {
                    cursor->addChildUid( kuid );
                }
            }
        }

        needsSave = TRUE;
        saveData();
    }
#endif

    if (m == ReadOnly) {
#ifndef QT_NO_COP
	QCopChannel *channel = new QCopChannel( "QPE/PIM",  this );

	connect( channel, SIGNAL(received(const QCString&,const QByteArray&)),
		this, SLOT(pimMessage(const QCString&,const QByteArray&)) );
#endif
    }
}

void EventXmlIO::pimMessage(const QCString &message, const QByteArray &data)
{
    QDataStream ds(data, IO_ReadOnly);
    if (message == "addedEvent(int,PimEvent)") {
	int pid;
	PimEvent event;
	ds >> pid;
	ds >> event;
	if (pid != getpid()) {
	    internalAddRecord(new PimEvent(event));
	    emit eventsUpdated();
	}
    } else if (message == "removedEvent(int,PimEvent)") {
	int pid;
	PimEvent event;
	ds >> pid;
	ds >> event;
	if (pid != getpid()) {
	    internalRemoveRecord(new PimEvent(event));
	    emit eventsUpdated();
	}
    } else if (message == "updatedEvent(int,PimEvent)") {
	int pid;
	PimEvent event;
	ds >> pid;
	ds >> event;
	if (pid != getpid()) {
	    internalUpdateRecord( new PimEvent(event) );
	    emit eventsUpdated();
	}
    } else if (message == "reloadEvents()") {
        ensureDataCurrent();
    } else if ( message == "reload(int)" ) {
	int force;
	ds >> force;
        ensureDataCurrent(force);
    }
}

EventXmlIO::~EventXmlIO() {
}

bool EventXmlIO::loadData()
{
    if (PimXmlIO::loadData()) {
	emit eventsUpdated();
	return TRUE;
    }
    return FALSE;
}

bool EventXmlIO::internalAddRecord(PimRecord *rec)
{
    PrEvent *ev = (PrEvent *)rec;

    m_PrEvents.append( ev );
    return TRUE;
}

bool EventXmlIO::internalRemoveRecord(PimRecord *rec)
{
    PrEvent *ev = (PrEvent *)rec;

    for (m_PrEvents.first(); m_PrEvents.current(); m_PrEvents.next()) {
	if (m_PrEvents.current()->uid() == ev->uid()) {
	    m_PrEvents.remove();
	    delete ev;
	    return TRUE;
	}
    }
    delete ev;
    return FALSE;
}

bool EventXmlIO::internalUpdateRecord(PimRecord *rec)
{
    PrEvent *ev = (PrEvent *)rec;

    for (m_PrEvents.first(); m_PrEvents.current(); m_PrEvents.next()) {
	PrEvent *current = m_PrEvents.current();
	if (current->uid() == ev->uid()) {
	    if ( current != ev ) {
		*current = *ev;
		delete ev;
	    }
	    return TRUE;
	}
    }
    delete ev;
    return FALSE;
}

bool EventXmlIO::saveData()
{
    if ( !QFile::exists( dataFilename() ) || QFile::exists( journalFilename() ) )
	needsSave = TRUE;

    if (!needsSave)
	return TRUE;

    if (accessMode() != ReadOnly ) {
	if (PimXmlIO::saveData((QList<PimRecord> &)m_PrEvents)) {
	    needsSave = FALSE;
	    return TRUE;
	}
    }
    return FALSE;
}

QString EventXmlIO::recordToXml(const PimRecord *p)
{
    const PrEvent *e = (const PrEvent *)p;

    QMap<int,QString> data = p->fields();

    bool hasAlarm = e->hasAlarm();
    bool hasRepeat = e->hasRepeat();
    bool isException = e->isException();
    bool hasException = e->hasExceptions();

    const QMap<int,QCString> keyToIdentifier = PimEvent::keyToIdentifierMap();
    QString out;
    for ( QMap<int, QString>::ConstIterator fit = data.begin();
	    fit != data.end(); ++fit ) {

	int key = fit.key();
	if ( !hasAlarm && ( key == PimEvent::HasAlarm || key == PimEvent::SoundType || key == PimEvent::AlarmDelay) )
	    continue;
	if ( key == PimEvent::SoundType && fit.data() == "silent" ) // No tr ; skip default
	    continue;
	if ( key == PimEvent::AlarmDelay && fit.data() == "0" ) // skip default
	    continue;
	if ( !hasRepeat )  {
	    if (key == PimEvent::RepeatPattern || key == PimEvent::RepeatWeekdays || key == PimEvent::RepeatFrequency )
	    	continue;
	    if ( key == PimEvent::RepeatHasEndDate || key == PimEvent::RepeatEndDate )
		continue;
	}
	if ( !isException && key == PimEvent::RecordParent )
	    continue;

	if ( !hasException && ( key == PimEvent::RecordChildren || key == PimEvent::Exceptions ) )
	    continue;

	const QString &value = fit.data();
	if ( !value.isEmpty() ) {
	    QString k = keyToIdentifier[key];
	    if ( !k.isEmpty() ) { // else custom
		out += k;
		out += "=\"" + Qtopia::escapeString(value) + "\" ";
	    }
	}
    }

    out += customToXml( p );

    return out;
}

const QList<PrEvent>& EventXmlIO::events()
{
    ensureDataCurrent();
    return m_PrEvents;
}

PrEvent EventXmlIO::eventForId(const QUuid &u, bool *ok) const
{
    QListIterator<PrEvent> it(m_PrEvents);

    PrEvent *p;
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
    return PrEvent();
}

/*!
 Returns the events between \a from and \a to where \a from and \a to are in local time.
 */
QValueList<Occurrence> EventXmlIO::getOccurrencesInCurrentTZ(const QDate& from, const QDate& to) const
{
    QValueList<Occurrence> results;

    QListIterator<PrEvent> it(m_PrEvents);

    for (; it.current(); ++it ) {
	PrEvent e(*(it.current()));
	QDate date = from.addDays(-1);

	int duration = e.start().date().daysTo(e.end().date()) + 1;

	bool ok;
	date = e.nextOccurrence(date, &ok);
	while (ok && date <= to.addDays(1)) {
	    // loose check succeed, tight check on TZ now.
	    Occurrence o(date, e);
	    if (o.startInCurrentTZ().date() <= to && (o.endInCurrentTZ().addSecs(-1)) >= from)
		results.append(o);
	    date = e.nextOccurrence(date.addDays(duration), &ok);
	}
    }
    return results;
}

Occurrence EventXmlIO::getNextOccurrenceInCurrentTZ( const QDateTime& from, 
                                                     bool * ok ) const
{
    if( ok ) *ok = false;

    bool closestOccurValid = false;
    Occurrence closestOccur;

    for( QListIterator<PrEvent> iter = m_PrEvents; iter.current(); ++iter ) {
        PrEvent e(*(iter.current()));

        bool ocok = false;
        QDate date = e.nextOccurrence( from.date(), &ocok );
        if( ocok ) {
            Occurrence o(date, e);
            if( o.startInCurrentTZ() >= from ) {

                if( false == closestOccurValid ||
                    o.startInCurrentTZ() < closestOccur.startInCurrentTZ() ||
                    (o.startInCurrentTZ() == closestOccur.startInCurrentTZ() &&
                     o.event().uid().toString() < 
                     closestOccur.event().uid().toString()) ) {

                    closestOccur = o;
                    closestOccurValid = true;

                }

            }
        }
    }

    if( ok ) *ok = closestOccurValid;
    return closestOccur;
}

Occurrence EventXmlIO::getNextOccurrenceInCurrentTZ( const Occurrence& from, 
                                                     bool * ok ) const
{
    if( ok ) *ok = false;

    bool closestOccurValid = false;
    Occurrence closestOccur;

    for( QListIterator<PrEvent> iter = m_PrEvents; iter.current(); ++iter ) {
        PrEvent e(*(iter.current()));

        bool ocok = false;
        QDate date = e.nextOccurrence( from.date(), &ocok );
        if( ocok ) {
            Occurrence o(date, e);
            
            if( o.startInCurrentTZ() == from.startInCurrentTZ() && 
                o.event().uid().toString() <= from.event().uid().toString() ) 
                continue;

            if( o.startInCurrentTZ() >= from.startInCurrentTZ() &&
                (false == closestOccurValid || 
                 o.startInCurrentTZ() < closestOccur.startInCurrentTZ()) ) {
                closestOccur = o;
                closestOccurValid = true;
            }
        }
    }

    if( ok ) *ok = closestOccurValid;
    return closestOccur;
}

QUuid EventXmlIO::addEvent(const PimEvent &event, bool assignUid )
{
    QUuid u;
    if (accessMode() == ReadOnly)
	return u;

    // Don't add events that will never occur.
    if (!event.isValid())
	return u;

    ensureDataCurrent();

    PrEvent *ev = new PrEvent((const PrEvent &)event);

    if ( assignUid || ev->uid().isNull() )
	assignNewUid(ev);

    u = ev->uid();

    if (internalAddRecord(ev)) {
	needsSave = TRUE;
	if (ev->hasAlarm())
	    addEventAlarm(*ev);

	updateJournal(*ev, ACTION_ADD);

	{
#ifndef QT_NO_COP
	    QCopEnvelope e("QPE/PIM", "addedEvent(int,PimEvent)");
	    e << getpid();
	    e << *ev;
#endif
	}
    }
    return u;
}

void EventXmlIO::addException(const QDate &d, const PimEvent &p)
{
    if (accessMode() == ReadOnly)
	return;

    ensureDataCurrent();
    PrEvent parent((const PrEvent &)p);
    parent.addException(d);
    // don't add empty uid to parent.
    parent.addChildUid(QUuid());

    if (!parent.isValid()) {
	// apperently this is really a delete of the event.
	removeEvent(p);
	return;
    }

    if (internalUpdateRecord(new PrEvent(parent))) {
	needsSave = TRUE;

	updateJournal(parent, ACTION_REPLACE);

#ifndef QT_NO_COP
	{
	    QCopEnvelope e("QPE/PIM", "updatedEvent(int,PimEvent)");
	    e << getpid();
	    e << parent;
	}
#endif
    }
}

QUuid EventXmlIO::addException(const QDate &d, const PimEvent &p, const PimEvent& event)
{
    QUuid u;
    if (accessMode() == ReadOnly)
	return u;

    ensureDataCurrent();
    // don't need to check is valid as can't disapear by adding an exception.
    PrEvent parent((const PrEvent &)p);
    PrEvent *ev = new PrEvent((const PrEvent &)event);

    assignNewUid(ev);
    u = ev->uid();

    ev->setParentUid(parent.uid());
    parent.addException(d);
    parent.addChildUid(ev->uid());

    if (internalAddRecord(ev) && internalUpdateRecord(new PrEvent(parent))) {
	needsSave = TRUE;
	if (ev->hasAlarm())
	    addEventAlarm(*ev);

	updateJournal(*ev, ACTION_ADD);
	updateJournal(parent, ACTION_REPLACE);

#ifndef QT_NO_COP
	{
	    QCopEnvelope e("QPE/PIM", "addedEvent(int,PimEvent)");
	    e << getpid();
	    e << *ev;
	}
	{
	    QCopEnvelope e("QPE/PIM", "updatedEvent(int,PimEvent)");
	    e << getpid();
	    e << parent;
	}
#endif
    }
    return u;
}

void EventXmlIO::removeEvent(const PimEvent &event)
{

    if (accessMode() == ReadOnly)
	return;

    ensureDataCurrent();
    // in case parent becomes invalid.
    PimEvent parent;

    PrEvent *ev = new PrEvent((const PrEvent &)event);

    if (event.isException()) {
	for (m_PrEvents.first(); m_PrEvents.current(); m_PrEvents.next()) {
	    if (m_PrEvents.current()->uid() == ev->seriesUid()) {

		QValueList<QDate>::ConstIterator eit = m_PrEvents.current()->exceptions().begin();
		QValueList<QUuid>::ConstIterator cit = m_PrEvents.current()->childUids().begin();

		QDate tdate;
		for(; eit != m_PrEvents.current()->exceptions().end() && cit != m_PrEvents.current()->childUids().end();
			++eit, ++cit)
		{
		    if (*cit == ev->uid()) {
			tdate = *eit;
		    }
		}
		m_PrEvents.current()->removeException(ev->uid());
		m_PrEvents.current()->PimEvent::addException(tdate, QUuid());

		if (!m_PrEvents.current()->isValid()) {
		    parent = *(m_PrEvents.current());
		}
	    }
	}
    }

    for (m_PrEvents.first(); m_PrEvents.current(); m_PrEvents.next()) {
	if (m_PrEvents.current()->uid() == ev->uid()) {
	    if (m_PrEvents.current()->hasAlarm())
		delEventAlarm(*(m_PrEvents.current()));
	}
    }


    if (internalRemoveRecord(ev)) {
	needsSave = TRUE;
	updateJournal(event, ACTION_REMOVE);
	{
#ifndef QT_NO_COP
	    QCopEnvelope e("QPE/PIM", "removedEvent(int,PimEvent)");
	    e << getpid();
	    e << event;
#endif
	}
    }

    if (!parent.uid().isNull()) {
	// parent is invalid,
	removeEvent(parent);
    }
}

void EventXmlIO::updateEvent(const PimEvent &event)
{
    if (accessMode() == ReadOnly)
	return;

    ensureDataCurrent();

    if (!event.isValid()) {
	// really a remove.
	removeEvent(event);
	return;
    }

    PrEvent *ev = new PrEvent((const PrEvent&)event);
    
    for (m_PrEvents.first(); m_PrEvents.current(); m_PrEvents.next()) {
	if (m_PrEvents.current()->uid() == ev->uid()) {
	    if (m_PrEvents.current()->hasAlarm())
		delEventAlarm(*(m_PrEvents.current()));
	}
    }

    if (internalUpdateRecord(ev)) {
	needsSave = TRUE;
	if (event.hasAlarm())
	    addEventAlarm((const PrEvent &)event);

	updateJournal(event, ACTION_REPLACE);
	{
#ifndef QT_NO_COP
	    QCopEnvelope e("QPE/PIM", "updatedEvent(int,PimEvent)");
	    e << getpid();
	    e << event;
#endif
	}
    }
}

void EventXmlIO::ensureDataCurrent(bool forceReload)
{
    if (accessMode() == WriteOnly || ( isDataCurrent() && !forceReload) )
	return;

    // get rid of all events first
    QListIterator<PrEvent> it(m_PrEvents);
    for ( ; it.current(); ++it ) {
	if ( it.current()->hasAlarm() )
	    delEventAlarm( *(it.current()) );
    }

    m_PrEvents.clear();
    loadData();
}

bool EventXmlIO::nextAlarm( const PrEvent &ev, QDateTime& when, int& warn)
{
    QDateTime now = QDateTime::currentDateTime();
    bool ok;
    warn = ev.alarmDelay();

    // -1 days to make room for timezone shift.
    QDate next = ev.nextOccurrence(now.date().addDays(-1), &ok);
    while (ok) {
	Occurrence o(next, ev);
	// only need to check in TZ... want to shift in orig time
	if (now <= o.startInCurrentTZ().addSecs(-60*ev.alarmDelay())) {
	    when = o.startInCurrentTZ().addSecs(-60*ev.alarmDelay());
	    return TRUE;
	}
	next = ev.nextOccurrence(
		next.addDays(ev.start().daysTo(ev.end()) + 1), &ok);
    }
    return FALSE;
}

void EventXmlIO::addEventAlarm(const PrEvent &ev)
{
#ifdef Q_WS_QWS
    QDateTime when;
    int warn;
    if (nextAlarm(ev, when, warn)) {
	AlarmServer::addAlarm(when, "QPE/Application/datebook",
		"alarm(QDateTime,int)", warn);
    }
#else
    Q_UNUSED( ev );
#endif
}

void EventXmlIO::delEventAlarm(const PrEvent &ev)
{
#ifdef Q_WS_QWS
    QDateTime when;
    int warn;
    if ( nextAlarm(ev,when,warn) ) {
	AlarmServer::deleteAlarm( when, "QPE/Application/datebook",
		"alarm(QDateTime,int)", warn);
    }
#else
    Q_UNUSED( ev );
#endif
}
