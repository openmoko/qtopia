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

#include "event.h"
#include <qtopia/calendar.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/private/qfiledirect_p.h>
#include <qtopia/timeconversion.h>
#include <qtopia/stringutil.h>
#include <qtopia/private/recordfields.h>
#include <qtopia/private/vobject_p.h>
#include <qtopia/private/cp1252textcodec_p.h>

#include <qtopia/pim/private/xmlio_p.h>
#include <qtopia/pim/private/eventio_p.h>

#include <qbuffer.h>
#include <qtextcodec.h>

#include <time.h>

#include <stdio.h>

/*!
  \class PimEvent
  \module qpepim
  \ingroup qpepim
  \brief The PimEvent class holds the data of a calendar event.

  This data includes descriptive data of the event and schedualing information.

*/

/*!
  \enum PimEvent::RepeatType

  This enum type defines how an event repeats.

  <ul>
  <li>\c NoRepeat - Event occurs only once.
  <li>\c Daily - Event occurs every N days.
  <li>\c Weekly - Event occurs every N weeks.
  <li>\c MonthlyDate - Event occurs on the Xth day of the month every N months.
  <li>\c MonthlyDay - Event occurs on the Xth week of the month every N months.
  <li>\c MonthlyEndDay - Event occurs on the Xth last week of the month every N months.
  <li>\c Yearly - Event occurs every N years.
  </ul>

  \sa frequency(), weekOffset() showOnNearest() repeatOnWeekDay()
*/

/*!
  \enum PimEvent::SoundTypeChoice

  This enum type defines what kind of sound is made when an alarm occurs
  for an event.  The currently defined types are:

  <ul>
  <li>\c Silent - No sound is produced.
  <li>\c Loud - A loud sound is produced.
  </ul>

  \sa setAlarm(), alarmSound()
*/

/*!
  \fn void PimEvent::description() const

  Returns the description of the event.

  \sa setDescription()
*/

/*!
  \fn void PimEvent::location() const

  Returns the location of the event.

  \sa setLocation()
*/

/*!
  \fn void PimEvent::notes() const

  Returns the notes of the event.

  \sa setNotes()
*/

/*!
  \fn bool PimEvent::hasAlarm() const

  Returns TRUE if there is an alarm set for the event.  Otherwise
 returns FALSE.

 \sa setAlarm()
*/

/*!
  \fn int PimEvent::alarmDelay() const

  Returns the number of minutes before the event to activate the alarm
  for the event.

 \sa setAlarm()
*/

/*!
  \fn PimEvent::SoundTypeChoice PimEvent::alarmSound() const

  Returns the type of alarm to sound.

  \sa setAlarm(), SoundTypeChoice
*/

/*!
  \fn RepeatType PimEvent::repeatType() const

  Returns the RepeatType of the event.

  \sa setRepeatType(), RepeatType
*/

/*!
  \fn bool PimEvent::hasRepeat() const

  Returns FALSE if the event has repeat type NoRepeat.  Otherwise returns TRUE.

  \sa setRepeatType(), RepeatType
*/

/*!
  \fn int PimEvent::frequency() const

  Returns how often the event repeats.

  \sa setFrequency()
*/

/*!
  \fn bool PimEvent::showOnNearest() const

  Returns if the event should be shown on the nearest match of an occurrence
  if the exact date the event would occur is not a valid date.

  \sa setShowOnNearest()
*/

/*!
  \fn bool PimEvent::isAllDay() const

  Returns TRUE if the event is an all day event.  Otherwise returns FALSE.

  \sa setAllDay()
*/

/*!
  \fn bool PimEvent::isTravel() const
  \internal
*/

/*!
  \fn QDateTime PimEvent::start() const

  Returns when the first occurrence of the event starts.

  \sa startInCurrentTZ() setStart()
*/

/*!
  \fn QDateTime PimEvent::end() const

  Returns when the first occurrence of the event starts.

  \sa endInCurrentTZ() setEnd()
*/

/*!
  \enum PimEvent::EventFields
  \internal
*/

/*!
  \fn QMap<int,QString> PimEvent::fields() const
  \internal
*/

/*!
  Constructs a new PimEvent.
*/
PimEvent::PimEvent() : PimRecord()
{
    QDateTime start = QDateTime::currentDateTime();
    QDateTime end = start.addSecs(5 * 60);
    init(start, end);
}

/*!
  Constructs a new PimEvent starting at \a start and running till \a end.
*/
PimEvent::PimEvent(const QDateTime &start, const QDateTime &end)
{
    // smallest event 5 minutes
    if (end < start.addSecs(5 * 60))
	init(start, start.addSecs(5 * 60));
    else 
	init(start, end);
}

/*!
  \internal
*/
void PimEvent::fromMap( const QMap<int,QString> &m )
{
    QDateTime start = QDateTime::currentDateTime();
    QDateTime end = start.addSecs(5 * 60);
    init(start, end);

    setFields( m );
}

void PimEvent::init(const QDateTime &s, const QDateTime &e)
{
    mStart = s;
    mEnd = e;
    weekMask = 0;

    mHasAlarm = FALSE;
    mAlarmDelay = 0;
    mAlarmSound = Silent;

    mRepeatTill = mStart.date();
    mHasEndDate = FALSE;
    mType = NoRepeat;
    mFrequency = 1;
    mShowOnNearest = FALSE;
    mAllDay = FALSE;
}

/*!
  Destroys a PimEvent.
*/
PimEvent::~PimEvent()
{
}

/*!
  Sets the description of the event to \a text.

  \sa description()
*/
void PimEvent::setDescription( const QString &text )
{
    mDescription = text;
}

/*!
  Set the location of the event to \a text.

  \sa location()
*/
void PimEvent::setLocation( const QString &text )
{
    mLocation = text;
}

/*!
  Sets the start time of the event to \a time.
  This will change the end time fo the event to maintain the same duration.

  \sa start(), startInCurrentTZ()
*/
void PimEvent::setStart( const QDateTime &time )
{
    // get existing, dependent state.
    int duration = mStart.secsTo(mEnd);

    // move the start, (end moves with it)
    mStart = time;
    mEnd = mStart.addSecs(duration);

    if (mRepeatTill < mEnd.date())
	mRepeatTill = mEnd.date();
}

/*!
  Sets the end time of the event to \a time.
  This will also change the duration of the event. \a time must be at least
  5 minutes after the start time of the event.

  \sa end(), endInCurrentTZ()
*/
void PimEvent::setEnd( const QDateTime &time )
{
    if (time < mStart.addSecs(60 * 5))
	return; // can't set end before start.

    mEnd = time;

    // make sure repeatForever and repeatTill are still valid.
    if (mRepeatTill < mEnd.date())
	mRepeatTill = mEnd.date();
}

/*!
  Sets the notes of the event to \a text.

  \sa notes()
*/
void PimEvent::setNotes( const QString &text )
{
    mNotes = text;
}

/*!
  Sets the time zone of the event to \a zone.
  This will affect when the event occurs in UTC.  All day events cannot have
  a time zone set.

  Setting the time zone to an invalid TimeZone will cause the event to
  have no associated time zone.

  \sa timeZone(), isAllDay()
*/
void PimEvent::setTimeZone( const TimeZone &zone )
{
    mTimeZone = zone;
}

