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

#ifndef __EVENT_H__
#define __EVENT_H__

#include <qtopia/qpeglobal.h>
#include <qdatetime.h>
#include <qvaluelist.h>
#include <qcolor.h>

#include <qmap.h>
#include <qstring.h>

#include <qtopia/qtopiawinexport.h> 

#ifdef PALMTOPCENTER
#include <common/qsorter.h>
#endif
#include <qtopia/private/palmtoprecord.h>

#include <qtopia/timeconversion.h>

static const QColor colorNormal      = QColor(255, 0  , 0  );
static const QColor colorRepeat      = QColor(0  , 0  , 255);
static const QColor colorNormalLight = QColor(255, 220, 220);
static const QColor colorRepeatLight = QColor(200, 200, 255);

class EventPrivate;
class QTOPIA_EXPORT Event : public Qtopia::Record
{
public:
    enum RepeatType { NoRepeat = -1, Daily, Weekly, MonthlyDay,
                      MonthlyDate, Yearly };

    // Don't use this.
    enum Days { MON = 0x01, TUE = 0x02, WED = 0x04, THU = 0x08,
                FRI = 0x10, SAT = 0x20, SUN = 0x40 };
    // Don't use this.
    struct QTOPIA_EXPORT RepeatPattern
    {
	RepeatPattern() {
	    type = NoRepeat; frequency = -1; days = 0;  position = 0; createTime = -1;
	    hasEndDate = FALSE; endDateUTC = 0; }
	bool operator ==( const RepeatPattern &right ) const;

	RepeatType type;
	int frequency;
	int position;	// the posistion in the month (e.g. the first sunday, etc) positive, count from the front negative count from the end...
	char days;  // a mask for days OR in your days!
	bool hasEndDate;
	QDate endDate() const { return TimeConversion::fromUTC( endDateUTC ).date(); }
	void setEndDate( const QDate &dt ) { endDateUTC = TimeConversion::toUTC( dt ); }
	time_t endDateUTC;
	time_t createTime;
    };

    Event();
    Event( const QMap<int, QString > & map );
    virtual ~Event();

    QMap<int, QString> toMap() const;

    static void writeVCalendar( const QString &filename, const QValueList<Event> &events);
    static void writeVCalendar( const QString &filename, const Event &event);
    static QValueList<Event> readVCalendar( const QString &filename );

    enum Type { Normal, AllDay };
    enum SoundTypeChoice { Silent, Loud };

    // Don't use these, there are essentially meaningless.
    bool operator<( const Event &e1) const { return start() < e1.start(); };
    bool operator<=( const Event &e1 ) const { return start() <= e1.start(); };
    bool operator!=( const Event &e1 ) const { return !( *this == e1 ); };
    bool operator>( const Event &e1 ) const { return start() > e1.start(); };
    bool operator>=(const Event &e1 ) const { return start() >= e1.start(); };
    bool operator==( const Event &e ) const;

    void setDescription( const QString &s );
    const QString &description() const;

    void setLocation( const QString &s );
    const QString &location() const;

    void setNotes( const QString &n );
    const QString &notes() const;

    void setType( Type t ); // Don't use me.
    Type type() const; // Don't use me.

    void setStart( const QDateTime &d );
    void setStart( time_t time ); // don't use me.
    QDateTime start( bool actual=FALSE ) const; // don't use me.
    time_t startTime() const { return startUTC; } // don't use me.
    void setEnd( const QDateTime &e );
    void setEnd( time_t time ); // don't use me
    QDateTime end( bool actual=FALSE ) const; // don't use me.
    time_t endTime() const { return endUTC; } // don't use me.
    void setTimeZone( const QString & );
    const QString &timeZone() const;
    void setAlarm( bool b, int minutes, SoundTypeChoice ); // Don't use me.
    bool hasAlarm() const;
    int alarmTime() const; // Don't use me.
    SoundTypeChoice alarmSound() const;

    // Don't use any of these.
    void setRepeat( bool b, const RepeatPattern &p );
    void setRepeat( const RepeatPattern &p );
    bool hasRepeat() const;
    const RepeatPattern &repeatPattern() const;
    RepeatPattern &repeatPattern();
    bool doRepeat() const { return pattern.type != NoRepeat; }

    void save( QString& buf );
    //void load( Node *n );

