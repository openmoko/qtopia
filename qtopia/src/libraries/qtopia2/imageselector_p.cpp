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

#include "imageselector_p.h"

#include "singleview_p.h"

#include "image.h"

#include <qtopia/resource.h>
#include <qtopia/global.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/storage.h>
#include <qtopia/contextbar.h>

#include <qlist.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qtoolbutton.h>

#define CATEGORY_ALL -2
#define CATEGORY_UNFILED -1

#define DEFAULT_VIEW ImageSelector::Thumbnail
#define DEFAULT_CATEGORY_ID CATEGORY_ALL

#define SINGLE_VIEW_ID 1
#define THUMBNAIL_VIEW_ID 2
#define MESSAGE_VIEW_ID 3

#define HIGH_STRETCH_FACTOR 100

ImageSelectorPrivate::ImageSelectorPrivate( const QStringList& source, 
    QWidget* parent, const char* name, WFlags f )
    : QWidget( parent, name, f ), source_directories( source ),
    category_id( DEFAULT_CATEGORY_ID ),
    current_view( DEFAULT_VIEW )
{
    // Respond to file changes
    connect( qApp, SIGNAL( linkChanged( const QString& ) ),
        this, SLOT( linkChanged( const QString& ) ) );

    // Construct widget stack
    widget_stack = new QWidgetStack( this );
    connect( widget_stack, SIGNAL( aboutToShow( QWidget* ) ),
        this, SLOT( setFocus( QWidget* ) ) );

    // Add single view to widget stack
    widget_stack->addWidget( single_view = new SingleView( widget_stack ), 
        SINGLE_VIEW_ID );
    connect( single_view, SIGNAL( selected() ), this, SLOT( select() ) );
    connect( single_view, SIGNAL( held( const QPoint& ) ), 
        this, SLOT( hold( const QPoint& ) ) );
    // Connect single view to image collection
    connect( &visible_collection, SIGNAL( currentChanged( const DocLnk& ) ),
        single_view, SLOT( setImage( const DocLnk& ) ) );
    connect( &visible_collection, SIGNAL( updated( const DocLnk& ) ),
        single_view, SLOT( setImage( const DocLnk& ) ) );
    connect( single_view, SIGNAL( forward() ),
        &visible_collection, SLOT( forward() ) );
    connect( single_view, SIGNAL( back() ),
        &visible_collection, SLOT( back() ) );

    // Add thumbnail view to widget stack
    widget_stack->addWidget( 
        thumbnail_view = new ThumbnailView( widget_stack ), THUMBNAIL_VIEW_ID );
    connect( thumbnail_view, SIGNAL( selected() ), this, SLOT( select() ) );
    connect( thumbnail_view, SIGNAL( held( const QPoint& ) ), 
        this, SLOT( hold( const QPoint& ) ) );
    // Connect thumbnail view to image collection
    connect( &visible_collection, SIGNAL( inserted( const DocLnk& ) ),
        thumbnail_view, SLOT( insert( const DocLnk& ) ) );
    connect( &visible_collection, SIGNAL( updated( const DocLnk& ) ),
        thumbnail_view, SLOT( update( const DocLnk& ) ) );
    connect( &visible_collection, SIGNAL( removed( const DocLnk& ) ),
        thumbnail_view, SLOT( remove( const DocLnk& ) ) );
    connect( &visible_collection, SIGNAL( cleared() ),
        thumbnail_view, SLOT( clear() ) );
    connect( &visible_collection, SIGNAL( currentChanged( const DocLnk& ) ),
        thumbnail_view, SLOT( setHighlighedImage( const DocLnk& ) ) );
    connect( thumbnail_view, SIGNAL( currentChanged( const DocLnk& ) ),
        &visible_collection, SLOT( setCurrentImage( const DocLnk& ) ) );

    // Add message view to widget stack
    widget_stack->addWidget( message_view = new QLabel( 
        tr( "<center><p>No images found.</p></center>" ), widget_stack ),
        MESSAGE_VIEW_ID );
        
#ifdef QTOPIA_PHONE
    // Construct context menu
    context_menu = new ContextMenu( this );

    // Add view category to context menu 
    category_dialog = new CategorySelectDialog( 
        "Document View", this, 0, true );
    category_dialog->setAllCategories( true );
    connect( category_dialog, SIGNAL( selected( int ) ),
        this, SLOT( changeCategory( int ) ) );
    context_menu->insertItem( Resource::loadIconSet( "viewcategory" ),
        tr( "View Category..." ), this, SLOT( launchCategoryDialog() ) );
        
#else
    // Construct view toggle buttons
    view_toggle_group = new QButtonGroup( this );
    view_toggle_group->setExclusive( true );
    view_toggle_group->hide();
    
    // Construct thumbnail view toggle button
    QToolButton *thumbnail_view_toggle = new QToolButton( this );
    thumbnail_view_toggle->setIconSet( Resource::loadIconSet( "thumbnail" ) );
    thumbnail_view_toggle->setToggleButton( true );
    thumbnail_view_toggle->toggle();
    thumbnail_toggle_id = view_toggle_group->insert( thumbnail_view_toggle );
    connect( thumbnail_view_toggle, SIGNAL( clicked() ),
        this, SLOT( setViewThumbnail() ) );

    // Construct single view toggle button
    QToolButton *single_view_toggle = new QToolButton( this );
    single_view_toggle->setIconSet( Resource::loadIconSet( "single" ) );
    single_view_toggle->setToggleButton( true );
    single_toggle_id = view_toggle_group->insert( single_view_toggle );
    connect( single_view_toggle, SIGNAL( clicked() ), 
        this, SLOT( setViewSingle() ) );
    
    // Construct category selector
    category_selector = new CategorySelect( 
        QArray<int>(), "Document View", this );
    category_selector->setRemoveCategoryEdit( true );
    category_selector->setAllCategories( true );
    connect( category_selector, SIGNAL( signalSelected( int ) ),
        this, SLOT( changeCategory( int ) ) );
#endif

    // Arrange widgets in layout
    QVBoxLayout *main_layout = new QVBoxLayout( this );
    main_layout->addWidget( widget_stack, HIGH_STRETCH_FACTOR );
#ifndef QTOPIA_PHONE
    QHBoxLayout *toolbar_layout = new QHBoxLayout( main_layout );
    toolbar_layout->addWidget( single_view_toggle );
    toolbar_layout->addWidget( thumbnail_view_toggle );
    toolbar_layout->addWidget( category_selector );
#endif

    // Update collection when disks have changed
    StorageInfo *storage_info = new StorageInfo( this );
    connect( storage_info, SIGNAL( disksChanged() ), this, SLOT( reload() ) );

    // Initialize image collection and views
    updateVisibleCollection();
    // Display message view if no images visible
    if( visible_collection.isEmpty() ) {
        // Raise message view to top of stack
        widget_stack->raiseWidget( MESSAGE_VIEW_ID );
        message_view->setFocus();
    } else {
        // Otherwise, load thumbnail view
        widget_stack->raiseWidget( THUMBNAIL_VIEW_ID );
        thumbnail_view->setFocus();
    }
}

