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

#include "thumbnailview_p.h"

#include "image.h"

#include <qtopia/qpeapplication.h>
#ifdef QTOPIA_PHONE
#include <qtopia/contextbar.h>
#endif


#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qstring.h>
#include <qfileinfo.h>

#define DEFAULT_THUMBNAIL_SIZE 65

ThumbnailItem::ThumbnailItem( QIconView* parent, const QString& description, 
    const QString& file, int s, ThumbnailServer* ts )
    : QIconViewItem( parent, description )
{
    init( file, s, ts );
}   
        
ThumbnailItem::ThumbnailItem( QIconView* parent, QIconViewItem* after,
    const QString& description, const QString& file, int s, 
    ThumbnailServer* ts ) : QIconViewItem( parent, after, description )
{
    init( file, s, ts );
}

void ThumbnailItem::init( const QString& file, int s, ThumbnailServer* ts )
{
    _size = s;
    _loaded = false;
    loading = false;
    thumbnail_server = ts;

    // Store filename in key
    setKey( file );

    // If image currently in cache, load from cache
    QString key = "_" + file + QFileInfo(file).lastModified().toString() + QString::number(size());
    key = key.replace( QRegExp(" "), "_" );
    QPixmap image;
    if ( QPixmapCache::find(key, image) )
        setPixmap( image );
}

void ThumbnailItem::setPixmap( const QPixmap& pix )
{
    _loaded = true;
    QIconViewItem::setPixmap( pix, true, false );
    iconView()->arrangeItemsInGrid( false );
    iconView()->viewport()->update();
}
        
void ThumbnailItem::paintItem( QPainter* p, const QColorGroup& cg )
{
    if( !_loaded && !loading ) {
        loading = true;
        thumbnail_server->load( this );
    }
     
    QIconViewItem::paintItem( p, cg );
}

ThumbnailServer::ThumbnailServer( QObject* parent, const char* name )
    : QObject( parent, name ), running( false )
{
    connect( &load_timer, SIGNAL( timeout() ), this, SLOT( loadFront() ) );
}

void ThumbnailServer::load( ThumbnailItem* item )
{
#define PAUSE 75
    // Enqueue thumbnail item
    queue.enqueue( item );
    // If load timer not started, start timer
    if( !running ) { 
        running = true;
        load_timer.start( PAUSE, true );
    }
}
    
void ThumbnailServer::loadFront()
{
    // Remove all items from front of queue that are not visible
    bool item_visible = false;
    while( !queue.isEmpty() && !item_visible ) {
        ThumbnailItem *item = queue.head();
        // If item visible, set item visible
        // Otherwise, remove from queue
        QScrollView *view = item->iconView();
        QRect viewport( view->contentsX(), view->contentsY(), 
            view->visibleWidth(), view->visibleHeight() );
        if( item->intersects( viewport ) ) item_visible = true;
        else queue.dequeue()->reload();
    } 
    
    // If queue is not empty, dequeue and load item
    if( !queue.isEmpty() ) {
        ThumbnailItem *item = queue.dequeue();
	if ( !item->loaded() ) {
	    // If image currently in cache, load from cache
	    // Otherwise, load image into cache
	    QFileInfo file_info(item->key());
	    QString key = "_" + item->key() + file_info.lastModified().toString() + QString::number(item->size());
	    key = key.replace( QRegExp(" "), "_" );
	    QPixmap image;
	    if ( !QPixmapCache::find(key, image) ) {
		image.convertFromImage(Image::loadScaled(item->key(), item->size(), item->size()));
		QPixmapCache::insert(key, image);
	    }
	    item->setPixmap( image );
	}
    }
        
    // If queue is not empty, start timer
    if( !queue.isEmpty() ) {
	load_timer.start( PAUSE, true );
    } else {
        running = false;
        load_timer.stop();
    }
}

void IconView::showEvent( QShowEvent* e )
{
    QIconView::showEvent( e );
    // Ensure current selection is visible
    ensureItemVisible( currentItem() );
}

ThumbnailView::ThumbnailView( QWidget* parent, const char* name, WFlags f )
    : QVBox( parent, name, f ), thumbnail_size( DEFAULT_THUMBNAIL_SIZE )
{
    thumbnail_server = new ThumbnailServer( this );

    // Construct icon view
    icon_view = new IconView( this );
    icon_view->setHScrollBarMode( QScrollView::AlwaysOff );
    icon_view->setItemsMovable( false );
    icon_view->setWordWrapIconText( false );
    icon_view->setResizeMode( QIconView::Adjust );
    icon_view->setMaxItemWidth( thumbnail_size );
    
    connect( icon_view, SIGNAL( currentChanged( QIconViewItem* ) ),
        this, SLOT( currentChanged( QIconViewItem* ) ) );
    connect( icon_view, 
        SIGNAL( mouseButtonClicked( int, QIconViewItem*, const QPoint& ) ),
        this, SLOT( clicked( int, QIconViewItem*, const QPoint& ) ) );
        
#ifndef QTOPIA_PHONE
    // Enable stylus press events
    QPEApplication::setStylusOperation( icon_view->viewport(), 
        QPEApplication::RightOnHold );
    connect( icon_view, 
        SIGNAL( mouseButtonPressed( int, QIconViewItem*, const QPoint& ) ),
        this, SLOT( pressed( int, QIconViewItem*, const QPoint& ) ) );
#endif
        
    // Filter key events before passing to icon view
    icon_view->installEventFilter( this );

    // Give icon view control
    setFocusProxy( icon_view );
}