/*!
  \internal for the moment. finish documenting when adding travel events.
  Sets the end TimeZone of the event.
*/
void PimEvent::setEndTimeZone( const TimeZone &zone )
{
    mEndTimeZone = zone;
}

/*!
  Sets an alarm for the event, \a minutes before the start of the event, with
  an alarm type of \a s.

  \sa clearAlarm(), hasAlarm(), alarmSound(), alarmDelay()
*/
void PimEvent::setAlarm( int minutes, SoundTypeChoice s )
{
    mHasAlarm = TRUE;
    mAlarmDelay = minutes;
    mAlarmSound = s;
}

/*!
  Clears any alarms set for the event.

  \sa setAlarm()
*/
void PimEvent::clearAlarm()
{
    mHasAlarm = FALSE;
}

/*!
  Sets the repeat type of the event to \a t.

  \sa repeatType(), hasRepeat(), RepeatType
*/
void PimEvent::setRepeatType( RepeatType t )
{
    mType = t;
}

/*!
  Sets the frequency of the event to \a freq.  If \a freq is less than 1
  will set the frequency of the event to 1.

  \sa frequency()
*/
void PimEvent::setFrequency( int freq )
{
    mFrequency = freq > 0 ? freq : 1;
}

/*!
  Sets the date the event will repeat until to \a date.  If date is before
  the end of the first event will set the event to occur once.

  \sa repeatTill(), repeatForever()
*/
void PimEvent::setRepeatTill( const QDate &date )
{
    mRepeatTill = date > mEnd.date() ? date : mEnd.date();
    mHasEndDate = TRUE;
}

/*!
  Sets whether the event is an an all day event to \a enable.
  All day events have a no set time-zone (empty string).

  \sa isAllDay(), setTimeZone()
*/
void PimEvent::setAllDay( bool enable )
{
    mAllDay = enable;
}

/*!
  Returns the time zone of the event or an invalid TimeZone if the event has
  no time zone.  All day events allways have no time zone.

  \sa setTimeZone(), isAllDay()
*/
TimeZone PimEvent::timeZone() const
{
    return mAllDay ? TimeZone() : mTimeZone;
}

/*!
  \internal
*/
TimeZone PimEvent::endTimeZone() const
{
    return mAllDay ? TimeZone() : mEndTimeZone;
}


/*!
  Returns the date the event will repeat until

  \sa repeatForever(), setRepeatTill()
*/
QDate PimEvent::repeatTill() const
{
    if (!hasRepeat())
	return mEnd.date();
    return mRepeatTill;
}

/*!
  Returns TRUE if the event will repeat forever.  Otherwise returns FALSE.

  \sa repeatTill(), setRepeatForever()
*/
bool PimEvent::repeatForever( ) const
{
    if (!hasRepeat())
	return FALSE;
    return !mHasEndDate;
}

/*!
  Returns the instance of the week the event will occur for the repeat types
  MonthlyDay and MonthlyEndDay.  The week offset is determined by the starting
  date of the event.

  if (weekOffset() == 1) event occurs in first week of the month.
  if (weekOffset() == 3) event occurs in third week of the month.
  if (weekOffset() == -1) event occurs in last week of the month.

  Returns 0 if there is no week offset for the event.
*/
int PimEvent::weekOffset( ) const
{
    if (mType == MonthlyDay) {
	if (mStart.date().day() == 1)
	    return 1;
	return (mStart.date().day() - 1) / 7 + 1;
    } else if (mType == MonthlyEndDay) {
	if (mStart.date().day() == mStart.date().daysInMonth())
	    return -1;
	return -((mStart.date().daysInMonth() - mStart.date().day()) / 7 + 1);
    }
    return 0;
}

/*!
  If \a b is TRUE then sets the event to repeat forever.  Otherwise event
  will repeat until repeatUntil()

  \sa repeatForever()
*/
void PimEvent::setRepeatForever( bool b )
{
    //if (b == repeatForever())
	//return;
    mHasEndDate = !b;
}

/*!
  Sets whether to show a repeating event on the nearest previous date if the
  day it would repeat on does not exist to \a b.

  An example would be a repeating event that occures on the 31st of each month.
  Setting showOnNearest to TRUE will have the event show up on the 30th on
  months that do not have 31 days, (or 28/29 in the case of Febuary).

  \sa showOnNearest()
*/
void PimEvent::setShowOnNearest( bool b)
{
    mShowOnNearest = b;
}

/*!
  If the repeat type of the event is Weekly and the event is set to repeat on
  \a day of the week, then returns TRUE.  Otherwise returns FALSE.

  \sa setRepeatOnWeekDay()
*/
bool PimEvent::repeatOnWeekDay(int day) const
{
    if (repeatType() != Weekly)
	return FALSE;

    if (day == mStart.date().dayOfWeek())
	return TRUE; // always repeat on the start day of week.

    return ((1 << (day - 1) & weekMask) != 0);
}

/*!
  Sets the event to repeat on the \a day of the wekif \a enable is TRUE.
  Otherwise sets the event not to repeat on the \a day of the week.

  Event will always repeat on the day of the week that it started on.

  \sa repeatOnWeekDay()
*/
void PimEvent::setRepeatOnWeekDay(int day, bool enable)
{
    if (repeatOnWeekDay(day) != enable)
	weekMask ^= 1 << (day - 1);
}

// helper functions
int monthsTo(const QDate &from, const QDate &to)
{
    int result = 12 * (to.year() - from.year());
    result += (to.month() - from.month());

    return result;
}

// weeks is the number of times this dayOfWeek of the
// week appears in the month for this date.
int weeksForDayInMonth(int dayOfWeek, const QDate &date)
{
    QDate result;
    result.setYMD(date.year(), date.month(), 1);
    int dayOfMonth = result.dayOfWeek();

    int fromStart = dayOfWeek - dayOfMonth;
    if (fromStart < 0)
	fromStart += 7;

    return (result.daysInMonth() - fromStart - 1) / 7 + 1;
}
/*! \fn PimEvent::seriesUid() const
  If the event is an exception to a recuring event
  returns the UID for the event this event is an exception to.
  Otherwise returns the UID of this event.

  \sa hasExceptions(), isException()
*/

/*!
  Returns TRUE if this event represents an exception to the repeat
  pattern of another event.  Otherwise returns FALSE.
  
  For example if a daily event at 10am
  starts on 11am one day, the 11am would be represent an exception
  to the 10am repeating event.

  \sa hasExceptions(), seriesUid()
*/
bool PimEvent::isException() const
{
    return !mParent.isNull();
}

/*!
  Returns TRUE if the event is a repeating event 
  that has exceptions to the repeat pattern for that event.
  Otherwise returns FALSE.

  \sa isException(), seriesUid()
*/
bool PimEvent::hasExceptions() const
{
    return !mExceptions.isEmpty();
}

/*! \internal */
void PimEvent::setSeriesUid( const QUuid &u )
{
    mParent = u;
}

/*! \internal */
void PimEvent::addException( const QDate &d, const QUuid &u )
{
    mExceptions.append(d);
    mChildren.append(u);
}

/*!
  Returns TRUE if the event can occur.  Otherwise returns FALSE.
*/
bool PimEvent::isValid() const
{
    bool ok;
    nextOccurrence(start().date(), &ok);
    if ( !ok && mChildren.isEmpty() )  {
	// no longer exist!
	return FALSE;
    }
    return TRUE;
}

