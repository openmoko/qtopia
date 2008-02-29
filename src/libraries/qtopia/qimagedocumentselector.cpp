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

/*!
  \class QImageDocumentSelector
  \brief The QImageDocumentSelector widget allows the selection of images from
  a list of image documents available on the device.

  The QImageDocumentSelector widget builds a list of documents by
  locating all images in the device document directories.
  Alternatively, the list can be built with images which match a custom content filter.

  The following functionality is provided:
  \list
  \o setFilter() : filter the list of image documents
  \o filter() :  retrieve the current content filter
  \o documents() : retrieve the list of image documents
  \o count() : retrieve the number of image documents in the list
  \o selectedDocument() : retrieve a selected document
  \endlist

  In addition \c QImageDocumentSelector provides control of the presentation of image documents for selection.
  If a single image selection is required, consider using the \l QImageDocumentSelectorDialog convience class.

  Documents may be presented in two modes:
  \list
  \o \c QImageDocumentSelector::Single - presents a thumbnail of the current image
  contained within the dimensions of the \c QImageDocumentSelector widget.
  \o \c QImageDocumentSelector::Thumbnail - presents thumbnails of the images
  in an icon-type scroll view allowing multiple image thumbnails to be
  viewed concurrently.

  The size of thumbnails is manipulated using:
    \list
    \o setThumbnailSize() - sets the size of the thumbnail
    \o thumbnailSize() - retrieves the size of the current thumbnail select key.
    \endlist
  \endlist

  Images are displayed one at a time and are selected via the stylus or select key and
  arrow keys are used to navigate through the list of image documents.

  When an image is selected the following occurs:
  \list
  \o QImageDocumentSelector emits a documentSelected() signal
  \o a QContent for the selected document is passed with the signal.
  \endlist

  Whenever the list of documents is changed as a result of a change of a
  category filter or a file system change QImageDocumentSelector will emit a
  documentsChanged() signal.

  For example, the following code allows the user to select from all image documents
  available on the device using the thumbnail view mode with a custom thumbnail
  size:

  \code
    QImageDocumentSelector *selector = new QImageDocumentSelector( this );
    selector->setThumbnailSize( QSize( 100, 100 ) );

    connect( selector, SIGNAL(documentSelected(const QContent&)),
        this, SLOT(openImage(const QContent&)) );
  \endcode

  QImageDocumentSelector is often the first widget seen in a \l {Qtopia - Main Document Widget}{document-oriented application }. When used
  with \l QStackedWidget, an application
  allows selection of a document using the selector before revealing
  the document viewer or editor.

  \ingroup qtopiaemb
  \sa QImageDocumentSelectorDialog, QDocumentSelector
*/

#include "qimagedocumentselector_p.h"

#include <qtopiaapplication.h>
#include <qtopianamespace.h>

#ifdef QTOPIA_KEYPAD_NAVIGATION
#include <qsoftmenubar.h>
#endif

#include <qdrmcontent.h>

#include <QLayout>
#include <QDesktopWidget>

/*!
  Construct an QImageDocumentSelector with parent \a parent.
*/
QImageDocumentSelector::QImageDocumentSelector( QWidget* parent )
    : QWidget( parent )
{
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    layout->setSpacing( 0 );

    d = new QImageDocumentSelectorPrivate( this );
    layout->addWidget( d );

    connect( d, SIGNAL(documentSelected(const QContent&)),
             this, SIGNAL(documentSelected(const QContent&)) );
    connect( d, SIGNAL(documentHeld(const QContent&,const QPoint&)),
        this, SIGNAL(documentHeld(const QContent&,const QPoint&)) );

    connect( d, SIGNAL(documentsChanged()), this, SIGNAL(documentsChanged()) );

    setFocusProxy( d );

#ifdef QTOPIA_KEYPAD_NAVIGATION
    QSoftMenuBar::addMenuTo( this, QSoftMenuBar::menuFor( d ) );
#endif
}

/*!
  Destroys the widget.
*/
QImageDocumentSelector::~QImageDocumentSelector()
{
    delete d;
}

/*!
  \enum QImageDocumentSelector::ViewMode

  This enum describes presentation modes.
  \value Single Images are displayed one at a time.
  \value Thumbnail Multiple images are displayed in an icon type scroll view.
*/

