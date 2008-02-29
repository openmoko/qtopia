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
#include "datebookweekheaderimpl.h"
#include <qtopia/timestring.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qdatetime.h>

/*
 *  Constructs a DateBookWeekHeader which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
WeekViewHeader::WeekViewHeader( bool startOnMonday, QWidget* parent,
					const char* name, WFlags fl )
    : DateBookWeekHeaderBase( parent, name, fl ),
      bStartOnMonday( startOnMonday )
{
    setBackgroundMode( PaletteButton );
    labelDate->setBackgroundMode( PaletteButton );
    TimeString::connectChange( this, SLOT(timeStringChanged()) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
WeekViewHeader::~WeekViewHeader()
{
    // no need to delete child widgets, Qt does it all for us
}

void  WeekViewHeader::keyPressEvent(QKeyEvent *e)
{
    e->ignore();
}

/*
 * public slot
 */
void WeekViewHeader::yearChanged( int y )
{
    setDate( y, week );
    emit dateChanged( year, week );
}
/*
 * public slot
 */
void WeekViewHeader::nextWeek()
{
    weekChanged(week+1);
}
/*
 * public slot
 */
void WeekViewHeader::prevWeek()
{
    weekChanged(week-1);
}
/*
 * public slot
 */
void WeekViewHeader::weekChanged( int w )
{
    setDate( year, w );
    emit dateChanged( year, week );
}

void WeekViewHeader::setDate( int y, int w )
{
    year = y;
    week = w;
    spinYear->setValue( y );
    spinWeek->setValue( w );

    QDate d = dateFromWeek( week, year, bStartOnMonday );
    QDate dend = d.addDays( 6 );

    QString s = tr("%1-%2","2 dates")
	.arg(TimeString::localMD(d))
	.arg(TimeString::localMD(dend));

    labelDate->setText( s );
}

void WeekViewHeader::setStartOfWeek( bool onMonday )
{
    bStartOnMonday = onMonday;
    setDate( year, week );
}

void WeekViewHeader::timeStringChanged()
{
    setDate( year, week );
}

// dateFromWeek
// compute the date from the week in the year

QDate dateFromWeek( int week, int year, bool startOnMonday )
{
    QDate d;
    d.setYMD( year, 1, 1 );
    int dayOfWeek = d.dayOfWeek();
    if ( startOnMonday ) {
	if ( dayOfWeek <= 4 ) {
	    d = d.addDays( ( week - 1 ) * 7 - dayOfWeek + 1 );
	} else {
	    d = d.addDays( (week) * 7 - dayOfWeek + 1 );
	}
    } else {
	if ( dayOfWeek <= 4 || dayOfWeek == 7) {
	    d = d.addDays( ( week - 1 ) * 7 - dayOfWeek % 7 );
	} else {
	    d = d.addDays( ( week ) * 7 - dayOfWeek % 7 );
	}
    }
    return d;
}