/*! \internal */
void PimEvent::clearExceptions()
{
    mExceptions.clear();
    mChildren.clear();
}

/*! \internal */
void PimEvent::removeException( const QDate &d )
{
    QValueList<QDate>::Iterator eit = mExceptions.begin();
    QValueList<QUuid>::Iterator cit = mChildren.begin();

    for(; eit != mExceptions.end() && cit != mChildren.end(); ++eit, ++cit) {
	if (*eit == d) {
	    mExceptions.remove(eit);
	    mChildren.remove(cit);
	    break;
	}
    }
}

/*! \internal */
void PimEvent::removeException( const QUuid &u )
{
    QValueList<QDate>::Iterator eit = mExceptions.begin();
    QValueList<QUuid>::Iterator cit = mChildren.begin();

    for(; eit != mExceptions.end() && cit != mChildren.end(); ++eit, ++cit) {
	if (*cit == u) {
	    mExceptions.remove(eit);
	    mChildren.remove(cit);
	    break;
	}
    }
}

/*!
  Returns the first date on or after \a from that the event will next occur.
  If the event only occurs once (no repeat) will return the date of the
  start of the event if the start of the event is on or after \a from.

  If \a ok is non-NULL, *ok is set to TRUE if the event occurs on or
  after \a from and FALSE if the event does not occur on or after
  \a from.
*/
QDate PimEvent::nextOccurrence( const QDate &from, bool *ok) const
{
    bool stillLooking;
    QDate looking = p_nextOccurrence(from, &stillLooking);
    while (stillLooking && mExceptions.contains(looking)) {
	looking = p_nextOccurrence(looking.addDays(p_duration()), &stillLooking);
    }
    if (ok)
	*ok = stillLooking;
    return looking;
}

int PimEvent::p_duration() const
{
    return mStart.daysTo(mEnd) + 1;
}

/*!
  \internal
  Does the work of nextOccurence, apart from the exception check
*/
QDate PimEvent::p_nextOccurrence( const QDate &from, bool *ok) const
{
    QDate result = mStart.date();
    // from should be for the start of the possible event.
    QDate after = from.addDays(mEnd.daysTo(mStart));

    if (result >= after) {
	if (ok)
	    *ok = TRUE;
	return result;
    }

    if (!repeatForever() && repeatTill() < after) {
	if (ok)
	    *ok = FALSE;
	return result;
    }

    switch(mType) {
	default:
	    if (ok)
		*ok = FALSE;
	    return result;
	case NoRepeat:
	    if (ok)
		*ok = FALSE;
	    return result;
	case Daily:
	    {
		int daysBetween = mStart.daysTo(after);
		int outBy = daysBetween % mFrequency;
		if (outBy) {
		    // this is in the wrong direction;
		    outBy = mFrequency - outBy;
		}
		result = after.addDays(outBy);
	    }
	    break;
	case Weekly:
	    {
		// first do a quick check to see if it i possble that there
		// is an overlap.
		int diff = result.daysTo(after);
		int mod = diff % (mFrequency * 7);

		// if diff is < 7, it may be not the day of week of the start
		// day.  Check.  Don't look more than 6 days after the start
		// day.
		if (mod < 7) {
		    // go after % to 6.  if day of week match, to a normal but
		    // start with that day.
		    for (int i = mod; i < 7; i++) {
			if (repeatOnWeekDay(result.addDays(i).dayOfWeek())) {
			    // move result forward to that week day
			    result = result.addDays(i);
			    break;
			}
		    }
		}

		// Now treat as a regular, daily, Freq*7 event.

		// Remember, new result may now be after after.  Check
		int daysBetween = result.daysTo(after);
		if (daysBetween > 0) {
		    int outBy = daysBetween % (mFrequency * 7);
		    if (outBy) {
			outBy = (mFrequency * 7) - outBy;
		    }
		    result = after.addDays(outBy);
		}
	    }
	    break;
	case MonthlyDate:
	    {
		int monthsBetween = monthsTo(mStart.date(), after);

		// check to see if will be in after month.
		if (mStart.date().day() < after.day()) {
		    // wont be in after month, move to the next month.
		    monthsBetween++;
		    after = Calendar::addMonths(1, after);
		}

		int outBy = monthsBetween % mFrequency;
		if (outBy) {
		    outBy = mFrequency - outBy;
		}
		result = Calendar::addMonths(outBy, after);

		if (mShowOnNearest) {
		    if (mStart.date().day() < result.daysInMonth())
			result.setYMD(result.year(), result.month(), mStart.date().day());
		    else
			result.setYMD(result.year(), result.month(), result.daysInMonth());
		} else {
		    // can't show on nearest, when is the next valid date.
		    while (!QDate::isValid(
				result.year(), result.month(), mStart.date().day())
			    ) {
			result = Calendar::addMonths(mFrequency, result);
		    }
		    result.setYMD(result.year(), result.month(), mStart.date().day());
		}
	    }
	    break;
	case MonthlyDay:
	case MonthlyEndDay:
	    {
		// + for MonthlyDay, - for MonthlyEndDay.
		// otherwise these are basically the same.
		int mWeekOffset = weekOffset();

		int monthsBetween = monthsTo(mStart.date(), after);
		int outBy = monthsBetween % mFrequency;
		if (outBy) {
		    outBy = mFrequency - outBy;
		}
		result = Calendar::addMonths(outBy, after);

		// this is tricky.  Need to move by mFreq months till we
		// get a good one.
		bool foundDate = FALSE;
		while(!foundDate) {
		    int day;
		    int weeks = weeksForDayInMonth(mStart.date().dayOfWeek(), result);
		    // get to first day for that day of week.
		    int weekShift = mStart.date().dayOfWeek()
			- QDate(result.year(), result.month(), 1).dayOfWeek();
		    if (weekShift < 0)
			weekShift += 7;

		    if (mWeekOffset > 0) {
			day = (mWeekOffset - 1) * 7 + 1 + weekShift;
		    } else {
			day = (weeks + mWeekOffset) * 7 + 1 + weekShift;
		    }

		    if (mShowOnNearest) {
			if (day > result.daysInMonth())
			    day -= 7;
			if (day < 1)
			    day += 7;
		    } else {
			if (day > result.daysInMonth() || day < 1) {
			    result = Calendar::addMonths(mFrequency, result);
			    continue;
			}
		    }

		    result.setYMD(result.year(), result.month(), day);
		    if (result < after)
			result = Calendar::addMonths(mFrequency, result);
		    else
			foundDate = TRUE; // success.
		}
	    }
	    break;
	case Yearly:
	    {
		int yearsBetween = after.year() - mStart.date().year();
		int outBy = yearsBetween % mFrequency;
		if (outBy) {
		    outBy = mFrequency - outBy;
		}

		result = Calendar::addYears(yearsBetween + outBy, mStart.date());

		if (result < after)
		    result = Calendar::addYears(mFrequency, result);

		// at least after, may not be valid though.
		if (!mShowOnNearest) {
		    while (!QDate::isValid(result.year(), result.month(), mStart.date().day()))
			result = Calendar::addYears(mFrequency, result);
		    result.setYMD(result.year(), result.month(), mStart.date().day());
		}
	    }
	    break;
    }
    if (ok)
	*ok = (result <= repeatTill() || repeatForever());

    return result;
}

