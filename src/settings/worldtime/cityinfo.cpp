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

#include "cityinfo.h"

#include <qtopia/timestring.h>
#include <qtopia/timezone.h>

#include <qstyle.h>
#include <qpainter.h>

CityInfo::CityInfo(QWidget *parent, char *name, int f)
    : QFrame(parent, name, f)
{
    mUtc = TimeZone::utcDateTime();
}

void CityInfo::setZone(const QString &zone)
{
    mZone = zone;
    update();
}

/*
void CityInfo::resize(int w, int h) 
{
}
*/

void CityInfo::setUtcTime(const QDateTime &dt)
{
    mUtc = dt;
    update();
}

QString CityInfo::text() const
{
    QDateTime cityTime;
    if ( !mZone.isNull() ) {
	TimeZone curZone( mZone );
	if ( curZone.isValid() ){
	    // may have to have this passed in slot.
	    cityTime = curZone.fromUtc( mUtc );
	}
	return TimeString::localHMDayOfWeek( cityTime);
    } else {
	return QString::null;
    }
}

void CityInfo::drawContents( QPainter *p )
{
    QRect cr = contentsRect();
    style().drawItem( p, 
#if (QT_VERSION-0 >= 0x030000)
	    cr,
#else
	    cr.x(), cr.y(), cr.width(), cr.height(),
#endif
	    AlignRight, colorGroup(), isEnabled(),
	    0, text());
}

QSize CityInfo::sizeHint() const
{
    QSize res;
    QFontMetrics fm( font() );
    res.setWidth(fm.width(text())+2);
    res.setHeight(fm.height());
    return res;
}
