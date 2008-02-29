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

#ifndef IMAGESELECTOR_P_H
#define IMAGESELECTOR_P_H

#include "imageselector.h"
#include "imagecollection_p.h"
#include "thumbnailview_p.h"

#include "categorydialog.h"
#include <qtopia/categoryselect.h>

#include <qtopia/applnk.h>
#ifdef QTOPIA_PHONE
#include <qtopia/contextmenu.h>
#endif

#include <qwidget.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qwidgetstack.h>
#include <qlabel.h>
#include <qbuttongroup.h>

class SingleView;

class ImageSelectorPrivate : public QWidget
{
    Q_OBJECT
public:
    ImageSelectorPrivate( const QStringList& source, QWidget* parent = 0, 
        const char* name = 0, WFlags f = 0 );

    // Images may be displayed as a collection of thumbnails
    // or singularly, default thumbnail mode	
    void setViewMode( ImageSelector::ViewMode mode );
    ImageSelector::ViewMode viewMode() const { return current_view; }

    // Set the maximum side of a square thumbnail
    void setThumbnailSize( int maxSide ) 
        { thumbnail_view->setThumbnailSize( maxSide ); }
    int thumbnailSize() const 
        { return thumbnail_view->thumbnailSize(); }

    // Set image category
    void setCategory( int id );
    int category() const { return category_id; }

    // Return currently selected image
    QString selectedFilename() const;
    DocLnk selectedDocument() const;
    // Selected image scaled to width hight with aspect ratio preserved
    QPixmap selectedImage( int width, int height ) const;

    // Return list of currently visible images
    QValueList<DocLnk> fileList() const;
    // Return number of currently visible images
    int fileCount() const;

#ifdef QTOPIA_PHONE
    ContextMenu* menu() const { return context_menu; }
#endif

#ifdef QTOPIA_PHONE
    // BCI: To overcome limitations in ImageSelectorDialog
    // Enable dialog style navigation
    void enableDialogMode();
#endif

signals:
    void selected( const DocLnk& image );
    void held( const DocLnk&, const QPoint& );
    
    void fileAdded( const DocLnk& image );
    void fileUpdated( const DocLnk& image );
    void fileRemoved( const DocLnk& image );
    
    void categoryChanged();
    void reloaded();

private slots:
    // Select current image
    void select();
    
    // Hold current image
    void hold( const QPoint& );

    // Set view mode to single
    void setViewSingle();

    // Set view mode to thumbnail
    void setViewThumbnail();
    
    // Only Qtopia Phone
    // Launch view category dialog
    void launchCategoryDialog();

    // Update collection and raise current view
    void reload();
    
    // Change of category
    void changeCategory( int );

    // Give focus to widget
    void setFocus( QWidget* widget );
    
    // Make appropriate changes to visible collection
    void linkChanged( const QString& ); 
    
    // Update collection with visible images
    void updateVisibleCollection();

protected:
    // Give focus to widget at top of stack
    void focusInEvent( QFocusEvent* );

private:
    // Raise current view to top of stack
    void raiseCurrentView();
    
    // Return true if image is in current category
    bool inCurrentCategory( const DocLnk& image ) const;

    QStringList source_directories;
    int category_id;
    ImageCollection visible_collection;
    
    ImageSelector::ViewMode current_view;
#ifndef QTOPIA_PHONE
    int single_toggle_id, thumbnail_toggle_id;
    QButtonGroup *view_toggle_group;
#endif

#ifdef QTOPIA_PHONE	
    ContextMenu *context_menu;
#endif
    // Only Qtopia Phone
    CategorySelectDialog *category_dialog;
    
    // Only Qtopia PDA
    CategorySelect *category_selector;
    
    QWidgetStack *widget_stack;

    SingleView *single_view;
    ThumbnailView *thumbnail_view;
    QLabel *message_view;
};

#endif // IMAGESELECTOR_P_H
