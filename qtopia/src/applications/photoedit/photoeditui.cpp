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

#include "photoeditui.h"

#include <qtopia/global.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/contextbar.h>
#include <qtopia/docproperties.h>
#include <qtopia/resource.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/ir.h>
#include <qtopia/services.h>

#include <qpoint.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qsize.h>
#include <math.h>

#define EDIT_ID 1
#define DELETE_ID 2
#define BEAM_ID 3
#define PROPERTIES_ID 4
#define SLIDE_SHOW_ID 5

PhotoEditUI::PhotoEditUI( QWidget* parent, const char* name, WFlags f )
    : QVBox( parent, name, f | Qt::WType_TopLevel ), ui_state( SELECTOR ),
    editor_state( VIEW ), only_editor( false ), service_requested( false )
{
#ifdef QTOPIA_PHONE
    was_fullscreen = false;
    edit_canceled = false;
    close_ok = false;
    editor_state_changed = false;
#endif

    setCaption( tr( "Pictures" ) );
    
    // Respond to service requests
    connect( qApp, SIGNAL( appMessage( const QCString&, const QByteArray& ) ),
        this, SLOT( appMessage( const QCString&, const QByteArray& ) ) );
    
    // Respond to file changes
    connect( qApp, SIGNAL( linkChanged( const QString& ) ),
        this, SLOT( linkChanged( const QString& ) ) );       

    // Construct widget stack
    widget_stack = new QWidgetStack( this );
    
    // Construct slide show
    slide_show = new SlideShow( this );
    
    // Construct slide show dialog
    slide_show_dialog = new SlideShowDialog( this );
    
    // Construct slide show ui
    slide_show_ui = new SlideShowUI( widget_stack );
    slide_show_ui->installEventFilter( this );
    // Update image when slide show has changed
    connect( slide_show, SIGNAL( changed( const DocLnk& ) ),
        slide_show_ui, SLOT( setImage( const DocLnk& ) ) );
    // Stop slide show when slide show ui pressed
    connect( slide_show_ui, SIGNAL( pressed() ), slide_show, SLOT( stop() ) );
    // Show selector when slide show has stopped
    connect( slide_show, SIGNAL( stopped() ), 
        this, SLOT( exitCurrentUIState() ) );
    
#ifndef QTOPIA_PHONE
    // Construct selector ui
    selector_ui = new SelectorUI( widget_stack );
    // When slide show selected launch slide show dialog
    connect( selector_ui, SIGNAL( slideShow() ), 
        this, SLOT( launchSlideShowDialog() ) );
    connect( selector_ui, SIGNAL( edit() ),
        this, SLOT( editCurrentSelection() ) );
    connect( selector_ui, SIGNAL( beam() ),
        this, SLOT( beamImage() ) );
    connect( selector_ui, SIGNAL( remove() ),
        this, SLOT( deleteImage() ) );
    // When properties selected launch properties dialog
    connect( selector_ui, SIGNAL( properties() ), 
        this, SLOT( launchPropertiesDialog() ) );
#endif
        
    // Construct image selector
#ifdef QTOPIA_PHONE
    image_selector = new ImageSelector( widget_stack );
    connect( image_selector, SIGNAL( selected( const DocLnk& ) ),
        this, SLOT( setViewSingle() ) );
#else
    image_selector = new ImageSelector( selector_ui );
    selector_ui->setCentralWidget( image_selector );
    connect( image_selector, SIGNAL( selected( const DocLnk& ) ),
        this, SLOT( setViewSingle() ) );
    connect( image_selector, SIGNAL( held( const DocLnk&, const QPoint& ) ),
        this, SLOT( launchPopupMenu( const DocLnk&, const QPoint& ) ) );
#endif
    connect( image_selector, SIGNAL( fileAdded( const DocLnk& ) ),
        this, SLOT( toggleActions() ) );
    connect( image_selector, SIGNAL( fileRemoved( const DocLnk& ) ),
        this, SLOT( toggleActions() ) );
    connect( image_selector, SIGNAL( categoryChanged() ),
        this, SLOT( toggleActions() ) );
    connect( image_selector, SIGNAL( reloaded() ),
        this, SLOT( toggleActions() ) );

#ifdef QTOPIA_PHONE
    // Construct context menu for selector ui
    selector_menu = image_selector->menu();
    separator_id = selector_menu->insertSeparator( 0 );
    // Add properties item to selector menu
    selector_menu->insertItem( Resource::loadIconSet( "info" ),
        tr( "Properties" ), this, SLOT( launchPropertiesDialog() ), 0, 
            PROPERTIES_ID, 0 );
    selector_menu->setItemEnabled( PROPERTIES_ID, false );
    // Add beam item to selector menu
    selector_menu->insertItem( Resource::loadIconSet( "beam" ),
        tr( "Beam" ), this, SLOT( beamImage() ), 0, BEAM_ID, 0 );
    selector_menu->setItemEnabled( BEAM_ID, false );
    // Add delete item to selector menu
    selector_menu->insertItem( Resource::loadIconSet( "trash" ),
        tr( "Delete" ), this, SLOT( deleteImage() ), 0, DELETE_ID, 0 );
    selector_menu->setItemEnabled( DELETE_ID, false );
    // Add edit item to selector menu
    selector_menu->insertItem( Resource::loadIconSet( "edit" ), 
        tr( "Edit" ), this, SLOT( editCurrentSelection() ), 0, EDIT_ID, 0 );
    selector_menu->setItemEnabled( EDIT_ID, false );
    // Add slide show item to selector menu
    selector_menu->insertItem( 
        Resource::loadIconSet( "slideshow" ), tr( "Slide Show..." ), this, 
        SLOT( launchSlideShowDialog() ), 0, SLIDE_SHOW_ID, 0 );
    selector_menu->setItemEnabled( SLIDE_SHOW_ID, true );
#else
    selector_menu = new QPopupMenu( this );
    selector_menu->insertItem( tr( "Edit" ), this, SLOT( editCurrentSelection() ) );
    selector_menu->insertItem( tr( "Delete" ), this, SLOT( deleteImage() ) );
    selector_menu->insertItem( tr( "Beam" ), this, SLOT( beamImage() ) );
    selector_menu->insertItem( tr( "Properties" ), 
        this, SLOT( launchPropertiesDialog() ) );
#endif
    
    // Construct image io
    image_io = new ImageIO( this );
    
    // Construct image processor
    image_processor = new ImageProcessor( image_io, this );
    
    // Construct editor ui
    editor_ui = new EditorUI( widget_stack );
    // Construct image ui
    image_ui = new ImageUI( image_processor, editor_ui );
    image_ui->installEventFilter( this );
    editor_ui->setCentralWidget( image_ui );
    
    // Construct image ui controls
    QVBoxLayout *box = new QVBoxLayout( image_ui );
    // Construct region selector
    region_selector = new RegionSelector( image_ui );
    box->addWidget( region_selector );
#ifdef QTOPIA_PHONE
    if( Global::mousePreferred() ) {
#endif
        connect( region_selector, SIGNAL( pressed() ), 
            this, SLOT( exitCurrentEditorState() ) );
        connect( region_selector, SIGNAL( canceled() ),
            this, SLOT( exitCurrentEditorState() ) );
#ifdef QTOPIA_PHONE
    }
#endif
    connect( region_selector, SIGNAL( selected() ), 
        this, SLOT( cropImage() ) );
    connect( region_selector, SIGNAL( selected() ),
        this, SLOT( exitCurrentEditorState() ) );

    QGridLayout *grid_layout = new QGridLayout( region_selector, 2, 2, 10 );
    // Construct brightness control
    brightness_slider = new Slider( -70, 70, 0, 0, region_selector );
    grid_layout->addWidget( brightness_slider, 2, 1 );
    brightness_slider->hide();
#ifdef QTOPIA_PHONE
    connect( brightness_slider, SIGNAL( selected() ),
        this, SLOT( exitCurrentEditorState() ) );
#endif
    connect( brightness_slider, SIGNAL( valueChanged( int ) ),
        this, SLOT( setBrightness( int ) ) );
        
    // Construct zoom control
    zoom_slider = new Slider( 100, 200, 10, 0, region_selector );
    zoom_slider->setLineStep( 5 );
    grid_layout->addWidget( zoom_slider, 2, 1 );
    zoom_slider->hide();
#ifdef QTOPIA_PHONE
    connect( zoom_slider, SIGNAL( selected() ),
        this, SLOT( exitCurrentEditorState() ) );
#endif
    connect( zoom_slider, SIGNAL( valueChanged( int ) ), 
        this, SLOT( setZoom( int ) ) );
        
    // Construct navigator
    navigator = new Navigator( image_ui, region_selector );
    grid_layout->addWidget( navigator, 2, 2 );
        
#ifdef QTOPIA_PHONE
    // Ignore changes if edit is canceled
    connect( editor_ui, SIGNAL( cancel() ),
        this, SLOT( cancelEdit() ) );
#else
    connect( editor_ui, SIGNAL( open() ), 
        this, SLOT( exitCurrentUIState() ) );
    connect( editor_ui, SIGNAL( open() ), 
        this, SLOT( exitCurrentEditorState() ) );
    connect( editor_ui, SIGNAL( rotate() ),
        this, SLOT( exitCurrentEditorState() ) );
#endif
    // Connect imaging functions to image processor
    connect( editor_ui, SIGNAL( zoom() ),
        this, SLOT( enterZoom() ) );
    connect( editor_ui, SIGNAL( brightness() ),
        this, SLOT( enterBrightness() ) );
    connect( editor_ui, SIGNAL( crop() ),
        this, SLOT( enterCrop() ) );
    connect( editor_ui, SIGNAL( rotate() ),
        image_processor, SLOT( rotate() ) );
    connect( editor_ui, SIGNAL( fullScreen() ),
        this, SLOT( enterFullScreen() ) );
    
    // Initialize selector and raise to top of widget stack
    toggleActions();
#ifdef QTOPIA_PHONE
    widget_stack->raiseWidget( image_selector );
#else
    widget_stack->raiseWidget( selector_ui );
#endif
}