void ImageSelectorPrivate::setViewMode( ImageSelector::ViewMode mode )
{
    // If there are images in the visible collection
    if( !visible_collection.isEmpty() ) {
        // Move new view to top of widget stack
        switch( mode ) {
        // If Single
        case ImageSelector::Single:
            if( current_view != ImageSelector::Single ) {
                // Move single view to top of stack
                widget_stack->raiseWidget( SINGLE_VIEW_ID );
#ifndef QTOPIA_PHONE
                view_toggle_group->setButton( single_toggle_id );
#endif
            }
            break;
        // If Thumbnail
        case ImageSelector::Thumbnail:
            if( current_view != ImageSelector::Thumbnail ) {
                // Move thumbnail view to top of stack
                widget_stack->raiseWidget( THUMBNAIL_VIEW_ID );
#ifndef QTOPIA_PHONE
                view_toggle_group->setButton( thumbnail_toggle_id );
#endif
            }
            break;
        }
    }

    // Update current view mode
    current_view = mode;
}

void ImageSelectorPrivate::setCategory( int id )
{
    if( id != category_id ) {
        // Assign new category id
        category_id = id;
        // Update collection with images visible in this category
        updateVisibleCollection();
        // Raise current view to top of stack
        raiseCurrentView();
        // Set current category in category selector
#ifdef QTOPIA_PHONE
        category_dialog->setCurrentCategory( id );
#else
        category_selector->setCurrentCategory( id );
#endif
        // Notify change of category
        emit categoryChanged();
    }
}

