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
#include <qtopia/db/database.h>
#include <sqlite.h>

QTime profiler;
int profileMark;

#if 0
class PimEventTable : public DatabaseTable
{
public:
    PimEventTable(QObject *parent = 0, const char *name = 0) 
	: DatabaseTable("datebook", parent, name) {}

    Record *makeRecord() const { return new PimEvent; }
};

class PimEventIndex : public DatabaseIndex {
public:

    enum SortType {
	Starts,
	Ends,
	Uids,
    };

    PimEventIndex(const QString indname, PimEventTable *store, SortType start) 
	: DatabaseIndex(indname, store), sortOnStart(start) {}

    // no select, this stores everything.
    // is a compare though
    int compare(const Record *a, const Record *b) const
    {
	PimEvent *e1 = (PimEvent *)a;
	PimEvent *e2 = (PimEvent *)b;

	switch (sortOnStart) {
	    case Starts:
		if (e1->start() == e2->start())
		    return 0;
		return (e1->start() > e2->start()) ? 1 : -1;
	    case Ends:
		{ // Ends is end of spans.
		    QDateTime d1, d2;
		    if (e1->hasRepeat())
			d1 = e1->repeatTill();
		    else 
			d1 = e1->end();
		    if (e2->hasRepeat())
			d2 = e2->repeatTill();
		    else
			d2 = e2->end();

		    if (e1->hasRepeat() && e1->repeatForever())
			if (e2->hasRepeat() && e2->repeatForever())
			    return 0;
		        else
			    return 1;
		    else
			if (e2->hasRepeat() && e2->repeatForever())
			    return -1;

		    if (d1 == d2)
			return 0;
		    return (d1 > d2) ? 1 : -1;
		}
	    case Uids:
		if (e1->uid() == e2->uid())
		    return 0;
		return (e2->uid() < e1->uid()) ? 1 : -1;
	}
	return 0;
    }

    PimEvent eventAt(int index) const
    {
	PimEvent e;
	e.fromByteArray(dataAt(index));
	return e;
    }

    PimEvent eventAtOffset(int off) const {
	PimEvent e;
	e.fromByteArray(dataAtOffset(off));
	return e;
    }

    // returns the actual db positions...  should hence get the records?
    QArray<int> intersect(const QDate &f, PimEventIndex *byEnd, 
	    const QDate &t)
    {
	// find from, find to.  For index of each, intersect, return indexes
	// this.
	QDate from = f.addDays(-1); // cover for local times.
	QDate to = t.addDays(1); // cover for local times.
	QArray<int> results;

	if (count() == 0)
	    return results; // empty set.

	PimEvent dummy;
	dummy.setStart(QDateTime(to, QTime(23,59,59)));
	dummy.setEnd(QDateTime(to, QTime(23,59,59)));

	QArray<int> startSet = indexSet(0, indexFor(dummy));
	qDebug("start count/index %d/%d", 
		count(), indexFor(dummy));

	dummy.setStart(QDateTime(from, QTime(0,0,0)));
	dummy.setEnd(QDateTime(from, QTime(0,0,0)));
	int endIndex = indexFor(dummy);
	if (endIndex < 0)
	    endIndex = byEnd->count() - 1;
	QArray<int> endSet = byEnd->indexSet(endIndex, byEnd->count() - 1);
	qDebug("end count/index %d/%d", 
		byEnd->count(), byEnd->indexFor(dummy));
	startSet.sort();
	endSet.sort();

	profileMark++;
	qDebug("mark %d %d (s %d, e %d)", profileMark, profiler.elapsed(),
		startSet.size(), endSet.size());
	qDebug("counts (s %d, e %d)", 
		count(), byEnd->count());


	int arraySize = 16;
	int foundCount = 0;
	results.resize(arraySize);
	uint sInd = 0;
	uint eInd = 0;
	while (sInd < startSet.size() && eInd < endSet.size()) {
	    int d1 = startSet[sInd];
	    int d2 = endSet[eInd];
	    int cmp = memcmp(&d1, &d2, sizeof(int));
	    if (cmp < 0){ 
		sInd++;
	    } else if (cmp > 0) {
		eInd++;
	    } else {
		results[foundCount] = d1;
		foundCount++;
		if (foundCount >= arraySize) {
		    arraySize = 2*arraySize;
		    results.resize(arraySize);
		}
		sInd++;
		eInd++;
	    }
	}
	results.resize(foundCount); // if too big.
	return results;
    }

private:
    SortType sortOnStart;
};
#endif
#if 0
Occurance::Occurance(const QDate &start, PimEventIndex *e, int i)
    : mStart(start), set(e), index(i)
{
    if (set)
	eventCache = set->eventAt(index);
}
#endif

// SQL helper functions;
// datetime as sortable string.
QString dateTimeString(const QDateTime &dt)
{
    QString res;
    res = res.sprintf("%4.4d-%2.2d-%2.2d_%2.2d-%2.2d-%2.2d", 
	    dt.date().year(), dt.date().month(),  dt.date().day(), 
	    dt.time().hour(), dt.time().minute(), dt.time().second());
    return res;
}