bool PhotoEditUI::eventFilter( QObject*, QEvent* e )
{
    if( e->type() == QEvent::WindowDeactivate ) {
        switch( ui_state ) {
        case SLIDE_SHOW:
            exitCurrentUIState();
            break;
        case EDITOR:
            if( editor_state == FULL_SCREEN ) {
                exitCurrentEditorState();
#ifdef QTOPIA_PHONE
                was_fullscreen = false;
#endif
            }
            break;
        default:
            // Ignore
            break;
        }
    }
    
    return false;
}

void PhotoEditUI::setDocument( const QString& lnk )
{
    service_lnk = lnk;
    // Ensure editor is raised
#ifndef QTOPIA_PHONE
    if( ui_state != EDITOR ) {
#endif
        image_ui->setEnabled( false );
        widget_stack->raiseWidget( editor_ui );
#ifndef QTOPIA_PHONE
    }
#endif
    QTimer::singleShot( 0, this, SLOT( processSetDocument() ) );
}

void PhotoEditUI::appMessage( const QCString& msg, const QByteArray& data )
{
    // If show category service requested, change category in selector
    // Otherwise if get image service requested, open editor with image passed
    if( msg == "showCategory(int)" ) {
        QPEApplication::setKeepRunning();
        QDataStream stream( data, IO_ReadOnly );
        stream >> service_category;
        QTimer::singleShot( 0, this, SLOT( processShowCategory() ) );
    } else if( msg == "getImage(QCString,QString,int,int,QImage)" ) {
        QPEApplication::setKeepRunning();
#ifndef QTOPIA_PHONE
        // Respond to previous service request
        if( ui_state == EDITOR && service_requested ) sendValueSupplied();
        service_image.reset();
#endif
        QDataStream stream( data, IO_ReadOnly );
        stream >> service_channel >> service_id >> service_width >> 
            service_height >> service_image;
        // Ensure editor is raised
#ifndef QTOPIA_PHONE
        if( ui_state != EDITOR ) {
#endif
            image_ui->setEnabled( false );
            widget_stack->raiseWidget( editor_ui );
#ifndef QTOPIA_PHONE
        }
#endif
        QTimer::singleShot( 0, this, SLOT( processGetImage() ) );
    }
}