/*!
  Sets the display mode to \a mode.

  The default mode is QImageDocumentSelector::Thumbnail.

  See \l QImageDocumentSelector::ViewMode for a listing of supported modes.

  \sa viewMode()
*/
void QImageDocumentSelector::setViewMode( ViewMode mode )
{
    d->setViewMode( mode );
}

/*!
  Returns the current display mode.

  See \l QImageDocumentSelector::ViewMode for a listing of supported modes.

  \sa setViewMode()
*/
QImageDocumentSelector::ViewMode QImageDocumentSelector::viewMode() const
{
    return d->viewMode();
}

/*!
  Sets the maximum size of a thumbnail to \a size.

  The default size is QSize( 65, 65 ).

  \sa thumbnailSize()
*/
void QImageDocumentSelector::setThumbnailSize( const QSize& size )
{
    d->setThumbnailSize( size );
}

/*!
  Returns the current maximum size of a thumbnail.

  \sa setThumbnailSize()
*/
QSize QImageDocumentSelector::thumbnailSize() const
{
    return d->thumbnailSize();
}

/*!
  Returns a \l QContent for the currently selected image, or an invalid \l QContent
  if there is no current selection.
*/
QContent QImageDocumentSelector::currentDocument() const
{
    return d->selectedDocument();
}

/*!
  Returns the list of documents available for selection.
*/
const QContentSet &QImageDocumentSelector::documents() const
{
    return d->documents();
}

/*!
  Returns the current documents filter.
*/
QContentFilter QImageDocumentSelector::filter() const
{
    return d->filter();
}

/*!
    Sets the \a filter which determines what documents from the backing store are displayed.
 */
void QImageDocumentSelector::setFilter( const QContentFilter &filter )
{
    d->setFilter( filter );
}

/*!
    Returns the attributes the content is ordered by.
 */
QDocumentSelector::SortMode QImageDocumentSelector::sortMode() const
{
    return d->sortMode();
}

/*!
    Sets the attributes content is ordered by to \a sortMode.
 */
void QImageDocumentSelector::setSortMode( QDocumentSelector::SortMode sortMode )
{
    d->setSortMode( sortMode );
}

/*! \internal */
QSize QImageDocumentSelector::sizeHint() const
{
    QDesktopWidget *desktop = QApplication::desktop();
    return QSize(width(),
                desktop->availableGeometry(desktop->screenNumber(this)).height());
}

/*!
    Gives the keyboard input focus to this widget (or its focus
    proxy) if this widget or one of its parents is the
    \l{isActiveWindow()}{active window}.
 */
void QImageDocumentSelector::setFocus()
{
    d->setFocus();
}


/*!
    Sets the \a categories selected by default in the document selector's category filter dialog.

    If a default category does not appear in the category filter dialog then the content of the document selector will
    not be filtered on that category.  If a default category is added to the list after the category selector has been
    initialized it will not be automatically selected.

    \sa defaultCategories()
 */
void QImageDocumentSelector::setDefaultCategories( const QStringList &categories )
{
    d->setDefaultCategories( categories );
}

/*!
    Returns the categories selected by default in the document selector's category filter dialog.

    \sa setDefaultCategories()
 */
QStringList QImageDocumentSelector::defaultCategories() const
{
    return d->defaultCategories();
}

/*!
    Sets the intended usage of the selected document.  If the document does not have the \a permission the document selector
    will attempt to activate before the selection succeeds.  If the document cannot be activated with that permission, it
    will not be selectable.

    If the permission is QDrmRights::InvalidPermission the default permission for the content is used.
 */
void QImageDocumentSelector::setSelectPermission( QDrmRights::Permission permission )
{
    d->setSelectPermission( permission );
}

/*!
    Returns the intended usage of the selected document.
 */
QDrmRights::Permission QImageDocumentSelector::selectPermission() const
{
    return d->selectPermission();
}

/*!
    Sets the \a permissions a document must have in order to be selectable in the document selector.

    Unlike the select permission if a document is missing a mandatory permission it is simply unselectable and can not
    be activated.

    \sa setSelectPermission()
 */
void QImageDocumentSelector::setMandatoryPermissions( QDrmRights::Permissions permissions )
{
    d->setMandatoryPermissions( permissions );
}

/*!
    Returns the permissions a document must have in order to be selectable in the document selector.
 */
