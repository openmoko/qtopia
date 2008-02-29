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

#include "selectorui.h"

#include <qtopia/resource.h>

#include <qtoolbar.h>
#include <qmenubar.h>
#include <qstring.h>
#include <qpopupmenu.h>

SelectorUI::SelectorUI( QWidget* parent, const char* name, WFlags f )
    : QMainWindow( parent, name, f )
{
#ifndef QTOPIA_PHONE
    setToolBarsMovable( false );

    QToolBar *toolbar = new QToolBar( this );
    toolbar->setHorizontalStretchable( true );
    QMenuBar *menubar = new QMenuBar( toolbar );
    toolbar = new QToolBar( this ); 

    // Make edit action
    edit_action = new QAction( tr( "Edit" ),
        Resource::loadIconSet( "edit" ), QString::null, 0, this );
    edit_action->setWhatsThis( tr( "Edit the current image." ) );
    connect( edit_action, SIGNAL( activated() ),
        this, SIGNAL( edit() ) );
    
    // Make delete action
    delete_action = new QAction( tr( "Delete" ),
        Resource::loadIconSet( "trash" ), QString::null, 0, this );
    delete_action->setWhatsThis( tr( "Delete the current image." ) );
    connect( delete_action, SIGNAL( activated() ),
        this, SIGNAL( remove() ) );
    
    // Make beam action
    beam_action = new QAction( tr( "Beam" ),
        Resource::loadIconSet( "beam" ), QString::null, 0, this );
    beam_action->setWhatsThis( tr( "Beam the current image." ) );
    connect( beam_action, SIGNAL( activated() ),
        this, SIGNAL( beam() ) );
        
    // Make properties action
    properties_action = new QAction( tr( "Properties" ),
        Resource::loadIconSet( "info" ), QString::null, 0, this );
    properties_action->setWhatsThis( 
        tr( "Edit the properties of the current image." ) );
    connect( properties_action, SIGNAL( activated() ),
        this, SIGNAL( properties() ) );
    
    // Make slide show action
    slide_show_action = new QAction( tr( "Slide Show..." ), 
        Resource::loadIconSet( "slideshow"  ), QString::null, 0, this );
    slide_show_action->setWhatsThis( 
        tr( "View a slide show of the images in the current category." ) );
    connect( slide_show_action, SIGNAL( activated() ), 
        this, SIGNAL( slideShow() ) );
    
    // Construct menu bar
    // Construct image menu
    QPopupMenu *image_menu = new QPopupMenu( this );
    edit_action->addTo( image_menu );
    delete_action->addTo( image_menu );
    beam_action->addTo( image_menu );
    properties_action->addTo( image_menu );
    menubar->insertItem( tr( "Image" ), image_menu );
    
    // Construct view menu
    QPopupMenu *view_menu = new QPopupMenu( this );
    slide_show_action->addTo( view_menu );
    menubar->insertItem( tr( "View" ), view_menu );
    
    // Construct tool button
    edit_action->addTo( toolbar );
    delete_action->addTo( toolbar );
    beam_action->addTo( toolbar );
    properties_action->addTo( toolbar );
    slide_show_action->addTo( toolbar );
#endif
}

void SelectorUI::setEnabled( bool b )
{
    edit_action->setEnabled( b );
    delete_action->setEnabled( b );
    beam_action->setEnabled( b );
    properties_action->setEnabled( b );
    slide_show_action->setEnabled( b );
}
