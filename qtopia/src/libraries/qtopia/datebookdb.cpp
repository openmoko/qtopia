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

#include <qasciidict.h>
#include <qfile.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qtextstream.h>
#include <qtl.h>

#include <qtopia/alarmserver.h>
#include <qtopia/global.h>
#include <qtopia/private/event.h>
#include "datebookdb.h"
#include <qtopia/stringutil.h>
#include <qtopia/timeconversion.h>

#include <errno.h>
#include <stdlib.h>

#include <qapplication.h> // for translate

QString qtopia_internal_homeDirPath();

/*!
  \class DateBookDB

  \brief The DateBookDB class is used to access event information in the
  Qtopia database.


  This class is now obsolete, the class DateBookAccess in the Qtopia PIM library
  should be used instead.

  \ingroup qtopiaemb

  \obsolete

  \sa DateBookIterator DateBookAccess
*/

class DateBookDBPrivate
{
public:
    bool clean;  // indcate whether we need to write to disk...
};


// QDOC_SKIP_BEGIN

// Helper functions

static QString dateBookJournalFile()
{
    QString str = ::qtopia_internal_homeDirPath(); 
    return QString( str +"/.caljournal" );
}

static QString dateBookFilename()
{
    return Global::applicationFileName("datebook","datebook.xml");
}

/* Calculating the next event of a recuring event is actually
   computationally inexpensive, esp. compared to checking each day
   individually.  There are bad worse cases for say the 29th of
   february or the 31st of some other months.  However
   these are still bounded */
