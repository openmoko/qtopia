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

#ifndef THUMBNAILVIEW_P_H
#define THUMBNAILVIEW_P_H

#include <qtopia/applnk.h>

#include <qvbox.h>
#include <qvaluelist.h>
#include <qiconview.h>
#include <qqueue.h>
#include <qtimer.h>
#include <qpoint.h>

class ThumbnailServer;

class ThumbnailItem : public QIconViewItem
{
public:
    ThumbnailItem( QIconView* parent, const QString& description, 
        const QString& file, int s, ThumbnailServer* ts );
        
    ThumbnailItem( QIconView* parent, QIconViewItem* after,
        const QString& description, const QString& file, int s, 
        ThumbnailServer* ts );

    void init( const QString& file, int s, ThumbnailServer* ts );
 
    // Set thumbnail
    void setPixmap( const QPixmap& pix );
    
    // Return size of thumbnail
    int size() const { return _size; }

    bool loaded() const { return _loaded; }
    
    // Resize and mark for reload 
    void resize( int s ) 
    {
        _size = s;
        _loaded = loading = false;
    }
    
    // Mark item for realod
    void reload() { _loaded = loading = false; }
    
protected:
    void ThumbnailItem::paintItem( QPainter*, const QColorGroup& );

private:
    int _size;
    bool _loaded, loading;
    ThumbnailServer* thumbnail_server;
};

class ThumbnailServer : public QObject
{
    Q_OBJECT
public:
    ThumbnailServer( QObject* parent = 0, const char* name = 0 );
    
    // Add thumbnail item to back of queue
    void load( ThumbnailItem* item );
    
    // Remove all thumbnail items from queue
    void clear() { queue.clear(); }
    
private slots:
    // Load thumbnail item at front of queue
    void loadFront();
   
private:
    bool running;
    QTimer load_timer;
    QQueue< ThumbnailItem > queue;
};

class IconView : public QIconView 
{
public:
    IconView( QWidget* parent )
        : QIconView( parent )
    { }

    // Ensure current selection is visible when shown
    void showEvent( QShowEvent* );
};

class ThumbnailView : public QVBox
{
    Q_OBJECT
public:
    ThumbnailView( QWidget* parent, const char* name = 0, 
        WFlags f = 0 );

    // Set the maximum side of a square thumbnail
    void setThumbnailSize( int maxSide );

    // Return current thumbnail size
    int thumbnailSize() const { return thumbnail_size; }

signals:
    // Select key pressed
    void selected();
    
    // Only Qtopia PDA
    // Stylus held on thumbnail
    void held( const QPoint& );
    
    // Currently highlighed image changed
    void currentChanged( const DocLnk& );
    
public slots:
    // Insert image into view
    void insert( const DocLnk& );
    
    // Update image in view
    void update( const DocLnk& );
    
    // Remove image from view
    void remove( const DocLnk& );
    
    // Remove all images from view
    void clear();

    // Set currently highlighted image
    void setHighlighedImage( const DocLnk& image );
    
private slots:
    // Transform current changed internal into current changed external
    void currentChanged( QIconViewItem* );
    
    // Emit selected signal
    void clicked( int, QIconViewItem*, const QPoint& );
    
    // Only Qtopia PDA
    // Emit held signal
    void pressed( int, QIconViewItem*, const QPoint& );

protected:
    // Convert function keys into signals
    bool eventFilter( QObject*, QEvent* );

private:
    // Find image in view using filename as key
    QIconViewItem* find( const DocLnk& );
    
    // Find first image in view with a user visible name >= image
    QIconViewItem* findGE( const DocLnk& );
    
    int thumbnail_size;
    ThumbnailServer *thumbnail_server;
    QIconView* icon_view;
};

#endif	// THUMBNAILVIEW_P_H