void PhotoEditUI::processSetDocument()
{
#ifndef QTOPIA_PHONE
    interruptCurrentState();
    service_requested = false;
    if( !only_editor ) {
        editor_ui->removeFileItems();
#endif
        only_editor = true;
#ifndef QTOPIA_PHONE
    }
#endif
    clearEditor();
    qApp->processEvents();
    current_image = service_lnk;
    enterEditor();
}

void PhotoEditUI::processShowCategory() 
{
#ifndef QTOPIA_PHONE
    interruptCurrentState();
    service_requested = false;
    if( only_editor ) {
        editor_ui->addFileItems();
#endif
        only_editor = false;
#ifndef QTOPIA_PHONE
    }
#endif
    image_selector->setCategory( service_category );
    qApp->processEvents();
    enterSelector();
}

void PhotoEditUI::processGetImage()
{
#ifndef QTOPIA_PHONE
    if( ui_state == EDITOR && !service_requested ) saveChanges();
    if( !only_editor ) {
        editor_ui->removeFileItems();
#endif
        only_editor = true;
#ifndef QTOPIA_PHONE
    }
#endif
    clearEditor();
    qApp->processEvents();
    service_requested = true;
    if( service_image.isNull() ) enterSelector();
    else enterEditor();
}

void PhotoEditUI::toggleActions()
{
    // If there are images in the visible collection, enable actions
    // Otherwise, disable action
    bool b = image_selector->fileCount();
#ifdef QTOPIA_PHONE
    switch( image_selector->viewMode() ) {
    case ImageSelector::Single:
        selector_menu->setItemEnabled( EDIT_ID, b );
        selector_menu->setItemEnabled( BEAM_ID, b );
        selector_menu->setItemEnabled( DELETE_ID, b );
        selector_menu->setItemEnabled( PROPERTIES_ID, b );
        break;
    case ImageSelector::Thumbnail:
        selector_menu->setItemEnabled( SLIDE_SHOW_ID, b );
        if( b ) ContextBar::setLabel( image_selector, Qt::Key_Select, 
            ContextBar::View );
        else ContextBar::setLabel( image_selector, Qt::Key_Select,
            ContextBar::NoLabel );
        break;
    }

    selector_menu->setItemEnabled( separator_id, b );
#else
    selector_ui->setEnabled( b );
#endif
}

