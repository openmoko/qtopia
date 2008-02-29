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

/*!
  \class ImageSelector
  \brief The ImageSelector widget allows the user to select from a collection of 
  images.
  
  ImageSelector allows the user to preview a collection of images and select
  individual images from the collection.
  
  You can control how ImageSelector displays the collection by using
  \c setViewMode(). ImageSelector supports two modes of display. One mode is
  \c ImageSelector::Single. In this mode images are resized to fit within the
  dimensions of the ImageSelector widget. Users navigate this mode by viewing
  each image one after another. The other mode is \c ImageSelector::Thumbnail.
  In this mode images are resized and displayed in groups. The maximum size of
  the resized images can be controlled with \c setThumbnailSize(). Users
  navigate this mode by moving a highlight through the collection to the desired
  image.
  
  ImageSelector emits a \c selected() signal when the user has made a selection. 
  A DocLnk to the selected image is passed with the signal. In addition you can
  retrieve the selected image with \c selectedFilename() or \c selectedImage().
  The \c selectedImage() function returns a QPixmap containing a scaled copy of
  the selected image.
  
  First availability: Qtopia 2.0
  
  \ingroup qtopiaemb
  \sa ImageSelectorDialog
*/

// BCI: To overcome limitations in ImageSelectorDialog
#define private public
#include "imageselector.h"
#undef private

#include "imageselector_p.h"

#include <qtopia/qpeapplication.h>

#include <qlayout.h>

/*!
  Construct an ImageSelector called \a name with parent \a parent and widget
  flags \a f.
  
  Images are taken from the Documents directory.
  
  First availability: Qtopia 2.1
*/
ImageSelector::ImageSelector( QWidget* parent, const char* name, WFlags f )
    : QVBox( parent, name, f )
{
    d = new ImageSelectorPrivate( QStringList(), this );
    
    connect( d, SIGNAL( selected( const DocLnk& ) ),
        this, SIGNAL( selected( const DocLnk& ) ) );
    connect( d, SIGNAL( held( const DocLnk&, const QPoint& ) ),
        this, SIGNAL( held( const DocLnk&, const QPoint& ) ) );
        
    connect( d, SIGNAL( fileAdded( const DocLnk& ) ),
        this, SIGNAL( fileAdded( const DocLnk& ) ) );
    connect( d, SIGNAL( fileUpdated( const DocLnk& ) ),
        this, SIGNAL( fileUpdated( const DocLnk& ) ) );
    connect( d, SIGNAL( fileRemoved( const DocLnk& ) ),
        this, SIGNAL( fileRemoved( const DocLnk& ) ) );
        
    connect( d, SIGNAL( categoryChanged() ),
        this, SIGNAL( categoryChanged() ) );
    connect( d, SIGNAL( reloaded() ), this, SIGNAL( reloaded() ) );
        
    setFocusProxy( d );
}

/*!
  Construct an ImageSelector called \a name with parent \a parent and widget
  flags \a f.
  
  Images are taken from the directories given in \a source. If \a source is 
  empty images are taken from the Documents directory.
  
  First availability: Qtopia 2.1
*/
ImageSelector::ImageSelector( const QStringList& source, QWidget* parent,
    const char* name, WFlags f )
    : QVBox( parent, name, f )
{
    d = new ImageSelectorPrivate( source, this );
    
    connect( d, SIGNAL( selected( const DocLnk& ) ),
        this, SIGNAL( selected( const DocLnk& ) ) );
    connect( d, SIGNAL( held( const DocLnk&, const QPoint& ) ),
        this, SIGNAL( held( const DocLnk&, const QPoint& ) ) );
        
    connect( d, SIGNAL( fileAdded( const DocLnk& ) ),
        this, SIGNAL( fileAdded( const DocLnk& ) ) );
    connect( d, SIGNAL( fileUpdated( const DocLnk& ) ),
        this, SIGNAL( fileUpdated( const DocLnk& ) ) );
    connect( d, SIGNAL( fileRemoved( const DocLnk& ) ),
        this, SIGNAL( fileRemoved( const DocLnk& ) ) );
        
    connect( d, SIGNAL( categoryChanged() ),
        this, SIGNAL( categoryChanged() ) );
    connect( d, SIGNAL( reloaded() ), this, SIGNAL( reloaded() ) );
        
    setFocusProxy( d );
}

