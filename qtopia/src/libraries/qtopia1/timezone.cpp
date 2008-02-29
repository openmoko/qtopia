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

#include "timezone.h"
#include <qfile.h>
#include <qfileinfo.h>
#include <qcstring.h>
#include <qdatastream.h>
#include <qasciidict.h>
#include <qcstring.h>
#include <qdatetime.h>
#include <qarray.h>
#include <qmap.h>
#include <qdir.h>
#include <qregexp.h>
#include <stdlib.h>
#include <time.h>

#include "qtopia/qpeapplication.h"
#include "qtopia/config.h"

#ifdef QTOPIA_DESKTOP
#include <qdconfig.h>
#endif

/*******************************************************************
 *
 * TimeZonePrivate
 *
 *******************************************************************/

class TimeZonePrivate
{
public:
    TimeZonePrivate( QCString idLocation ) : id(idLocation)
	{
	}

    QCString id;

    static QCString zonePath();
    static QCString zoneFile();

    static QDateTime setUtcTime( long t )
{
    QDateTime r;
    time_t tmpTime = t;
    tm *tM = gmtime( &tmpTime );
    if (tM){
	r.setDate( QDate( tM->tm_year + 1900, tM->tm_mon + 1, tM->tm_mday ) );
	r.setTime( QTime( tM->tm_hour, tM->tm_min, tM->tm_sec ) );
    }else{
	r.setDate(QDate(1970, 1, 1));
	r.setTime(QTime(0,0,0));
    }

    return r;
}

    static uint toTime_t( const QDateTime &utc )
	{
	    QTime t = utc.time();
	    QDate d = utc.date();
	    tm brokenDown;
	    brokenDown.tm_sec = t.second();
	    brokenDown.tm_min = t.minute();
	    brokenDown.tm_hour = t.hour();
	    brokenDown.tm_mday = d.day();
	    brokenDown.tm_mon = d.month() - 1;
	    brokenDown.tm_year = d.year() - 1900;
	    brokenDown.tm_isdst = -1;

	    QCString origTz = getenv( "TZ" );
	    setenv( "TZ", "GMT", TRUE );
	    tzset();
	    int secsSince1Jan1970UTC = (int) mktime( &brokenDown );
	    if ( !origTz.isEmpty() )
	        setenv( "TZ", origTz, TRUE );

	    if ( secsSince1Jan1970UTC < -1 )
		secsSince1Jan1970UTC = -1;
	    return (uint) secsSince1Jan1970UTC;
	}
private:
    static QCString sZonePath;
    static QCString sZoneFile;

};


QCString TimeZonePrivate::sZonePath(0);
QCString TimeZonePrivate::zonePath()
{
    if ( sZonePath.isNull() ) {
#if defined(Q_OS_WIN32)
	sZonePath = QPEApplication::qpeDir() + "etc\\zoneinfo\\";
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	sZonePath = "/usr/share/zoneinfo/";
#else
	sZonePath = QPEApplication::qpeDir() + "etc/zoneinfo/";
#endif
    }
    return sZonePath;
}

QCString TimeZonePrivate::sZoneFile(0);
QCString TimeZonePrivate::zoneFile()
{
    if ( sZoneFile.isNull() ) {
	sZoneFile = zonePath() + "zone.tab";
    }
    return sZoneFile;
}


/*******************************************************************
 *
 * TimeZoneData
 *
 *******************************************************************/

class TimeZoneData
{
public:
    TimeZoneData() : mId(0) { }
    TimeZoneData( const QCString &id );

    bool match( const QDateTime &t,
	long utcOffset, bool dst,
	QCString standardAbbrev,
	QCString daylightAbbrev ) const;

    bool isValid() const;
    bool isDaylightSavings( const QDateTime & ) const;
    QDateTime toUtc( const QDateTime & ) const;
    QDateTime fromUtc( const QDateTime & ) const;
    QDateTime convert( const QDateTime &, const TimeZoneData & ) const;
    QCString id() const { return mId; }
    QCString standardAbbreviation() const;
    QCString dstAbbreviation() const;

    /* for debugging purposes */
    void dump() const;

    static TimeZoneData *null;

private:
    /** find the transition time that should apply to \a t
    */
    int findTranstionTimeIndex( const QDateTime & t, bool utc ) const;
    int findTimeTypeIndex( const QDateTime & t, bool utc ) const;

