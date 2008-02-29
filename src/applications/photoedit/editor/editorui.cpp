/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "editorui.h"

#include <qsoftmenubar.h>

#include <QAction>
#include <QMenu>
#include <QCloseEvent>
#ifndef QTOPIA_PHONE
#include <QToolBar>
#include <QMenuBar>
#endif

EditorUI::EditorUI( QWidget* parent, Qt::WFlags f )
    : QMainWindow( parent, f )
{
#ifndef QTOPIA_PHONE
    // Make open action
    open_action = new QAction( QIcon( ":icon/fileopen" ), tr( "Open" ), this );
    open_action->setWhatsThis( tr( "Open an image." ) );
    connect( open_action, SIGNAL( triggered() ), this, SIGNAL( open() ) );
#endif

    // Make crop action
    QAction *crop_action = new QAction( QIcon( ":icon/cut" ), tr( "Crop" ), this );
    crop_action->setWhatsThis( tr( "Crop the image." ) );
    connect( crop_action, SIGNAL( triggered() ), this, SIGNAL( crop() ) );

    // Make brightness action
    QAction *brightness_action = new QAction( QIcon( ":icon/color" ), tr( "Brightness" ), this );
    brightness_action->setWhatsThis( tr( "Adjust the image brightness." ) );
    connect( brightness_action, SIGNAL( triggered() ), this, SIGNAL( brightness() ) );

    // Make rotate actions
    QAction *rotate_action = new QAction( QIcon( ":icon/rotate" ), tr( "Rotate" ), this );
    rotate_action->setWhatsThis( tr( "Rotate the image." ) );
    connect( rotate_action, SIGNAL( triggered() ), this, SIGNAL( rotate() ) );

    // Make zoom action
    QAction *zoom_action = new QAction( QIcon( ":icon/find" ), tr( "Zoom" ), this );
    zoom_action->setWhatsThis( tr( "Zoom in and out." ) );
    connect( zoom_action, SIGNAL( triggered() ), this, SIGNAL( zoom() ) );

    // Make fullscreen action
    QAction *fullscreen_action = new QAction( QIcon( ":icon/fullscreen" ), tr( "Full Screen" ), this );
    fullscreen_action->setWhatsThis( tr( "View the image in full screen." ) );
    connect( fullscreen_action, SIGNAL( triggered() ), this, SIGNAL( fullScreen() ) );

#ifdef QTOPIA_PHONE
    // Clear context bar
    QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );

    // Construct context menu
    QMenu *context_menu = QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setHelpEnabled( this, true );
    context_menu->addAction( crop_action );
    context_menu->addAction( brightness_action );
    context_menu->addAction( rotate_action );
    context_menu->addSeparator();
    context_menu->addAction( zoom_action );
    context_menu->addAction( fullscreen_action );
    context_menu->addSeparator();
    context_menu->addAction( QIcon( ":icon/cancel" ), tr( "Cancel" ), this, SIGNAL( cancel() ) );
#else
    // Construct menu bar
    QToolBar *toolbar = new QToolBar( this );
    toolbar->setMovable( false );
    QMenuBar *menubar = new QMenuBar( toolbar );
    toolbar->addWidget( menubar );
    addToolBar( toolbar );
    toolbar = new QToolBar( this );
    addToolBar( toolbar );

    // Construct file menu
    file_menu = new QMenu( tr( "File" ), this );
    file_menu->addAction( open_action );
    menubar->addMenu( file_menu );

    // Construct edit menu
    QMenu *edit_menu = new QMenu( tr( "Edit" ), this );
    edit_menu->addAction( brightness_action );
    edit_menu->addAction( rotate_action );
    edit_menu->addAction( crop_action );
    menubar->addMenu( edit_menu );

    // Construct view menu
    QMenu *view_menu = new QMenu( tr( "View" ), this );
    view_menu->addAction( zoom_action );
    view_menu->addAction( fullscreen_action );
    menubar->addMenu( view_menu );

    // Construct tool buttons
    toolbar->addAction( open_action );
    toolbar->addAction( brightness_action );
    toolbar->addAction( rotate_action );
    toolbar->addAction( crop_action );
    toolbar->addAction( zoom_action );
#endif
}

#ifndef QTOPIA_PHONE
void EditorUI::addFileItems()
{
    file_menu->menuAction()->setVisible( true );
    open_action->setVisible( true );
}
#endif

#ifndef QTOPIA_PHONE
void EditorUI::removeFileItems()
{
    file_menu->menuAction()->setVisible( false );
    open_action->setVisible( false );
}
#endif

void EditorUI::closeEvent( QCloseEvent *e )
{
    parentWidget()->close();
    e->ignore();
}