/*! \obsolete */
ImageSelector::ImageSelector( QWidget* parent, const char* name,
    const QString& srcdir, int wflags )
    : QVBox( parent, name, wflags )
{ 
    if( !srcdir.isNull() ) d = new ImageSelectorPrivate( srcdir, this );
    else d = new ImageSelectorPrivate( QStringList(), this );
    
    connect( d, SIGNAL( selected( const DocLnk& ) ),
        this, SIGNAL( selected( const DocLnk& ) ) );
    connect( d, SIGNAL( held( const DocLnk&, const QPoint& ) ),
        this, SIGNAL( held( const DocLnk&, const QPoint& ) ) );
        
    connect( d, SIGNAL( fileAdded( const DocLnk& ) ),
        this, SIGNAL( fileAdded( const DocLnk& ) ) );
    connect( d, SIGNAL( fileUpdated( const DocLnk& ) ),
        this, SIGNAL( fileUpdated( const DocLnk& ) ) );
    connect( d, SIGNAL( fileRemoved( const DocLnk& ) ),
        this, SIGNAL( fileRemoved( const DocLnk& ) ) );
        
    connect( d, SIGNAL( categoryChanged() ),
        this, SIGNAL( categoryChanged() ) );
    connect( d, SIGNAL( reloaded() ), this, SIGNAL( reloaded() ) );
        
    setFocusProxy( d );
}

/*!
  Destroy the widget.
*/
ImageSelector::~ImageSelector()
{
    delete d;
}

/*!
  \enum ImageSelector::ViewMode
  
  This enum describes modes of display
  \value Single The images are display one at a time.
  \value Thumbnail Groups of images are displayed as thumbnails in a scrolling
  viewport.
*/

/*!
  Set the view mode to \a mode.
  
  See \l ImageSelector::ViewMode for a listing of supported modes.
  
  First availability: Qtopia 2.1
  
  \sa viewMode()
*/
void ImageSelector::setViewMode( ViewMode mode )
{
    d->setViewMode( mode );
}

/*!
  Return the current view mode.
  
  See \l ImageSelector::ViewMode for a listing of supported modes.
  
  First availability: Qtopia 2.1
  
  \sa setViewMode()
*/
ImageSelector::ViewMode ImageSelector::viewMode() const
{
    return d->viewMode();
}

/*!
  Set the maximum side (in pixel units) of a thumbnail to \a maxSide. The
  largest thumbnail will be no greater than \a maxSide x \a maxSide pixels.
  
  \a maxSide must be greater than 0.
  
  \sa thumbnailSize()
*/
void ImageSelector::setThumbnailSize( const int maxSide )
{
    d->setThumbnailSize( maxSide );
}

/*!
  Return the current maximum side (in pixel units) of a thumbnail.
  
  \sa setThumbnailSize()
*/
int ImageSelector::thumbnailSize() const
{
    return d->thumbnailSize();
}

/*!
  Set the current category to \a id. Only images in this category will be 
  displayed.
  
  See \l Categories for more information on category IDs.
  
  \sa category()
*/
void ImageSelector::setCategory( int id )
{
    d->setCategory( id );
}

/*!
  Return the current category.
  
  See \l Categories for more information on category IDs.
  
  First availability: Qtopia 2.1
  
  \sa setCategory()
*/
int ImageSelector::category() const
{
    return d->category();
}

/*!
  Return the file path of the currently selected image, or QString::null if
  there is no current selection.
*/
QString ImageSelector::selectedFilename() const
{
    return d->selectedFilename();
}

/*!
  Return the \l DocLnk of the currently selected image, or invalid \l DocLnk if
  there is no current selection.
*/
DocLnk ImageSelector::selectedDocument() const
{
    return d->selectedDocument();
}

/*!
  Return a QPixmap containing a scaled copy of the currently selected image, or
  null QPixmap if there is no current selection.
  
  The image will be scaled to fit within \a width and \a height while
  maintaining the original width to height ratio.
*/
QPixmap ImageSelector::selectedImage( const int width, const int height ) const
{
    return d->selectedImage( width, height );
}

/*!
  Return a list of images in the current category.
*/
QValueList<DocLnk> ImageSelector::fileList() const
{
    return d->fileList();
}