void PhotoEditUI::enterSelector()
{
    // Clear current image
    current_image = DocLnk();
    // Raise selector to top of stack
#ifdef QTOPIA_PHONE
    widget_stack->raiseWidget( image_selector );
    image_selector->setFocus();
#else
    widget_stack->raiseWidget( selector_ui );
#endif
    ui_state = SELECTOR;
}

void PhotoEditUI::enterSlideShow()
{
    // Set slide show collection from currently visible collection in selector
    slide_show->setCollection( image_selector->fileList() );
    // Set first image in slideshow to currently selected image in selector
    slide_show->setFirstImage( image_selector->selectedDocument() );
    // Show slide show ui in full screen
    slide_show_ui->reparent( 0, QPoint( 0, 0 ) );
    slide_show_ui->showFullScreen();
    // Start slideshow
    slide_show->start();
    ui_state = SLIDE_SHOW;
}

void PhotoEditUI::enterEditor()
{
#define LIMIT( X, Y, Z ) ( (X) > (Y) ? (X) > (Z) ? (Z) : (X) : (Y) )
#define REDUCTION_RATIO( dw, dh, sw, sh ) \
    ( (dw)*(sh) > (dh)*(sw) ? (double)(dh)/(double)(sh) : \
    (double)(dw)/(double)(sw) )
    
    navigator->hide();
    image_ui->setEnabled( false );
    // Raise editor to top of widget stack
    widget_stack->raiseWidget( editor_ui );
    qApp->processEvents();
    
    // Update image io with current image
    ImageIO::Status status;
    if( service_requested && !service_image.isNull() )
        status = image_io->load( service_image );
    else status = image_io->load( current_image );
    
    switch( status ) {
    case ImageIO::NORMAL:
        {
            // Initialize editor controls
            brightness_slider->setValue( 0 );
            // Zoom to fit image in screen
            QSize size = image_io->size();
            QSize view = image_ui->size();
            disconnect( zoom_slider, SIGNAL( valueChanged( int ) ), 
                this, SLOT( setZoom( int ) ) );
            if( size.width() > view.width() || size.height() > view.height() ) {
                double ratio = REDUCTION_RATIO( view.width(), view.height(),
                    size.width(), size.height() );
                ratio = LIMIT( ratio, 0.1, 1.0 );
                image_processor->setZoom( ratio );
                zoom_slider->setValue( (int) ( log( ratio * 100 ) / 
                    log( 10 ) * 100 ) );
            } else {
                image_processor->setZoom( 1.0 );
                zoom_slider->setValue( 200 );
            }
            connect( zoom_slider, SIGNAL( valueChanged( int ) ), 
                this, SLOT( setZoom( int ) ) );
            image_ui->reset();
            image_ui->setEnabled( true );
            navigator->show();
            navigator->setFocus();
            ui_state = EDITOR;
        }
        break;
    case ImageIO::LOAD_ERROR:
        QMessageBox::warning( 0, tr( "Load Error" ), 
            tr( "<qt>Unable to load image.</qt>" ) );
        if( only_editor ) close();
        else enterSelector();
        break;
    case ImageIO::DEPTH_ERROR:
        QMessageBox::warning( 0, tr( "Depth Error" ),
            tr( "<qt>Image depth is not supported.</qt>" ) );
        if( only_editor ) close();
        else enterSelector();
        break;
    }
}