// In pimrecord.cpp
void qpe_startVObjectInput();
bool qpe_vobjectCompatibility(const char* misfeature);
void qpe_endVObjectInput();
void qpe_startVObjectOutput();
void qpe_setVObjectProperty(const QString&, const QString&, const char* type, PimRecord*);
void qpe_endVObjectOutput(VObject *,const char* type,const PimRecord*);
VObject *qpe_safeAddPropValue( VObject *o, const char *prop, const QString &value );
static inline VObject *safeAddPropValue( VObject *o, const char *prop, const QString &value )
{ return qpe_safeAddPropValue(o,prop,value); }
VObject *qpe_safeAddProp( VObject *o, const char *prop);
static inline VObject *safeAddProp( VObject *o, const char *prop)
{ return qpe_safeAddProp(o,prop); }

const char *dayToString(int d)
{
    switch (d) {
	case 1:
	    return "MO ";
	case 2:
	    return "TU ";
	case 3:
	    return "WE ";
	case 4:
	    return "TH ";
	case 5:
	    return "FR ";
	case 6:
	    return "SA ";
	case 7:
	default:
	    return "SU ";
    }
}

static VObject *createVObject( const PimEvent &e )
{
    qpe_startVObjectOutput();

    VObject *vcal = newVObject( VCCalProp );
    safeAddPropValue( vcal, VCVersionProp, "1.0" );
    VObject *event = safeAddProp( vcal, VCEventProp );

    bool timeAsUTC = FALSE;
    QString start, end;
    if ( !e.isAllDay() ) {
	// don't give UTC times if we don't have a timezone
	timeAsUTC = e.timeZone().isValid();
	start = TimeConversion::toISO8601( e.start(), timeAsUTC );
	end = TimeConversion::toISO8601( e.end(), timeAsUTC );
    } else {
	start = TimeConversion::toISO8601( e.start().date(), FALSE );
	end = TimeConversion::toISO8601( e.end().date(), FALSE );
    }
    safeAddPropValue( event, VCDTstartProp, start );
    safeAddPropValue( event, VCDTendProp, end );

    // vCal spec: VCSummaryProp is required
    // Palm m100:     Yes (but accepts VCDescriptionProp VCAttachProp)
    // SL5500:        No
    // Ericsson T39m: Yes
    if ( qpe_vobjectCompatibility("Palm-Event-DN") ) {
	safeAddPropValue( event, VCDescriptionProp, e.description() );
	safeAddPropValue( event, VCAttachProp, e.notes() );
    } else {
	safeAddPropValue( event, VCSummaryProp, e.description() );
	safeAddPropValue( event, VCDescriptionProp, e.notes() );
    }

    safeAddPropValue( event, VCLocationProp, e.location() );

    if ( e.hasAlarm() ) {
	QDateTime dt = e.start();
	dt = dt.addSecs( -e.alarmDelay()*60 );
	VObject *alarm = safeAddProp( event, VCDAlarmProp );
	safeAddPropValue( alarm, VCRunTimeProp, 
		TimeConversion::toISO8601( dt, timeAsUTC ) );
	if (e.alarmSound() != PimEvent::Silent)  {
	    VObject *aalarm = safeAddProp( event, VCAAlarmProp );
	    safeAddPropValue( aalarm, VCRunTimeProp, 
		    TimeConversion::toISO8601( dt , timeAsUTC ) );
	}
    }

    if (timeAsUTC)
	safeAddPropValue( event, "X-Qtopia-TIMEZONE", e.timeZone().id() );

    if (e.hasRepeat()) {
	// minimal data.  if its optional and we want the default, stay quiet.
	QString repeat_format;
	switch (e.repeatType())
	{
	    default:
	    case PimEvent::NoRepeat:
		break;
	    case PimEvent::Daily:
		repeat_format = "D%1 %2";
		break;
	    case PimEvent::Weekly:
		repeat_format = "W%1 ";
		{
		    for (int i = 1; i < 8; i++) {
			if (e.repeatOnWeekDay(i)) {
			    repeat_format += dayToString(i);
			}
		    }
		}
		repeat_format += "%2";
		break;
	    case PimEvent::MonthlyDate:
		repeat_format = "MD%1 %2";
		break;
	    case PimEvent::MonthlyDay:
		repeat_format = "MP%3 %1 %2%4";
		repeat_format = repeat_format.arg(e.weekOffset());
		repeat_format = repeat_format.arg(dayToString(e.start().date().dayOfWeek()));
		// other stuff is default.
		break;
	    case PimEvent::MonthlyEndDay:
		repeat_format = "MP%3 %1- %2%4";
		repeat_format = repeat_format.arg(-e.weekOffset());
		repeat_format = repeat_format.arg(dayToString(e.start().date().dayOfWeek()));
		break;
	    case PimEvent::Yearly:
		repeat_format = "YM%1 %2";
		break;
	}

	repeat_format = repeat_format.arg(e.frequency())
	    .arg(
		    e.repeatForever() ? "#0" : (const char *)TimeConversion::toISO8601(e.repeatTill(), FALSE));

	safeAddPropValue( event, VCRRuleProp, repeat_format );
	// Palm enters exceptions, but not their parents/not-parents.
	// We will to because the uid's are meaningless outside of the little world of one
	// device and one Qtopia Desktop.

	if (e.hasExceptions()) {
	    QValueList<QDate>::ConstIterator it;
	    QString val;
	    const QValueList<QDate> datelist = ((PrEvent &)e).exceptions();
	    for (it = datelist.begin(); it != datelist.end(); it++) {
		if (it != datelist.begin())
		    val += " ";
		val += TimeConversion::toISO8601(*it, FALSE);
	    }
	    // list of ISO (not UTC) dates.  may need to express as datetimes.
	    safeAddPropValue( event, VCExpDateProp, val );
	}
    }

    qpe_endVObjectOutput(event,"Calendar",&e); // No tr

    return vcal;
}