QString ImageSelectorPrivate::selectedFilename() const
{
    DocLnk image = visible_collection.currentImage();
    // If there is no current selection, return a null QString
    if( !image.fileKnown() ) return QString();
    // Otherwise, return the image filename
    return image.file();
}

DocLnk ImageSelectorPrivate::selectedDocument() const
{
    // If there is no current selection, return invalid DocLnk
    // Otherwise, return DocLnk for image
    return visible_collection.currentImage();
}

QPixmap ImageSelectorPrivate::selectedImage( int width, int height ) const
{
    DocLnk image = visible_collection.currentImage();
    // If there is no current selection, return a null QPixmap
    if( !image.fileKnown() ) return QPixmap();
    // Otherwise, copy selected image and resize maintaining w:h ratio
    // Return resized image
    QPixmap return_image;
    return_image.convertFromImage( 
        Image::loadScaled( image.file(), width, height ) );
    return return_image;
}

QValueList< DocLnk > ImageSelectorPrivate::fileList() const
{
    // Return list of currently visible images
    return visible_collection.collection();
}

int ImageSelectorPrivate::fileCount() const
{
    return visible_collection.collection().count();
}

#ifdef QTOPIA_PHONE
void ImageSelectorPrivate::enableDialogMode()
{
    single_view->enableDialogMode();
    ContextBar::setLabel( single_view, Qt::Key_Select, ContextBar::Select );
    ContextBar::setLabel( single_view, Qt::Key_Back, ContextBar::Cancel );
    ContextBar::setLabel( thumbnail_view, Qt::Key_Select, ContextBar::View );
    // Switch to thumbnail view when canceled recieved from single view
    connect( single_view, SIGNAL( canceled() ), 
        this, SLOT( setViewThumbnail() ) );
    // Disable selection in thumbnail mode
    disconnect( thumbnail_view, SIGNAL( selected() ), this, SLOT( select() ) );
    // Switch to single view when selected recieved from thumbnail view
    connect( thumbnail_view, SIGNAL( selected() ), 
        this, SLOT( setViewSingle() ) );
}
#endif

void ImageSelectorPrivate::select()
{
    // Emit selected signal
    emit selected( visible_collection.currentImage() );
}

void ImageSelectorPrivate::hold( const QPoint& pos )
{
    // Emit held signal
    emit held( visible_collection.currentImage(), pos );
}

void ImageSelectorPrivate::setViewSingle()
{
    setViewMode( ImageSelector::Single );
}

void ImageSelectorPrivate::setViewThumbnail()
{
    setViewMode( ImageSelector::Thumbnail );
}

void ImageSelectorPrivate::launchCategoryDialog()
{
    // Launch category selection dialog
    QPEApplication::execDialog( category_dialog );
}

void ImageSelectorPrivate::reload()
{
    updateVisibleCollection();
    raiseCurrentView();
    emit reloaded();
}

void ImageSelectorPrivate::changeCategory( int id )
{
    setCategory( id );
    widget_stack->visibleWidget()->setFocus();
}

void ImageSelectorPrivate::setFocus( QWidget* widget )
{
    widget->setFocus();
}