bool nextOccurance(const Event &e, const QDate &from, QDateTime &next)
{
    // easy checks, first are we too far in the future or too far in the past?
    QDate tmpDate;
    int freq = e.repeatPattern().frequency;
    int diff, diff2, a;
    int iday, imonth, iyear;
    int dayOfWeek = 0;
    int firstOfWeek = 0;
    int weekOfMonth;
    

    if (e.repeatPattern().hasEndDate && e.repeatPattern().endDate() < from)
	return FALSE;

    if (e.start() >= from) {
	next = e.start();
	return TRUE;
    }

    switch ( e.repeatPattern().type ) {
	case Event::Weekly:
	    /* weekly is just daily by 7 */
	    /* first convert the repeatPattern.Days() mask to the next
	       day of week valid after from */
            dayOfWeek = from.dayOfWeek();
	    dayOfWeek--; /* we want 0-6, doco for above specs 1-7 */

	    /* this is done in case freq > 1 and from in week not
	       for this round */
	    // firstOfWeek = 0; this is already done at decl.
	    while(!((1 << firstOfWeek) & e.repeatPattern().days))
		firstOfWeek++;

	    /* there is at least one 'day', or there would be no event */
	    while(!((1 << (dayOfWeek % 7)) & e.repeatPattern().days))
		dayOfWeek++;

	    dayOfWeek = dayOfWeek % 7; /* the actual day of week */
	    dayOfWeek -= e.start().date().dayOfWeek() -1;

	    firstOfWeek = firstOfWeek % 7; /* the actual first of week */
	    firstOfWeek -= e.start().date().dayOfWeek() -1;

	    // dayOfWeek may be negitive now
	    // day of week is number of days to add to start day

	    freq *= 7;
	    // FALL-THROUGH !!!!!
	case Event::Daily:
	    // the add is for the possible fall through from weekly */
	    if(e.start().date().addDays(dayOfWeek) > from) {
		/* first week exception */
		next = QDateTime(e.start().date().addDays(dayOfWeek),
				 e.start().time());
		if ((next.date() > e.repeatPattern().endDate())
		    && e.repeatPattern().hasEndDate)
		    return FALSE;
		return TRUE;
	    }
	    /* if from is middle of a non-week */

	    diff = e.start().date().addDays(dayOfWeek).daysTo(from) % freq;
	    diff2 = e.start().date().addDays(firstOfWeek).daysTo(from) % freq;

	    if(diff != 0)
		diff = freq - diff;
	    if(diff2 != 0)
		diff2 = freq - diff2;
	    diff = QMIN(diff, diff2);

	    next = QDateTime(from.addDays(diff), e.start().time());
	    if ( (next.date() > e.repeatPattern().endDate())
		 && e.repeatPattern().hasEndDate )
		return FALSE;
	    return TRUE;
	case Event::MonthlyDay:
	    iday = from.day();
	    iyear = from.year();
	    imonth = from.month();
	    /* find equivelent day of month for this month */
	    dayOfWeek = e.start().date().dayOfWeek();
	    weekOfMonth = (e.start().date().day() - 1) / 7;

	    /* work out when the next valid month is */
	    a = from.year() - e.start().date().year();
	    a *= 12;
	    a = a + (imonth - e.start().date().month());
	    /* a is e.start()monthsFrom(from); */
	    if(a % freq) {
		a = freq - (a % freq);
		imonth = from.month() + a;
		if (imonth > 12) {
		    imonth--;
		    iyear += imonth / 12;
		    imonth = imonth % 12;
		    imonth++;
		}
	    }
	    /* imonth is now the first month after or on
	       from that matches the frequency given */

	    /* find for this month */
	    tmpDate = QDate( iyear, imonth, 1 );

	    iday = 1;
	    iday += (7 + dayOfWeek - tmpDate.dayOfWeek()) % 7;
	    iday += 7 * weekOfMonth;
	    while (iday > tmpDate.daysInMonth()) {
		imonth += freq;
		if (imonth > 12) {
		    imonth--;
		    iyear += imonth / 12;
		    imonth = imonth % 12;
		    imonth++;
		}
		tmpDate = QDate( iyear, imonth, 1 );
		/* these loops could go for a while, check end case now */
		if ((tmpDate > e.repeatPattern().endDate()) && e.repeatPattern().hasEndDate)
		    return FALSE;
		iday = 1;
		iday += (7 + dayOfWeek - tmpDate.dayOfWeek()) % 7;
		iday += 7 * weekOfMonth;
	    }
	    tmpDate = QDate(iyear, imonth, iday);

	    if (tmpDate >= from) {
		next = QDateTime(tmpDate, e.start().time());
		if ((next.date() > e.repeatPattern().endDate()) && e.repeatPattern().hasEndDate)
		    return FALSE;
		return TRUE;
	    }

	    /* need to find the next iteration */
	    do {
		imonth += freq;
		if (imonth > 12) {
		    imonth--;
		    iyear += imonth / 12;
		    imonth = imonth % 12;
		    imonth++;
		}
		tmpDate = QDate( iyear, imonth, 1 );
		/* these loops could go for a while, check end case now */
		if ((tmpDate > e.repeatPattern().endDate()) && e.repeatPattern().hasEndDate)
		    return FALSE;
		iday = 1;
		iday += (7 + dayOfWeek - tmpDate.dayOfWeek()) % 7;
		iday += 7 * weekOfMonth;
	    } while (iday > tmpDate.daysInMonth());
	    tmpDate = QDate(iyear, imonth, iday);

	    next = QDateTime(tmpDate, e.start().time());
	    if ((next.date() > e.repeatPattern().endDate()) && e.repeatPattern().hasEndDate)
		return FALSE;
	    return TRUE;
	case Event::MonthlyDate:
	    iday = e.start().date().day();
	    iyear = from.year();
	    imonth = from.month();

	    a = from.year() - e.start().date().year();
	    a *= 12;
	    a = a + (imonth - e.start().date().month());
	    /* a is e.start()monthsFrom(from); */
	    if(a % freq) {
		a = freq - (a % freq);
		imonth = from.month() + a;
		if (imonth > 12) {
		    imonth--;
		    iyear += imonth / 12;
		    imonth = imonth % 12;
		    imonth++;
		}
	    }
	    /* imonth is now the first month after or on
	       from that matches the frequencey given */

	    /* this could go for a while, worse case, 4*12 iterations, probably */
	    while(!QDate::isValid(iyear, imonth, iday) ) {
		imonth += freq;
		if (imonth > 12) {
		    imonth--;
		    iyear += imonth / 12;
		    imonth = imonth % 12;
		    imonth++;
		}
		/* these loops could go for a while, check end case now */
		if ((QDate(iyear, imonth, 1) > e.repeatPattern().endDate()) && e.repeatPattern().hasEndDate)
		    return FALSE;
	    }

	    if(QDate(iyear, imonth, iday) >= from) {
		/* done */
		next = QDateTime(QDate(iyear, imonth, iday),
			e.start().time());
		if ((next.date() > e.repeatPattern().endDate()) && e.repeatPattern().hasEndDate)
		    return FALSE;
		return TRUE;
	    }

	    /* ok, need to cycle */
	    imonth += freq;
	    imonth--;
	    iyear += imonth / 12;
	    imonth = imonth % 12;
	    imonth++;

	    while(!QDate::isValid(iyear, imonth, iday) ) {
		imonth += freq;
		imonth--;
		iyear += imonth / 12;
		imonth = imonth % 12;
		imonth++;
		if ((QDate(iyear, imonth, 1) > e.repeatPattern().endDate()) && e.repeatPattern().hasEndDate)
		    return FALSE;
	    }

	    next = QDateTime(QDate(iyear, imonth, iday), e.start().time());
	    if ((next.date() > e.repeatPattern().endDate()) && e.repeatPattern().hasEndDate)
		return FALSE;
	    return TRUE;
	case Event::Yearly:
	    iday = e.start().date().day();
	    imonth = e.start().date().month();
	    iyear = from.year(); // after all, we want to start in this year

	    diff = 1;
	    if(imonth == 2 && iday > 28) {
		/* leap year, and it counts, calculate actual frequency */
		if(freq % 4)
		    if (freq % 2)
			freq = freq * 4;
		    else
			freq = freq * 2;
		/* else divides by 4 already, leave freq alone */
		diff = 4;
	    }

	    a = from.year() - e.start().date().year();
	    if(a % freq) {
		a = freq - (a % freq);
		iyear = iyear + a;
	    }

	    /* under the assumption we won't hit one of the special not-leap years twice */
	    if(!QDate::isValid(iyear, imonth, iday)) {
		/* must have been skipping by leap years and hit one that wasn't, (e.g. 2100) */
		iyear += freq;
	    }

	    if(QDate(iyear, imonth, iday) >= from) {
		next = QDateTime(QDate(iyear, imonth, iday),
			e.start().time());
		if ((next.date() > e.repeatPattern().endDate()) && e.repeatPattern().hasEndDate)
		    return FALSE;
		return TRUE;
	    }
	    /* iyear == from.year(), need to advance again */
	    iyear += freq;
	    /* under the assumption we won't hit one of the special not-leap years twice */
	    if(!QDate::isValid(iyear, imonth, iday)) {
		/* must have been skipping by leap years and hit one that wasn't, (e.g. 2100) */
		iyear += freq;
	    }

	    next = QDateTime(QDate(iyear, imonth, iday), e.start().time());
	    if ((next.date() > e.repeatPattern().endDate()) && e.repeatPattern().hasEndDate)
		return FALSE;
	    return TRUE;
	default:
	    return FALSE;
    }
}

