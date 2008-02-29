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

#include "editorui.h"

#include <qtopia/resource.h>
#include <qtopia/contextbar.h>

#include <qstring.h>
#include <qpopupmenu.h>

EditorUI::EditorUI( QWidget* parent, const char* name, WFlags f )
    : QMainWindow( parent, name, f )
{
    // Make open action
    open_action = new QAction( tr( "Open" ),
        Resource::loadIconSet( "fileopen" ), QString::null, 0, this );
    connect( open_action, SIGNAL( activated() ), this, SIGNAL( open() ) );
    open_action->setWhatsThis( tr( "Open an image." ) );
        
    // Make crop action
#ifdef QTOPIA_PHONE
    QAction *crop_action;
#endif
    crop_action = new QAction( tr( "Crop" ), 
        Resource::loadIconSet( "cut" ), QString::null, 0, this );
    crop_action->setWhatsThis( tr( "Crop the image." ) );
    connect( crop_action, SIGNAL( activated() ), 
        this, SIGNAL( crop() ) );
    
    // Make brightness action
#ifdef QTOPIA_PHONE
    QAction *brightness_action;
#endif
    brightness_action = new QAction( tr( "Brightness" ),
        Resource::loadIconSet( "color" ), QString::null, 0, this );
    brightness_action->setWhatsThis( tr( "Adjust the image brightness." ) );
    connect( brightness_action, SIGNAL( activated() ),
        this, SIGNAL( brightness() ) );
    
    // Make rotate actions
#ifdef QTOPIA_PHONE
    QAction *rotate_action;
#endif
    rotate_action = new QAction( tr( "Rotate" ),
        Resource::loadIconSet( "rotate" ), QString::null, 0, this );
    rotate_action->setWhatsThis( tr( "Rotate the image." ) );
    connect( rotate_action, SIGNAL( activated() ),
        this, SIGNAL( rotate() ) );
       
    // Make zoom action
#ifdef QTOPIA_PHONE
    QAction *zoom_action;
#endif
    zoom_action = new QAction( tr( "Zoom" ),
        Resource::loadIconSet( "find" ), QString::null, 0, this );
    zoom_action->setWhatsThis( tr( "Zoom in and out." ) );
    connect( zoom_action, SIGNAL( activated() ),
        this, SIGNAL( zoom() ) );    
    
    // Make fullscreen action
#ifdef QTOPIA_PHONE
    QAction *fullscreen_action;
#endif
    fullscreen_action = new QAction( tr( "Full Screen" ), 
        Resource::loadIconSet( "fullscreen" ), QString::null, 0, this );
    fullscreen_action->setWhatsThis( tr( "View the image in full screen." ) );
    connect( fullscreen_action, SIGNAL( activated() ),
        this, SIGNAL( fullScreen() ) );
    
#ifdef QTOPIA_PHONE
    // Clear context bar
    ContextBar::setLabel( this, Qt::Key_Select, ContextBar::NoLabel );
    
    // Construct context menu
    context_menu = new ContextMenu( this );
    context_menu->setEnableHelp( false );
    crop_action->addTo( context_menu );
    brightness_action->addTo( context_menu );
    rotate_action->addTo( context_menu );
    context_menu->insertSeparator();
    zoom_action->addTo( context_menu );
    fullscreen_action->addTo( context_menu );
    context_menu->insertSeparator();
    context_menu->insertItem( Resource::loadIconSet( "help_icon" ),
        tr( "Help" ), context_menu, SLOT( help() ) );
    context_menu->insertItem( Resource::loadIconSet( "close" ),
        tr( "Cancel" ), this, SIGNAL( cancel() ) );
#else
    // Construct menu bar
    setToolBarsMovable( false );
    toolbar = new QToolBar( this );
    toolbar->setHorizontalStretchable( true );
    menubar = new QMenuBar( toolbar );
    toolbar = new QToolBar( this );
    
    // Construct file menu
    file_menu = new QPopupMenu( this );
    open_action->addTo( file_menu );
    file_menu_id = menubar->insertItem( tr( "File" ), file_menu );
    
    // Construct edit menu
    QPopupMenu *edit_menu = new QPopupMenu( this );
    brightness_action->addTo( edit_menu );
    rotate_action->addTo( edit_menu );
    crop_action->addTo( edit_menu );
    menubar->insertItem( tr( "Edit" ), edit_menu );
    
    // Construct view menu
    QPopupMenu *view_menu = new QPopupMenu( this );
    zoom_action->addTo( view_menu );
    fullscreen_action->addTo( view_menu );
    menubar->insertItem( tr( "View" ), view_menu );
    
    // Construct tool buttons
    open_action->addTo( toolbar );
    brightness_action->addTo( toolbar );
    rotate_action->addTo( toolbar );
    crop_action->addTo( toolbar );
    zoom_action->addTo( toolbar );
#endif
}

#ifndef QTOPIA_PHONE
void EditorUI::addFileItems()
{
    // Add file menu
    file_menu_id = menubar->insertItem( tr( "File" ), file_menu, -1, 0 );
    // Remove tool buttons
    brightness_action->removeFrom( toolbar );
    rotate_action->removeFrom( toolbar );
    crop_action->removeFrom( toolbar );
    zoom_action->removeFrom( toolbar );
    // Add tool buttons
    open_action->addTo( toolbar );
    brightness_action->addTo( toolbar );
    rotate_action->addTo( toolbar );
    crop_action->addTo( toolbar );
    zoom_action->addTo( toolbar );
}
#endif

#ifndef QTOPIA_PHONE
void EditorUI::removeFileItems()
{
    open_action->removeFrom( toolbar );
    menubar->removeItem( file_menu_id );
}
#endif
