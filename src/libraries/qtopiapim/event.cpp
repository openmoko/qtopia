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

#include "event.h"
#include "calendar.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/private/qfiledirect_p.h>
#include <qtopia/timeconversion.h>
#include <qtopia/stringutil.h>
#include <qtopia/private/recordfields.h>
#include <qtopia/private/vobject_p.h>

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

  \sa startInTZ() setStart()
*/

/*!
  \fn QDateTime PimEvent::end() const

  Returns when the first occurrence of the event starts.

  \sa endInTZ() setEnd()
*/

/*!
  \fn QDateTime PimEvent::endTimeZone() const
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

    init(start, end);
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

  \sa start(), startInTZ()
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

  \sa end(), endInTZ()
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
  Sets the time zone of the event to \a text.
  This will affect when the event occurs in UTC.  All day events cannot have
  a time zone set.

  \sa timeZone(), isAllDay()
*/
void PimEvent::setTimeZone( const QString &text )
{
    mTimeZone = text;
}

/*!
  \internal for the moment. finish documenting when adding travel events.
  Sets the end TimeZone of the event.  
*/
void PimEvent::setEndTimeZone( const QString &text )
{
    mEndTimeZone = text;
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
  Returns the time zone of the event or a empty string if the event has
  no time zone.  All day events allways have no time zone.
  
  \sa setTimeZone(), isAllDay()
*/
QString PimEvent::timeZone() const
{
    return mAllDay ? QString("") : mTimeZone;
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

/*!
  Returns the first date on or after \a from that the event will next occur.
  If the event only occurs once (no repeat) will return the date of the
  start of the event if the start of the event is on or after \a from.

  If \a ok is non-null, *ok is set to TRUE if the event occurs on or 
  after \a from and FALSE if the event does not occur on or after 
  \a from.
*/
QDate PimEvent::nextOccurrence( const QDate &from, bool *ok) const
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
		    after = PimCalendar::addMonths(1, after);
		}

		int outBy = monthsBetween % mFrequency;
		if (outBy) {
		    outBy = mFrequency - outBy;
		}
		result = PimCalendar::addMonths(outBy, after);

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
			result = PimCalendar::addMonths(mFrequency, result);
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
		result = PimCalendar::addMonths(outBy, after);

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
			    result = PimCalendar::addMonths(mFrequency, result);
			    continue;
			}
		    }

		    result.setYMD(result.year(), result.month(), day);
		    if (result < after)
			result = PimCalendar::addMonths(mFrequency, result);
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

		result = PimCalendar::addYears(yearsBetween + outBy, mStart.date());

		if (result < after)
		    result = PimCalendar::addYears(mFrequency, result);

		// at least after, may not be valid though.
		if (!mShowOnNearest) {
		    while (!QDate::isValid(result.year(), result.month(), mStart.date().day())) 
			result = PimCalendar::addYears(mFrequency, result);
		    result.setYMD(result.year(), result.month(), mStart.date().day());
		}
	    }
	    break;
    }
    if (ok) 
	*ok = (result < repeatTill() || repeatForever());

    return result;
}

// vcal conversion code
static inline VObject *safeAddPropValue( VObject *o, const char *prop, const QString &value )
{
    VObject *ret = 0;
    if ( o && !value.isEmpty() )
	ret = addPropValue( o, prop, value.latin1() );
    return ret;
}

static inline VObject *safeAddProp( VObject *o, const char *prop)
{
    VObject *ret = 0;
    if ( o )
	ret = addProp( o, prop );
    return ret;
}