    QCString mId;
    bool mDstRule;

    struct transitionInfo {
	QDateTime time;
	int timeTypeIndex;
    };

    struct ttinfo {
	long          utcOffset; // in secs
	bool           isDst;
	unsigned int  abbreviationIndex;
	bool          isWallTime;
	bool          isTransitionLocal;
    };

    QArray<transitionInfo> transitionTimes;
    QArray<ttinfo> timeTypes;
    QMap<uint, QCString> abbreviations;
};

TimeZoneData *TimeZoneData::null = new TimeZoneData();

bool TimeZoneData::match( const QDateTime &c,
			  long utcOffset, bool hasDst,
			  QCString standardAbbrev,
			  QCString daylightAbbrev ) const
{
    if ( !isValid() )
	return FALSE;

    ttinfo transInfo = timeTypes[ findTimeTypeIndex( c, FALSE ) ];

    utcOffset *= -1;

    if ( (utcOffset == transInfo.utcOffset) && (hasDst == mDstRule) ) {
	// if not given abbreviation string to match, then matching
	// on offset is the best we can do
	if ( standardAbbrev.isEmpty() )
	    return TRUE;

	if ( hasDst )
	    return ( daylightAbbrev == abbreviations[ transInfo.abbreviationIndex ] );
	return ( standardAbbrev == abbreviations[ transInfo.abbreviationIndex ] );
    }

    return FALSE;
}


bool TimeZoneData::isValid() const
{
    return (transitionTimes.count() > 0) || (timeTypes.count() > 0);
}

int TimeZoneData::findTranstionTimeIndex( const QDateTime & c, bool utc ) const
{
    for ( int i = 0; i < (int)transitionTimes.count(); ++i ) {
	transitionInfo transTime = transitionTimes[i];
	ttinfo transInfo = timeTypes[ transTime.timeTypeIndex ];
	QDateTime thisItTime = transTime.time;

	// if comparing with utc, then shift the it time to utc
	if ( !utc )
	    thisItTime = thisItTime.addSecs( transInfo.utcOffset );

	if ( thisItTime > c ) {
	    // gone one too far; we've passed it. return the previous one, if
	    // it exists
	    if ( i > 0 )
		return i-1;
	    return i;
	}
    }
    // must be the last index
    return transitionTimes.count()-1;
}

int TimeZoneData::findTimeTypeIndex( const QDateTime & c, bool utc ) const
{
    int transIndex = findTranstionTimeIndex( c, utc );
    if ( transIndex < 0 )
	return 0;
    return transitionTimes[ transIndex ].timeTypeIndex;
}

QCString TimeZoneData::standardAbbreviation() const
{
    QDateTime dt = QDateTime::currentDateTime();
    int transIndex = findTranstionTimeIndex( dt, FALSE );

    while ( transIndex >= 0 ) {
	int timeIndex = transitionTimes[ transIndex ].timeTypeIndex;
	if ( !timeTypes[ timeIndex ].isDst )
	    return abbreviations[ timeTypes[ timeIndex ].abbreviationIndex ];

	transIndex--;
    }
    return "";
}

QCString TimeZoneData::dstAbbreviation() const
{
    QDateTime dt = QDateTime::currentDateTime();
    int transIndex = findTranstionTimeIndex( dt, FALSE );

    while ( transIndex >= 0 ) {
	int timeIndex = transitionTimes[ transIndex ].timeTypeIndex;
	if ( timeTypes[ timeIndex ].isDst )
	    return abbreviations[ timeTypes[ timeIndex ].abbreviationIndex ];

	transIndex--;
    }
    return "";
}

bool TimeZoneData::isDaylightSavings( const QDateTime & c ) const
{
    int timeIndex = findTimeTypeIndex( c, FALSE );
    return timeTypes[ timeIndex ].isDst;
}

QDateTime TimeZoneData::toUtc( const QDateTime &thisT ) const
{
    if ( !isValid() ) { qWarning("TimeZoneData::toUtc invalid"); return QDateTime(); }
    // find the appropriate utc time
    int timeIndex = findTimeTypeIndex( thisT, FALSE );
    return thisT.addSecs( -1 * timeTypes[ timeIndex ].utcOffset );
}

