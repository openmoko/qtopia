/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include "qimagedocumentselector_p.h"

#include "singleview_p.h"

#include <qtopiaapplication.h>
#include <qthumbnail.h>
#include <qsoftmenubar.h>
#include <qcontentfilterselector.h>

#include <qtopia/private/drmcontent_p.h>

#include <QList>
#include <QLayout>
#include <QComboBox>
#include <QImageReader>
#include <QMenu>
#include <QTimer>

#define DEFAULT_VIEW QImageDocumentSelector::Thumbnail

#define HIGH_STRETCH_FACTOR 100

QImageDocumentSelectorPrivate::QImageDocumentSelectorPrivate( QWidget* parent )
    : QWidget( parent )
    , image_collection( QContentSet::Asynchronous )
    , sort_mode( QDocumentSelector::Alphabetical )
    , drm_content( QDrmRights::Display )
    , current_view( DEFAULT_VIEW )
{
    setFilter( QContentFilter( QContent::Document ) & QContentFilter( QContentFilter::MimeType, QLatin1String( "image/*" ) ) );

    init();
}

void QImageDocumentSelectorPrivate::init()
{
    model = new QContentSetModel(&image_collection, this);
    // Update selection and view when model changes
    connect( model, SIGNAL(rowsInserted(QModelIndex,int,int)), this,
             SLOT(rowAddedSelection()) );
    connect( model, SIGNAL(rowsInserted(QModelIndex,int,int)), this,
             SIGNAL(documentsChanged()) );
    connect( model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this,
             SLOT(rowRemovedSelection(QModelIndex,int,int)) );
    connect( model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this,
             SIGNAL(documentsChanged()) );

    connect( model, SIGNAL(updateFinished()), this, SLOT(updateFinished()) );

    // Initialize selection and update view when model reset
    connect( model, SIGNAL(modelReset()), this, SIGNAL(documentsChanged()) );
    connect( model, SIGNAL(modelReset()), this, SLOT(resetSelection()) );

    // Construct widget stack
    widget_stack = new QStackedLayout( this );

    // Construct single view
    single_view = new SingleView( this );
    connect( single_view, SIGNAL(selected()), this, SLOT(emitSelected()) );
    connect( single_view, SIGNAL(held(QPoint)), this, SLOT(emitHeld(QPoint)) );
    // Connect single view to model
    single_view->setModel( model );

    // Construct thumbnail repository
    ThumbnailCache *cache = new ThumbnailCache( this );

    ThumbnailLoader *loader = new DRMThumbnailLoader( cache, this );
    ThumbnailRepository *repository = new ThumbnailRepository( cache, loader, this );

    // Construct thumbnail view
    thumbnail_view = new ThumbnailView( this );
    connect( repository, SIGNAL(loaded(ThumbnailRequest,QPixmap)),
             thumbnail_view, SLOT(repaintThumbnail(ThumbnailRequest)) );
    connect( thumbnail_view, SIGNAL(selected()), this, SLOT(emitSelected()) );
    thumbnail_view->setViewMode( QListView::IconMode );
    thumbnail_view->setIconSize( QSize( 64, 64 ) );
    thumbnail_view->setSpacing( 2 );
    thumbnail_view->setUniformItemSizes( true );

    // Construct thumbnail delegate
    ThumbnailDelegate *delegate = new ThumbnailDelegate( repository, this );
    thumbnail_view->setItemDelegate( delegate );

    // Set thumbnail loader rules
    loader->setVisibleRule( VisibleRule( thumbnail_view ) );
    loader->setCacheRule( CacheRule( cache ) );

    // Connect thumbnail view to model
    thumbnail_view->setModel( model );

    // Share selection model between single view and thumbnail view
    thumbnail_view->setSelectionModel( single_view->selectionModel() );

    // If there are images, set selection to first image in collection
    if( model->rowCount() ) {
        single_view->selectionModel()->setCurrentIndex( model->index( 0 ), QItemSelectionModel::ClearAndSelect );
    }

    // Construct message view
    message_view = new QLabel( tr( "<center><p>No images found.</p></center>"), this );
    message_view->setAlignment( Qt::AlignCenter );
    message_view->setWordWrap( true );

    widget_stack->addWidget( single_view );
    widget_stack->addWidget( thumbnail_view );
    widget_stack->addWidget( message_view );

    // Construct context menu
    QMenu *context_menu = QSoftMenuBar::menuFor( this );

    // Add view category to context menu
    QContentFilterModel::Template categoryPage;

    categoryPage.setOptions( QContentFilterModel::CheckList | QContentFilterModel::SelectAll |
            QContentFilterModel::AndCheckedFilters );

    categoryPage.addList( QContentFilter::Category );
    categoryPage.addList( QContentFilter::Category, QLatin1String( "Documents" ) );

    category_dialog = new QContentFilterDialog( categoryPage, this );

    category_dialog->setFilter( content_filter );

    context_menu->addAction( QIcon( ":icon/viewcategory" ),
                             tr( "View Category..." ), this, SLOT(launchCategoryDialog()) );

    connect( &drm_content, SIGNAL(rightsExpired(QDrmContent)), this, SLOT(setViewThumbnail()) );

    model->setSelectPermission( QDrmRights::Display );

    widget_stack->setCurrentWidget( thumbnail_view );
    thumbnail_view->setFocus();

    // We need this because we're no longer getting the modelReset signal on startup. This schedules
    // the function that ensures the scrollbar is positioned at the top, after the documents have loaded.
    //QTimer::singleShot(0, this, SLOT(delayResetSelection()));
}

