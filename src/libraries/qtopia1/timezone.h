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

#ifndef QTOPIA_TIMEZ0NE_H
#define QTOPIA_TIMEZ0NE_H

#include <qstring.h>
#include <qdatetime.h>
#include <qstrlist.h>
#include <qdatastream.h>
#include <qtopia/qpeglobal.h>

class TimeZonePrivate;

class QTOPIA_EXPORT TimeZone
{
public:
    TimeZone();
    TimeZone( const char *id );
    TimeZone( const TimeZone & );
    ~TimeZone();

    bool isValid() const;

    void setId( const char *id );

    TimeZone &operator=( const TimeZone &);
    bool operator==( const TimeZone &) const;
    bool operator!=( const TimeZone &) const;

    /*! returns QDateTime in this timezone */
    QDateTime fromTime_t( uint t ) const;
    uint toTime_t( const QDateTime &thisT ) const;

    /** /a dt is in this time zone */
    QDateTime toUtc( const QDateTime &thisT ) const;
    QDateTime fromUtc( const QDateTime &utc ) const;

    QDateTime toCurrent( const QDateTime &thisT ) const;
    QDateTime fromCurrent( const QDateTime &curT ) const;

    // convert the /a dt which is in the timezone /a tz to "this" timezone
    QDateTime convert( const QDateTime &dt, const TimeZone &dtTz ) const;

    static TimeZone current();
    static TimeZone utc();
    static QDateTime utcDateTime();

    /** in seconds */
    int lat() const;
    /** in seconds */
    int lon() const;

    QCString dstAbbreviation() const;
    QCString standardAbbreviation() const;

    QCString latStr() const;
    QCString lonStr() const;

    QString description() const;
    QString area() const;
    QString city() const;
    QCString countryCode();
    QCString id() const;
    int distance( const TimeZone &e ) const;

    static QStrList ids();

    /* for debugging purposes */
    void dump() const;

private:
    TimeZonePrivate *d;
};

QTOPIA_EXPORT QDataStream &operator<<( QDataStream &, const TimeZone & );
QTOPIA_EXPORT QDataStream &operator>>( QDataStream &, TimeZone & );

#endif
