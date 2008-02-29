/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "selectorui.h"


#include <qtoolbar.h>
#include <qmenubar.h>
#include <qstring.h>

#include <QMenu>

SelectorUI::SelectorUI( QWidget* parent, Qt::WFlags f )
    : QMainWindow( parent, f )
{
#ifndef QTOPIA_PHONE
    QToolBar *toolbar = new QToolBar( this );
    toolbar->setMovable( false );
    // toolbar->setHorizontalStretchable( true );
    QMenuBar *menubar = new QMenuBar( toolbar );
    toolbar->addWidget( menubar );
    addToolBar( toolbar );
    toolbar = new QToolBar( this );
    addToolBar( toolbar );

    // Make edit action
    edit_action = new QAction( QIcon( ":icon/edit" ) , tr( "Edit" ), this );
    edit_action->setWhatsThis( tr( "Edit the current image." ) );
    connect( edit_action, SIGNAL( triggered() ),
        this, SIGNAL( edit() ) );

    // Make delete action
    delete_action = new QAction( QIcon( ":icon/trash" ), tr( "Delete" ), this );
    delete_action->setWhatsThis( tr( "Delete the current image." ) );
    connect( delete_action, SIGNAL( triggered() ),
        this, SIGNAL( remove() ) );

    // Make beam action
    beam_action = new QAction( QIcon( ":icon/beam" ), tr( "Beam" ), this );
    beam_action->setWhatsThis( tr( "Beam the current image." ) );
    connect( beam_action, SIGNAL( triggered() ),
        this, SIGNAL( beam() ) );

    // Make properties action
    properties_action = new QAction( QIcon( ":icon/info" ), tr( "Properties" ), this );
    properties_action->setWhatsThis(
        tr( "Edit the properties of the current image." ) );
    connect( properties_action, SIGNAL( triggered() ),
        this, SIGNAL( properties() ) );

    // Make slide show action
    slide_show_action = new QAction( QIcon( ":icon/slideshow" ), tr( "Slide Show..." ), this );
    slide_show_action->setWhatsThis(
        tr( "View a slide show of the images in the current category." ) );
    connect( slide_show_action, SIGNAL( triggered() ),
        this, SIGNAL( slideShow() ) );

    // Construct menu bar
    // Construct image menu
    QMenu *image_menu = new QMenu( tr( "Image" ), this );
    image_menu->addAction( edit_action );
    image_menu->addAction( delete_action );
    image_menu->addAction( beam_action );
    image_menu->addAction( properties_action );
    menubar->addMenu( image_menu );

    // Construct view menu
    QMenu *view_menu = new QMenu( tr( "View" ), this );
    view_menu->addAction( slide_show_action );
    menubar->addMenu( view_menu );

    // Construct tool button
    toolbar->addAction( edit_action );
    toolbar->addAction( delete_action);
    toolbar->addAction( beam_action );
    toolbar->addAction( properties_action );
    toolbar->addAction( slide_show_action );
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