static void addEventAlarm( const Event & )
{
}

static void delEventAlarm( const Event & )
{
}

/*! */
DateBookDB::DateBookDB()
{
    init();
}

/*! */
DateBookDB::~DateBookDB()
{
    save();
    eventList.clear();
    repeatEvents.clear();
}


//#### Why is this code duplicated in getEffectiveEvents ?????
//#### Addendum.  Don't use this function, lets faze it out if we can.

/*! obsolete \a from \a to */
QValueList<Event> DateBookDB::getEvents( const QDate &from, const QDate &to )
{
    QValueList<Event> tmpList;
    tmpList = getNonRepeatingEvents( from, to );

    // check for repeating events...
    for (QValueList<Event>::ConstIterator it = repeatEvents.begin();
	 it != repeatEvents.end(); ++it) {
	QDate itDate = from;
	QDateTime due;

	/* create a false end date, to short circuit on hard
	   MonthlyDay recurences */
	Event dummy_event = *it;
	Event::RepeatPattern r = dummy_event.repeatPattern();
	if ( !r.hasEndDate || r.endDate() > to ) {
	    r.setEndDate( to );
	    r.hasEndDate = TRUE;
	}
	dummy_event.setRepeat(TRUE, r);

	while (nextOccurance(dummy_event, itDate, due)) {
	    if (due.date() > to)
		break;
	    Event newEvent = *it;
	    newEvent.setStart(due);
	    newEvent.setEnd(due.addSecs((*it).start().secsTo((*it).end())));

	    tmpList.append(newEvent);
	    itDate = due.date().addDays(1);  /* the next event */
	}
    }
    qHeapSort(tmpList);
    return tmpList;
}

