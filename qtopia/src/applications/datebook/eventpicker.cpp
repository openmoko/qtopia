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

#include "eventpicker.h"
#include "datebook.h"
#include "dayview.h"
#include "monthview.h"

#include <qtopia/pim/event.h>
#include <qtopia/resource.h>
#include <qtopia/global.h>

#include <qwidgetstack.h>
#include <qaction.h>
#include <qlayout.h>

#define DAY 1
#define MONTH 3


EventPicker::EventPicker( DateBook *db, QWidget *parent, const char *name, bool modal, WFlags f )
    : QDialog( parent, name, modal, f ),
    datebook( db )
{
    setCaption( tr("Event Picker") );
    setIcon( Resource::loadPixmap( "EventPicker" ) );

    QAction *actionNextView = new QAction( this );
    connect( actionNextView, SIGNAL( activated() ), this, SLOT( nextView() ) );
    actionNextView->setAccel('*');

    QAction *cancelAction = new QAction( this );
    connect( cancelAction, SIGNAL( activated() ), this, SLOT( reject() ) );
#ifdef QTOPIA_PHONE
    if( !Global::mousePreferred() )
	cancelAction->setAccel(Key_Back);
#endif

    views = new QWidgetStack( this );
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( views );

    dayView = 0;
    monthView = 0;

    lastToday = QDate::currentDate();
    viewMonth( lastToday );
}

EventPicker::~EventPicker()
{
}

void EventPicker::viewToday()
{
    lastToday = QDate::currentDate();
    viewDay( lastToday );
}

void EventPicker::viewDay()
{
    viewDay( currentDate() );
}

void EventPicker::viewDay(const QDate& dt)
{
    initDay();
    dayView->selectDate( dt );
    views->raiseWidget( dayView );
    dayView->setFocus();
    dayView->redraw();
}

void EventPicker::viewMonth()
{
    viewMonth( currentDate() );
}

void EventPicker::viewMonth( const QDate& dt)
{
    initMonth();
    monthView->setDate( dt );
    views->raiseWidget( monthView );
    monthView->updateOccurrences();
}

bool EventPicker::eventSelected() const
{
    if (views->visibleWidget() && views->visibleWidget() == dayView) {
	return dayView->hasSelection();
    }
    return FALSE;
}

PimEvent EventPicker::currentEvent() const
{
    return dayView->currentEvent();
}

QDate EventPicker::currentDate()
{
    if ( dayView && views->visibleWidget() == dayView ) {
	return dayView->currentDate();
    } else if ( monthView && views->visibleWidget() == monthView ) {
	return monthView->selectedDate();
    } else {
	return QDate(); // invalid;
    }
}

void EventPicker::initDay()
{
    if ( !dayView ) {
	dayView = new DayView( datebook->db, datebook->onMonday, views, "day view" ); // No tr
	dayView->setCompressDay( datebook->compressDay );
	views->addWidget( dayView, DAY );
	dayView->setDayStarts( datebook->startTime );

	connect( dayView, SIGNAL(showDetails()),
		this, SLOT(accept()) );
    }
}

void EventPicker::initMonth()
{
    if ( !monthView ) {
	monthView = new MonthView( datebook->db, views, "month view" ); // No tr
	monthView->setMargin(0);
	views->addWidget( monthView, MONTH );
	connect( monthView, SIGNAL( dateClicked(const QDate&) ),
             this, SLOT( viewDay(const QDate&) ) );
    }
}

void EventPicker::nextView()
{
    QWidget* cur = views->visibleWidget();
    if ( cur ) {
	if ( cur == dayView )
	    viewMonth();
	else if ( cur == monthView )
	    viewDay();
    }
}