QDateTime TimeZoneData::fromUtc( const QDateTime &utc ) const
{
    if ( !isValid() ) { qWarning("TimeZoneData::fromUtc invalid"); return QDateTime(); }
    // convert from utc to "this" timezone
    int timeIndex = findTimeTypeIndex( utc, TRUE );
    return utc.addSecs( timeTypes[ timeIndex ].utcOffset );
}

QDateTime TimeZoneData::convert( const QDateTime &,
				 const TimeZoneData & ) const
{
    if ( !isValid() ) { qWarning("TimeZoneData::convert invalid"); return QDateTime(); }

    qWarning("TimeZoneData::convert not implemented yet");
    return QDateTime();
}

TimeZoneData::TimeZoneData( const QCString & loc ) : mId( loc ), mDstRule( FALSE )
{
    long numUtcIndicators, numStandardIndicators, numLeapSecs,
	numTransitionTimes, numTimeTypes, tzAbbreviationChars;

    QString fname = TimeZonePrivate::zonePath() + mId;
    fname = QDir::convertSeparators( fname );
    QFile f( fname );
    //qDebug(" fname %s exists %d", fname.data(), QFile::exists( fname ) );
    if ( !f.open( IO_ReadOnly ) ) {
	qDebug("Unable to open '%s'", fname.latin1() );
	//qDebug("\texists %d", QFile::exists( fname ));
	//exit(1);
	return;
    }

    QByteArray data = f.readAll();
    f.close();
    //qDebug("data size = %d; value='%s'", data.size(),data.data());
    if ( !data.size() ) {
	qDebug("invalid data size = %d", data.size());
	return;
    }

    QDataStream ds( data, IO_ReadOnly );

    char rawMagic[5];
    uint nbytes=4;
    rawMagic[4] = 0;
    ds.readRawBytes( rawMagic, nbytes );
    QCString magic( rawMagic, nbytes+1 );
    if ( magic != "TZif"
#ifdef Q_OS_MAC
	&& ( magic[0] || magic[1] || magic[2] || magic[3] )
#endif
    ) {
	//qDebug("magic match failed: '%s'",rawMagic);
	return;
    }

    char reserved[16];
    nbytes = 16;
    ds.readRawBytes(reserved, nbytes);
    if ( nbytes != 16 ) {
	qDebug("reserved size failed");
	return;
    }

    ds >> numUtcIndicators >> numStandardIndicators >> numLeapSecs
       >> numTransitionTimes >> numTimeTypes >> tzAbbreviationChars;

    transitionInfo transInfo;
    transitionTimes.resize( numTransitionTimes );
    int i =0;
    for ( i = 0; i < numTransitionTimes; ++i ) {
	long secs;
	ds >> secs;

	transInfo.time = TimeZonePrivate::setUtcTime( secs );
	if (transInfo.time.isNull()){
	    qWarning("Invalid transistion time for %s", id().data());
	}
	transInfo.timeTypeIndex = 0;
	transitionTimes[i] = transInfo;
    }

    unsigned char byte = 0;
    for ( i = 0; i < numTransitionTimes; ++i ) {
	ds >> byte;
	transInfo = transitionTimes[i];
	transInfo.timeTypeIndex = (int) byte;
	transitionTimes[i] = transInfo;
    }

    timeTypes.resize( numTimeTypes );
    unsigned int maxAbbrevIndex = 0;
    ttinfo offsetInfo;
    for ( i =0; i < numTimeTypes; ++i ) {
	ds >> offsetInfo.utcOffset;
	ds >> byte;
	offsetInfo.isDst = (bool) byte;
	if ( offsetInfo.isDst )
	    mDstRule = TRUE;
	ds >> byte;
	offsetInfo.abbreviationIndex = (unsigned int)byte;
	timeTypes[i] = offsetInfo;

	if (  offsetInfo.abbreviationIndex > maxAbbrevIndex )
	    maxAbbrevIndex = offsetInfo.abbreviationIndex;
    }

    char abbrev[5];
    abbrev[4] = 0;
    nbytes = 4;
    for ( uint index = 0; index <= maxAbbrevIndex; index += 4 ) {
	ds.readRawBytes( abbrev, nbytes );
	abbreviations.insert( index, abbrev );
    }

    // leap seconds aren't set for any file, so not exactly sure
    // what they are used for
    for ( i = 0; i < numLeapSecs; ++i ) {
	long leapSecOccurs, applyLeapSecs;
	ds >> leapSecOccurs >> applyLeapSecs;
    }

    for ( i = 0; i < numStandardIndicators; ++i ) {
	ds >> byte;
	offsetInfo = timeTypes[i];
	offsetInfo.isWallTime = !(bool) byte;
	timeTypes[i] = offsetInfo;
    }

    for ( i = 0; i < numUtcIndicators; ++i ) {
	ds >> byte;
	offsetInfo = timeTypes[i];
	offsetInfo.isTransitionLocal = !(bool) byte;
	timeTypes[i] = offsetInfo;
    }
}