void ImageSelectorPrivate::linkChanged( const QString& lnk )
{
    DocLnk doclnk( lnk );
    
    // Gather images from the source directories
    DocLnkSet images;
    if( !source_directories.isEmpty() ) {
        QStringList::Iterator it;
        for( it = source_directories.begin(); it != source_directories.end();
            ++it ) {
            DocLnkSet set( *it, "*/jpeg;*/png" );
            images.appendFrom( set );
        }
    } else Global::findDocuments( &images, "image/*" );
    
    // Find lnk in current directory
    QList<DocLnk> image_list = images.children();
    DocLnk *list_element;
    for( list_element = image_list.first(); list_element && 
        !( list_element->linkFileKnown() && 
            list_element->linkFile() == lnk ) &&
        list_element->file() != lnk && list_element->file() != doclnk.file(); 
        list_element = image_list.next() );
    
    // Find lnk in collection
    QValueList< DocLnk >::ConstIterator it;
    for( it = visible_collection.collection().begin(); 
        it != visible_collection.collection().end() &&
        !( (*it).linkFileKnown() && (*it).linkFile() == lnk ) &&
        (*it).file() != lnk && (*it).file() != doclnk.file(); ++it );
    
    // If lnk is in the current directory and lnk not in the collection
    if( list_element && it == visible_collection.collection().end() ) {
        // If the image is in the current category, add to the colleciton
        if( inCurrentCategory( doclnk ) ) {
            visible_collection.insert( doclnk );
            // If first image in collection, make image current
            if( visible_collection.collection().count() == 1 )
                visible_collection.makeHeadCurrent();
            emit fileAdded( doclnk );
        }
    }
    
    // If lnk is in the current directory and lnk is in the collection
    else if( list_element && it != visible_collection.collection().end() ) {
        // If image is in the current category, update image in collection
        // Otherwise, remove from collection
        if( inCurrentCategory( doclnk ) ) {
            visible_collection.update( doclnk );
            emit fileUpdated( doclnk );
        } else {
            visible_collection.remove( doclnk );
            emit fileRemoved( doclnk );
        }
    }
    
    // If lnk is not in the current directory and lnk is in the collection
    else if( !list_element && it != visible_collection.collection().end() ) {
        // Remove image from collection
        doclnk = *it;
        visible_collection.remove( doclnk );
        emit fileRemoved( doclnk );
    }
    
    // Raise current view to top of stack
    raiseCurrentView();
}

void ImageSelectorPrivate::updateVisibleCollection()
{
    // Clear current collection
    visible_collection.clear();
    
    // Add images that match the current category to the visible collection
    // Gather images from the source directories
    DocLnkSet images;
    if( !source_directories.isEmpty() ) {
        QStringList::Iterator it;
        for( it = source_directories.begin(); it != source_directories.end();
            ++it ) {
            DocLnkSet set( *it, "*/jpeg;*/png" );
            images.appendFrom( set );
        }
    } else Global::findDocuments( &images, "image/*" );
    
    // Add only thoes images in the current category
    QList<DocLnk> image_list = images.children();
    DocLnk *list_element;
    for( list_element = image_list.first(); list_element;
        list_element = image_list.next() )
        if( inCurrentCategory( *list_element ) )
            visible_collection.insert( *list_element );
    
    // Make the first image the current image
    visible_collection.makeHeadCurrent();
}

void ImageSelectorPrivate::focusInEvent( QFocusEvent* )
{
    widget_stack->visibleWidget()->setFocus();
}

void ImageSelectorPrivate::raiseCurrentView()
{
    // If no images in collection, raise message view to top of stack
    if( visible_collection.isEmpty() ) {
        widget_stack->raiseWidget( message_view );  
    } else {
        if( widget_stack->visibleWidget() == message_view ) {
            // Otherwise, raise current view to top of stack
            switch( current_view ) {
            case ImageSelector::Single:
                widget_stack->raiseWidget( single_view );
                break;
            case ImageSelector::Thumbnail:
                widget_stack->raiseWidget( thumbnail_view );
                break;
            }
        }
    }
}

bool ImageSelectorPrivate::inCurrentCategory( const DocLnk& image ) const
{
    switch( category_id ) {
    case CATEGORY_ALL:
        // If category all, pass all images
        return true;
    case CATEGORY_UNFILED:
        // If category unfiled, pass only images without category
        if( image.categories().isEmpty() )
            return true;
        break;
    default:
        // Otherwise, pass only images that belong to the current category
        if( image.categories().contains( category_id ) )
            return true;
    }
        
    return false;
}