    bool match( const QRegExp &r ) const;

    // Don't use these either.  Functionality will be moved elsewhere.

    // helper function to calculate the week of the given date
    static int week( const QDate& date );
    // calculates the number of occurrences of the week day of
    // the given date from the start of the month
    static int occurrence( const QDate& date );
    // returns a proper days-char for a given dayOfWeek()
    static char day( int dayOfWeek ) { return 1 << ( dayOfWeek - 1 ); }
    // returns the dayOfWeek for the *first* day it finds (ignores
    // any further days!). Returns 1 (Monday) if there isn't any day found
    static int dayOfWeek( char day );
    // returns the difference of months from first to second.
    static int monthDiff( const QDate& first, const QDate& second );

private:
#ifndef Q_OS_WIN32
    Qtopia::UidGen &uidGen() { return sUidGen; }
#else
    Qtopia::UidGen &uidGen();
#endif
    static Qtopia::UidGen sUidGen;

    QString descript, locat, categ;
    Type typ : 4;
    bool startTimeDirty : 1;
    bool endTimeDirty : 1;
    time_t startUTC, endUTC;
    QString tz;
    bool hAlarm, hRepeat;
    int aMinutes;
    SoundTypeChoice aSound;
    RepeatPattern pattern;
    QString note;
    // ADDITION
    int mRid;	// Recode ID
    int mRinfo;	// Recode Info
    //
    EventPrivate *d;

};

#define QTOPIA_DEFINED_EVENT
#include <qtopia/qtopiawinexport.h>

// Since an event spans multiple day, it is better to have this
// class to represent a day instead of creating many
// dummy events...

class EffectiveEventPrivate;
class QTOPIA_EXPORT EffectiveEvent
{
public:
    // If we calculate the effective event of a multi-day event
    // we have to figure out whether we are at the first day,
    // at the end, or anywhere else ("middle"). This is important
    // for the start/end times (00:00/23:59)
    // MidWay: 00:00 -> 23:59, as we are "in the middle" of a multi-
    //         day event
    // Start: start time -> 23:59
    // End: 00:00 -> end time
    // Start | End == StartEnd: for single-day events (default)
    //                          here we draw start time -> end time
    enum Position { MidWay = 0, Start = 1, End = 2, StartEnd = 3 };

    EffectiveEvent();
    EffectiveEvent( const Event &event, const QDate &startDate, Position pos = StartEnd );
    EffectiveEvent( const EffectiveEvent & );
    EffectiveEvent& operator=( const EffectiveEvent & );
    ~EffectiveEvent();


    bool operator<( const EffectiveEvent &e ) const;
    bool operator<=( const EffectiveEvent &e ) const;
    bool operator==( const EffectiveEvent &e ) const;
    bool operator!=( const EffectiveEvent &e ) const;
    bool operator>( const EffectiveEvent &e ) const;
    bool operator>= ( const EffectiveEvent &e ) const;

    void setStart( const QTime &start );
    void setEnd( const QTime &end );
    void setEvent( Event e );
    void setDate( const QDate &date );
    void setEffectiveDates( const QDate &from, const QDate &to );

    //    QString category() const;
    const QString &description() const;
    const QString &location() const;
    const QString &notes() const;
    const Event &event() const;
    const QTime &start() const;
    const QTime &end() const;
    const QDate &date() const;
    int length() const;
    int size() const;

    QDate startDate() const;
    QDate endDate() const;

private:
    class EffectiveEventPrivate *d;
    Event mEvent;
    QDate mDate;
    QTime mStart,
	  mEnd;

};

#define QTOPIA_DEFINED_EFFECTIVEEVENTPRIVATE
#include <qtopia/qtopiawinexport.h>

#ifdef PALMTOPCENTER
class QTOPIA_EXPORT EffectiveEventSizeSorter : public QSorter<EffectiveEvent>
{
public:
    int compare( const EffectiveEvent& a, const EffectiveEvent& b ) const
    {
	return a.size() - b.size();
    }
};

class QTOPIA_EXPORT EffectiveEventTimeSorter : public QSorter<EffectiveEvent>
{
public:
    int compare( const EffectiveEvent& a, const EffectiveEvent& b ) const
    {
	return a.start().secsTo( b.start() );
    }
};
#endif

#endif