/*! \a start */
QValueList<Event> DateBookDB::getEvents( const QDateTime &start )
{
    QValueList<Event> day = getEvents(start.date(),start.date());

    QValueListConstIterator<Event> it;
    QDateTime dtTmp;
    QValueList<Event> tmpList;
    for (it = day.begin(); it != day.end(); ++it ) {
        dtTmp = (*it).start(TRUE);
        if ( dtTmp == start )
            tmpList.append( *it );
    }
    return tmpList;
}

//#### Why is this code duplicated in getEvents ?????

/*! \a from \a to */
QValueList<EffectiveEvent> DateBookDB::getEffectiveEvents( const QDate &from,
							   const QDate &to )
{
    QValueList<EffectiveEvent> tmpList;
    QValueListIterator<Event> it;

    EffectiveEvent effEv;
    QDateTime dtTmp,
	      dtEnd;

    for (it = eventList.begin(); it != eventList.end(); ++it ) {
	// next to lines are a cover against possible invalid Uids.
	if (!(*it).isValidUid())
	    (*it).assignUid();
        dtTmp = (*it).start(TRUE);
	dtEnd = (*it).end(TRUE);

        if ( dtTmp.date() >= from && dtTmp.date() <= to ) {
	    Event tmpEv = *it;
	    effEv.setEvent(tmpEv);
	    effEv.setDate( dtTmp.date() );
	    effEv.setStart( dtTmp.time() );
	    if ( dtTmp.date() != dtEnd.date() )
		effEv.setEnd( QTime(23, 59, 0) );
	    else
		effEv.setEnd( dtEnd.time() );
            tmpList.append( effEv );
	}
	// we must also check for end date information...
	if ( dtEnd.date() != dtTmp.date() && dtEnd.date() >= from ) {
	    QDateTime dt = dtTmp.addDays( 1 );
	    dt.setTime( QTime(0, 0, 0) );
	    QDateTime dtStop;
	    if ( dtEnd > to ) {
		dtStop = to;
	    } else
		dtStop = dtEnd;
	    while ( dt <= dtStop ) {
		Event tmpEv = *it;
		effEv.setEvent( tmpEv );
		effEv.setDate( dt.date() );
		if ( dt >= from ) {
		    effEv.setStart( QTime(0, 0, 0) );
		    if ( dt.date() == dtEnd.date() )
			effEv.setEnd( dtEnd.time() );
		    else
			effEv.setEnd( QTime(23, 59, 59) );
		    tmpList.append( effEv );
		}
		dt = dt.addDays( 1 );
	    }
	}
    }
    // check for repeating events...
    QDateTime repeat;
    for ( it = repeatEvents.begin(); it != repeatEvents.end(); ++it ) {
	// next to lines are a cover against possible invalid Uids.
	if (!(*it).isValidUid())
	    (*it).assignUid();

	/* create a false end date, to short circuit on hard
	   MonthlyDay recurences */
	Event dummy_event = *it;
	int duration = (*it).start().date().daysTo( (*it).end().date() );
	QDate itDate = from.addDays(-duration);

	Event::RepeatPattern r = dummy_event.repeatPattern();
	if ( !r.hasEndDate || r.endDate() > to ) {
	    r.setEndDate( to );
	    r.hasEndDate = TRUE;
	}
	dummy_event.setRepeat(TRUE, r);

	while (nextOccurance(dummy_event, itDate, repeat)) {
	    if(repeat.date() > to)
		break;
	    effEv.setDate( repeat.date() );
	    if ((*it).type() == Event::AllDay) {
		effEv.setStart( QTime(0,0,0) );
		effEv.setEnd( QTime(23,59,59) );
	    } else {
		/* we only occur by days, not hours/minutes/seconds.  Hence
		   the actual end and start times will be the same for
		   every repeated event.  For multi day events this is
		   fixed up later if on wronge day span */
		effEv.setStart( (*it).start().time() );
		effEv.setEnd( (*it).end().time() );
	    }
	    if ( duration != 0 ) {
		// multi-day repeating events
		QDate sub_it = QMAX( repeat.date(), from );
		QDate startDate = repeat.date();
		QDate endDate = startDate.addDays( duration );

		while ( sub_it <= endDate && sub_it  <= to ) {
		    EffectiveEvent tmpEffEv = effEv;
		    Event tmpEv = *it;
		    tmpEffEv.setEvent( tmpEv );

		    if ( sub_it != startDate )
			tmpEffEv.setStart( QTime(0,0,0) );
		    if ( sub_it != endDate )
			tmpEffEv.setEnd( QTime(23,59,59) );
		    tmpEffEv.setDate( sub_it );
		    tmpEffEv.setEffectiveDates( startDate, endDate );
		    tmpList.append( tmpEffEv );
		    sub_it = sub_it.addDays( 1 );
		}
		itDate = endDate;
	    } else {
		Event tmpEv = *it;
		effEv.setEvent( tmpEv );
		tmpList.append( effEv );
		itDate = repeat.date().addDays( 1 );
	    }
	}
    }

    qHeapSort( tmpList );
    return tmpList;
}

