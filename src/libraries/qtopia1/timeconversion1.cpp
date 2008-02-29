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

#include <qglobal.h>
#include <qtopia/timeconversion.h>
#include <qregexp.h>
#include <stdlib.h>

QCString TimeConversion::toISO8601( const QDateTime &dt , bool asUTC)
{
    if (asUTC)
	return toISO8601(dt);

    // else we are just formating
    QDate d = dt.date();
    QTime t = dt.time();
    QCString str;
    str.sprintf("%04d%02d%02dT%02d%02d%02d",
		d.year(), d.month(), d.day(),
		t.hour(), t.minute(), t.second() );
    return str;
}

QCString TimeConversion::toISO8601( const QDate &d , bool asUTC)
{
    if (asUTC)
	return toISO8601(d);

    QCString str;
    str.sprintf("%04d%02d%02d",
		d.year(), d.month(), d.day());
    return str;
}