void TimeZoneData::dump() const
{
    if ( !isValid() ) {
	qDebug("Loading failed!");
	//return;
    }

    qDebug("TimeZoneData::dump() for %s", (TimeZonePrivate::zonePath() + id()).data());
    qDebug("numTransitionTimes %d numTimeTypes %d",
	   transitionTimes.count(), timeTypes.count() );

    int i;
    for ( i = 0; i < (int) transitionTimes.count(); ++i )
	qDebug(" transitionTimes[%d] = %s\t\ttimeTypeIndex %d", i, transitionTimes[i].time.toString().latin1(), transitionTimes[i].timeTypeIndex );


    for ( i = 0; i < (int) timeTypes.count(); ++i ) {
	ttinfo offsetInfo = timeTypes[i];
	qDebug("time type[%d] => offset: %ld isdst %d abbrev timeTypeIndex: %d abbrev %s isWall %d isTransitionLocal %d",
	       i, offsetInfo.utcOffset/(60*60), offsetInfo.isDst, offsetInfo.abbreviationIndex, (const char *)abbreviations[offsetInfo.abbreviationIndex],
	       (int) offsetInfo.isWallTime, (int) offsetInfo.isTransitionLocal );
    }

    qDebug("TimeZoneData::dump() for %s", (TimeZonePrivate::zonePath() + id()).data());
}

/*******************************************************************
 *
 * TimeZoneLocation
 *
 *******************************************************************/

class TimeZoneLocation
{
public:
    TimeZoneLocation( const char *line );

    bool isValid() const { return mCity != NULL; }

// in seconds
    int lat() const;
// in seconds
    int lon() const;

    QCString latStr() const;
    QCString lonStr() const;

    QString description() const;
    QString area() const;
    QString city() const;
    QCString countryCode() const;
    QCString id() const;
    int distance( const TimeZoneLocation &e ) const
	{ return lat() - e.lat() + lon() - e.lon(); }

    void dump() const;

    static void load( QAsciiDict< TimeZoneLocation > &store );
    static QStringList languageList();

private:
    QCString mLine;
    const char * mArea;
    const char * mCity;
    const char * mDescription;
    const char *mCountryCode;
    QString mLatStr;
    QString mLonStr;
    QCString mId;
};

TimeZoneLocation::TimeZoneLocation( const char *line )
    : mLine( line ), mArea( NULL ), mCity( NULL ), mDescription( NULL )
{
    char *pos = mLine.data();
    char *endPos = pos + mLine.length();
    char *tokenBegin = pos;
    int tokenNum = 0;

    while( pos < endPos ) {
	if ( *pos == '\t' || *pos == '\n'
	     || *pos == '\r' || pos+1 == endPos ) {
	    *pos = '\0';
	    switch ( tokenNum ) {
	    case 0:
		mCountryCode = tokenBegin;
		break;
	    case 1: {
		QString latLonStr( tokenBegin );
		if ( latLonStr.length() == 15 ) {
		    mLatStr = latLonStr.mid(0, 7);
		    mLonStr = latLonStr.mid(7);
		} else if ( latLonStr.length() == 11 ) {
		    mLatStr = latLonStr.mid(0, 5);
		    mLonStr = latLonStr.mid(5);
		}
		else {
		    qWarning("can't parse lat lon str %s", latLonStr.latin1() );
		    return;
		}

		// sanity check
		if ( mLatStr[0] != '+' && mLatStr[0] != '-') {
		    qWarning("lat/lon is invalid");
		    return;
		}
	    }
		break;
	    case 2: {
		mArea = tokenBegin;

		// the string after the last slash is the city
		// also, convert the _ to ' ' in the area strings
		char *areaPos = pos-1;
		for ( ;areaPos > mArea && *areaPos != '/'; --areaPos ) ;

		*areaPos = '\0';
		mCity = areaPos+1;
	    } break;
	    case 3:
		mDescription = tokenBegin;
		break;
	    }

//	    if ( debug )
	    //	qDebug("\ttoken %d = =%s=", tokenNum, tokenBegin);
	    tokenNum++;
	    tokenBegin = pos+1;
	}

	pos++;
    }

    mId  = QCString(mArea) + '/' + QCString(mCity);
}

