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
#include "dayviewheaderimpl.h"

#include <qtopia/datepicker.h>
#include <qtopia/resource.h>
#include <qtopia/timestring.h>

#include <qbuttongroup.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qtoolbutton.h>
#include <qdatetime.h>

/*
 *  Constructs a DateBookDayHeader which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
DayViewHeader::DayViewHeader( bool useMonday,
				      QWidget* parent,  const char* name )
    : DateBookDayHeaderBase( parent, name ),
      bUseMonday( useMonday )
{
    back->setIconSet(Resource::loadIconSet("back"));
    forward->setIconSet(Resource::loadIconSet("forward"));

    setupNames();

    setBackgroundMode( PaletteButton );
    grpDays->setBackgroundMode( PaletteButton );

    dButton->setDate( currDate.year(), currDate.month(), currDate.day() );
    connect(dButton,SIGNAL(dateSelected(const QDate &)),this,SIGNAL(dateChanged(const QDate &)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
DayViewHeader::~DayViewHeader()
{
    // no need to delete child widgets, Qt does it all for us
}

void DayViewHeader::setStartOfWeek( bool onMonday )
{
    bUseMonday = onMonday;
    setupNames();
    setDate( currDate.year(), currDate.month(), currDate.day() );
}

void DayViewHeader::setupNames()
{
    if ( bUseMonday ) {
	cmdDay1->setText( DateBookDayHeaderBase::tr("Monday").left(1) );
	cmdDay2->setText( DateBookDayHeaderBase::tr("Tuesday").left(1) );
	cmdDay3->setText( DateBookDayHeaderBase::tr("Wednesday").left(1) );
	cmdDay4->setText( DateBookDayHeaderBase::tr("Thursday").left(1) );
	cmdDay5->setText( DateBookDayHeaderBase::tr("Friday").left(1) );
	cmdDay6->setText( DateBookDayHeaderBase::tr("Saturday").left(1) );
	cmdDay7->setText( DateBookDayHeaderBase::tr("Sunday").left(1) );
    } else {
	cmdDay1->setText( DateBookDayHeaderBase::tr("Sunday").left(1) );
	cmdDay2->setText( DateBookDayHeaderBase::tr("Monday").left(1) );
	cmdDay3->setText( DateBookDayHeaderBase::tr("Tuesday").left(1) );
	cmdDay4->setText( DateBookDayHeaderBase::tr("Wednesday").left(1) );
	cmdDay5->setText( DateBookDayHeaderBase::tr("Thursday").left(1) );
	cmdDay6->setText( DateBookDayHeaderBase::tr("Friday").left(1) );
	cmdDay7->setText( DateBookDayHeaderBase::tr("Saturday").left(1) );
    }
}

/*
 * public slot
 */
void DayViewHeader::goBack()
{
    currDate = currDate.addDays( -7 );
    setDate( currDate.year(), currDate.month(), currDate.day() );
}
/*
 * public slot
 */
void DayViewHeader::goForward()
{
    currDate = currDate.addDays( 7 );
    setDate( currDate.year(), currDate.month(), currDate.day() );
}


/*
 * public slot
 */
void DayViewHeader::setDate( int y, int m, int d )
{
    currDate.setYMD( y, m, d );
    dButton->setDate(y,m,d);

    int iDayOfWeek = currDate.dayOfWeek();
    // cleverly adjust the day depending on how we start the week
    if ( bUseMonday )
	iDayOfWeek--;
    else {
	if ( iDayOfWeek == 7 )  // Sunday
	    iDayOfWeek = 0;
    }
    grpDays->setButton( iDayOfWeek );
    emit dateChanged( currDate );
}

/*
 * public slot
 */
void DayViewHeader::setDay( int day )
{
    int realDay;
    int dayOfWeek = currDate.dayOfWeek();

    // a little adjustment is needed...
    if ( bUseMonday )
	realDay = day + 1 ;
    else if ( !bUseMonday && day == 0 ) // sunday
	realDay = 7;
    else
	realDay = day;
    // special cases first...
    if ( realDay == 7 && !bUseMonday )  {
	while ( currDate.dayOfWeek() != realDay )
	    currDate = currDate.addDays( -1 );
    } else if ( !bUseMonday && dayOfWeek == 7 && dayOfWeek > realDay ) {
	while ( currDate.dayOfWeek() != realDay )
	    currDate = currDate.addDays( 1 );
    } else if ( dayOfWeek < realDay ) {
	while ( currDate.dayOfWeek() < realDay )
	    currDate = currDate.addDays( 1 );
    } else if ( dayOfWeek > realDay ) {
	while ( currDate.dayOfWeek() > realDay )
	    currDate = currDate.addDays( -1 );
    }
    // update the date...
    setDate( currDate.year(), currDate.month(), currDate.day() );
}