void ThumbnailView::setThumbnailSize( int maxSide )
{
    // If new size not equal to current, update size and resize thumbnails
    if( maxSide != thumbnail_size ) {
        thumbnail_size = maxSide;
        icon_view->setMaxItemWidth( thumbnail_size );
    
        // Reload thumbnails in view
        thumbnail_server->clear();
        icon_view->setMaxItemWidth( thumbnail_size );
        ThumbnailItem *item;
        for( item = static_cast< ThumbnailItem* >( icon_view->firstItem() );
            item; item = static_cast< ThumbnailItem* >( item->nextItem() ) )
            item->resize( thumbnail_size );
        icon_view->viewport()->update();
    }
}

void ThumbnailView::insert( const DocLnk& image )
{
    // Insert image into view sorted by user visible name
    // Find first item in view with name greater than image name 
    QIconViewItem* item = findGE( image );
    // If item exists, insert image before item
    // Otherwise, append image to view
    if( item ) {
        item = item->prevItem();
        if( item ) new ThumbnailItem( icon_view, item, image.name(), 
            image.file(), thumbnail_size, thumbnail_server );
        else {
            item = icon_view->firstItem();
            // Insert image after first item
            new ThumbnailItem( icon_view, item, image.name(), image.file(),
                thumbnail_size, thumbnail_server );
            // Swap first and second items in view
            icon_view->takeItem( item );
            icon_view->insertItem( item, icon_view->firstItem() );
        }
    } else new ThumbnailItem( icon_view, image.name(), image.file(), 
        thumbnail_size, thumbnail_server );
}

void ThumbnailView::update( const DocLnk& image )
{
    QIconViewItem* item = find( image );
    // Disable changed singnals
    disconnect( icon_view, SIGNAL( currentChanged( QIconViewItem* ) ),
        this, SLOT( currentChanged( QIconViewItem* ) ) );
        
    // Update image name and thumbnail
    item->setText( image.name() );
    static_cast< ThumbnailItem* > ( item )->reload();
        
    // Remove image from view
    icon_view->takeItem( item );
        
    // Insert image into view sorted by user visible name
    QIconViewItem* p = findGE( image );
    // If image exists with name >= image to be updated, insert before image
    // Otherwise, append image to view
    if( p ) {
        p = p->prevItem();
        if( p ) icon_view->insertItem( item, p );
        else {
            p = icon_view->firstItem();
            icon_view->insertItem( item, p );
            icon_view->takeItem( p );
            icon_view->insertItem( p, item );
        }
    } else icon_view->insertItem( item, p );
    
    item->setSelected( true );
    icon_view->ensureItemVisible( item );
    
    // Enable changed signals
    connect( icon_view, SIGNAL( currentChanged( QIconViewItem* ) ),
        this, SLOT( currentChanged( QIconViewItem* ) ) );
}

void ThumbnailView::remove( const DocLnk& image )
{
    // Disable changed singnals
    disconnect( icon_view, SIGNAL( currentChanged( QIconViewItem* ) ),
        this, SLOT( currentChanged( QIconViewItem* ) ) );
        
    // Remove image from view
    QIconViewItem *item = find( image );
    delete item;
    
    // Enable changed signals
    connect( icon_view, SIGNAL( currentChanged( QIconViewItem* ) ),
        this, SLOT( currentChanged( QIconViewItem* ) ) );
}

void ThumbnailView::clear()
{
    // Remove all pending thumbnails from thumbnail server
    thumbnail_server->clear();
    // Remove all images from view
    icon_view->clear();
}

void ThumbnailView::setHighlighedImage( const DocLnk& image )
{
    // Disable changed singnals
    disconnect( icon_view, SIGNAL( currentChanged( QIconViewItem* ) ),
        this, SLOT( currentChanged( QIconViewItem* ) ) );

    // Highlight image and make sure image is visable
    QIconViewItem *item = find( image );
    icon_view->setSelected( item, true );

    icon_view->ensureItemVisible( item );
    
    // Enable changed signals
    connect( icon_view, SIGNAL( currentChanged( QIconViewItem* ) ),
        this, SLOT( currentChanged( QIconViewItem* ) ) );
}

void ThumbnailView::currentChanged( QIconViewItem* item )
{
    if( item ) {
        // Make currently highlighted image current in collection
        DocLnk image;
        image.setFile( item->key() );
        emit currentChanged( image );
    }
}

void ThumbnailView::clicked( int button, QIconViewItem* item, 
    const QPoint& )
{
    // If left button clicked on thumbnail, emit selected
    if( item && button == Qt::LeftButton ) emit selected();
}

void ThumbnailView::pressed( int button, QIconViewItem* item,
    const QPoint& pos )
{
    // If right button pressed on thumbnail, emit held
    if( item && button == Qt::RightButton ) emit held( pos );
}

bool ThumbnailView::eventFilter( QObject*, QEvent* e )
{
    if( e->type() == QEvent::KeyPress ) {
        switch( ( static_cast< QKeyEvent* > ( e ) )->key() ) {
            // If select key, emit select signal
#ifdef QTOPIA_PHONE
            case Qt::Key_Select:
#endif
            case Qt::Key_Enter:
                emit selected();
                return true;
        }
    }
    
    return false;
}

QIconViewItem* ThumbnailView::find( const DocLnk& image )
{
    QIconViewItem *item;
    for( item = icon_view->firstItem(); item && item->key() != image.file(); 
        item = item->nextItem() );
    return item;
}

QIconViewItem* ThumbnailView::findGE( const DocLnk& image )
{
    QIconViewItem *item;
    for( item = icon_view->firstItem(); item &&
        item->text().lower().compare( image.name().lower() ) < 0; 
        item = item->nextItem() );
    return item;
}