/*! \a dt */
QValueList<EffectiveEvent> DateBookDB::getEffectiveEvents( const QDateTime &dt)
{
    QValueList<EffectiveEvent> day = getEffectiveEvents(dt.date(), dt.date());
    QValueListConstIterator<EffectiveEvent> it;
    QValueList<EffectiveEvent> tmpList;
    QDateTime dtTmp;

    for (it = day.begin(); it != day.end(); ++it ) {
        dtTmp = QDateTime( (*it).date(), (*it).start() );
	// at the moment we don't have second granularity, be nice about that..
        if ( QABS(dt.secsTo(dtTmp)) < 60 )
            tmpList.append( *it );
    }
    return tmpList;
}


/*! \a ev \a doalarm */
void DateBookDB::addEvent( const Event &ev, bool doalarm )
{
    // write to the journal...
    saveJournalEntry( ev, ACTION_ADD, -1, false );
    addJFEvent( ev, doalarm );
    d->clean = false;
}

/*! \a ev \a doalarm */
void DateBookDB::addJFEvent( const Event &ev, bool doalarm )
{
    if ( doalarm && ev.hasAlarm() )
	addEventAlarm( ev );
    if ( ev.hasRepeat() )
	repeatEvents.append( ev );
    else
	eventList.append( ev );
}

/*! \a old \a editedEv */
void DateBookDB::editEvent( const Event &old, Event &editedEv )
{
    int oldIndex=0;
    bool oldHadRepeat = old.hasRepeat();
    Event orig;

    // write to the journal...
    if ( oldHadRepeat ) {
	if ( origRepeat( old, orig ) ) // should work always...
	    oldIndex = repeatEvents.findIndex( orig );
    } else
	oldIndex = eventList.findIndex( old );
    saveJournalEntry( editedEv, ACTION_REPLACE, oldIndex, oldHadRepeat );

    if ( old.hasAlarm() )
	delEventAlarm( old );
    if ( oldHadRepeat ) {
	if ( editedEv.hasRepeat() ) {
	    // assumption, when someone edits a repeating event, they
	    // want to change them all, maybe not perfect, but it works
	    // for the moment...
	    repeatEvents.remove( orig );
	} else
	    removeRepeat( old );
    } else {
	QValueList<Event>::Iterator it = eventList.find( old );
	if ( it != eventList.end() )
	    eventList.remove( it );
    }
    if ( editedEv.hasAlarm() )
	addEventAlarm( editedEv );
    if ( editedEv.hasRepeat() )
	repeatEvents.append( editedEv );
    else
	eventList.append( editedEv );
    d->clean = false;
}

/*! \a ev */
void DateBookDB::removeEvent( const Event &ev )
{
    // write to the journal...
    saveJournalEntry( ev, ACTION_REMOVE, -1, false );
    removeJFEvent( ev );
    d->clean = false;
}