QDrmRights::Permissions QImageDocumentSelector::mandatoryPermissions() const
{
    return d->mandatoryPermissions();
}

/*! \fn void QImageDocumentSelector::documentSelected( const QContent& image );

  This signal is emitted when the user selects an image. A \l QContent for the
  image document is given in \a image.
*/

/*! \fn void QImageDocumentSelector::documentHeld( const QContent& image, const QPoint& pos );

  \internal

  Not currently supported.

  This signal is emitted when the user has held down on an image. A QContent to
  the image is given in \a image. The global position of the hold is given
  in \a pos.
*/

/*! \fn void QImageDocumentSelector::documentsChanged();

   This signal is emitted when the list of documents is changes as a result of a
   change to the category filter or a file system change.
*/

/*!
  \class QImageDocumentSelectorDialog
  \brief The QImageDocumentSelectorDialog class allows the user to select an image
  from a list of image documents available on the device.

  QImageDocumentSelectorDialog is a convenience class that presents the \l
  QImageDocumentSelector widget in a dialog.

  The following code uses QImageDocumentSelectorDialog to allow the user
  to select a wallpaper image from the Qtopia directories:

  \code
    QContentFilter locations;
    QStringList qtopiaPaths = Qtopia::installPaths();
    foreach( QString path, qtopiaPaths ) {
        locations |= QContentFilter( QContentFilter::Location, path + "pics/wallpaper" );
    }

    QImageDocumentSelectorDialog dialog( this );

    dialog.setFilter( locations );

    if( QtopiaApplication::execDialog( &dialog ) ) {
        // Accept
        QContent wallpaper = dialog.selectedDocument();
    } else {
        // Reject
    }
  \endcode

  \ingroup qtopiaemb
  \sa QImageDocumentSelector
*/

/*!
  Construct an QImageDocumentSelectorDialog with parent \a parent.

  The dialog lists all documents with the \i image mime type.
  The dialog is modal by default.
*/
QImageDocumentSelectorDialog::QImageDocumentSelectorDialog( QWidget* parent )
    : QDialog( parent )
{
    selector = new QImageDocumentSelector( this );

    init();
}

void QImageDocumentSelectorDialog::init()
{
    setWindowTitle( tr( "Select Image" ) );
    QVBoxLayout *vb = new QVBoxLayout( this );

#ifdef QTOPIA_KEYPAD_NAVIGATION
    connect( selector, SIGNAL(documentSelected(const QContent&)), this, SLOT(accept()) );
    connect( selector, SIGNAL(documentsChanged()), this, SLOT(setContextBar()) );
#else
    connect( selector, SIGNAL(documentSelected(const QContent&)),
             this, SLOT(setViewSingle()) );
#endif

    vb->addWidget( selector );

#ifdef QTOPIA_KEYPAD_NAVIGATION
    // Set thumbnail view
    selector->setViewMode( QImageDocumentSelector::Thumbnail );
    QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Cancel );
    setContextBar();
#endif

    setModal( true );
#ifdef QTOPIA_KEYPAD_NAVIGATION
    QtopiaApplication::setMenuLike( this, true );
    QSoftMenuBar::addMenuTo( this, QSoftMenuBar::menuFor( selector ) );
#endif
}

/*!
  Destroys the widget.
*/
QImageDocumentSelectorDialog::~QImageDocumentSelectorDialog()
{
}

/*!
  Sets the maximum size of a thumbnail to \a size.

  The default size is QSize( 65, 65 ).

  \sa thumbnailSize()
*/
void QImageDocumentSelectorDialog::setThumbnailSize( const QSize& size )
{
    selector->setThumbnailSize( size );
}

/*!
  Returns the current maximum size of a thumbnail.

  \sa setThumbnailSize()
*/
QSize QImageDocumentSelectorDialog::thumbnailSize() const
{
    return selector->thumbnailSize();
}

/*!
  Returns a \l QContent for the currently selected image, or an invalid \l QContent
  if there is no current selection.
*/
QContent QImageDocumentSelectorDialog::selectedDocument() const
{
    return selector->currentDocument();
}

/*!
  Returns the list of documents available for selection.
*/
const QContentSet &QImageDocumentSelectorDialog::documents() const
{
    return selector->documents();
}

/*!
  Returns the current documents filter.
 */