void PhotoEditUI::enterZoom()
{
#ifndef QTOPIA_PHONE
    exitCurrentEditorState();
#endif
    // Hide navigator
    navigator->hide();
    // Show zoom control
    zoom_slider->show();
#ifdef QTOPIA_PHONE
    zoom_slider->setFocus();
#endif
    editor_state = ZOOM;
}

void PhotoEditUI::enterBrightness()
{
#ifndef QTOPIA_PHONE
    exitCurrentEditorState();
#endif
    // Hide navigator
    navigator->hide();
    // Show brightness control
    brightness_slider->show();
#ifdef QTOPIA_PHONE
    brightness_slider->setFocus();
#endif
    editor_state = BRIGHTNESS;
}

void PhotoEditUI::enterCrop()
{
#ifndef QTOPIA_PHONE
    exitCurrentEditorState();
#endif
    // Hide navigator
    navigator->hide();
    // Enable selection in region selector
    region_selector->reset();
    region_selector->setEnabled( true );
    region_selector->setFocus();
    editor_state = CROP;
}

void PhotoEditUI::enterFullScreen()
{
#ifndef QTOPIA_PHONE
    exitCurrentEditorState();
#endif
    // Show editor view in full screen
    image_ui->reparent( 0, QPoint( 0, 0 ) );
    image_ui->showFullScreen();
    navigator->setFocus();
#ifdef QTOPIA_PHONE
    if( !Global::mousePreferred() ) {
        connect( region_selector, SIGNAL( pressed() ), 
            this, SLOT( exitCurrentEditorState() ) );
    }
#endif
    editor_state = FULL_SCREEN;
}

void PhotoEditUI::setViewSingle()
{
#ifdef QTOPIA_PHONE
    // If image selector not in single, change to single and update context menu
    if( image_selector->viewMode() != ImageSelector::Single ) {
        image_selector->setViewMode( ImageSelector::Single );
        selector_menu->setItemEnabled( EDIT_ID, true );
        selector_menu->setItemEnabled( BEAM_ID, true );
        selector_menu->setItemEnabled( DELETE_ID, true );
        selector_menu->setItemEnabled( PROPERTIES_ID, true );
        selector_menu->setItemEnabled( SLIDE_SHOW_ID, false );
        ContextBar::setLabel( image_selector, Qt::Key_Select, 
            ContextBar::NoLabel );
    }
#else
    image_selector->setViewMode( ImageSelector::Single );
#endif
}

void PhotoEditUI::launchPopupMenu( const DocLnk&, const QPoint& 
#ifndef QTOPIA_PHONE
pos 
#endif
)
{
#ifndef QTOPIA_PHONE
    selector_menu->popup( pos );
#endif
}

void PhotoEditUI::launchSlideShowDialog()
{
    // If slide show dialog accepted, start slideshow
    if( QPEApplication::execDialog( slide_show_dialog, true ) ) {
        // Set slide show options
        slide_show_ui->setDisplayName( slide_show_dialog->isDisplayName() );
        slide_show->setSlideLength( 
            slide_show_dialog->slideLength() );
        slide_show->setLoopThrough( slide_show_dialog->isLoopThrough() );
        enterSlideShow();
    }
}

void PhotoEditUI::launchPropertiesDialog()
{
    DocLnk doc( image_selector->selectedDocument() );
    DocPropertiesDialog dialog( &doc );
    // Launch properties dialog with current image
    QPEApplication::execDialog( &dialog );
}