int TimeZoneLocation::lat() const
{
    int deg, min, sec;
    int sign = 1;
    if ( mLatStr[0] == '-' )
	sign = -1;
    deg = mLatStr.mid(1, 2).toInt();
    min = mLatStr.mid(3, 2).toInt();
    sec = 0;
    if ( mLatStr.length() == 7 )
	sec = mLatStr.mid(5, 2).toInt();
    return sign*deg*3600 + sign*min*60 + sign*sec;
}

int TimeZoneLocation::lon() const
{
    int deg, min, sec;

    int sign = 1;
    if ( mLonStr[0] == '-' )
	sign = -1;

    deg = mLonStr.mid(1, 3).toInt();
    min = mLonStr.mid( 4, 2 ).toInt();
    sec = 0;
    if ( mLonStr.length() == 8 )
	sec = mLonStr.mid( 6, 2 ).toInt();

    return sign*deg*3600 + sign*min*60 + sign*sec;
}

QCString TimeZoneLocation::latStr() const
{
    return mLatStr.latin1();
}

QCString TimeZoneLocation::lonStr() const
{
    return mLonStr.latin1();
}

QString TimeZoneLocation::description() const
{
    return qApp->translate( "TimeZone", mDescription );
}

QString TimeZoneLocation::area() const
{
    QCString displayArea = mArea;
#if QT_VERSION < 0x030000
    displayArea.replace( QRegExp("_"), " ");
#else
    displayArea.replace( '_', ' ' );
#endif
    return qApp->translate( "TimeZone", displayArea );
}

QString TimeZoneLocation::city() const
{
    QCString displayCity = mCity;
#if QT_VERSION < 0x030000
    displayCity.replace( QRegExp("_"), " ");
#else
    displayCity.replace( '_', ' ' );
#endif

    return qApp->translate( "TimeZone", displayCity );
}

QCString TimeZoneLocation::countryCode() const
{
    return mCountryCode;
}

QCString TimeZoneLocation::id() const
{
    return mId;
}

void TimeZoneLocation::dump() const
{
    if (!this) {
	return;
    }
    qDebug("%s at [%s,%s] [%d,%d]: %s",
	   id().data(), mLatStr.data(), mLonStr.data(),
	   lat(), lon(), mDescription );
    qDebug("City :%s, Area: %s, Country Code %s", city().latin1(), area().data(), countryCode().data());
}


QStringList TimeZoneLocation::languageList()
{
#if QT_VERSION >= 0x040000
# error "Use Global::languageList()"
#endif

    QString lang;
    QStringList langs;
#ifdef QTOPIA_DESKTOP
    langs = gQtopiaDesktopConfig->languages();
#else
    if (lang.isEmpty())
	lang = getenv("LANG");

    int i  = lang.find(".");
    if ( i > 0 )
	lang = lang.left( i );
    langs.append(lang);
    i = lang.find( "_" );
    if ( i > 0 )
	langs.append(lang.left(i));
#endif
    return langs;
}