// datetime as sortable string.
QDateTime stringDateTime(const QString &dt)
{
    int y = dt.mid(0,4).toInt();
    int m = dt.mid(5,2).toInt();
    int d = dt.mid(8,2).toInt();
    int h = dt.mid(11,2).toInt();
    int i = dt.mid(14,2).toInt();
    int s = dt.mid(17,2).toInt();
    QDateTime res(QDate(y,m,d), QTime(h,i,s));
    return res;
}

QString eventAsValues(const PimEvent &ev)
{
    QString rs("VALUES ('%1','%2','%3','%4','%5','%6','%7','%8','%9'");
    
    rs = rs
	.arg(ev.uid().toString())
	.arg(ev.description())
	.arg(ev.location())
	.arg(dateTimeString(ev.start()))
	.arg(dateTimeString(ev.end()))
	.arg(ev.notes())
	.arg(ev.timeZone())
	.arg(ev.endTimeZone())
	.arg(ev.hasAlarm() ? "1" : "0");
    rs += ",'%1'," "'%2','%3','%4','%5','%6','%7','%8')";

    rs = rs
	.arg(ev.alarmDelay() ? "1" : "0")
	.arg(ev.alarmSound())
	.arg(ev.repeatType())
	.arg(ev.frequency())
	.arg(dateTimeString(ev.repeatTill()))
	.arg(ev.repeatForever() ? "1" : "0")
	.arg(ev.showOnNearest() ? "1" : "0")
	.arg("NULL");

    return rs;
}

int sqlAddItem(void *set, int columns, char **argv, char **columnNames)
{

    if (!set || columns != 17)
	return 0;

    // we know the columNames, build us an event.
    PimEvent ev;
    if (argv[0]) {
	QString v(argv[0]);
	ev.setUid(QUuid(v));
    }
    if (argv[1]) {
	QString v(argv[1]);
	ev.setDescription(v);
    }
    if (argv[2]) {
	QString v(argv[2]);
	ev.setLocation(v);
    }
    if (argv[3]) {
	QString v(argv[3]);
	QDateTime vdt = stringDateTime(v);
	ev.setStart(vdt);
    }
    if (argv[4]) {
	QString v(argv[4]);
	QDateTime vdt = stringDateTime(v);
	ev.setEnd(vdt);
    }
    if (argv[5]) {
	QString v(argv[5]);
	ev.setNotes(v);
    }
    if (argv[6]) {
	QString v(argv[6]);
	ev.setTimeZone(v);
    }
    if (argv[7]) {
	QString v(argv[7]);
	ev.setEndTimeZone(v);
    }
    if (argv[8]) {
	QString v(argv[8]);
	if (v != "0") {
	    if (argv[9] && argv[9]) {
		ev.setAlarm(
			QString(argv[9]).toInt(), (PimEvent::SoundTypeChoice)QString(argv[10]).toInt());
	    } else {
		ev.clearAlarm();
	    }
	}
    }
    if (argv[11]) {
	QString v(argv[11]);
	ev.setRepeatType((PimEvent::RepeatType)v.toInt());
	if (ev.hasRepeat()) {
	    if (argv[12]) {
		QString v2(argv[12]);
		ev.setFrequency(v2.toInt());
	    }
	    if (argv[14]) {
		QString v2(argv[14]);
		if (v2 != "0")
		    ev.setRepeatForever(TRUE);
		else
		    ev.setRepeatForever(FALSE);
	    }
	    if (argv[13] && !ev.repeatForever()) {
		QString v2(argv[13]);
		QDateTime vdt = stringDateTime(v2);
		ev.setRepeatTill(vdt.date());
	    }
	    if (argv[15]) {
		QString v2(argv[15]);
		ev.setShowOnNearest(v2 != "0");
	    }
	    if (argv[16]) {
		QString v2(argv[16]);
		// for now don't do anything.
	    }
	}
    }
    QValueList<PimEvent> *evlist = (QValueList<PimEvent> *)set;
    evlist->append(ev);
    return 0;
}

Occurance::Occurance(const QDate &start, const PimEvent &ca )
    : eventCache(ca), mStart(start)
{
}

PimEvent Occurance::event() const
{
    return eventCache;
}

#if 0
PimEvent Occurance::eventDirect() const
{
    if (set)
	eventCache = set->eventAt(index);
    else 
	eventCache = PimEvent();
    return eventCache;
}
#endif

QDate Occurance::endDate() const
{
    return date().addDays(eventCache.start().date().daysTo(eventCache.end().date()));
}

QDateTime Occurance::start() const
{
    return QDateTime(date(), eventCache.start().time());
}

QDateTime Occurance::end() const
{
    return QDateTime(endDate(), eventCache.end().time());
}

