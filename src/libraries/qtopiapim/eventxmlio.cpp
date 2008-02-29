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
#include <qpe/config.h>
#include <qpe/global.h>
#include <qpe/stringutil.h>
#include <qpe/quuid.h>
#include <qfileinfo.h>
#include <qpe/qcopenvelope_qws.h>
#include <qpe/timeconversion.h>
#include <qpe/alarmserver.h>
#include <qapplication.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include "eventxmlio_p.h"
#include "event.h"


// from time_t to QDateTime, but no timezone conversion.
QDateTime asQDateTime( time_t time )
{
    struct tm *lt;
#if defined(_OS_WIN32) || defined (Q_OS_WIN32) || defined (Q_OS_WIN64)
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

#if defined(_OS_WIN32) || defined (Q_OS_WIN32) || defined (Q_OS_WIN64)
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


EventXmlIO::EventXmlIO(AccessMode m) : EventIO(m), dict(20), needsSave(FALSE)
{
    m_PrEvents.setAutoDelete(TRUE);

    dict.setAutoDelete( TRUE );
    dict.insert( "description", new int(FDescription) );
    dict.insert( "location", new int(FLocation) );
    dict.insert( "categories", new int(FCategories) );
    dict.insert( "uid", new int(FUid) );
    dict.insert( "type", new int(FType) );
    dict.insert( "alarm", new int(FAlarm) );
    dict.insert( "sound", new int(FSound) );
    dict.insert( "rtype", new int(FRType) );
    dict.insert( "rweekdays", new int(FRWeekdays) );
    dict.insert( "rposition", new int(FRPosition) );
    dict.insert( "rfreq", new int(FRFreq) );
    dict.insert( "rhasenddate", new int(FRHasEndDate) );
    dict.insert( "enddt", new int(FREndDate) );
    dict.insert( "start", new int(FRStart) );
    dict.insert( "end", new int(FREnd) );
    dict.insert( "note", new int(FNote) );
    dict.insert( "created", new int(FCreated) );
    dict.insert( "timezone", new int (FTimeZone) );

    ensureDataCurrent();

    if (m == ReadOnly) {
	QCopChannel *channel = new QCopChannel( "QPE/PIM",  this );

	connect( channel, SIGNAL(received(const QCString&, const QByteArray&)),
		this, SLOT(pimMessage(const QCString&, const QByteArray&)) );

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

// used when loading to finalize records.
static bool loading = FALSE;
static int alarmTime = -1;
static PrEvent::SoundTypeChoice alarmSound = PrEvent::Loud;
static bool isAllDay = FALSE;

static time_t startUtc = 0;
static time_t endUtc = 0;
static time_t endDateUtc = 0;
static bool hasTimeZone = FALSE;

bool EventXmlIO::loadData()
{
    loading = TRUE;
    bool result = PimXmlIO::loadData();
    loading = FALSE;
    return result;
}

void EventXmlIO::assignField(PimRecord *rec, const QCString &attr, const QString &value) 
{
    // used to finalize data
    static PimRecord *prevrec = 0;

    if (rec != prevrec) {
	alarmTime = -1;
	alarmSound = PrEvent::Loud;
	isAllDay = FALSE;

	startUtc = 0;
	endUtc = 0;
	endDateUtc = 0;
	hasTimeZone = FALSE;
	prevrec = rec;
    }

    PrEvent *ev = (PrEvent *)rec;

    int *lookup = dict[ attr.data() ];
    if ( !lookup ) {
	ev->setCustomField(attr.data(), value);
	return;
    }

    switch( *lookup ) {
	case FDescription:
	    ev->setDescription( value );
	    break;
	case FLocation:
	    ev->setLocation( value );
	    break;
	case FCategories: 
	    ev->setCategories( idsFromString( value ) );
	    break;
	case FUid:
	    setUid( *ev, uuidFromInt(value.toInt()) );
	    break;
	case FType:
	    isAllDay = TRUE;
	    break;
	case FAlarm:
	    alarmTime = value.toInt();
	    break;
	case FSound:
	    alarmSound = 
		value == "loud" ? PrEvent::Loud : PrEvent::Silent;
	    break;
	case FRType:
	    if ( value == "Daily" )
		ev->setRepeatType(PrEvent::Daily);
	    else if ( value == "Weekly" )
		ev->setRepeatType(PrEvent::Weekly);
	    else if ( value == "MonthlyDay" )
		ev->setRepeatType(PrEvent::MonthlyDay);
	    else if ( value == "MonthlyDate" )
		ev->setRepeatType(PrEvent::MonthlyDate);
	    else if ( value == "MonthlyEndDay" )
		ev->setRepeatType(PrEvent::MonthlyEndDay);
	    else if ( value == "Yearly" )
		ev->setRepeatType(PrEvent::Yearly);
	    else
		ev->setRepeatType(PrEvent::NoRepeat);
	    break;
	case FRWeekdays:
	    if (!ev->hasRepeat())
		ev->setRepeatType(PrEvent::Weekly);
	    ev->p_setWeekMask(value.toInt());
	    break;
	case FRFreq:
	    if (!ev->hasRepeat())
		ev->setRepeatType(PrEvent::Daily);
	    ev->setFrequency( value.toInt() );
	    break;
	case FRHasEndDate:
	    if (!ev->hasRepeat())
		ev->setRepeatType(PrEvent::Daily);
	    ev->setRepeatForever( value.toInt() == 0 );
	    break;
	case FREndDate:
	    endDateUtc = (time_t) value.toLong();
	    if (!ev->hasRepeat())
		ev->setRepeatType(PrEvent::Daily);
	    break;
	case FRStart:
	    startUtc = (time_t) value.toLong();
	    break;
	case FREnd:
	    endUtc = (time_t) value.toLong();
	    break;
	case FNote:
	    ev->setNotes( value );
	    break;
	case FTimeZone:
	    if (value != "None")
		ev->setTimeZone( value);
	    // hasTimeZone in that one is set, even if it is none.
	    hasTimeZone = TRUE;
	    break;
	default:
	    // FCreated and FPosition Ignored.
	    qDebug( "huh??? missing enum? -- attr.: %s", attr.data() );
	    break;
    }
}

void EventXmlIO::finalizeRecord(PrEvent *ev) 
{
    // post Processing.
    if (!hasTimeZone) {
	// make one up.  Can't be "None" because in old datebook all
	// events were in UTC.  So make it the current locale.
	ev->setTimeZone( QString::fromLocal8Bit( getenv( "TZ" ) ) );
    }

    // if there was a timezone, it would be set by now.
    ev->setStartAsUTC( startUtc );
    ev->setEndAsUTC( endUtc );
    if (ev->hasRepeat() && endDateUtc != 0 && !ev->repeatForever())
	ev->setRepeatTillAsUTC( endDateUtc );

    if ( isAllDay )
	ev->setAllDay( TRUE );

    if (alarmTime >= 0)
	ev->setAlarm(alarmTime, alarmSound);

    // 0' out elements although this will be done in assignField anyway
    alarmTime = -1;
    alarmSound = PrEvent::Loud;
    isAllDay = FALSE;

    startUtc = 0;
    endUtc = 0;
    endDateUtc = 0;
    hasTimeZone = FALSE;
}

bool EventXmlIO::internalAddRecord(PimRecord *rec) 
{
    PrEvent *ev = (PrEvent *)rec;

    if (loading)
	finalizeRecord(ev);

    m_PrEvents.append( ev );
    return TRUE;
}

bool EventXmlIO::internalRemoveRecord(PimRecord *rec)
{
    PrEvent *ev = (PrEvent *)rec;

    // no need to finalize what we plan to delete

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
    if (loading)
	finalizeRecord(ev);

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

    if (accessMode() == ReadWrite) {
	if (PimXmlIO::saveData((QList<PimRecord> &)m_PrEvents)) {
	    needsSave = FALSE;
	    return TRUE;
	}
    }
    return FALSE;
}

QString EventXmlIO::recordToXml(const PimRecord *rec)
{
    const PrEvent *e = (const PrEvent *)rec;

    QString buf;
    buf += " description=\"" + Qtopia::escapeString(e->description()) + "\"";
    if ( !e->location().isEmpty() )
	buf += " location=\"" + Qtopia::escapeString(e->location()) + "\"";
    // save the categoies differently....
    QString strCats = idsToString( e->categories() );
    buf += " categories=\"" + Qtopia::escapeString(strCats) + "\"";
    buf += " uid=\"" + QString::number( uuidToInt(e->uid()) ) + "\"";
    if ( e->isAllDay())
	buf += " type=\"AllDay\"";


    if ( !e->timeZone().isEmpty() ) {
	buf += " timezone=\"" + Qtopia::escapeString(e->timeZone()) + "\"";
    } else {
	buf += " timezone=\"None\"";
    }

    if ( e->hasAlarm() ) {
	buf += " alarm=\"" + QString::number( e->alarmDelay() ) + "\" sound=\"";
	if ( e->alarmSound() == PrEvent::Loud )
	    buf += "loud";
	else
	    buf += "silent";
	buf += "\"";
    }
    if ( e->hasRepeat() ) {
	buf += " rtype=\"";
	switch ( e->repeatType() ) {
	    case PrEvent::Daily:
		buf += "Daily";
		break;
	    case PrEvent::Weekly:
		buf += "Weekly";
		break;
	    case PrEvent::MonthlyDay:
		buf += "MonthlyDay";
		break;
	    case PrEvent::MonthlyEndDay:
		buf += "MonthlyEndDay";
		break;
	    case PrEvent::MonthlyDate:
		buf += "MonthlyDate";
		break;
	    case PrEvent::Yearly:
		buf += "Yearly";
		break;
	    default:
		buf += "NoRepeat";
		break;
	}
	buf += "\"";
	if ( e->repeatType() == PrEvent::Weekly )
	    buf += " rweekdays=\"" + QString::number( static_cast<int>( e->p_weekMask() ) ) + "\"";

	buf += " rfreq=\"" + QString::number( e->frequency() ) + "\"";
	buf += " rhasenddate=\"" + QString::number( static_cast<int>( !e->repeatForever() ) ) + "\"";
	if ( !e->repeatForever() )
	    buf += " enddt=\""
		+ QString::number( e->repeatTillAsUTC() )
		+ "\"";
    }

    buf += " start=\""
	+ QString::number( e->startAsUTC() )
	+ "\"";

    buf += " end=\""
	+ QString::number( e->endAsUTC() )
	+ "\"";

    if ( !e->notes().isEmpty() )
	buf += " note=\"" + Qtopia::escapeString( e->notes() ) + "\"";

    // custom last
    buf += customToXml(e);

    return buf;
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

void EventXmlIO::addEvent(const PimEvent &event)
{
    if (accessMode() != ReadWrite)
	return;

    PrEvent *ev = new PrEvent((const PrEvent &)event);
    assignNewUid(ev);

    if (internalAddRecord(ev)) {
	needsSave = TRUE;
	if (ev->hasAlarm())
	    addEventAlarm(*ev);

	updateJournal(*ev, ACTION_ADD);

	{
	    QCopEnvelope e("QPE/PIM", "addedEvent(int,PimEvent)"); 
	    e << getpid();
	    e << *ev;
	}
    }
}


void EventXmlIO::removeEvent(const PimEvent &event)
{

    if (accessMode() != ReadWrite)
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
	    QCopEnvelope e("QPE/PIM", "removedEvent(int,PimEvent)"); 
	    e << getpid();
	    e << event;
	}
    }
}

void EventXmlIO::updateEvent(const PimEvent &event)
{
    if (accessMode() != ReadWrite)
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
	    QCopEnvelope e("QPE/PIM", "updatedEvent(int,PimEvent)"); 
	    e << getpid();
	    e << *ev;
	}
    }
}

void EventXmlIO::ensureDataCurrent(bool forceReload) {
  if (isDataCurrent() && !forceReload)
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

const QString EventXmlIO::dataFilename() const {
  QString filename = Global::applicationFileName("datebook",
						 "datebook.xml");
  return filename;
}

const QString EventXmlIO::journalFilename() const {
  QString str = getenv("HOME");
  str +="/.caljournal";
  return str;
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
    QDateTime when;
    int warn;
    if (nextAlarm(ev, when, warn)) {
	AlarmServer::addAlarm(when, "QPE/Application/datebook",
		"alarm(QDateTime,int)", warn);
    }
}

void EventXmlIO::delEventAlarm(const PrEvent &ev)
{
    QDateTime when;
    int warn;
    if ( nextAlarm(ev,when,warn) ) {
	AlarmServer::deleteAlarm( when, "QPE/Application/datebook",
		"alarm(QDateTime,int)", warn);
    }
}