static void parseRrule( PimEvent &e, const QString &v)
{
    QString value = v.simplifyWhiteSpace();
    enum state {
	type,
	interval,
	occurrencelist,
	weekdaylist,
	daynumberlist,
	monthlist,
	daylist, // not used, we don't do this type.
	duration,
    };
    state st = type; // state;
    int i = 0; // index;
    int acc = 0; // for building ints.

    for (i = 0; i < (int)value.length(); i++) {
	switch (st) {
	    case type: // repeat class/type
		// work out the basic rule type.
		if (value[i] == QChar('D')) {
		    e.setRepeatType(PimEvent::Daily);
		} else if (value[i] == QChar('W')) {
		    e.setRepeatType(PimEvent::Weekly);
		} else if (value[i] == QChar('M')) {
		    i++;
		    if (i >= (int)value.length())
			break;

		    // may need to change from MonthlyDay to MonthlyEndDay
		    // later.
		    if (value[i] == QChar('P'))
			e.setRepeatType(PimEvent::MonthlyDay);
		    else 
			e.setRepeatType(PimEvent::MonthlyDate);
		} else if (value[i] == QChar('Y')) {
		    i++;
		    if (i >= (int)value.length() ||
			    value[i] != QChar('M')) {
			// force exit from lup
			i = (int)value.length();
			break;;  // only know Yearly Month.
		    }
		    e.setRepeatType(PimEvent::Yearly);
		}
		st = interval; // frequency;
		break;
	    case interval: // repeat frequency
		if (value[i].isSpace()) {
		    // finished frequency;
		    e.setFrequency(acc);
		    if (e.repeatType() == PimEvent::Daily)
			st = duration; // duration;
		    else if (e.repeatType() == PimEvent::Weekly)
			st = weekdaylist;
		    else if (e.repeatType() == PimEvent::MonthlyDay)
			st = occurrencelist;
		    else if (e.repeatType() == PimEvent::MonthlyDate)
			st = daylist;
		    else 
			st = monthlist;
		    acc = 0;
		} else if (value[i].isDigit()) {
		    if (acc)
			acc *=10;
		    acc += value[i].digitValue();
		} else {
		    // fail.  parse error.
		    qDebug("failed to parse RRULE: non-digit in frequency");
		    return;
		}
		break;
	    case occurrencelist:
		// this could actually be duration.
		// read next two to check, i+1 should be either 
		// a + or a -;
		if (i+1 < (int)value.length() &&
			value[i].isDigit()) {
		    // what the digit is won't help, we always work
		    // of the start date.
		    if (value[i+1] == QChar('+')) {
			e.setRepeatType(PimEvent::MonthlyDay);
			i += 2; // get past inevitable ' '
			st = weekdaylist;
			break;
		    } else if (value[i+1] == QChar(' ')) {
			e.setRepeatType(PimEvent::MonthlyDay);
			i += 1; // get past ' '
			st = weekdaylist;
			break;
		    } else if (value[i+1] == QChar('-')) {
			e.setRepeatType(PimEvent::MonthlyEndDay);
			st = weekdaylist;
			i += 2; // get past inevitable ' '
			break;;
		    }
		}
		// not an occurance list, but still monthly day.
		i--;
		st = weekdaylist;;
		break;
	    case weekdaylist:
		// end on digit or #.  otherwise try it as a day.
		if (value[i] == QChar('#') || value[i].isDigit()) {
		    st = duration;
		    i--;
		    break;
		}
		// read the next 2/3 (if third is space)
		if (i+1 >= (int)value.length()) {
		    i = (int)value.length();
		    break;  // only know Yearly Month.
		}
		if (value[i] == QChar('M'))
		    e.setRepeatOnWeekDay(1, TRUE);
		else if (value[i] == QChar('T') && 
			value[i+1] == QChar('U'))
		    e.setRepeatOnWeekDay(2, TRUE);
		else if (value[i] == QChar('W'))
		    e.setRepeatOnWeekDay(3, TRUE);
		else if (value[i] == QChar('T') && 
			value[i+1] == QChar('H'))
		    e.setRepeatOnWeekDay(4, TRUE);
		else if (value[i] == QChar('F'))
		    e.setRepeatOnWeekDay(5, TRUE);
		else if (value[i] == QChar('S') && 
			value[i+1] == QChar('A'))
		    e.setRepeatOnWeekDay(6, TRUE);
		else if (value[i] == QChar('S') && 
			value[i+1] == QChar('U'))
		    e.setRepeatOnWeekDay(7, TRUE);

		// no to the inc.
		i += 2;  // safe, as either while will cut out, or i+1 would be a ' '
		break;
		// can't use either of these of these.
	    case daylist:
		// FALL THROUGH
	    case daynumberlist:
		// FALL THROUGH
	    case monthlist:
		// find the optional duration.
		// find a # or more than 3 digets from end.
		// note we will be at the start of the string here.
		{
		    int space = value.findRev(QChar(' '));
		    if (space < i-1) {
			i = (int)value.length();
			break;
		    }
		    if (space + 4 < (int)value.length()
			    || value[i] == QChar('#')) {
			i = space;
			st = duration;
		    } else {
			i = (int)value.length();
			break;
		    }
		}
		break;
	    case duration: // repeat duration
		// duration.
		// expect either a # or a datetime.
		// if # just finish of the number now.
		if (value[i] == QChar('#')) {
		    i++;
		    acc = 0;
		    while (i < (int)value.length()) {
			if (value[i].isDigit()) {
			    acc *= 10;
			    acc += value[i].digitValue();
			} else {
			    qDebug("failed to parse RRULE: non-digit in duration count");
			}
			i++;
		    }
		    // if 0, repeat forever.  if anything else will need
		    if (acc == 0)
			e.setRepeatForever(TRUE);
		    i = (int)value.length();
		    // XXX Could add code to work out the count -> date.
		} else {
		    // from hear till the end is an ISO value, and we want it.
		    e.setRepeatTill(
			    TimeConversion::fromISO8601(
				QCString(value.mid(i))).date());
		    i = (int)value.length();
		}
	}
    }

    // out without errors.
    if (!(e.frequency() % 12) && e.repeatType() == PimEvent::MonthlyDate)
    {
	e.setRepeatType(PimEvent::Yearly);
	e.setFrequency(e.frequency() / 12);
    }
}

static PimEvent parseVObject( VObject *obj )
{
    PimEvent e;

    bool haveAlarm = FALSE;
    bool haveStart = FALSE;
    bool haveEnd = FALSE;
    QDateTime alarmTime;
    QDateTime startTime;
    PimEvent::SoundTypeChoice soundType = PimEvent::Silent;

    static CP1252TextCodec *palmCodec = 0;
    if( !palmCodec )
        palmCodec = new CP1252TextCodec();
    
    VObjectIterator it;
    initPropIterator( &it, obj );
    QString summary, description, attach; // vCal properties, not Qtopias
    while( moreIteration( &it ) ) {
	VObject *o = nextVObject( &it );
	QCString name = vObjectName( o );

	// check this key/value for a CHARSET field.
	VObjectIterator tnit;
	initPropIterator( &tnit, o );
	QTextCodec *tc = 0;
	while( moreIteration( &tnit ) ) {
	    VObject *otc = nextVObject( &tnit );
	    if ( qstrcmp(vObjectName(otc), VCCharSetProp ) == 0) {
		tc = QTextCodec::codecForName(vObjectStringZValue(otc));
		break;
	    }
	}
	QString value;
	if (tc)
	    value = tc->toUnicode( vObjectStringZValue( o ) );
	else
	    value = vObjectStringZValue( o );


	// XXX We may need to modify this by timezone later if there
	// is a timezone.  if you have a recieve event bug, check
	// this first.
	if ( name == VCDTstartProp ) {
	    // check string-length.  if no time, its an allday.
	    if (value.length() == 8)
		e.setAllDay(TRUE);
	    startTime = TimeConversion::fromISO8601( QCString(value) );
	    e.setStart( startTime );
	    haveStart = TRUE;
	}
	else if ( name == VCDTendProp ) {
	    e.setEnd( TimeConversion::fromISO8601( QCString(value) ) );
	    haveEnd = TRUE;
	}
	// X-Qtopia-NOTES is for 1.5.0 compatibility
	else if ( name == "X-Qtopia-NOTES" || name == VCAttachProp) {
	    attach = value;
	}
	else if ( name == VCSummaryProp ) {
	    summary = value;
	}
	else if ( name == VCDescriptionProp ) {
	    description = value;
	}
	else if ( name == VCLocationProp ) {
	    e.setLocation( value );
	}
	else if ( name == VCAAlarmProp || name == VCDAlarmProp ) {
	    haveAlarm = TRUE;
	    VObjectIterator nit;
	    initPropIterator( &nit, o );
	    while( moreIteration( &nit ) ) {
		VObject *o = nextVObject( &nit );
		QCString subname = vObjectName( o );
		QCString subvalue = vObjectStringZValue( o );
		if ( subname == VCRunTimeProp ) {
		    alarmTime = TimeConversion::fromISO8601( subvalue );
		}
	    }

	    if ( name == VCAAlarmProp )
		soundType = PimEvent::Loud;
	}
	// We don't use VCTimeZoneProp as that has the form +05:30.
	// We are a bit better at timezones than this so we need the actual name
	// We _may_ want to use VCGeoLocationProp ( lang long )
	else if ( name == "X-Qtopia-TIMEZONE") {
	    e.setTimeZone( TimeZone(value) );
	}
	else if ( name == VCRRuleProp) {
	    parseRrule(e, value);
	}
	else if ( name == VCExpDateProp ) {
	    QStringList list = QStringList::split(' ', value);
	    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
		((PrEvent&)e).addException(TimeConversion::fromISO8601(QCString(*it)).date());
	    }
	} else {
	    qpe_setVObjectProperty(name,value,"Calendar",&e); // No tr
	}
    }

    // Find best mapping from (Summary,Description,Attach) to our (Description,Notes)
    // Similar code in task.cpp
    if ( !!summary && !!description && summary != description ) {
	e.setDescription( summary );
	e.setNotes( description );
	// all 3 - drop attach
    } else if ( !!summary ) {
	e.setDescription( summary );
	e.setNotes( attach );
    } else {
	e.setDescription( description );
	e.setNotes( attach );
    }

    if ( !haveStart && !haveEnd )
	e.setStart( QDateTime::currentDateTime() );

    if ( !haveEnd ) {
	QDateTime end = e.start();
	end.setTime(QTime(32, 59, 59));
	e.setEnd( end );
    }

    if ( haveAlarm ) {
	int minutes = alarmTime.secsTo( startTime ) / 60;
	e.setAlarm( minutes, soundType );
    }
    return e;
}

