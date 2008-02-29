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

#include "datebookgui.h"

#include <qtopia/resource.h>
#ifdef Q_WS_QWS
#include <qtopia/ir.h>
#endif

#ifndef QTOPIA_PHONE
#include <qtopia/qpemenubar.h>
#include <qtopia/qpetoolbar.h>
#include <qapplication.h>
#endif

#include <qaction.h>

DateBookGui::DateBookGui( QWidget *parent, const char *, WFlags f )
    : QMainWindow( parent, "datebook", f ),
    actionFind( 0 ),
    sub_bar( 0 ),
    details_bar( 0 )
{
}

DateBookGui::~DateBookGui()
{
}

void DateBookGui::init()
{
    parentWidget = this;

    setCaption( tr("Calendar") );
    setIcon( Resource::loadPixmap( "DateBook" ) );

    setToolBarsMovable( FALSE );
    setBackgroundMode( PaletteButton );

    // Create the actions

    actionNew = new QAction( tr( "New" ), Resource::loadIconSet( "new" ), QString::null, 0, parentWidget, 0 );
    actionNew->setWhatsThis( tr("Create a new event") );
    connect( actionNew, SIGNAL( activated() ), this, SLOT( fileNew() ) );

    actionEdit = new QAction( tr( "Edit" ), Resource::loadIconSet( "edit" ), QString::null, 0, parentWidget, 0 );
    actionEdit->setWhatsThis( tr("Edit the selected event") );
    connect( actionEdit, SIGNAL( activated() ), this, SLOT( editCurrentEvent() ) );

    actionDelete = new QAction( tr( "Delete" ), Resource::loadIconSet( "trash" ), QString::null, 0, parentWidget, 0 );
    actionDelete->setWhatsThis( tr("Delete the selected event") );
    connect( actionDelete, SIGNAL( activated() ), this, SLOT( removeCurrentEvent() ) );

#ifdef Q_WS_QWS
    if (Ir::supported()) {
	actionBeam = new QAction( tr( "Beam" ), Resource::loadIconSet( "beam" ), QString::null, 0, parentWidget, 0 );
	actionBeam->setWhatsThis( tr("Beam the selected event") );
	connect( actionBeam, SIGNAL( activated() ), this, SLOT( beamCurrentEvent() ) );
    }
#endif

    QActionGroup *g = new QActionGroup( parentWidget );
    g->setExclusive( TRUE );

    actionToday = new QAction( tr( "Today" ), Resource::loadIconSet( "today" ), QString::null, 0, g, 0 );
    actionToday->setWhatsThis( tr("Show today's events") );
    connect( actionToday, SIGNAL( activated() ), this, SLOT( slotToday() ) );

    actionDay = new QAction( tr( "Day", "day, not date" ), Resource::loadIconSet( "day" ), QString::null, 0, g, 0 );
    actionDay->setWhatsThis( tr("Show selected day's events") );
    actionDay->setToggleAction( TRUE );
    actionDay->setOn( TRUE );
    connect( actionDay, SIGNAL( activated() ), this, SLOT( viewDay() ) );

#if !defined(QTOPIA_PHONE)
    actionWeek = new QAction( tr( "Week" ), Resource::loadIconSet( "week" ), QString::null, 0, g, 0 );
    actionWeek->setWhatsThis( tr("Show selected week's events") );
    actionWeek->setToggleAction( TRUE );
    connect( actionWeek, SIGNAL( activated() ), this, SLOT( viewWeek() ) );
#endif

    actionMonth = new QAction( tr( "Month" ), Resource::loadIconSet( "month" ), QString::null, 0, g, 0 );
    actionMonth->setWhatsThis( tr("Show selected month's events") );
    actionMonth->setToggleAction( TRUE );
    connect( actionMonth, SIGNAL( activated() ), this, SLOT( viewMonth() ) );

#if !defined(QTOPIA_PHONE)
    actionFind = new QAction( tr( "Find" ), Resource::loadIconSet( "find" ), QString::null, 0, g, 0 );
    connect( actionFind, SIGNAL(activated()), this, SLOT(slotFind()) );
#endif

#ifdef QTOPIA_PHONE
    actionNextView = new QAction( tr( "Next View" ), Resource::loadIconSet( "month" ), QString::null, 0, g, 0 );
    actionNextView->setAccel('*');
    connect( actionNextView, SIGNAL( activated() ), this, SLOT( nextView() ) );
#endif

    actionSettings = new QAction( tr( "Settings..." ), Resource::loadIconSet("settings"), QString::null, 0, g );
    connect( actionSettings, SIGNAL( activated() ), this, SLOT( showSettings() ) );

#ifndef QTOPIA_PHONE
    actionBack = new QAction( tr("Back"), Resource::loadIconSet("contextback"), QString::null, 0, g );
    connect( actionBack, SIGNAL(activated()), this, SLOT(hideEventDetails()) );
#endif

#if 0
    actionPurge = new QAction( tr( "Purge..." ), Resource::loadIconSet( "trash" ), QString::null, 0, g, 0 );
    actionPurge->setWhatsThis( tr("Remove old events") );
    connect( actionPurge, SIGNAL( activated() ), this, SLOT( slotPurge() ) );
#endif

    // Setup Menus
#if !defined(QTOPIA_PHONE)
    QPEToolBar *bar = new QPEToolBar( (QMainWindow *)parentWidget );
    bar->setHorizontalStretchable( TRUE );

    QPEMenuBar *mb = new QPEMenuBar( bar );
    mb->setMargin( 0 );

#ifndef QTOPIA_NO_POINTER_INPUT
    sub_bar = new QPEToolBar( (QMainWindow *)parentWidget );
#endif
#ifndef QTOPIA_PHONE
    details_bar = new QPEToolBar( (QMainWindow *)parentWidget );
#endif

    QPopupMenu *eventMenu = new QPopupMenu( parentWidget );
    QPopupMenu *view = new QPopupMenu( parentWidget );

    mb->insertItem( tr( "Event" ), eventMenu );
    mb->insertItem( tr( "View" ), view );

    actionNew->addTo( eventMenu );
    actionEdit->addTo( eventMenu );
    actionDelete->addTo( eventMenu );
    //actionPurge->addTo( eventMenu );
#ifdef Q_WS_QWS
    if (Ir::supported())
	actionBeam->addTo( eventMenu );
#endif
    eventMenu->insertSeparator();
    actionFind->addTo( eventMenu );

    if ( sub_bar ) {
	actionNew->addTo( sub_bar );
	actionToday->addTo( sub_bar );
	sub_bar->addSeparator();
	bool thinScreen = QApplication::desktop()->width() < 200;
	if ( !thinScreen ) {
	    actionDay->addTo( sub_bar );
	    actionWeek->addTo( sub_bar );
	    actionMonth->addTo( sub_bar );
	}
	//actionFind->addTo( sub_bar );
    }

    if ( details_bar ) {
	actionEdit->addTo( details_bar );
	actionDelete->addTo( details_bar );
#ifdef Q_WS_QWS
	if (Ir::supported())
	    actionBeam->addTo( details_bar );
#endif
	details_bar->addSeparator();
	actionBack->addTo( details_bar );

	details_bar->hide();
    }

    actionToday->addTo( view );
    view->insertSeparator();
    actionDay->addTo( view );
    actionWeek->addTo( view );
    actionMonth->addTo( view );
    view->insertSeparator();
    actionSettings->addTo( view );

#else
    contextMenu = new ContextMenu(parentWidget);
    
    actionNew->addTo( contextMenu );
    actionEdit->addTo( contextMenu );
    actionDelete->addTo( contextMenu );
    //actionPurge->addTo( contextMenu );
#ifdef Q_WS_QWS
    if (Ir::supported())
	actionBeam->addTo( contextMenu );
#endif
    actionToday->addTo( contextMenu );
    actionMonth->addTo( contextMenu );
    actionSettings->addTo( contextMenu );
#endif
}