void QImageDocumentSelectorPrivate::setViewMode( QImageDocumentSelector::ViewMode mode )
{
    // If there are images in the visible collection
    if( model->rowCount() > 0 ) {
        // Move new view to top of widget stack
        switch( mode )
        {
        case QImageDocumentSelector::Single:
            if( current_view != QImageDocumentSelector::Single ) {
                // Move single view to top of stack
                    drm_content.renderStarted();

                widget_stack->setCurrentIndex( widget_stack->indexOf( single_view ) );
            }
            break;
        case QImageDocumentSelector::Thumbnail:
            if( current_view != QImageDocumentSelector::Thumbnail ) {
                current_view = mode;
                // Move thumbnail view to top of stack
                drm_content.renderStopped();
                drm_content.releaseLicense();

                widget_stack->setCurrentIndex( widget_stack->indexOf( thumbnail_view ) );
                thumbnail_view->setFocus();
            }
            break;
        }
    }

    // Update current view mode
    current_view = mode;
}

void QImageDocumentSelectorPrivate::setThumbnailSize( const QSize& size )
{
    thumbnail_view->setIconSize( size );
}

QSize QImageDocumentSelectorPrivate::thumbnailSize() const
{
    return thumbnail_view->iconSize();
}

void QImageDocumentSelectorPrivate::setFilter( const QContentFilter &filter, bool enableForce )
{
    content_filter = filter;

    content_filter &= QContentFilter( QContentFilter::MimeType, QLatin1String( "image/*" ) );

    applyFilters(enableForce);
}

void QImageDocumentSelectorPrivate::setSortMode( QDocumentSelector::SortMode mode, bool enableForce )
{
    if( mode != sort_mode || enableForce )
    {
        sort_mode = mode;

        switch( mode )
        {
        case QDocumentSelector::Alphabetical:
            image_collection.setSortCriteria( QContentSortCriteria( QContentSortCriteria::Name, Qt::AscendingOrder ) );
            break;
        case QDocumentSelector::ReverseAlphabetical:
            image_collection.setSortCriteria( QContentSortCriteria( QContentSortCriteria::Name, Qt::DescendingOrder ) );
            break;
        case QDocumentSelector::Chronological:
            image_collection.setSortCriteria( QContentSortCriteria( QContentSortCriteria::LastUpdated, Qt::AscendingOrder ) );
            break;
        case QDocumentSelector::ReverseChronological:
            image_collection.setSortCriteria( QContentSortCriteria( QContentSortCriteria::LastUpdated, Qt::DescendingOrder ) );
            break;
        case QDocumentSelector::SortCriteria:
            break;
        }
    }
}