/*!
   Write the list of \a events as vCalendar objects to the file
   specified by \a filename.

   \sa readVCalendar()
*/
void PimEvent::writeVCalendar( const QString &filename, const QValueList<PimEvent> &events)
{
    QFileDirect f( filename.utf8().data() );
    if ( !f.open( IO_WriteOnly ) ) {
	qWarning("Unable to open vcard write");
	return;
    }

    QValueList<PimEvent>::ConstIterator it;
    for( it = events.begin(); it != events.end(); ++it ) {
	VObject *obj = createVObject( *it );
	writeVObject( f.directHandle() , obj );
	cleanVObject( obj );
    }

    cleanStrTbl();
}

/*!
   Write the \a event as a vCalendar to the file specified by \a filename.

   \sa readVCalendar()
*/
void PimEvent::writeVCalendar( const QString &filename, const PimEvent &event)
{
    QFileDirect f( filename.utf8().data() );
    if ( !f.open( IO_WriteOnly ) ) {
	qWarning("Unable to open vcard write");
	return;
    }

    VObject *obj = createVObject( event );
	writeVObject( f.directHandle() , obj );
	cleanVObject( obj );

	cleanStrTbl();
}

/*!
  Reads the file specified by \a filename as a list of vCalendar objects
  and returns the list of near equivalent events.

  \sa writeVCalendar()
*/
QValueList<PimEvent> PimEvent::readVCalendar( const QString &filename )
{
    VObject *obj = Parse_MIME_FromFileName( (char *)filename.utf8().data() );

    QValueList<PimEvent> events;

    qpe_startVObjectInput();
    while ( obj ) {
	QCString name = vObjectName( obj );
	if ( name == VCCalProp ) {
	    VObjectIterator nit;
	    initPropIterator( &nit, obj );
	    while( moreIteration( &nit ) ) {
		VObject *o = nextVObject( &nit );
		QCString name = vObjectName( o );
		if ( name == VCEventProp )
		    events.append( parseVObject( o ) );
	    }
	} else if ( name == VCEventProp ) {
	    // shouldn't happen, but just to be sure
	    events.append( parseVObject( obj ) );
	}
	VObject *t = obj;
	obj = nextVObjectInList(obj);
	cleanVObject( t );
    }
    qpe_endVObjectInput();

    return events;
}

/*!
  returns the color associated with the event.
*/
QColor PimEvent::color() const
{
    return color(mType != NoRepeat);
}

/*!
  \internal
*/
QColor PimEvent::color(bool t)
{
    if (t)
	return QColor(0,0,255);
    else
	return QColor(255,0,0);
}

/*!
  Returns the start time of the event in the current system timezone.

  \sa start()
*/
QDateTime PimEvent::startInCurrentTZ() const
{
    if (!timeZone().isValid())
	return start();

    // if no zone given.. assume local
    return timeZone().toCurrent(start());
}

/*!
  Returns the end time of the event in the current system timezone.

  \sa end()
*/
QDateTime PimEvent::endInCurrentTZ() const
{
    if (!timeZone().isValid())
	return end();

    QDateTime nStart = timeZone().toCurrent(start());

    return end().addSecs(start().secsTo(nStart));
}

/*!
  Returns the date the event will repeat till in the current system timezone.

  \sa repeatTill()
*/
QDate PimEvent::repeatTillInCurrentTZ() const
{
    if (!timeZone().isValid())
	return repeatTill();

    // duration should be the same... shift the start.. dif to end.
    // if no zone given.. assume local
    QDateTime nStart = timeZone().toCurrent(start());

    QDateTime rtDateTime = QDateTime(repeatTill(), QTime(0,0,0));
    return rtDateTime.addSecs(start().secsTo(nStart)).date();
}

time_t asUTC(const QDateTime &dt, const TimeZone &z)
{
    if (z.isValid())
	return z.toTime_t(dt);
    else
	return TimeZone::utc().toTime_t(dt);
}

QDateTime asDateTime( time_t dt, const TimeZone &z)
{
    if (z.isValid())
	return z.fromTime_t(dt);
    else
	return TimeZone::utc().fromTime_t(dt);
}

/*!
  \internal
*/
time_t PimEvent::startAsUTC() const
{
    return asUTC(start(), timeZone());
}

/*!
  \internal
*/
time_t PimEvent::endAsUTC() const
{
    return asUTC(end(), timeZone());
}

/*!
  \internal
*/
time_t PimEvent::repeatTillAsUTC() const
{
    return asUTC(repeatTill(), timeZone());
}

/*!
  \internal
*/
void PimEvent::setStartAsUTC(time_t time)
{
    setStart(asDateTime(time, timeZone()));
}

/*!
  \internal
*/
void PimEvent::setEndAsUTC(time_t time)
{
    setEnd(asDateTime(time, timeZone()));
}

/*!
  \internal
*/
void PimEvent::setRepeatTillAsUTC(time_t time)
{
    setRepeatTill(asDateTime(time, timeZone()).date());
}

Occurrence::Occurrence(const QDate &start, const PimEvent &ca ) : eventCache(ca), mStart(start)
{
}

PimEvent Occurrence::event() const
{
    return eventCache;
}