void TimeZoneLocation::load( QAsciiDict< TimeZoneLocation > &store )
{
    QStringList langs = languageList();
    for (QStringList::ConstIterator lit = langs.begin(); lit!=langs.end(); ++lit) {
	QString lang = *lit;
	QTranslator * trans = new QTranslator(qApp);
	QString tfn = QPEApplication::qpeDir()+"i18n/"+lang+"/timezone.qm";
	if ( trans->load( tfn ))
	    qApp->installTranslator( trans );
	else
	    delete trans;
    }

    QFile file( TimeZonePrivate::zoneFile() );
    if ( !file.open( IO_ReadOnly ) ) {
	qWarning( "Unable to open %s", file.name().latin1() );
	qWarning( "Timezone data must be installed at %s", TimeZonePrivate::zonePath().data() );
	qApp->exit(1);
	return;
    }

    char line[2048];
    TimeZoneLocation *tz = NULL;
    while ( !file.atEnd() ) {

	file.readLine( line, 2048 );
	if ( line[0] == '#' )
	    continue;

	tz = new TimeZoneLocation( line );

	if ( !tz->isValid() ) {
	    qWarning("TimeZoneLocation::load Unable to parse line %s", (const char *)line );
	    delete tz; tz = NULL;
	    continue;
	}
	store.insert( tz->id(), tz );
    }

    file.close();
}

/*******************************************************************
 *
 * TzCache
 *
 *******************************************************************/

class TzCache
{
public:
    static TzCache &instance();
    TimeZoneData *data( const QCString &id );
    TimeZoneLocation *location( const QCString &id );
    QStrList ids();

private:
    TzCache();
    QAsciiDict< TimeZoneData > mDataDict;
    QAsciiDict< TimeZoneLocation > mLocationDict;
    static TzCache *sInstance;
};

TzCache *TzCache::sInstance = NULL;

TzCache &TzCache::instance()
{
    if ( !sInstance )
	sInstance = new TzCache();
    return *sInstance;
}

TzCache::TzCache()
{
    // load the zone.tab file
    TimeZoneLocation::load( mLocationDict );
}

TimeZoneData *TzCache::data( const QCString &id )
{
    if (  id.isEmpty() )
	return TimeZoneData::null;

    TimeZoneData *d = mDataDict.find( id );
    if ( d )
	return d;

    d = new TimeZoneData( id );
    if ( !d->isValid() ) {
	qWarning("TimeZone::data Can't create a valid data object for '%s'", id.data() );
	delete d;
	return TimeZoneData::null;
    }

    mDataDict.insert( id, d );
    return d;
}

TimeZoneLocation *TzCache::location( const QCString &id )
{
    if ( id.isEmpty() )
	return 0;
    TimeZoneLocation * l = mLocationDict[ id ];
#if 0
    if ( !l )
	qDebug("TzCache::location unable to find %s ", id.data());
#endif
    return l;
}

QStrList TzCache::ids()
{
    QStrList l;
    for ( QAsciiDictIterator<TimeZoneLocation> it( mLocationDict );
	  it.current(); ++it )
	l.append( it.currentKey() );
    return l;
}

/*******************************************************************
 *
 * TimeZone
 *
 *******************************************************************/

/*!
  \class TimeZone timezone.h
  \brief The TimeZone class provides access to time zone data.

  TimeZone provides access to timezone data and conversion between
  times in different time zones and formats.

  First availability: Qtopia 1.6

  \ingroup qtopiaemb
*/

/*!
  Construct an invalid time zone.
*/
TimeZone::TimeZone() : d( new TimeZonePrivate( 0 ) )
{
}

/*!
  Construct a TimeZone for location \a locId.
*/
TimeZone::TimeZone( const char * locId ) : d( new TimeZonePrivate( locId ) )
{
}

/*!
  Copy constructor.
*/
TimeZone::TimeZone( const TimeZone & copyFrom ) :
    d( new TimeZonePrivate( copyFrom.d->id ) )
{
}

/*!
  Destruct TimeZone.
*/
TimeZone::~TimeZone()
{
    delete d; d = 0;
}

/*!
  Sets the current time zone id to \a id.
*/
void TimeZone::setId( const char *id )
{
    d->id = id;
}

/*!
  Assign \a from to this.
*/
TimeZone &TimeZone::operator=( const TimeZone &from)
{
    d->id = from.d->id;
    return *this;
}

/*!
  Returns TRUE if \a c is equal to this, otherwise FALSE.
*/
bool TimeZone::operator==( const TimeZone &c) const
{
    return (d->id == c.d->id);
}

/*!
  Returns TRUE if \a c is not equal to this, otherwise FALSE.
*/
bool TimeZone::operator!=( const TimeZone &c) const
{
    return (d->id != c.d->id);
}