static VObject *createVObject( const PimEvent &e )
{
    VObject *vcal = newVObject( VCCalProp );
    safeAddPropValue( vcal, VCVersionProp, "1.0" );
    VObject *event = safeAddProp( vcal, VCEventProp );

    safeAddPropValue( event, VCDTstartProp, TimeConversion::toISO8601( e.start() ) );
    safeAddPropValue( event, VCDTendProp, TimeConversion::toISO8601( e.end() ) );
    safeAddPropValue( event, "X-Qtopia-NOTES", e.description() );
    safeAddPropValue( event, VCDescriptionProp, e.description() );
    safeAddPropValue( event, VCLocationProp, e.location() );

    if ( e.hasAlarm() ) {
	QDateTime dt = e.start();
	dt = dt.addSecs( -e.alarmDelay()*60 );
	VObject *alarm = safeAddProp( event, VCDAlarmProp );
	safeAddPropValue( alarm, VCRunTimeProp, TimeConversion::toISO8601( dt ) );
	if (e.alarmSound() != PimEvent::Silent)  {
	    VObject *aalarm = safeAddProp( event, VCAAlarmProp );
	    safeAddPropValue( aalarm, VCRunTimeProp, TimeConversion::toISO8601( dt ) );
	}
    }

    safeAddPropValue( event, "X-Qtopia-TIMEZONE", e.timeZone() );

    // ### repeat missing

    // ### categories missing

    return vcal;
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

    VObjectIterator it;
    initPropIterator( &it, obj );
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


	if ( name == VCDTstartProp ) {
	    startTime = TimeConversion::fromISO8601( QCString(value) );
	    e.setStart( startTime );
	    haveStart = TRUE;
	}
	else if ( name == VCDTendProp ) {
	    e.setEnd( TimeConversion::fromISO8601( QCString(value) ) );
	    haveEnd = TRUE;
	}
	else if ( name == "X-Qtopia-NOTES" ) {
	    e.setNotes( value );
	}
	else if ( name == VCDescriptionProp ) {
	    e.setDescription( value );
	}
	else if ( name == VCLocationProp ) {
	    e.setLocation( value );
	}
#if 0
	else if ( name == VCAudioContentProp ) {
	    haveAlarm = TRUE;
	    VObjectIterator nit;
	    initPropIterator( &nit, o );
	    while( moreIteration( &nit ) ) {
		VObject *o = nextVObject( &nit );
		QCString name = vObjectName( o );
		// in this case, is unlikely to be encoded.. so don't bother converting.
		//if (tc) value = tc->toUnicode( vObjectStringZValue( o ) ); else value = vObjectStringZValue( o );
		QCString value = vObjectStringZValue( o );
		if ( name == VCRunTimeProp )
		    alarmTime = TimeConversion::fromISO8601( value );
		else if ( name == VCAudioContentProp ) {
		    if ( value != "silent" )
			soundType = PimEvent::Loud;
		}
	    }
	}
#endif
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
	else if ( name == "X-Qtopia-TIMEZONE") {
	    e.setTimeZone( value );
	}
#if 0
	else {
	    printf("Name: %s, value=%s\n", name.data(), vObjectStringZValue( o ) );
	    VObjectIterator nit;
	    initPropIterator( &nit, o );
	    while( moreIteration( &nit ) ) {
		VObject *o = nextVObject( &nit );
		QCString name = vObjectName( o );
		QString value = vObjectStringZValue( o );
		printf(" subprop: %s = %s\n", name.data(), value.latin1() );
	    }
	}
#endif
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

// Timezone dependent functions.

time_t asUTC(const QDateTime &time, const QString &z)
{
    QString realTZ = QString::fromLocal8Bit( getenv("TZ") );

    if (z.isEmpty())
	unsetenv("TZ");
    else
	if ( setenv( "TZ", z, true ) != 0 )
	    qWarning( "There was a problem setting the timezone" );

    time_t result = TimeConversion::toUTC( time );

    unsetenv("TZ");
    if (!realTZ.isNull())
	if ( setenv( "TZ", realTZ, true ) != 0 )
	    qWarning( "There was a problem resetting the timezone" );

    return result;
}

QDateTime asDateTime(time_t time, const QString &z)
{
    QString realTZ = QString::fromLocal8Bit( getenv("TZ") );

    if (z.isEmpty())
	unsetenv("TZ");
    else
	if ( setenv( "TZ", z, true ) != 0 )
	    qWarning( "There was a problem setting the timezone" );

    // 5. get QDateTime from utc.
    QDateTime result = TimeConversion::fromUTC( time );

    unsetenv("TZ");
    if (!realTZ.isNull())
	if ( setenv( "TZ", realTZ, true ) != 0 )
	    qWarning( "There was a problem resetting the timezone" );

    return result;
}


QDateTime shiftZones(const QDateTime &time, const QString &z1, const QString &z2)
{
    // need to do a conversion.  to utc and back out again on the start.. then
    
    // 1. store current timezone
    QString realTZ = QString::fromLocal8Bit( getenv("TZ") );

    // 2. set current timezone to z1
    if (z1.isEmpty())
	unsetenv("TZ");
    else
	if ( setenv( "TZ", z1, true ) != 0 )
	    qWarning( "There was a problem setting the timezone" );

    // 3. get utc time.
    time_t start_utc = TimeConversion::toUTC( time );

    // 4. set current timezone to z2.
    if (z2.isEmpty())
	unsetenv("TZ");
    else
	if ( setenv( "TZ", z2, true ) != 0 )
	    qWarning( "There was a problem setting the timezone" );

    // 5. get QDateTime from utc.
    QDateTime result = TimeConversion::fromUTC( start_utc );

    // 6. reset timezone.
    unsetenv("TZ");
    if (!realTZ.isNull())
	if ( setenv( "TZ", realTZ, true ) != 0 )
	    qWarning( "There was a problem resetting the timezone" );

    return result;
}

/*!
  Returns the start time of the event in timezone \a zone.
  If \a zone is null, returns the start time of the event
  in the current time zone.

  \sa start()
*/
QDateTime PimEvent::startInTZ(const QString &zone) const
{
    if (timeZone().isEmpty())
	return start();

    // if no zone given.. assume local
    if (zone.isEmpty())
	return shiftZones(start(), timeZone(), 
		QString::fromLocal8Bit( getenv("TZ") ) );

    return shiftZones(start(), timeZone(), zone);
}

/*!
  Returns the end time of the event in timezone \a zone.
  If \a zone is null, returns the end time of the event
  in the current time zone.

  \sa end()
*/
QDateTime PimEvent::endInTZ(const QString &zone) const
{
    if (timeZone().isEmpty())
	return end();

    // duration should be the same... shift the start.. dif to end.
    // if no zone given.. assume local
    QDateTime nStart;
    if (zone.isEmpty())
	nStart = shiftZones(start(), timeZone(), 
	    QString::fromLocal8Bit( getenv("TZ") ) );
    else
	nStart = shiftZones(start(), timeZone(), zone);

    return end().addSecs(start().secsTo(nStart));
}

/*!
  Returns the date the event will repeat till in timezone \a zone.
  If \a zone is null, returns the date the event will repeat till
  in the current time zone.

  \sa repeatTill()
*/
QDate PimEvent::repeatTillInTZ(const QString &zone) const
{
    if (timeZone().isEmpty())
	return repeatTill();

    // duration should be the same... shift the start.. dif to end.
    // if no zone given.. assume local
    QDateTime nStart;
    if (zone.isEmpty())
	nStart = shiftZones(start(), timeZone(), 
	    QString::fromLocal8Bit( getenv("TZ") ) );
    else
	nStart = shiftZones(start(), timeZone(), zone);

    QDateTime rtDateTime = QDateTime(repeatTill(), QTime(0,0,0));
    return rtDateTime.addSecs(start().secsTo(nStart)).date();
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

QDateTime Occurrence::startInTZ(const QString &zone) const 
{
    if (eventCache.timeZone().isEmpty())
	return start();
    // if no zone given.. assume local
    if (zone.isEmpty())
	return shiftZones(start(), eventCache.timeZone(), 
		QString::fromLocal8Bit( getenv("TZ") ) );

    return shiftZones(start(), eventCache.timeZone(), zone);
}

QDateTime Occurrence::endInTZ(const QString &zone) const 
{
    if (eventCache.timeZone().isEmpty())
	return end();
    // if no zone given.. assume local
    if (zone.isEmpty())
	return shiftZones(end(), eventCache.timeZone(), 
		QString::fromLocal8Bit( getenv("TZ") ) );

    return shiftZones(end(), eventCache.timeZone(), zone);
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
    return s;
}
#endif

