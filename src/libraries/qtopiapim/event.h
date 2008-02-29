/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef __PIMEVENT_H__
#define __PIMEVENT_H__

#include <qdatetime.h>
#include <qvaluelist.h>
#include <qcolor.h>

#include <qtopia/pim/pimrecord.h>

#include <qtopia/timeconversion.h>

//#define EVENT_USE_CACHING


class QDataStream;
class PimEventPrivate;

class QTOPIA_EXPORT PimEvent : public PimRecord
{
public:
    enum RepeatType 
    {
	NoRepeat, 
	Daily,
	Weekly,
	MonthlyDate, // 26th of each month
	MonthlyDay, // third thursday of each month
	Yearly, // 26 of Feb.
	MonthlyEndDay, // third last thursday of each month
    };

    //enum Type { Normal, AllDay };
    //
    enum SoundTypeChoice { Silent, Loud };

    PimEvent();
    PimEvent(const QDateTime &start, const QDateTime &end);
    virtual ~PimEvent();

    QString description() const { return mDescription; }
    QString location() const { return mLocation; }
    QDateTime start() const { return mStart; }
    QDateTime startInTZ( const QString & = QString::null ) const;
    QDateTime end() const { return mEnd; }
    QDateTime endInTZ( const QString & = QString::null ) const;
    QString notes() const { return mNotes; }
    QString timeZone() const;
    QString endTimeZone() const { return mEndTimeZone; }

    bool hasAlarm() const { return mHasAlarm; }
    int alarmDelay() const { return mAlarmDelay; } // in minutes.
    SoundTypeChoice alarmSound() const { return mAlarmSound; }

    RepeatType repeatType() const { return mType; }
    bool hasRepeat() const { return mType != NoRepeat; }
    int frequency() const { return mFrequency; }
    int weekOffset() const;
    QDate repeatTill() const;
    QDate repeatTillInTZ( const QString & = QString::null ) const;
    bool repeatForever() const;
    bool showOnNearest() const { return mShowOnNearest; }
    bool repeatOnWeekDay(int day) const;

    // helper functions
    bool isAllDay() const { return mAllDay; }
    void setAllDay(bool enable = TRUE);

    bool isTravel() const
    {
	return !mEndTimeZone.isEmpty() && !mTimeZone.isEmpty();
    }

    void setDescription( const QString &s );
    void setLocation( const QString &s );
    void setStart( const QDateTime &d );
    void setEnd( const QDateTime &e );
    void setNotes( const QString &n );
    void setTimeZone( const QString & );
    void setEndTimeZone( const QString & );

    void setAlarm( int minutes, SoundTypeChoice );
    void clearAlarm();

    void setRepeatType( RepeatType t );
    void setFrequency( int );
    void setRepeatTill( const QDate & );
    void setRepeatForever(bool);
    void setShowOnNearest( bool );
    void setRepeatOnWeekDay(int day, bool enable);

    QDate nextOccurrence(const QDate &from, bool * = 0) const;

    static void writeVCalendar( const QString &, const QValueList<PimEvent> & );
    static void writeVCalendar( const QString &, const PimEvent & );
    static QValueList<PimEvent> readVCalendar( const QString & );

    QColor color() const;

#ifndef QT_NO_DATASTREAM
friend QTOPIA_EXPORT QDataStream &operator>>( QDataStream &, PimEvent & );
friend QTOPIA_EXPORT QDataStream &operator<<( QDataStream &, const PimEvent & );
#endif

    // protected so that we have a way for PimLib to access the guts of this.
protected:

    time_t startAsUTC() const;
    time_t endAsUTC() const;
    time_t repeatTillAsUTC() const;

    // set the timezone first before calling this, if timezone is to be set.
    void setStartAsUTC(time_t);
    void setEndAsUTC(time_t);
    void setRepeatTillAsUTC(time_t);

    QString mDescription;
    QString mLocation;
    QDateTime mStart;
    QDateTime mEnd;
    QString mNotes;
    QString mTimeZone;
    QString mEndTimeZone;

    bool mHasAlarm;
    int mAlarmDelay;
    SoundTypeChoice mAlarmSound;

    RepeatType mType;
    int mFrequency;
    bool mHasEndDate;
    QDate mRepeatTill;
    bool mShowOnNearest;
    uchar weekMask;
    bool mAllDay;

private:
    void init(const QDateTime &, const QDateTime &);

    static QColor color(bool repeats);

    PimEventPrivate *d;
};

class QTOPIA_EXPORT Occurrence
{
    public:
	Occurrence() {}
	Occurrence(const Occurrence &other)
	{
	    eventCache = other.eventCache;
	    mStart = other.mStart;
	}
	Occurrence(const QDate &start, const PimEvent &);

	Occurrence& operator=(const Occurrence &other)
	{
	    eventCache = other.eventCache;
	    mStart = other.mStart;

	    return *this;
	}

	~Occurrence() {}

	QDate date() const { return mStart; }
	QDate endDate() const;
	QDateTime start() const;
	QDateTime end() const;

	QDateTime startInTZ( const QString &zone = QString::null ) const;
	QDateTime endInTZ( const QString &zone = QString::null ) const;

	PimEvent event() const;
    private:
	PimEvent eventCache;
	QDate mStart;
};

#ifndef QT_NO_DATASTREAM
QTOPIA_EXPORT QDataStream &operator>>( QDataStream &, PimEvent & );
QTOPIA_EXPORT QDataStream &operator<<( QDataStream &, const PimEvent & );
#endif

#endif