/*!
  \internal
*/
void TimeZone::dump() const
{
    TzCache::instance().data( d->id )->dump();
    TzCache::instance().location( d->id )->dump();
}

/*!
  Return a time zone located at the UTC reference.
*/
TimeZone TimeZone::utc()
{
    return TimeZone("Europe/London");
}

/*!
  Return the UTC date and time.
*/
QDateTime TimeZone::utcDateTime()
{
    return TimeZonePrivate::setUtcTime( time(0) );
}

/*!
  Returns the date and time in this time zone from the number of seconds
  since 1 January 1970.
*/
QDateTime TimeZone::fromTime_t( uint secs ) const
{
    QDateTime utc = TimeZonePrivate::setUtcTime( secs );
    return fromUtc( utc );
}

/*!
  Returns the date and time \a thisT as the number of seconds
  since 1 January 1970.
*/
uint TimeZone::toTime_t( const QDateTime &thisT ) const
{
    QDateTime utc = toUtc( thisT );
    return TimeZonePrivate::toTime_t( utc );
}

/*!
  Returns the date and time \a thisT in this time zone as UTC.
*/
QDateTime TimeZone::toUtc( const QDateTime &thisT ) const
{
    TimeZoneData *data = TzCache::instance().data( d->id );
    return data->toUtc( thisT );
}

/*!
  Returns the UTC date and time \a utc as the date and time in this time zone.
*/
QDateTime TimeZone::fromUtc( const QDateTime &utc ) const
{
    TimeZoneData *data = TzCache::instance().data( d->id );
    return data->fromUtc( utc );
}

/*!
  Returns the date and time \a thisT in this time zone as the date and time
  in the current time zone.
*/
QDateTime TimeZone::toCurrent( const QDateTime &thisT ) const
{
    TimeZone curTz = current();
    return curTz.convert( thisT, *this );
}

/*!
  Returns the date and time \a curT in the current time zone as the
  date and time in this time zone.
*/
QDateTime TimeZone::fromCurrent( const QDateTime &curT ) const
{
    TimeZone curTz = current();
    return convert( curT, curTz );
}

/*!
  Return the date and time \a dt in time zone \a dtTz as the date and time
  int this time zone.
*/
QDateTime TimeZone::convert( const QDateTime &dt, const TimeZone &dtTz ) const
{
    QDateTime utc = dtTz.toUtc( dt );
    return fromUtc( utc );
}

QCString lastZoneRead;
QCString lastLocRead;

/*!
  Returns the current system time zone.
*/
TimeZone TimeZone::current()
{
    QCString cZone;
    cZone = getenv("TZ");
    QString currentLoc;
    if (lastLocRead.isEmpty() || lastZoneRead != cZone) {
	Config lconfig("locale");
	lconfig.setGroup( "Location" );
	currentLoc = lconfig.readEntry( "Timezone" ).latin1();
	lastZoneRead = cZone;
	lastLocRead = currentLoc;
    } else {
	currentLoc = lastLocRead;
    }

    if ( !currentLoc.isEmpty() )
	return TimeZone( currentLoc );

    // The code down here seems to be particular to both the Win32 API and GNU libc
    // Since we currently don't support running Qtopia on Mac OS X anyway, just skip
    // this code. Qtopia Desktop asks the system for it's timezone rather than trying
    // to guess with a cludge like this. If Qtopia ever runs on Mac OS X, then that
    // code needs to move somewhere else (or copied into here).
#ifndef Q_WS_MAC
#ifndef Q_OS_WIN32
    qWarning("TimeZone::current Location information is not set in the Config file locale!");
#endif
    // this is mainly for windows side code, in the initial case
    tzset();
    QCString standardAbbrev, daylightAbbrev;

#ifndef Q_OS_WIN32
    standardAbbrev = tzname[0];
    daylightAbbrev = tzname[1];
#endif
    QDateTime today = QDateTime::currentDateTime();
    QStrList allIds = TzCache::instance().ids();
    for ( QStrListIterator it( allIds); it.current() && currentLoc.isEmpty();
	  ++it ) {
	QCString id = it.current();
	//qDebug("\tchecking %s", id.data() );
	TimeZoneData *data = TzCache::instance().data( id );
	if ( data->match( today, timezone, daylight, standardAbbrev, daylightAbbrev ) )
	    currentLoc = it.current();
    }

    if ( !currentLoc.isEmpty() ) {
	Config lconfig("locale");
	lconfig.setGroup( "Location" );
	lconfig.writeEntry( "Timezone", currentLoc.data() );
	return TimeZone (currentLoc);
    }
#endif

    return TimeZone();
}

