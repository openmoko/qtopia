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
#include "dayviewheaderimpl.h"

#include <qtopia/datetimeedit.h>
#include <qtopia/resource.h>
#include <qtopia/timestring.h>

#include <qbuttongroup.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qtoolbutton.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <qlayout.h>

/*
 *  Constructs a DateBookDayHeader which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
DayViewHeader::DayViewHeader( bool useMonday, QWidget* parent, const char *name )
    : QWidget( parent, name ),
    bUseMonday( useMonday )
{
    init();

    setBackgroundMode( PaletteButton );
    grpDays->setBackgroundMode( PaletteButton );

    dButton->setDate( currDate );
    connect(dButton,SIGNAL(valueChanged(const QDate&)),this,SIGNAL(dateChanged(const QDate&)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
DayViewHeader::~DayViewHeader()
{
    // no need to delete child widgets, Qt does it all for us
}

void DayViewHeader::init()
{
    
    back = new QToolButton(this);
    back->setIconSet(Resource::loadIconSet("back"));
    back->setAutoRepeat( TRUE );
    back->setAutoRaise( TRUE );
    connect( back, SIGNAL(clicked()), this, SLOT(goBack()) );

    forward = new QToolButton(this);
    forward->setIconSet(Resource::loadIconSet("forward"));
    forward->setAutoRepeat( TRUE );
    forward->setAutoRaise( TRUE );
    connect( forward, SIGNAL(clicked()), this, SLOT(goForward()) );

    grpDays = new QButtonGroup(this);
    grpDays->setExclusive( TRUE );
    connect( grpDays, SIGNAL(clicked(int)), this, SLOT(setDay(int)) );

    cmdDay1 = new QToolButton(grpDays);
    cmdDay2 = new QToolButton(grpDays);
    cmdDay3 = new QToolButton(grpDays);
    cmdDay4 = new QToolButton(grpDays);
    cmdDay5 = new QToolButton(grpDays);
    cmdDay6 = new QToolButton(grpDays);
    cmdDay7 = new QToolButton(grpDays);
    QToolButton *cmdDays[7] = { cmdDay1, cmdDay2, cmdDay3, cmdDay4, cmdDay5, cmdDay6, cmdDay7 };
    for ( int i = 0; i < 7; i++ ) {
	cmdDays[i]->setAutoRaise( TRUE );
	cmdDays[i]->setToggleButton( TRUE );
    }

    setupNames();

    dButton = new QPEDateEdit(this);

    QHBoxLayout *hbox = new QHBoxLayout( this );
    hbox->add(back);
    hbox->add(grpDays);
    hbox->add(forward);
    hbox->add(dButton);

    hbox = new QHBoxLayout( grpDays );
    hbox->add( cmdDay1 );
    hbox->add( cmdDay2 );
    hbox->add( cmdDay3 );
    hbox->add( cmdDay4 );
    hbox->add( cmdDay5 );
    hbox->add( cmdDay6 );
    hbox->add( cmdDay7 );
}

void DayViewHeader::setStartOfWeek( bool onMonday )
{
    bUseMonday = onMonday;
    setupNames();
    setDate( currDate.year(), currDate.month(), currDate.day() );
}

static void setButton( QButton *btn, int day )
{
    btn->setText( TimeString::localDayOfWeek( day + 1, TimeString::Short ) );
}

void DayViewHeader::setupNames()
{
    int i = 0;
    ::setButton( cmdDay1, (bUseMonday?i:(i+6)%7) ); i++;
    ::setButton( cmdDay2, (bUseMonday?i:(i+6)%7) ); i++;
    ::setButton( cmdDay3, (bUseMonday?i:(i+6)%7) ); i++;
    ::setButton( cmdDay4, (bUseMonday?i:(i+6)%7) ); i++;
    ::setButton( cmdDay5, (bUseMonday?i:(i+6)%7) ); i++;
    ::setButton( cmdDay6, (bUseMonday?i:(i+6)%7) ); i++;
    ::setButton( cmdDay7, (bUseMonday?i:(i+6)%7) );
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
    dButton->setDate(QDate(y,m,d));

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