void QImageDocumentSelectorPrivate::setSortCriteria( const QContentSortCriteria &sort )
{
    image_collection.setSortCriteria( sort );
}

void QImageDocumentSelectorPrivate::applyFilters( bool enableForce )
{
    QContentFilter filters = content_filter & category_filter;

    if ( enableForce || image_collection.filter() != filters ) {
        // Set the filter criteria
        image_collection.setCriteria(filters);
    }
}

QContentFilter QImageDocumentSelectorPrivate::filter() const
{
    return image_collection.filter();
}

QDocumentSelector::SortMode QImageDocumentSelectorPrivate::sortMode() const
{
    return sort_mode;
}

QContentSortCriteria QImageDocumentSelectorPrivate::sortCriteria() const
{
    return image_collection.sortCriteria();
}

QContent QImageDocumentSelectorPrivate::selectedDocument() const
{
    // If current selection valid, return selected DocLnk
    // Otherwise, return invalid DocLnk
    return model->content( single_view->selectionModel()->currentIndex() );
}

const QContentSet &QImageDocumentSelectorPrivate::documents() const
{
    // Return list of currently visible images
    return image_collection;
}

int QImageDocumentSelectorPrivate::count() const
{
    return model->rowCount();
}

void QImageDocumentSelectorPrivate::setFocus()
{
    if( current_view == QImageDocumentSelector::Thumbnail )
        thumbnail_view->setFocus();
    else
        QWidget::setFocus();
}

void QImageDocumentSelectorPrivate::emitSelected()
{
    if( single_view->selectionModel()->currentIndex().flags() & Qt::ItemIsSelectable )
    {
        if( current_view == QImageDocumentSelector::Thumbnail )
        {
            if( drm_content.requestLicense( selectedDocument() ) || model->selectPermission() != QDrmRights::Display )
                emit documentSelected( selectedDocument() );
        }
        else if( model->selectPermission() == QDrmRights::Display ||
                 DrmContentPrivate::activate( selectedDocument(), model->selectPermission() ) )
            emit documentSelected( selectedDocument() );
    }
}

void QImageDocumentSelectorPrivate::emitHeld( const QPoint& pos )
{
    emit documentHeld( selectedDocument(), pos );
}

void QImageDocumentSelectorPrivate::setViewSingle()
{
    setViewMode( QImageDocumentSelector::Single );
}

void QImageDocumentSelectorPrivate::setViewThumbnail()
{
    setViewMode( QImageDocumentSelector::Thumbnail );
}

void QImageDocumentSelectorPrivate::launchCategoryDialog()
{
    if( default_categories_dirty && category_dialog )
    {
        delete category_dialog;

        category_dialog = 0;
    }

    if( !category_dialog )
    {
        default_categories_dirty = false;

        QContentFilterModel::Template categoryPage;

        categoryPage.setOptions( QContentFilterModel::CheckList | QContentFilterModel::SelectAll );

        categoryPage.addList( QContentFilter::Category, QString(), filtered_default_categories );
        categoryPage.addList( QContentFilter::Category, QLatin1String( "Documents" ), filtered_default_categories );

        category_dialog = new QContentFilterDialog( categoryPage, this );

        category_dialog->setFilter( content_filter );
    }

    QtopiaApplication::execDialog( category_dialog );

    category_filter = category_dialog->checkedFilter();

    applyFilters();
}