/*! \a ev */
void DateBookDB::removeJFEvent( const Event&ev )
{
    if ( ev.hasAlarm() )
	delEventAlarm( ev );
    if ( ev.hasRepeat() ) {
	removeRepeat( ev );
    } else {
	QValueList<Event>::Iterator it = eventList.find( ev );
	if ( it != eventList.end() )
	    eventList.remove( it );
    }
}

// also handles journaling...
/*! \a strFile */
void DateBookDB::loadFile( const QString &strFile )
{

    QFile f( strFile );
    if ( !f.open( IO_ReadOnly ) )
	return;

    enum Attribute {
	FDescription = 0,
	FLocation,
	FCategories,
	FUid,
	FType,
	FAlarm,
	FSound,
	FRType,
	FRWeekdays,
	FRPosition,
	FRFreq,
	FRHasEndDate,
	FREndDate,
	FRStart,
	FREnd,
	FNote,
	FCreated,
	FAction,
	FActionKey,
	FJournalOrigHadRepeat
    };

    QAsciiDict<int> dict( 97 );
    dict.setAutoDelete( TRUE );
    dict.insert( "description", new int(FDescription) ); // No tr
    dict.insert( "location", new int(FLocation) ); // No tr
    dict.insert( "categories", new int(FCategories) ); // No tr
    dict.insert( "uid", new int(FUid) );
    dict.insert( "type", new int(FType) ); // No tr
    dict.insert( "alarm", new int(FAlarm) ); // No tr
    dict.insert( "sound", new int(FSound) ); // No tr
    dict.insert( "rtype", new int(FRType) );
    dict.insert( "rweekdays", new int(FRWeekdays) );
    dict.insert( "rposition", new int(FRPosition) );
    dict.insert( "rfreq", new int(FRFreq) );
    dict.insert( "rhasenddate", new int(FRHasEndDate) );
    dict.insert( "enddt", new int(FREndDate) );
    dict.insert( "start", new int(FRStart) ); // No tr
    dict.insert( "end", new int(FREnd) ); // No tr
    dict.insert( "note", new int(FNote) ); // No tr
    dict.insert( "created", new int(FCreated) ); // No tr
    dict.insert( "action", new int(FAction) ); // No tr
    dict.insert( "actionkey", new int(FActionKey) );
    dict.insert( "actionorig", new int (FJournalOrigHadRepeat) );

    
    QByteArray ba = f.readAll();
    char* dt = ba.data();
    if ( !dt )
	return;
    int len = ba.size();
    int currentAction,
	journalKey,
	origHadRepeat;  // should be bool, but we need tri-state(not being used)

    int i = 0;
    char *point;
    while ( (dt+i !=0) && (( point = strstr( dt+i, "<event " ) ) != 0 )) { // No tr
	i = point - dt;
	// if we are reading in events in the general case,
	// we are just adding them, so let the actions represent that...
	currentAction = ACTION_ADD;
	journalKey = -1;
	origHadRepeat = -1;
	// some temporary variables for dates and times ...
	//int startY = 0, startM = 0, startD = 0, starth = 0, startm = 0, starts = 0;
	//int endY = 0, endM = 0, endD = 0, endh = 0, endm = 0, ends = 0;
	//int enddtY = 0, enddtM = 0, enddtD = 0;

	// ... for the alarm settings ...
	int alarmTime = -1; Event::SoundTypeChoice alarmSound = Event::Silent;
	// ... and for the recurrence
	Event::RepeatPattern rp;
	Event e;

	i += 7;

	while( 1 ) {
	    while ( i < len && (dt[i] == ' ' || dt[i] == '\n' || dt[i] == '\r') )
		++i;
	    if ( i >= len-2 || (dt[i] == '/' && dt[i+1] == '>') )
		break;
	    // we have another attribute, read it.
	    int j = i;
	    while ( j < len && dt[j] != '=' )
		++j;
	    char *attr = dt+i;
	    dt[j] = '\0';
	    i = ++j; // skip =
	    while ( i < len && dt[i] != '"' )
		++i;
	    j = ++i;
	    bool haveAmp = FALSE;
	    bool haveUtf = FALSE;
	    while ( j < len && dt[j] != '"' ) {
		if ( dt[j] == '&' )
		    haveAmp = TRUE;
		if ( ((unsigned char)dt[j]) > 0x7f )
		    haveUtf = TRUE;
		++j;
	    }

	    if ( i == j ) {
		// leave out empty attributes
		i = j + 1;
		continue;
	    }

	    QString value = haveUtf ? QString::fromUtf8( dt+i, j-i )
			    : QString::fromLatin1( dt+i, j-i );
	    if ( haveAmp )
		value = Qtopia::plainString( value );
	    i = j + 1;

	    //qDebug("attr='%s' value='%s'", attr.data(), value.latin1() );
	    int * find = dict[ attr ];
#if 1
	    if ( !find ) {
		// custom field
		e.setCustomField(attr, value);
		continue;
	    }

	    switch( *find ) {
	    case FDescription:
		e.setDescription( value );
		break;
	    case FLocation:
		e.setLocation( value );
		break;
	    case FCategories:
		e.setCategories( Qtopia::Record::idsFromString( value ) );
		break;
	    case FUid:
		e.setUid( value.toInt() );
		break;
	    case FType:
		if ( value == "AllDay" )
		    e.setType( Event::AllDay );
		else
		    e.setType( Event::Normal );
		break;
	    case FAlarm:
		alarmTime = value.toInt();
		break;
	    case FSound:
		alarmSound = value == "loud" ? Event::Loud : Event::Silent; // No tr
		break;
		// recurrence stuff
	    case FRType:
		if ( value == "Daily" ) // No tr
		    rp.type = Event::Daily;
		else if ( value == "Weekly" ) // No tr
		    rp.type = Event::Weekly;
		else if ( value == "MonthlyDay" )
		    rp.type = Event::MonthlyDay;
		else if ( value == "MonthlyDate" )
		    rp.type = Event::MonthlyDate;
		else if ( value == "Yearly" ) // No tr
		    rp.type = Event::Yearly;
		else
		    rp.type = Event::NoRepeat;
		break;
	    case FRWeekdays:
		rp.days = value.toInt();
		break;
	    case FRPosition:
		rp.position = value.toInt();
		break;
	    case FRFreq:
		rp.frequency = value.toInt();
		break;
	    case FRHasEndDate:
		rp.hasEndDate = value.toInt();
		break;
		case FREndDate: {
		rp.endDateUTC = (time_t) value.toLong();
		break;
		}
		case FRStart: {
		e.setStart( (time_t) value.toLong() );
		break;
		}
		case FREnd: {
		e.setEnd( (time_t) value.toLong() );
		break;
		}
	    case FNote:
		e.setNotes( value );
		break;
	    case FCreated:
		rp.createTime = value.toInt();
		break;
	    case FAction:
		currentAction = value.toInt();
		break;
	    case FActionKey:
		journalKey = value.toInt();
		break;
	    case FJournalOrigHadRepeat:
		origHadRepeat = value.toInt();
		break;
	    default:
		qDebug( "huh??? missing enum? -- attr.: %s", attr );
		break;
	    }
#endif
	}
	// "post processing" (dates, times, alarm, recurrence)
	// start date/time
	e.setRepeat( rp.type != Event::NoRepeat, rp );

	if ( alarmTime != -1 )
	    e.setAlarm( TRUE, alarmTime, alarmSound );

	// now do our action based on the current action...
	switch ( currentAction ) {
	case ACTION_ADD:
	    addJFEvent( e );
	    break;
	case ACTION_REMOVE:
	    removeJFEvent( e );
	    break;
	case ACTION_REPLACE:
	    // be a little bit careful,
	    // in case of a messed up journal...
	    if ( journalKey > -1 && origHadRepeat > -1 ) {
		// get the original from proper list...
		if ( origHadRepeat )
		    removeJFEvent( *(repeatEvents.at(journalKey)) );
		else
		    removeJFEvent( *(eventList.at(journalKey)) );
		addJFEvent( e );
	    }
	    break;
	default:
	    break;
	}
    }
    f.close();
}