void PhotoEditUI::exitCurrentUIState()
{
    switch( ui_state ) {
    case SELECTOR:
#ifdef QTOPIA_PHONE
        switch( image_selector->viewMode() ) {
        // If in single view
        case ImageSelector::Single:
            // If there are images, change to thumbnail and update context menu
            // Otherwise, close application
            if( image_selector->fileCount() ) {
                image_selector->setViewMode( ImageSelector::Thumbnail );
                selector_menu->setItemEnabled( SLIDE_SHOW_ID, true );
                selector_menu->setItemEnabled( EDIT_ID, false );
                selector_menu->setItemEnabled( BEAM_ID, false );
                selector_menu->setItemEnabled( DELETE_ID, false );
                selector_menu->setItemEnabled( PROPERTIES_ID, false );
                ContextBar::setLabel( image_selector, Qt::Key_Select, 
                    ContextBar::View );
            } else close_ok = true;
            break;
        // If in thumbnail view, close application
        case ImageSelector::Thumbnail:
            close_ok = true;
            break;
        }
#endif
        break;
    case SLIDE_SHOW:
        // Return from slide show
        slide_show_ui->reparent( widget_stack, QPoint( 0, 0 ) );
        ui_state = SELECTOR;
        break;
    case EDITOR:
#ifdef QTOPIA_PHONE
        if( !edit_canceled ) {
#endif
            if( service_requested ) sendValueSupplied();
            else saveChanges();
#ifdef QTOPIA_PHONE
        }
        if( only_editor ) {
            if( service_requested && service_image.isNull() && edit_canceled ) 
                enterSelector();
            else close_ok = true;
        } else enterSelector();
        edit_canceled = false;
#else
        enterSelector();
#endif
        break;
    }
}

void PhotoEditUI::exitCurrentEditorState()
{
    switch( editor_state ) {
    // If in view, no change
    case VIEW:
#ifdef QTOPIA_PHONE
        editor_state_changed = false;
#endif
        break;
    // If in full screen, return from full screen
    case FULL_SCREEN:
#ifdef QTOPIA_PHONE
        if( !Global::mousePreferred() ) {
            disconnect( region_selector, SIGNAL( pressed() ), 
                this, SLOT( exitCurrentEditorState() ) );
        }
#endif
        // Set editor central widget to editor view
        image_ui->reparent( editor_ui, QPoint( 0, 0 ) );
        editor_ui->setCentralWidget( image_ui );
        editor_ui->setFocus();
#ifdef QTOPIA_PHONE
        if( !Global::mousePreferred() ) was_fullscreen = true;
#endif
        editor_state = VIEW;
#ifdef QTOPIA_PHONE
        editor_state_changed = true;
#endif
        break;
    // If in zoom, hide zoom control
    case ZOOM:
        zoom_slider->hide();
        // Show navigator
        navigator->show();
#ifdef QTOPIA_PHONE
        navigator->setFocus();
#endif
        editor_state = VIEW;
#ifdef QTOPIA_PHONE
        editor_state_changed = true;
#endif
        break;
    // If in brightness, hide brightness control
    case BRIGHTNESS:
        brightness_slider->hide();
        // Show navigator
        navigator->show();
#ifdef QTOPIA_PHONE
        navigator->setFocus();
#endif
        editor_state = VIEW;
#ifdef QTOPIA_PHONE
        editor_state_changed = true;
#endif
        break;
    // If in crop, disable region selector and show navigator
    case CROP:
        region_selector->setEnabled( false );
        region_selector->update();
        // Show navigator
        navigator->show();
        navigator->setFocus();
        editor_state = VIEW;
#ifdef QTOPIA_PHONE
        editor_state_changed = true;
#endif
        break;
    };
}

void PhotoEditUI::setZoom( int x )
{
    image_processor->setZoom( pow( 10.0, (double)x / 100.0 ) / 100.0 );
}

void PhotoEditUI::setBrightness( int x )
{
    image_processor->setBrightness( (double)x / 100.0 );
}

void PhotoEditUI::editCurrentSelection()
{
    // Retrieve current selection from image selector and open for editing
    current_image = image_selector->selectedDocument();
    enterEditor();
}

void PhotoEditUI::cancelEdit()
{
#ifdef QTOPIA_PHONE
    edit_canceled = true;
    close();
#endif
}

void PhotoEditUI::cropImage()
{
    // Ensure cropping region is valid
    QRect region( region_selector->region() );
    if( region.isValid() ) {
        // Retrieve region from region selector
        // Calculate cropped viewport and crop
        image_processor->crop( image_ui->viewport( 
            region_selector->region() ) );
        // Reset viewport
        image_ui->reset();
    }
}

void PhotoEditUI::beamImage()
{
    // Send current image over IR link
    DocLnk image = image_selector->selectedDocument();
    Ir().send( image, image.name() );
}

