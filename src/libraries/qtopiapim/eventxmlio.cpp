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
** Contact info\@trolltech.com if any conditions of this licensing are
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

    if (m == ReadOnly) {

#ifndef QT_NO_COP
	QCopChannel *channel = new QCopChannel( "QPE/PIM",  this );

	connect( channel, SIGNAL(received(const QCString&, const QByteArray&)),
		this, SLOT(pimMessage(const QCString&, const QByteArray&)) );

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
	    internalUpdateRecord(new PimEvent(event));
	    emit eventsUpdated();
	}
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
	    m_PrEvents.remove();
	    m_PrEvents.append(ev);
	    return TRUE;
	}
    }
    delete rec;
    return FALSE;
}

bool EventXmlIO::saveData()
{
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
	if ( !hasAlarm && ( key == PimEvent::HasAlarm || key == PimEvent::SoundType) || key == PimEvent::AlarmDelay)
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
	    out += keyToIdentifier[key];
	    out += "=\"" + Qtopia::escapeString(value) + "\" ";
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

/*!
 Returns the events between \a from and \a to where \a from and \a to are in local time.
 */
QValueList<Occurrence> EventXmlIO::getOccurrences(const QDate& from, const QDate& to) const
{
    QString localZone = QString::fromLocal8Bit( getenv( "TZ" ) );
    return getOccurrencesInTZ(from, to, localZone);
}

/*!
 Returns the events between \a from and \a to where \a from and \a to are in
 in the time zone \a zone.
 */
QValueList<Occurrence> EventXmlIO::getOccurrencesInTZ(const QDate& from, const QDate& to, const QString &zone) const
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
	    if (o.startInTZ(zone).date() <= to && o.endInTZ(zone) >= from)
		results.append(o);
	    date = e.nextOccurrence(date.addDays(duration), &ok);
	}
    }
    return results;
}

void EventXmlIO::addEvent(const PimEvent &event, bool assignUid )
{
    if (accessMode() == ReadOnly)
	return;

    PrEvent *ev = new PrEvent((const PrEvent &)event);

    if ( assignUid || ev->uid().isNull() )
	assignNewUid(ev);

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
}

void EventXmlIO::addException(const QDate &d, const PimEvent &p)
{
    if (accessMode() == ReadOnly)
	return;

    PrEvent *parent = new PrEvent((const PrEvent &)p);
    parent->addException(d);
    parent->addChildUid(QUuid());

    if (internalUpdateRecord(parent)) {
	needsSave = TRUE;

	updateJournal(*parent, ACTION_REPLACE);

#ifndef QT_NO_COP
	{
	    QCopEnvelope e("QPE/PIM", "updatedEvent(int,PimEvent)");
	    e << getpid();
	    e << *parent;
	}
#endif
    }
}

void EventXmlIO::addException(const QDate &d, const PimEvent &p, const PimEvent& event)
{
    if (accessMode() == ReadOnly)
	return;

    PrEvent *parent = new PrEvent((const PrEvent &)p);
    PrEvent *ev = new PrEvent((const PrEvent &)event);

    assignNewUid(ev);

    ev->setParentUid(parent->uid());
    parent->addException(d);
    parent->addChildUid(ev->uid());

    if (internalAddRecord(ev) && internalUpdateRecord(parent)) {
	needsSave = TRUE;
	if (ev->hasAlarm())
	    addEventAlarm(*ev);

	updateJournal(*ev, ACTION_ADD);
	updateJournal(*parent, ACTION_REPLACE);

#ifndef QT_NO_COP
	{
	    QCopEnvelope e("QPE/PIM", "addedEvent(int,PimEvent)");
	    e << getpid();
	    e << *ev;
	}
	{
	    QCopEnvelope e("QPE/PIM", "updatedEvent(int,PimEvent)");
	    e << getpid();
	    e << *parent;
	}
#endif
    }
}

void EventXmlIO::removeEvent(const PimEvent &event)
{

    if (accessMode() == ReadOnly)
	return;

    PrEvent *ev = new PrEvent((const PrEvent &)event);

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
}

void EventXmlIO::updateEvent(const PimEvent &event)
{
    if (accessMode() == ReadOnly)
	return;

    PrEvent *ev = new PrEvent((const PrEvent &)event);

    for (m_PrEvents.first(); m_PrEvents.current(); m_PrEvents.next()) {
	if (m_PrEvents.current()->uid() == ev->uid()) {
	    if (m_PrEvents.current()->hasAlarm())
		delEventAlarm(*(m_PrEvents.current()));
	}
    }

    if (internalUpdateRecord(ev)) {
	needsSave = TRUE;
	if (ev->hasAlarm())
	    addEventAlarm(*ev);

	updateJournal(*ev, ACTION_REPLACE);
	{
#ifndef QT_NO_COP
	    QCopEnvelope e("QPE/PIM", "updatedEvent(int,PimEvent)");
	    e << getpid();
	    e << *ev;
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
    QString where = QString::fromLocal8Bit( getenv("TZ") );
    bool ok;
    warn = ev.alarmDelay();

    // -1 days to make room for timezone shift.
    QDate next = ev.nextOccurrence(now.date().addDays(-1), &ok);
    while (ok) {
	Occurrence o(next, ev);
	// only need to check in TZ... want to shift in orig time
	if (now <= o.startInTZ(where).addSecs(-60*ev.alarmDelay())) {
	    when = o.startInTZ(where).addSecs(-60*ev.alarmDelay());
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
#endif
}