/*! */
void DateBookDB::init()
{
    d = new DateBookDBPrivate;
    d->clean = false;
    QString str = dateBookFilename();
    if ( str.isNull() ) {
	QMessageBox::warning( 0, qApp->translate( "DateBookDB", "Out of Space"),
			      qApp->translate( "DateBookDB", 
					       "<qt>Unable to create start up files. "
					       "Please free up some space "
					       "before entering data</qt>"));
    }
    // continuing along, we call this datebook filename again,
    // because they may fix it before continuing, though it seems
    // pretty unlikely...
    loadFile( dateBookFilename() );

    if ( QFile::exists( dateBookJournalFile() ) ) {
	// merge the journal
	loadFile( dateBookJournalFile() );
	// save in our changes and remove the journal...
	save();
    }
    d->clean = true;
}

/*! */
bool DateBookDB::save()
{
    return false;
}

/*! */
void DateBookDB::reload()
{
    QValueList<Event>::Iterator it = eventList.begin();
    for ( ; it != eventList.end(); ++it ) {
	if ( (*it).hasAlarm() )
	    delEventAlarm( *it );
	if ( (*it).hasRepeat() )
	    removeRepeat( *it );
    }
    eventList.clear();
    repeatEvents.clear(); // should be a NOP
    init();
}