void PhotoEditUI::deleteImage()
{
    // Retrieve currently highlighted image from selector
    DocLnk image = image_selector->selectedDocument();
    // Lauch confirmation dialog
    // If deletion confirmed, delete image
    if( QMessageBox::information( this, tr( "Delete" ), tr( "<qt>Are you sure "
        "you want to delete %1?</qt>"," %1 = file name" ).arg(image.name()),
        QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes )
        image.removeFiles();
}

void PhotoEditUI::linkChanged( const QString& file )
{
    // If lnk is current image and has been deleted, show selector
    DocLnk lnk( file );
    if( !lnk.fileKnown() &&
        ( ( current_image.linkFileKnown() && current_image.linkFile() == file ) 
        || ( current_image.fileKnown() && current_image.file() == file ) ) ) {
            if( only_editor ) close();
            else enterSelector();
        }
}

void PhotoEditUI::closeEvent( QCloseEvent* e )
{
#ifdef QTOPIA_PHONE
    if( Global::mousePreferred() ) {
        if( ui_state != EDITOR || ( editor_state == VIEW && !editor_state_changed ) || edit_canceled ) {
            exitCurrentUIState();
            if( close_ok ) e->accept();
        }
    } else {
        if( was_fullscreen ) was_fullscreen = false;
        else {
            if( ui_state == EDITOR && editor_state != VIEW ) 
                exitCurrentEditorState();
            else {
                exitCurrentUIState();
                if( close_ok ) e->accept();
            }
        }
    }
#else
    exitCurrentEditorState();
    exitCurrentUIState();
    // Reverse effects of only editor launch for fast load
    if( only_editor ) {
        widget_stack->raiseWidget( selector_ui );
        editor_ui->addFileItems();
        only_editor = service_requested = false;
        service_image.reset();
    }
    e->accept();
#endif
}

#ifndef QTOPIA_PHONE
void PhotoEditUI::interruptCurrentState()
{
    // If editing, conclude editing
    if( ui_state == EDITOR ) {
        if( service_requested ) sendValueSupplied();
        else saveChanges();
    }
}
#endif

void PhotoEditUI::clearEditor()
{
    navigator->hide();
#ifndef QTOPIA_PHONE
    zoom_slider->hide();
    brightness_slider->hide();
#endif
    image_ui->setEnabled( false );
    image_ui->repaint();
}

void PhotoEditUI::saveChanges()
{
    // If image was changed, prompt user to save changes
    if( image_processor->isChanged() ) {
#ifndef QTOPIA_PHONE
        if( QMessageBox::information( this, tr( "Save Changes" ), 
            tr( "<qt>Do you want to save your changes?</qt>" ),
            QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes ) {
#endif
            // If save supported, prompt user to overwrite original
            // Otherwise, save as new file
            bool overwrite = false;
            if( image_io->isSaveSupported() ) {
                if( QMessageBox::information( this, tr( "Save Changes " ),
                    tr( "<qt>Do you want to overwrite the original?</qt>" ),
                    QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes )
                    overwrite = true;
            } else {
                const char* format = image_io->format();
                QMessageBox::warning( this, 
                    tr( "Saving %1" ).arg( format ),
                    tr( "<qt>Saving as %1 is not supported. "
                        "Using the default format instead.</qt>" )
                        .arg( format ) );
            }

            QImage image = image_processor->image();
            // Attempt to save changes
            bool saving = true;
            while( saving && !image_io->save( image, overwrite ) ) {
                QMessageBox mb( tr( "Out Of Space" ),
                    tr( "<qt>Try to cleanup storage space?</qt>" ),
                    QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No, 
                    QMessageBox::NoButton, this );
                
                if( mb.exec() == QMessageBox::Yes ) {
                    ServiceRequest req(
                        "CleanupWizard", "showCleanupWizard()" );
                    req.send();
                    
                    mb.setCaption( tr( "Save Changes" ) );
                    mb.setText( tr( "Try again?" ) );
                    mb.exec();
                }
                
                if( mb.result() == QMessageBox::No ) saving = false;
            }
#ifndef QTOPIA_PHONE
        }
#endif
    }
}

void PhotoEditUI::sendValueSupplied()
{
    QCopEnvelope e( service_channel, "valueSupplied(QString,QImage)" );
    e << service_id << image_processor->image(
        QSize( service_width, service_height ) );
}