QDate Occurrence::endDate() const
{
    return date().addDays(eventCache.start().date().daysTo(eventCache.end().date()));
}

QDateTime Occurrence::start() const
{
    return QDateTime(date(), eventCache.start().time());
}

QDateTime Occurrence::end() const
{
    return QDateTime(endDate(), eventCache.end().time());
}

// Timezone dependent functions...

QDateTime Occurrence::startInCurrentTZ() const
{
    if (!eventCache.timeZone().isValid())
	return start();

    return eventCache.timeZone().toCurrent(start());
}

QDateTime Occurrence::endInCurrentTZ() const
{
    if (!eventCache.timeZone().isValid())
	return end();


    QDateTime nStart = eventCache.timeZone().toCurrent(start());

    return end().addSecs(start().secsTo(nStart));
}

// used when loading to finalize records.
static time_t startUtc = 0;
static time_t endUtc = 0;
static time_t endDateUtc = 0;
static bool hasTimeZone = FALSE;

/*!
  \internal
*/
void PimEvent::setFields(const QMap<int,QString> &m)
{
    // Reimplemenmted to handle post processing of reading an event
    startUtc = 0;
    endUtc = 0;
    endDateUtc = 0;
    hasTimeZone = FALSE;

    PimRecord::setFields( m );

    finalizeRecord();
}

// post Processing.
void PimEvent::finalizeRecord()
{
    if (!hasTimeZone) {
	// make one up.  Can't be "None" because in old datebook all
	// events were in UTC.  So make it the current locale.
	setTimeZone( TimeZone::current() );
    }

    // if there was a timezone, it would be set by now.
    setStartAsUTC( startUtc );
    setEndAsUTC( endUtc );
    if (hasRepeat() && endDateUtc != 0 && !repeatForever())
	setRepeatTillAsUTC( endDateUtc );

    // 0' out elements although this will be done in assignField anyway
    startUtc = 0;
    endUtc = 0;
    endDateUtc = 0;
    hasTimeZone = FALSE;
}

/*!
  \internal

  Don't do flow-on values here..  gets are direct from data bypassing
  relavent checks, puts will be the same.
*/
void PimEvent::setField(int key,const QString &value)
{
    switch( key ) {
	case Description:
	    setDescription( value );
	    break;
	case Location:
	    setLocation( value );
	    break;
	case StartTimeZone:
	    if (value != "None") // No tr
		setTimeZone( TimeZone(value) );
	    // hasTimeZone in that one is set, even if it is none.
	    hasTimeZone = TRUE;
	    break;
	case Notes:
	    setNotes( value );
	    break;
	case StartDateTime:
	    startUtc = (time_t) value.toLong();
	    break;
	case EndDateTime:
	    endUtc = (time_t) value.toLong();
	    break;
	case DatebookType:
	    setAllDay( value == "AllDay" );
	    break;
	case HasAlarm:
	    if ( value.contains("true", FALSE)) { // No tr
		mHasAlarm = TRUE;
	    } else {
		mHasAlarm = FALSE;
	    }
	    break;
	case AlarmDelay:
	    mAlarmDelay = value.toInt();
	    break;
	case SoundType:
	    mAlarmSound = value == "loud" ? Loud : Silent; // No tr
	    break;
	case RepeatPattern:
	    if ( value == "Daily" ) // No tr
		setRepeatType(Daily);
	    else if ( value == "Weekly" ) // No tr
		setRepeatType(Weekly);
	    else if ( value == "MonthlyDay" )
		setRepeatType(MonthlyDay);
	    else if ( value == "MonthlyDate" )
		setRepeatType(MonthlyDate);
	    else if ( value == "MonthlyEndDay" )
		setRepeatType(MonthlyEndDay);
	    else if ( value == "Yearly" ) // No tr
		setRepeatType(Yearly);
	    else
		setRepeatType(NoRepeat);
	    break;
	case RepeatFrequency:
	    if (value.toInt() != 0) {
		setFrequency( value.toInt() );
	    }
	    break;
	case RepeatHasEndDate:
	    if ( value.toInt() != 0 ) {
		setRepeatForever( FALSE );
	    } else {
		setRepeatForever( TRUE );
	    }
	    break;
	case RepeatWeekdays:
	    weekMask = value.toInt();
	    break;
	case RepeatEndDate:
	    endDateUtc = (time_t) value.toLong();
	    break;

	case RecordParent:
	    mParent = PimXmlIO::uuidFromInt(value.toInt());
	    break;
	case RecordChildren:
	    {
		QStringList list = QStringList::split(' ', value);
		for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
		    mChildren.append( PimXmlIO::uuidFromInt((*it).toInt()) );
		}
	    }
	    break;
	case Exceptions:
	    {
		QStringList list = QStringList::split(' ', value);
		for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
		    int tday = 1, tmonth = 1, tyear = 2000;
		    switch( (*it).length() ) {
			case 8:
			    tday = (*it).right( 2 ).toInt();
			case 6:
			    tmonth = (*it).mid( 4, 2 ).toInt();
			case 4:
			    tyear = (*it).left( 4 ).toInt();
			    break;
			default:
			    break;
		    }
		    //QDate date = TimeConversion::fromISO8601( QCString(*it) ).date();
		    QDate date(tyear, tmonth, tday);
		    mExceptions.append( date );
		}
	    }
	    break;
	default: PimRecord::setField(key, value);
    }
}

/*!
  \internal
*/
QString PimEvent::field(int key) const
{
    switch( key ) {
	case Description: return mDescription;
	case Location: return mLocation;
	case StartTimeZone:
        {
	    if (timeZone().isValid())
		return timeZone().id();
	    else
		return "None"; // No tr
	}
	case Notes: return mNotes;
	case StartDateTime: return QString::number( startAsUTC() );
	case EndDateTime: return QString::number( endAsUTC() );
	case DatebookType:
	{
	    if ( isAllDay() )
		return "AllDay";
	    else
		return "Timed"; // No tr
	}

	case HasAlarm: return hasAlarm() ? "true" : "false"; // No tr
	case AlarmDelay: return QString::number( alarmDelay() );
	case SoundType:
	{
	    if ( alarmSound() == Loud )
		return "loud"; // No tr
	    else
		return "silent"; // No tr
	}

	case RepeatPattern:
	{
	    switch ( repeatType() ) {
		case Daily: return "Daily"; // No tr
		case Weekly: return "Weekly"; // No tr
		case MonthlyDay: return "MonthlyDay";
		case MonthlyEndDay: return "MonthlyEndDay";
		case MonthlyDate: return "MonthlyDate";
		case Yearly: return "Yearly"; // No tr
		default: return "NoRepeat";
	    }
	}
	case RepeatFrequency: return QString::number( frequency() );
	case RepeatHasEndDate: return QString::number( !repeatForever() );
	case RepeatEndDate: return QString::number( repeatTillAsUTC() );
	case RepeatWeekdays: return QString::number( weekMask );
	case RecordParent: return QString::number( PimXmlIO::uuidToInt( mParent ) );
	case RecordChildren:
	{
	    const QValueList<QUuid> &vlc = mChildren;
	    QValueList<QUuid>::ConstIterator cit;
	    QString out;
	    for( cit = vlc.begin(); cit != vlc.end(); ++cit ) {
		if (cit != vlc.begin())
		    out += " ";
		out += QString::number( PimXmlIO::uuidToInt(*cit) );
	    }

	    return out;
	}
	case Exceptions:
	{
	    const QValueList<QDate> &vle = mExceptions;
	    QValueList<QDate>::ConstIterator eit;
	    QString out;
	    for( eit = vle.begin(); eit != vle.end(); ++eit ) {
		QDate date = *eit;
		if (eit != vle.begin())
		    out += " ";

		QCString str;
		str.sprintf("%04d%02d%02d", date.year(), date.month(), date.day());
		out += str;
	    }
	    return out;
	}

	default: return PimRecord::field(key);
    }
}