/*! \a ev */
bool DateBookDB::removeRepeat( const Event &ev )
{
    time_t removeMe = ev.repeatPattern().createTime;
    QValueListIterator<Event> it;
    for ( it = repeatEvents.begin(); it != repeatEvents.end(); ++it ) {
	if ( removeMe == (*it).repeatPattern().createTime ) {
	    repeatEvents.remove( *it );
	    // best break, or we are going into undefined territory!
	    return TRUE;
	}
    }
    return FALSE;
}

/*! \a ev \a orig */
bool DateBookDB::origRepeat( const Event &ev, Event &orig ) const
{
    time_t removeMe = ev.repeatPattern().createTime;
    QValueListConstIterator<Event> it;
    for ( it = repeatEvents.begin(); it != repeatEvents.end(); ++it ) {
	if ( removeMe == (*it).repeatPattern().createTime ) {
	    orig = (*it);
	    return TRUE;
	}
    }
    return FALSE;
}

/*! \a ev \a action */
void DateBookDB::saveJournalEntry( const Event &, journal_action )
{
}

/*! \a evOld \a action \a key \a origHadRepeat */
bool DateBookDB::saveJournalEntry( const Event &, journal_action,
				   int, bool )
{
    return FALSE;
}

/*! */
QValueList<Event> DateBookDB::getRawRepeats() const
{
    return repeatEvents;
}

/*! \a from \a to */
QValueList<Event> DateBookDB::getNonRepeatingEvents( const QDate &from,
						     const QDate &to ) const
{
    QValueListConstIterator<Event> it;
    QDateTime dtTmp, dtEnd;
    QValueList<Event> tmpList;
    for (it = eventList.begin(); it != eventList.end(); ++it ) {
        dtTmp = (*it).start(TRUE);
	dtEnd = (*it).end(TRUE);

        if ( dtTmp.date() >= from && dtTmp.date() <= to ) {
	    Event e = *it;
	    if ( dtTmp.date() != dtEnd.date() )
		e.setEnd( QDateTime(dtTmp.date(), QTime(23, 59, 0)) );
            tmpList.append( e );
	}
	// we must also check for end date information...
	if ( dtEnd.date() != dtTmp.date() && dtEnd.date() >= from ) {
	    QDateTime dt = dtTmp.addDays( 1 );
	    dt.setTime( QTime(0, 0, 0) );
	    QDateTime dtStop;
	    if ( dtEnd > to ) {
		dtStop = to;
	    } else
		dtStop = dtEnd;
	    while ( dt <= dtStop ) {
		Event ev = *it;
		if ( dt >= from ) {
		    ev.setStart( QDateTime(dt.date(), QTime(0, 0, 0)) );
		    if ( dt.date() == dtEnd.date() )
			ev.setEnd( QDateTime(dt.date(), dtEnd.time()) );
		    else
			ev.setEnd( QDateTime(dt.date(), QTime(23, 59, 0)) );
		    tmpList.append( ev );
		}
		dt = dt.addDays( 1 );
	    }
	}
    }
    return tmpList;
}

// QDOC_SKIP_END