void QImageDocumentSelectorPrivate::rowRemovedSelection(const QModelIndex &,
        int rstart, int rend)
{
    Q_UNUSED(rend);

    if (!model->rowCount())
        return;

    QModelIndexList selection = single_view->selectionModel()->selectedIndexes();

    // If the selection contains some indexes, proceed
    if (selection.size() > 0)
        return;

    // Otherwise the currently selected row was removed, update the selection
    int desired_row_to_select = rstart;
    if (rstart >= model->rowCount())
        desired_row_to_select = model->rowCount() - 1;

    QModelIndex current = model->index(desired_row_to_select);
    single_view->selectionModel()->setCurrentIndex(current, QItemSelectionModel::SelectCurrent);
}

void QImageDocumentSelectorPrivate::rowAddedSelection()
{
    if (!model->rowCount())
        return;

    QModelIndexList selection = single_view->selectionModel()->selectedIndexes();

    // If the selection contains some indexes, proceed
    if (selection.size() > 0)
        return;

    // Otherwise a new row was added and we have not selected anything, select the first
    // item
    QModelIndex current = model->index(0);
    single_view->selectionModel()->setCurrentIndex(current, QItemSelectionModel::SelectCurrent);
}

void QImageDocumentSelectorPrivate::resetSelection()
{
    // TODO: Figure out why this is really needed, something is wrong with
    // QContentSetModel
    QTimer::singleShot(0, this, SLOT(delayResetSelection()));
}

void QImageDocumentSelectorPrivate::delayResetSelection()
{
    // If there are images, set selection to first image in collection
    if( model->rowCount() > 0 ) {
        QModelIndex idx = model->index(0);
        single_view->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent);
        // Ensure scrollbar is positioned at the top once visible images are loaded.
        thumbnail_view->scrollToTop();
    }
}

void QImageDocumentSelectorPrivate::showEvent( QShowEvent *event )
{
    if( current_view == QImageDocumentSelector::Single )
        drm_content.renderStarted();

    QWidget::showEvent( event );
}

void QImageDocumentSelectorPrivate::hideEvent( QHideEvent *event )
{
    if( current_view == QImageDocumentSelector::Single )
        drm_content.renderPaused();

    QWidget::hideEvent( event );
}

void QImageDocumentSelectorPrivate::setDefaultCategories( const QStringList &categories )
{
    default_categories = categories;

    filterDefaultCategories();

    applyFilters();
}

QStringList QImageDocumentSelectorPrivate::defaultCategories() const
{
    return default_categories;
}

void QImageDocumentSelectorPrivate::setSelectPermission( QDrmRights::Permission permission )
{
    model->setSelectPermission( permission );
}

QDrmRights::Permission QImageDocumentSelectorPrivate::selectPermission() const
{
    return model->selectPermission();
}

void QImageDocumentSelectorPrivate::setMandatoryPermissions( QDrmRights::Permissions permissions )
{
    model->setMandatoryPermissions( permissions );
}

QDrmRights::Permissions QImageDocumentSelectorPrivate::mandatoryPermissions() const
{
    return model->mandatoryPermissions();
}

void QImageDocumentSelectorPrivate::filterDefaultCategories()
{
    if( default_categories.isEmpty() )
    {
        category_filter = QContentFilter();

        return;
    }

    QContentFilter filter;

    foreach( QString category, default_categories )
        filter |= QContentFilter( QContentFilter::Category, category );

    filter &= content_filter;

    filtered_default_categories
            = filter.argumentMatches( QContentFilter::Category, QString() )
            + filter.argumentMatches( QContentFilter::Category, QLatin1String( "Documents" ) );

    default_categories_dirty = true;

    category_filter = QContentFilter();

    foreach( QString category, filtered_default_categories )
        category_filter |= QContentFilter( QContentFilter::Category, category );
}

void QImageDocumentSelectorPrivate::updateFinished()
{
    if( !model->rowCount() )
    {
        widget_stack->setCurrentWidget( message_view );
    }
    else if( widget_stack->currentWidget() == message_view )
    {
        widget_stack->setCurrentWidget( thumbnail_view );
    }
}

