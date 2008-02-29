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

#ifndef IMAGESELECTOR_H
#define IMAGESELECTOR_H

#include <qtopia/applnk.h>
#ifdef QTOPIA_PHONE
#include <qtopia/contextmenu.h>
#endif

#include <qvbox.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qvaluelist.h>
#include <qsize.h>
#include <qpoint.h>
#include <qdialog.h>

class ImageSelectorPrivate;

class QTOPIA_EXPORT ImageSelector: public QVBox
{
    Q_OBJECT
public:
    ImageSelector( QWidget* parent = 0, const char* name = 0, WFlags f = 0 );
    
    ImageSelector( const QStringList& source, QWidget* parent = 0,
        const char* name = 0, WFlags f = 0 );

    // Obsolete
    ImageSelector( QWidget* parent, const char* name, const QString& srcdir, 
         int wflags = 0 );

    ~ImageSelector();

    enum ViewMode { Single, Thumbnail };
    void setViewMode( ViewMode mode );
    ViewMode viewMode() const;

    void setThumbnailSize( const int maxSide );
    int thumbnailSize() const;

    void setCategory( int id );
    int category() const;

    QString selectedFilename() const;
    DocLnk selectedDocument() const;
    QPixmap selectedImage( const int width, const int height ) const;

    QValueList<DocLnk> fileList() const;
    int fileCount() const;

    QSize sizeHint() const;

#ifdef QTOPIA_PHONE
    ContextMenu* menu() const;
#endif

    // Obsolete
    void showImageView( bool show, bool fullscreen = false );
    bool imageViewVisible() const;

signals:
    void selected( const DocLnk& image );
    void held( const DocLnk& image, const QPoint& pos );
    
    void fileAdded( const DocLnk& image );
    void fileUpdated( const DocLnk& image );
    void fileRemoved( const DocLnk& image );
    
    void categoryChanged();
    void reloaded();
    
private:
    ImageSelectorPrivate *d;
};

class ImageSelectorDialogPrivate;

class QTOPIA_EXPORT ImageSelectorDialog : public QDialog
{
    Q_OBJECT
public:
    ImageSelectorDialog( QWidget* parent = 0, const char* name = 0, 
        bool modal = TRUE, WFlags f = 0 );
        
    ImageSelectorDialog( const QStringList& source, QWidget* parent = 0,
        const char* name = 0, bool modal = TRUE, WFlags f = 0 );

    // Obsolete
    ImageSelectorDialog( QWidget* parent, const char* name,
        bool modal, const QString& srcdir, int wflags = 0 );
        
    ~ImageSelectorDialog ();

    QString selectedFilename() const { return selector->selectedFilename(); }
    DocLnk selectedDocument() const { return selector->selectedDocument(); }
    QPixmap selectedImage( const int width, const int height ) const 
        { return selector->selectedImage(width, height); }
    QValueList<DocLnk> fileList() const { return selector->fileList(); }

    void setThumbnailSize( const int pixels )
        { selector->setThumbnailSize(pixels); }
    int	thumbnailSize() const { return selector->thumbnailSize(); }

#ifdef QTOPIA_PHONE
    ContextMenu* menu() const { return selector->menu(); }
#endif

signals:
    void selected( const DocLnk & );

private slots:
    // Only Qtopia PDA
    void setViewSingle();
    
private:
    ImageSelectorDialogPrivate *d;
    ImageSelector *selector;
};

#endif // IMAGESELECTOR_H

