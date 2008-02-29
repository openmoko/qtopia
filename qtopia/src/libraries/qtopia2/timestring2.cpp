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

#include <qtopia/timestring.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>

#include <time.h>

/*!
  Returns time \a t as a string,
  showing hours and minutes.

  The format, including order depends on the user's settings.

  First availability: Qtopia 2.1
*/
QString TimeString::localHM( const QTime &t, Length len )
{
    if ( len == Medium || len == Long )
	return localHM(t);

    bool ampm = TimeString::currentAMPM();
    QString time;
    int hour = t.hour();
    int minute = t.minute();
    if ( ampm && hour > 12 )
	hour -= 12;
    if ( !ampm && hour < 10 )
	time.append( "0" );
    time.append( QString::number(hour) );
    time.append( ":" );
    if ( minute < 10 )
	time.append( "0" );
    time.append( QString::number(minute) );
    return time;
}