DateBookTable::DateBookTable(QObject *parent, const char *name)
    : QObject(parent, name)
{
    acc = sqlite_open("/tmp/datebook.sqlite", 0, 0);
    qWarning("got acc %d", acc);

    qDebug("create table events "
		"(uid,description,location,start,end,notes,"
		"tz,endtz,hasAlarm,alarmDelay,alarmSound,repeatType,"
		"frequency,repeatTill,repeatForever,onNearest,weekmask);");
    qDebug("create table %d", sqlite_exec(acc, "create table events "
		"(uid,description,location,start,end,notes,"
		"tz,endtz,hasAlarm,alarmDelay,alarmSound,repeatType,"
		"frequency,repeatTill,repeatForever,onNearest,weekmask);", 0, 0, 0));

#if 0
    store = new PimEventTable(this);
    byStart = new PimEventIndex("byStart", store, PimEventIndex::Starts);
    byEnd = new PimEventIndex("byEnd", store, PimEventIndex::Ends);
    byUid = new PimEventIndex("byEnd", store, PimEventIndex::Uids);
#endif
}

DateBookTable::~DateBookTable()
{
    sqlite_close(acc);
}


QValueList<Occurance> DateBookTable::getOccurances( 
	const QDate &from, const QDate &to ) const
{
    profiler.start();
    profileMark = 0;
    profileMark++;
    qDebug("mark %d %d", profileMark, profiler.elapsed());
    QValueList<Occurance> results;
    QValueList<PimEvent> candidates;

    static const QString sel(
		"select * from events where start <= '%1' AND "
		"(end >= '%2' || (repeatType != '0' AND (repeatTill >= '%3' || repeatForever != '0')));"); // No tr
    qDebug(sel
		.arg(dateTimeString(QDateTime(to, QTime(23,59,59))))
		.arg(dateTimeString(QDateTime(from, QTime(0,0,0))))
		.arg(dateTimeString(QDateTime(from, QTime(0,0,0)))).latin1());
    qDebug("select intersecition %d", 
	    sqlite_exec(acc, sel
		.arg(dateTimeString(QDateTime(to, QTime(23,59,59))))
		.arg(dateTimeString(QDateTime(from, QTime(0,0,0))))
		.arg(dateTimeString(QDateTime(from, QTime(0,0,0)))).latin1(),
		sqlAddItem, &candidates, 0));

   
    profileMark++;
    qDebug("mark %d %d, candidates.count = %d", profileMark, profiler.elapsed(),
	    candidates.count());
    for (uint i = 0; i < candidates.count(); i++) {
	PimEvent e = candidates[i];
	// treat them all like they re-occur
	bool ok;
	QDate date = from;
	date = e.nextOccurance(date, &ok);
	int duration = e.start().date().daysTo(e.end().date()) + 1;
	while (ok && date <= to) {
	    Occurance o(date, e);
	    results.append(o);
	    date = e.nextOccurance(date.addDays(duration), &ok);
	}
    }
    profileMark++;
    qDebug("mark %d %d", profileMark, profiler.elapsed());
    return results;
}

Occurance DateBookTable::getNextAlarm( const QDateTime &from, bool *ok) const
{

    QValueList<Occurance> candidates = getOccurances(from.date(), from.date());
    QDateTime earliest;
    bool found = FALSE;
    Occurance result;

    QValueListConstIterator<Occurance> it;
    for ( it = candidates.begin(); it != candidates.end(); ++it ) {
	PimEvent e = (*it).event();
	QDateTime test = e.start();

	// test may be out.
	// XXX WE NEED if (!e.timeZone().isNull())
	    //test.addMinutes(TimeConversion::adjustment());
		
	if (!found) {
	    earliest = test;
	    result = *it;
	} else if (test < earliest) {
	    earliest = test;
	    result = *it;
	}
    }
    if (ok)
	*ok = found;
    return result;
}

void DateBookTable::addEvent(const PimEvent &ev)
{
    qDebug((QString("insert into events ") + eventAsValues(ev) + QString(";")).latin1());
    qDebug("insert into %d", sqlite_exec(acc, "insert into events " 
		+ eventAsValues(ev) + ";", 0, 0, 0));
    emit eventAdded(ev);
}

void DateBookTable::removeEvent(const PimEvent &e)
{
    qDebug("remove into %d", sqlite_exec(acc, 
		"delete from events where uid = " + e.uid().toString() + ";"
		, 0, 0, 0));
#if 0
    qDebug("create table %d", sqlite_exec(acc, "create table events "
		"(uid,description,location,start,end,notes,"
		"tz,endtz,hasAlarm,alarmDelay,alarmSound,repeatType,"
		"frequency,repeatTill,onNearest,weekmask);", 0));

    int ind = byUid->indexOf(e);
    if (ind != -1) {
	byUid->removeAt(ind);
	emit eventRemoved(e);
    }
#endif
}

// replace event &uid with ev (but keep the uid);
void DateBookTable::replaceEvent(const PimEvent &oldevent, const PimEvent &newevent)
{
    removeEvent(oldevent);
    addEvent(newevent);
}