/*!
  Return the number of images in the current category.
  
  First availability: Qtopia 2.1
*/
int ImageSelector::fileCount() const
{
    return d->fileCount();
}

#ifdef QTOPIA_PHONE
/*!
  Return the context menu.
*/
ContextMenu* ImageSelector::menu() const
{
    return d->menu();
}
#endif

/*! \internal */
QSize ImageSelector::sizeHint() const
{
    return QSize(width(), qApp->desktop()->height());
}

/*! \obsolete */
void ImageSelector::showImageView( bool show, bool )
{
    // If show, set view mode to Single
    if( show ) d->setViewMode( Single );
    // Otherwise, set view mode to Thumbnail
    else d->setViewMode( Thumbnail );
    // Full screen no longer supported
}

/*! \obsolete */
bool ImageSelector::imageViewVisible() const
{
    // If view mode is Single, return true
    return d->viewMode() == Single;
}

/*! \fn void ImageSelector::selected( const DocLnk& image );
  
  This signal is emitted when the user has selected an image from the 
  collection. A DocLnk to the selected image is given in \a image.
*/

/*! \fn void ImageSelector::held( const DocLnk& image, const QPoint& pos );

  This signal is emitted when the user has held down on an image. A DocLnk to
  the image is given in \a image. The global position of the hold is given
  in \a pos.
  
  First availability: Qtopia 2.1
*/

/*! \fn void ImageSelector::fileAdded( const DocLnk& image );

  This signal is emitted when an image is added to the current category. A
  DocLnk to the image is given in \a image.
  
  First availability: Qtopia 2.1
*/

/*! \fn void ImageSelector::fileUpdated( const DocLnk& image );

  This signal is emitted when an image is updated in the current category. A
  DocLnk to the image is given in \a image.
  
  First availability: Qtopia 2.1
*/

/*! \fn void ImageSelector::fileRemoved( const DocLnk& image );

  This signal is emitted when an image is removed from the current category.
  A DocLnk to the image is given in \a image.
  
  First availability: Qtopia 2.1
*/

/*! \fn void ImageSelector::categoryChanged();

  This signal is emitted when the category has changed.
  
  First availability: Qtopia 2.1
*/

/*! \fn void ImageSelector::reloaded();

  This signal is emitted when the collection has reloaded due to a disk mount
  or unmount.
  
  First availability: Qtopia 2.1
  
  \sa StorageInfo::disksChanged()
*/

/*!
  \class ImageSelectorDialog imageselector.h
  \brief The ImageSelectorDialog widget presents the ImageSelector widget as a 
  dialog. It allows the user to select from a collection of images.
  
  This code snippet allows the user to select a wallpaper from the Qtopia directories:
  
  \code
    QStringList locations;
    QStringList qtopiaPaths = Global::qtopiaPaths();
    for( QStringList::Iterator it = qtopiaPaths.begin(); it != qtopiaPaths.end(); ++it )
        locations.append( *it + "pics/wallpaper" );
      
    ImageSelectorDialog dialog( locations, this );
    if( QPEApplication::execDialog( &dialog ) ) {
        // Accept
    } else {
        // Reject
    }\endcode
    
  You can retrieve the selected image with \c selectedDocument(),
  \c selectedFilename() and \c selectedImage(). The \c selectedImage() function
  returns a QPixmap containing a scaled copy of the selected image.

  First availability: Qtopia 2.0

  \ingroup qtopiaemb
  \sa ImageSelector
*/

/*!
  Construct an ImageSelectorDialog called \a name with parent \a parent and 
  widget flags \a f.
  
  The dialog is modal if \a modal is TRUE (default), otherwise the dialog is 
  modeless.
  
  Images are taken from the Documents directory.
  
  First availability: Qtopia 2.1
*/
ImageSelectorDialog::ImageSelectorDialog( QWidget* parent, const char* name, 
    bool modal, WFlags f )
    : QDialog( parent, name, modal, f )
{
    setCaption( tr( "Select Image" ) );
    QVBoxLayout *vb = new QVBoxLayout( this );
    
    selector = new ImageSelector( this );
    
#ifdef QTOPIA_PHONE
    // BCI: To overcome limitations in ImageSelectorDialog
    selector->d->enableDialogMode();
    connect( selector, SIGNAL( selected( const DocLnk& ) ), 
        this, SLOT( accept() ) );
#else
    connect( selector, SIGNAL( selected( const DocLnk& ) ),
        this, SLOT( setViewSingle() ) );
#endif

    vb->addWidget( selector );
    QPEApplication::setMenuLike( this, true );
}