static QMap<int, int> *uniquenessMapPtr = 0;
static QMap<QCString, int> *identifierToKeyMapPtr = 0;
static QMap<int, QCString> *keyToIdentifierMapPtr = 0;
static QMap<int, QString> * trFieldsMapPtr = 0;


QMap<int, QString> PimEvent::fields() const
{
    QMap<int, QString> m = PimRecord::fields();

    if (!keyToIdentifierMapPtr)
	initMaps();
    QMap<int, QCString>::Iterator it;
    for (it = keyToIdentifierMapPtr->begin(); 
	    it != keyToIdentifierMapPtr->end(); ++it) {
	int i = it.key();
	QString str = field(i);
	if (!str.isEmpty())
	    m.insert(i, str);
    }

    return m;
}

static const QtopiaPimMapEntry datebookentries[] = {
    { "description", // No tr
	QT_TRANSLATE_NOOP("PimEvent", "Description"), PimEvent::Description, 40},
    { "location", QT_TRANSLATE_NOOP("PimEvent", "Location"), PimEvent::Location, 0 },
    { "timezone", QT_TRANSLATE_NOOP("PimEvent", "Time zone"), PimEvent::StartTimeZone, 0 },
    { "note", QT_TRANSLATE_NOOP("PimEvent", "Notes"), PimEvent::Notes, 0 },
    { "start", QT_TRANSLATE_NOOP("PimEvent", "Start Date"), PimEvent::StartDateTime, 25 },
    { "end", QT_TRANSLATE_NOOP("PimEvent", "End Date"), PimEvent::EndDateTime, 25 },
    { "type", NULL, PimEvent::DatebookType, 0 },
    { "alarm", NULL, PimEvent::AlarmDelay, 0 },
    { "balarm", NULL, PimEvent::HasAlarm, 0 },
    { "sound", NULL, PimEvent::SoundType, 0 },

    { "rtype", NULL, PimEvent::RepeatPattern, 10 },
    { "rfreq", NULL, PimEvent::RepeatFrequency, 0 },
    { "rweekdays", NULL, PimEvent::RepeatWeekdays, 0 },
    { "rhasenddate", NULL, PimEvent::RepeatHasEndDate, 0 },
    { "enddt", NULL, PimEvent::RepeatEndDate, 0 },

    { "recparent", NULL, PimEvent::RecordParent, 0 },
    { "recchildren", NULL, PimEvent::RecordChildren, 0 },
    { "exceptions", NULL, PimEvent::Exceptions, 0 },

    { 0, 0, 0, 0 }
};

void PimEvent::initMaps()
{
    delete keyToIdentifierMapPtr;
    keyToIdentifierMapPtr = new QMap<int, QCString>;

    delete identifierToKeyMapPtr;
    identifierToKeyMapPtr = new QMap<QCString, int>;

    delete trFieldsMapPtr;
    trFieldsMapPtr = new QMap<int,QString>;

    delete uniquenessMapPtr;
    uniquenessMapPtr = new QMap<int, int>;

    PimRecord::initMaps("PimEvent", datebookentries, *uniquenessMapPtr, *identifierToKeyMapPtr, *keyToIdentifierMapPtr,
			*trFieldsMapPtr );

    // Datebook xml file used lowercase letters for Categories and Uid
    (*keyToIdentifierMapPtr)[UID_ID] = "uid";
    identifierToKeyMapPtr->remove("Uid");
    (*identifierToKeyMapPtr)["uid"] = UID_ID;

    (*keyToIdentifierMapPtr)[Categories] = "categories"; // No tr
    identifierToKeyMapPtr->remove("Categories"); // No tr
    (*identifierToKeyMapPtr)["categories"] = Categories; // No tr
}

/*!
  \internal
*/
const QMap<int, QCString> &PimEvent::keyToIdentifierMap()
{
    if ( !keyToIdentifierMapPtr )
	initMaps();
    return *keyToIdentifierMapPtr;
}

/*!
  \internal
*/
const QMap<QCString,int> &PimEvent::identifierToKeyMap()
{
    if ( !identifierToKeyMapPtr )
	initMaps();
    return *identifierToKeyMapPtr;
}

/*!
  \internal
*/
const QMap<int, QString> & PimEvent::trFieldsMap()
{
    if ( !trFieldsMapPtr )
	initMaps();
    return *trFieldsMapPtr;
}

/*!
  \internal
*/
const QMap<int,int> & PimEvent::uniquenessMap()
{
    if ( !uniquenessMapPtr )
	initMaps();
    return *uniquenessMapPtr;
}

#ifndef QT_NO_DATASTREAM
QDataStream &operator>>( QDataStream &s, PimEvent &c )
{
    s >> (PimRecord&)c;
    s >> c.mDescription;
    s >> c.mLocation;
    s >> c.mStart;
    s >> c.mEnd;
    s >> c.mNotes;
    s >> c.mTimeZone;
    s >> c.mEndTimeZone;
    uchar val;
    s >> val;
    c.mHasAlarm = val == 0 ? FALSE : TRUE;
    s >> c.mAlarmDelay;
    s >> val;
    c.mAlarmSound = (PimEvent::SoundTypeChoice)val;
    s >> val;
    c.mType = (PimEvent::RepeatType)val;
    s >> c.mFrequency;
    s >> val;
    c.mHasEndDate = val == 0 ? FALSE : TRUE;
    s >> c.mRepeatTill;
    s >> val;
    c.mShowOnNearest = val == 0 ? FALSE : TRUE;
    s >> c.weekMask;
    s >> val;
    c.mAllDay = val == 0 ? FALSE : TRUE;

    // exceptions.
    s >> c.mExceptions;
    s >> c.mParent;
    s >> c.mChildren;

    return s;
}

QDataStream &operator<<( QDataStream &s, const PimEvent &c )
{
    s << (const PimRecord&)c;
    s << c.mDescription;
    s << c.mLocation;
    s << c.mStart;
    s << c.mEnd;
    s << c.mNotes;
    s << c.mTimeZone;
    s << c.mEndTimeZone;
    s << (c.mHasAlarm ? (uchar)1 : (uchar)0);
    s << c.mAlarmDelay;
    s << (uchar)c.mAlarmSound;
    s << (uchar)c.mType;
    s << c.mFrequency;
    s << (c.mHasEndDate ? (uchar)1 : (uchar)0);
    s << c.mRepeatTill;
    s << (c.mShowOnNearest ? (uchar)1 : (uchar)0);
    s << c.weekMask;
    s << (c.mAllDay ? (uchar)1 : (uchar)0);

    // exceptions.
    s << c.mExceptions;
    s << c.mParent;
    s << c.mChildren;

    return s;
}
#endif