/*!
  Return the time zone identifier, e.g. Europe/London
*/
QCString TimeZone::id() const
{
    return d->id;
}

/*!
  Returns TRUE if this is a valid time zone, otherwise FALSE.
*/
bool TimeZone::isValid() const
{
    TimeZoneData *data = TzCache::instance().data( d->id );
    TimeZoneLocation *loc = TzCache::instance().location( d->id );
#if 0
    if ( data->isValid() && (!loc || !loc->isValid() )) {
	if ( !loc )
	    qWarning("data but no loc");
	else {
	    qWarning("invalid loc for valid data");
	    loc->dump();
	}
    }
#endif
    return data->isValid() && loc && loc->isValid();
}

/*!
  Returns the Daylight Savings Time (DST) time zone abbreviation.
*/
QCString TimeZone::dstAbbreviation() const
{
    TimeZoneData *data = TzCache::instance().data( d->id );
    return data->dstAbbreviation();
}

/*!
  Returns the time zone abbreviation, e.g. EST
*/
QCString TimeZone::standardAbbreviation() const
{
    TimeZoneData *data = TzCache::instance().data( d->id );
    return data->dstAbbreviation();
}

/*!
  Returns a list of the time zone ids.
*/
QStrList TimeZone::ids()
{
    return TzCache::instance().ids();
}

/*!
  Returns the latitude in seconds.
*/
int TimeZone::lat() const
{
    TimeZoneLocation *loc = TzCache::instance().location( d->id );
    if ( loc )
	return loc->lat();
    return 0;
}


/*!
  Returns the longitude in seconds.
*/
int TimeZone::lon() const
{
    TimeZoneLocation *loc = TzCache::instance().location( d->id );
    if ( loc )
	return loc->lon();
    return 0;
}

/*!
  Returns the latitude as a string.

  The format is: +-DDMM[SS]
*/
QCString TimeZone::latStr() const
{
    TimeZoneLocation *loc = TzCache::instance().location( d->id );
    if ( loc )
	return loc->latStr();

    return "";
}

/*!
  Returns the longitude as a string.

  The format is: +-DDMM[SS]
*/
QCString TimeZone::lonStr() const
{
    TimeZoneLocation *loc = TzCache::instance().location( d->id );
    if ( loc )
	return loc->lonStr();
    return "";
}

/*!
  Returns the translated description of this time zone.
*/
QString TimeZone::description() const
{
    TimeZoneLocation *loc = TzCache::instance().location( d->id );
    if ( loc )
	return loc->description();
    return QString::null;
}

/*!
  Returns the translated area of this time zone, e.g. Europe.
*/
QString TimeZone::area() const
{
    TimeZoneLocation *loc = TzCache::instance().location( d->id );
    if ( loc )
	return loc->area();
    return QString::null;
}

/*!
  Returns the translated city of this time zone, e.g. Oslo.
*/
QString TimeZone::city() const
{
    TimeZoneLocation *loc = TzCache::instance().location( d->id );
    if ( loc )
	return loc->city();
    return QString::null;
}

/*!
  Returns the ISO 3166 2-character country code.
*/
QCString TimeZone::countryCode()
{
    TimeZoneLocation *loc = TzCache::instance().location( d->id );
    if ( loc )
	return loc->countryCode();
    return "";
}

/*!
  \internal
*/
int TimeZone::distance( const TimeZone &e ) const
{
    TimeZoneLocation *loc = TzCache::instance().location( d->id );
    TimeZoneLocation *comp = TzCache::instance().location( e.d->id );
    if ( loc && comp )
	return loc->distance( *comp );
    return 0;
}

QDataStream &operator<<( QDataStream &o, const TimeZone &t )
{
    o << t.id();
    return o;
}

QDataStream &operator>>( QDataStream &o, TimeZone &t )
{
    QCString id;
    o >> id;
    t.setId( id );
    return o;
}