/*!
  Construct an ImageSelectorDialog called \a name with parent \a parent and 
  widget flags \a f.
  
  The dialog is modal if \a modal is TRUE (default), otherwise the dialog is 
  modeless.
  
  Images are taken from the directories given in \a source. If \a source is 
  empty images are taken from the Documents directory.
  
  First availability: Qtopia 2.1
*/
ImageSelectorDialog::ImageSelectorDialog( const QStringList& source, 
    QWidget* parent, const char* name, bool modal, WFlags f )
    : QDialog( parent, name, modal, f )
{
    setCaption( tr( "Select Image" ) );
    QVBoxLayout *vb = new QVBoxLayout( this );
    
    selector = new ImageSelector( source, this );
    
#ifdef QTOPIA_PHONE
    // BCI: To overcome limitations in ImageSelectorDialog
    selector->d->enableDialogMode();
    connect( selector, SIGNAL( selected( const DocLnk& ) ), 
        this, SLOT( accept() ) );
#else
    connect( selector, SIGNAL( selected( const DocLnk& ) ),
        this, SLOT( setViewSingle() ) );
#endif

    vb->addWidget( selector );
    QPEApplication::setMenuLike( this, true );
}

/*! \obsolete */
ImageSelectorDialog::ImageSelectorDialog( QWidget* parent, const char* name,
    bool modal, const QString& srcdir, int wflags )
    : QDialog( parent, name, modal, wflags )
{
    setCaption( tr( "Select Image" ) );
    QVBoxLayout *vb = new QVBoxLayout( this );
    
    if( !srcdir.isNull() ) selector = new ImageSelector( srcdir, this );
    else selector = new ImageSelector( this );
    
#ifdef QTOPIA_PHONE
    // BCI: To overcome limitations in ImageSelectorDialog
    selector->d->enableDialogMode();
    connect( selector, SIGNAL( selected( const DocLnk& ) ), 
        this, SLOT( accept() ) );
#else
    connect( selector, SIGNAL( selected( const DocLnk& ) ),
        this, SLOT( setViewSingle() ) );
#endif

    vb->addWidget( selector );
    QPEApplication::setMenuLike( this, true );
}

/*!
  Destroy the widget.
*/
ImageSelectorDialog::~ImageSelectorDialog()
{ }

void ImageSelectorDialog::setViewSingle()
{
    selector->setViewMode( ImageSelector::Single );
}

/*! \fn QString ImageSelectorDialog::selectedFilename() const;
  
  Return the file path of the currently selected image, or QString::null if
  there is no current selection.
*/

/*! \fn DocLnk ImageSelectorDialog::selectedDocument() const;

  Return the \l DocLnk of the currently selected image, or invalid \l DocLnk if
  there is no current selection.
*/

/*! \fn QPixmap ImageSelectorDialog::selectedImage( const int width, 
  const int height ) const;
  
  Return a QPixmap containing a scaled copy of the currently selected image, or
  null QPixmap if there is no current selection.
  
  The image will be scaled to fit within \a width and \a height while
  maintaining the original width to height ratio.
*/

/*! \fn QValueList<DocLnk> ImageSelectorDialog::fileList() const;

  Return a list of images in the current category.
*/

/*! \fn void ImageSelectorDialog::setThumbnailSize( const int maxSide )

  Set the maximum side (in pixel units) of a thumbnail to \a maxSide. The
  largest thumbnail will be no greater than \a maxSide x \a maxSide pixels.
  
  \a maxSide must be greater than 0.
  
  \sa thumbnailSize()
*/

/*! \fn int ImageSelectorDialog::thumbnailSize() const;

  Return the current maximum side (in pixel units) of a thumbnail.
  
  \sa setThumbnailSize()
*/

/*! \fn ContextMenu* ImageSelectorDialog::menu() const

  Return the context menu.
*/

/*! \obsolete \fn void ImageSelectorDialog::selected( const DocLnk& image ); */