QContentFilter QImageDocumentSelectorDialog::filter() const
{
    return selector->filter();
}

/*!
    Sets the \a filter which determines what documents from the backing store are displayed.
 */
void QImageDocumentSelectorDialog::setFilter( const QContentFilter &filter )
{
    selector->setFilter( filter );
}

/*!
    Returns the attributes the content is ordered by.
*/
QDocumentSelector::SortMode QImageDocumentSelectorDialog::sortMode() const
{
    return selector->sortMode();
}

/*!
    Sets the attributes content is ordered by to \a sortMode.
*/
void QImageDocumentSelectorDialog::setSortMode( QDocumentSelector::SortMode sortMode )
{
    selector->setSortMode( sortMode );
}

/*! \internal */
void QImageDocumentSelectorDialog::accept()
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    // If thumbnail view, switch to single view
    // Otherwise, accept
    if( selector->viewMode() == QImageDocumentSelector::Thumbnail && selector->documents().count() )
    {
        selector->setViewMode( QImageDocumentSelector::Single );
        setContextBar();
    }
    else {
#endif
        QDialog::accept();
#ifdef QTOPIA_KEYPAD_NAVIGATION
    }
#endif
}

/*! \internal */
void QImageDocumentSelectorDialog::reject()
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    // If single view, switch to thumbnail view
    // Otherwise, reject
    if( selector->viewMode() == QImageDocumentSelector::Single && selector->documents().count() ) {
        selector->setViewMode( QImageDocumentSelector::Thumbnail );
        setContextBar();
    } else {
#endif
        QDialog::reject();
#ifdef QTOPIA_KEYPAD_NAVIGATION
    }
#endif
}

#ifdef QTOPIA_KEYPAD_NAVIGATION
void QImageDocumentSelectorDialog::setContextBar()
{
    bool hasImages = selector->documents().count();
    switch( selector->viewMode() )
    {
    case QImageDocumentSelector::Single:
        if( hasImages ) {
            QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::Select );
        } else {
            QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );
        }
        break;
    case QImageDocumentSelector::Thumbnail:
        if( hasImages ) {
            QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::View );
        } else {
            QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );
        }
        break;
    }
}
#endif

#ifdef QTOPIA_PDA
void QImageDocumentSelectorDialog::setViewSingle()
{
    selector->setViewMode( QImageDocumentSelector::Single );
}
#endif

/*!
    Sets the \a categories selected by default in the document selector's category filter dialog.

    If a default category does not appear in the category filter dialog then the content of the document selector will
    not be filtered on that category.  If a default category is added to the list after the category selector has been
    initialized it will not be automatically selected.

    \sa defaultCategories()
 */
void QImageDocumentSelectorDialog::setDefaultCategories( const QStringList &categories )
{
    selector->setDefaultCategories( categories );
}

/*!
    Returns the categories selected by default in the document selector's category filter dialog.

    \sa setDefaultCategories()
 */
QStringList QImageDocumentSelectorDialog::defaultCategories() const
{
    return selector->defaultCategories();
}

/*!
    Sets the intended usage of the selected document.  If the document does not have the \a permission the document selector
    will attempt to activate before the selection succeeds.  If the document cannot be activated with that permission, it
    will not be selectable.

    If the permission is QDrmRights::InvalidPermission the default permission for the content is used.
 */
void QImageDocumentSelectorDialog::setSelectPermission( QDrmRights::Permission permission )
{
    selector->setSelectPermission( permission );
}

/*!
    Returns the intended usage of the selected document.
 */
QDrmRights::Permission QImageDocumentSelectorDialog::selectPermission() const
{
    return selector->selectPermission();
}

/*!
    Sets the \a permissions a document must have in order to be selectable in the document selector.

    Unlike the select permission if a document is missing a mandatory permission it is simply unselectable and can not
    be activated.

    \sa setSelectPermission()
 */
void QImageDocumentSelectorDialog::setMandatoryPermissions( QDrmRights::Permissions permissions )
{
    selector->setMandatoryPermissions( permissions );
}

/*!
    Returns the permissions a document must have in order to be selectable in the document selector.
 */
QDrmRights::Permissions QImageDocumentSelectorDialog::mandatoryPermissions() const
{
    return selector->mandatoryPermissions();
}

