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

#ifndef __timeconversion_h__
#define __timeconversion_h__

#include <time.h>
#include <sys/types.h>
#include <qtopia/qpeglobal.h>
#include <qdatetime.h>


class QTOPIA_EXPORT TimeConversion
{
public:
    static QString toString( const QDate &d );
    static QDate fromString( const QString &datestr );

    static time_t toUTC( const QDateTime& dt );
    static QDateTime fromUTC( time_t time );
    static int secsTo( const QDateTime &from, const QDateTime &to );

    static QCString toISO8601( const QDate & );
    static QCString toISO8601( const QDate &, bool asUTC ); // libqtopia
    static QCString toISO8601( const QDateTime & );
    static QCString toISO8601( const QDateTime & , bool asUTC); // libqtopia
    static QDateTime fromISO8601( const QCString & );
};

#endif // __timeconversion_h__
