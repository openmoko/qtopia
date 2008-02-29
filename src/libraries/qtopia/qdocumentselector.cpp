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

#include "qdocumentselector.h"
#include <QMenu>
#include <QVBoxLayout>
#include <QListView>
#include <QAction>
#include <qcontentfiltermodel.h>
#include <qdocumentproperties.h>
#include <qtopiaapplication.h>
#include <QAbstractProxyModel>
#include <qcontentfilterselector.h>
#include <qdrmcontent.h>
#include <qwaitwidget.h>
#include "drmcontent_p.h"
#include <QFocusEvent>

#ifdef QTOPIA_KEYPAD_NAVIGATION
#include <qsoftmenubar.h>
#endif

class NewDocumentProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
public:
    NewDocumentProxyModel( QIcon newIcon, QObject *parent = 0 )
        : QAbstractProxyModel( parent )
        , m_newEnabled( false )
        , m_newIcon( newIcon )
    {
    }

    virtual ~NewDocumentProxyModel()
    {
    }

    virtual QModelIndex mapFromSource( const QModelIndex &index ) const
    {
        if( index.isValid() )
        {
            return m_newEnabled
                    ? createIndex( index.row() + 1, index.column() )
                    : createIndex( index.row()    , index.column() );
        }
        else
            return index;
    }

    virtual QModelIndex mapToSource( const QModelIndex &index ) const
    {
        if( index.isValid() && sourceModel() )
        {
            return m_newEnabled
                    ? sourceModel()->index( index.row() - 1, index.column() )
                    : sourceModel()->index( index.row()    , index.column() );
        }
        else
            return index;
    }

    void setSourceModel( QAbstractItemModel *model )
    {
        QAbstractItemModel *oldModel = sourceModel();

        if( oldModel )
        {
            disconnect( oldModel, SIGNAL(columnsAboutToBeInserted(const QModelIndex&,int,int)),
                        this    , SLOT (_columnsAboutToBeInserted(const QModelIndex&,int,int)) );
            disconnect( oldModel, SIGNAL(columnsAboutToBeRemoved(const QModelIndex&,int,int)),
                        this    , SLOT (_columnsAboutToBeRemoved(const QModelIndex&,int,int)) );
            disconnect( oldModel, SIGNAL(columnsInserted(const QModelIndex&,int,int)),
                        this    , SLOT (_columnsInserted(const QModelIndex&,int,int)) );
            disconnect( oldModel, SIGNAL(columnsRemoved(const QModelIndex&,int,int)),
                        this    , SLOT (_columnsRemoved(const QModelIndex&,int,int)) );
            disconnect( oldModel, SIGNAL(dataChanged(const QModelIndex&,const QModelIndex&)),
                        this    , SLOT (_dataChanged(const QModelIndex&,const QModelIndex&)) );
            disconnect( oldModel, SIGNAL(layoutAboutToBeChanged()),
                        this    , SIGNAL(layoutAboutToBeChanged()) );
            disconnect( oldModel, SIGNAL(layoutChanged()),
                        this    , SIGNAL(layoutChanged()) );
            disconnect( oldModel, SIGNAL(modelReset()),
                        this    , SIGNAL(modelReset()) );
            disconnect( oldModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex&,int,int)),
                        this    , SLOT (_rowsAboutToBeInserted(const QModelIndex&,int,int)) );
            disconnect( oldModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&,int,int)),
                        this    , SLOT (_rowsAboutToBeRemoved(const QModelIndex&,int,int)) );
            disconnect( oldModel, SIGNAL(rowsInserted(const QModelIndex&,int,int)),
                        this    , SLOT (_rowsInserted(const QModelIndex&,int,int)) );
            disconnect( oldModel, SIGNAL(rowsRemoved(const QModelIndex&,int,int)),
                        this    , SLOT (_rowsRemoved(const QModelIndex&,int,int)) );
        }

        QAbstractProxyModel::setSourceModel( model );

        connect( model, SIGNAL(columnsAboutToBeInserted(const QModelIndex&,int,int)),
                 this , SLOT (_columnsAboutToBeInserted(const QModelIndex&,int,int)) );
        connect( model, SIGNAL(columnsAboutToBeRemoved(const QModelIndex&,int,int)),
                 this , SLOT (_columnsAboutToBeRemoved(const QModelIndex&,int,int)) );
        connect( model, SIGNAL(columnsInserted(const QModelIndex&,int,int)),
                 this , SLOT (_columnsInserted(const QModelIndex&,int,int)) );
        connect( model, SIGNAL(columnsRemoved(const QModelIndex&,int,int)),
                 this , SLOT (_columnsRemoved(const QModelIndex&,int,int)) );
        connect( model, SIGNAL(dataChanged(const QModelIndex&,const QModelIndex&)),
                 this , SLOT (_dataChanged(const QModelIndex&,const QModelIndex&)) );
        connect( model, SIGNAL(layoutAboutToBeChanged()),
                 this , SIGNAL(layoutAboutToBeChanged()) );
        connect( model, SIGNAL(layoutChanged()),
                 this , SIGNAL(layoutChanged()) );
        connect( model, SIGNAL(modelReset()),
                 this , SIGNAL(modelReset()) );
        connect( model, SIGNAL(rowsAboutToBeInserted(const QModelIndex&,int,int)),
                 this , SLOT (_rowsAboutToBeInserted(const QModelIndex&,int,int)) );
        connect( model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&,int,int)),
                 this , SLOT (_rowsAboutToBeRemoved(const QModelIndex&,int,int)) );
        connect( model, SIGNAL(rowsInserted(const QModelIndex&,int,int)),
                 this , SLOT (_rowsInserted(const QModelIndex&,int,int)) );
        connect( model, SIGNAL(rowsRemoved(const QModelIndex&,int,int)),
                 this , SLOT (_rowsRemoved(const QModelIndex&,int,int)) );
    }

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const
    {
        return !parent.isValid() && sourceModel() ? 1 : 0;
    }

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const
    {
        if( m_newEnabled && index.row() == 0 )
        {
            if( role == Qt::DisplayRole )
                return tr( "New" );
            else if( role == Qt::DecorationRole )
                return m_newIcon;
        }
        else if( sourceModel()->rowCount() == 0 && index.row() == 0 )
        {
            if( role == Qt::DisplayRole )
                return tr( "No documents found" );
        }
        else if( index.isValid() )
        {
            return sourceModel() ? sourceModel()->data( mapToSource( index ), role ) : 0;
        }

        return QVariant();
    }

    virtual Qt::ItemFlags flags( const QModelIndex & index ) const
    {
        if( m_newEnabled && index.row() == 0 )
        {
            return QAbstractItemModel::flags( index );
        }
        else if( sourceModel()->rowCount() == 0 && index.row() == 0 )
        {
            return Qt::ItemIsEnabled;
        }
        else if( index.isValid() )
        {
            QAbstractItemModel *model = sourceModel();

            if( sourceModel() )
                return model->flags( mapToSource( index ) );
        }

        return QAbstractProxyModel::flags( index );
    }

    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const
    {
        return !parent.isValid() ? createIndex( row, column ) : QModelIndex();
    }

    virtual QModelIndex parent( const QModelIndex &index ) const
    {
        Q_UNUSED( index );

        return QModelIndex();
    }

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const
    {
        if( !parent.isValid() )
        {
            int count = sourceModel()->rowCount();

            if( m_newEnabled )
                count++;

            return count > 0 ? count : 1;
        }

        return 0;
    }

    void setNewEnabled( bool enabled )
    {
        if( enabled && !m_newEnabled )
        {
            beginInsertRows( QModelIndex(), 0, 0 );
            m_newEnabled = true;
            endInsertRows();
        }
        else if( !enabled && m_newEnabled )
        {
            beginRemoveRows( QModelIndex(), 0, 0 );
            m_newEnabled = false;
            endRemoveRows();
        }
    }

private slots:
    void _columnsAboutToBeInserted( const QModelIndex &parent, int start, int end )
    {
        if( !parent.isValid() )
        {
            if( m_newEnabled )
                beginRemoveColumns( QModelIndex(), start + 1, end + 1 );
            else
                beginRemoveColumns( QModelIndex(), start, end );
        }
    }

    void _columnsAboutToBeRemoved( const QModelIndex &parent, int start, int end )
    {
        if( !parent.isValid() )
        {
            if( m_newEnabled )
                beginRemoveColumns( QModelIndex(), start + 1, end + 1 );
            else
                beginRemoveColumns( QModelIndex(), start, end );
        }
    }

    void _columnsInserted( const QModelIndex &parent, int start, int end )
    {
        Q_UNUSED( parent );
        Q_UNUSED( start );
        Q_UNUSED( end );

        endInsertColumns();
    }

    void _columnsRemoved( const QModelIndex &parent, int start, int end )
    {
        Q_UNUSED( parent );
        Q_UNUSED( start );
        Q_UNUSED( end );

        endRemoveColumns();
    }

    void _dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
    {
        if( m_newEnabled )
            emit dataChanged( createIndex( topLeft    .row() + 1, topLeft    .column() ),
                              createIndex( bottomRight.row() + 1, bottomRight.column() ) );
        else
            emit dataChanged( createIndex( topLeft    .row(), topLeft    .column() ),
                              createIndex( bottomRight.row(), bottomRight.column() ) );
    }

    void _rowsAboutToBeInserted( const QModelIndex &parent, int start, int end )
    {
        if( !parent.isValid() )
        {
            if( m_newEnabled )
                beginInsertRows( QModelIndex(), start + 1, end + 1 );
            else
                beginInsertRows( QModelIndex(), start, end );
        }
    }

    void _rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end )
    {
        if( !parent.isValid() )
        {
            if( m_newEnabled )
                beginRemoveRows( QModelIndex(), start + 1, end + 1 );
            else
                beginRemoveRows( QModelIndex(), start, end );
        }
    }

    void _rowsInserted( const QModelIndex &parent, int start, int end )
    {
        Q_UNUSED( parent );
        Q_UNUSED( start );
        Q_UNUSED( end );

        endInsertRows();
    }

    void _rowsRemoved( const QModelIndex &parent, int start, int end )
    {
        Q_UNUSED( parent );
        Q_UNUSED( start );
        Q_UNUSED( end );

        endRemoveRows();
    }

private:
    bool m_newEnabled;
    QIcon m_newIcon;
};

class DocumentView : public QListView
{
    Q_OBJECT
public:
    DocumentView( QWidget *parent = 0 );
    virtual ~DocumentView();

    QContentFilter baseFilter() const;
    void setBaseFilter( const QContentFilter &filter );

    void setDefaultCategories( const QStringList &categories );
    QStringList defaultCategories() const;

    void setSelectPermission( QDrmRights::Permission permission );
    QDrmRights::Permission selectPermission() const;

    void setMandatoryPermissions( QDrmRights::Permissions permissions );
    QDrmRights::Permissions mandatoryPermissions() const;

    void setSortMode( QDocumentSelector::SortMode mode );
    QDocumentSelector::SortMode sortMode() const;

    QDocumentSelector::Options options() const;
    void setOptions( QDocumentSelector::Options options );

    QContent currentDocument() const;

    bool newCurrent() const;

    const QContentSet &documents() const;

signals:
    void documentSelected( const QContent &content );
    void newSelected();
    void documentsChanged();

#ifdef QTOPIA_KEYPAD_NAVIGATION
protected slots:
    virtual void currentChanged( const QModelIndex &current, const QModelIndex &previous );
    virtual void focusInEvent( QFocusEvent *event );
#endif
private slots:
    void indexActivated( const QModelIndex &index );
    void selectTypeFilter();
    void selectCategoryFilter();
    void showProperties();
    void deleteCurrent();

private:
    void setCombinedFilter();

    static QContentFilterModel::Template typeTemplate();

    void filterDefaultCategories();

    QContentFilter m_baseFilter;
    QContentFilter m_typeFilter;
    QContentFilter m_categoryFilter;

    QContentSet m_contentSet;

    QContentFilterDialog *m_typeDialog;
    QContentFilterDialog *m_categoryDialog;

    QDocumentPropertiesDialog *m_propertiesDialog;

    QWaitWidget *m_waitWidget;

    QContentSetModel *m_contentModel;
    NewDocumentProxyModel *m_proxyModel;

    QDocumentSelector::Options m_options;

    QDocumentSelector::SortMode m_sortMode;

    QStringList m_defaultCategories;
    QStringList m_filteredDefaultCategories;
    bool m_defaultCategoriesDirty;

#ifdef QTOPIA_KEYPAD_NAVIGATION
    QMenu *m_softMenu;

    QAction *m_newAction;
    QAction *m_deleteAction;
    QAction *m_propertiesAction;
    QAction *m_typeAction;
    QAction *m_categoryAction;
#endif
};

DocumentView::DocumentView( QWidget *parent )
    : QListView( parent )
    , m_baseFilter( QContent::Document )
    , m_typeDialog( 0 )
    , m_categoryDialog( 0 )
    , m_propertiesDialog( 0 )
    , m_waitWidget( 0 )
    , m_options( QDocumentSelector::ContextMenu )
    , m_sortMode( QDocumentSelector::Alphabetical )
    , m_defaultCategoriesDirty( false )
{
    QIcon newIcon( ":icon/new" );

    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

#ifdef QTOPIA_KEYPAD_NAVIGATION
    m_softMenu = QSoftMenuBar::menuFor( this );

    m_newAction = m_softMenu->addAction( newIcon, tr( "New" ) );
    m_deleteAction = m_softMenu->addAction( QIcon( ":icon/trash" ), tr( "Delete" ) );

    m_propertiesAction = m_softMenu->addAction( QIcon(":icon/info"), tr( "Properties..." ) );

    m_softMenu->addSeparator();

    m_typeAction = m_softMenu->addAction( tr( "Select Type..." ) );
    m_categoryAction = m_softMenu->addAction( QIcon( ":icon/viewcategory" ), tr( "Select Category..." ) );

    m_newAction->setVisible( false );
    m_deleteAction->setVisible( false );
    m_propertiesAction->setVisible( false );
    m_typeAction->setVisible( false );

    connect( m_newAction, SIGNAL(triggered()), this, SIGNAL(newSelected()) );
    connect( m_deleteAction, SIGNAL(triggered()), this, SLOT(deleteCurrent()) );
    connect( m_propertiesAction, SIGNAL(triggered()), this, SLOT(showProperties()) );
    connect( m_typeAction, SIGNAL(triggered()), this, SLOT(selectTypeFilter()) );
    connect( m_categoryAction, SIGNAL(triggered()), this, SLOT(selectCategoryFilter()) );
#endif

    connect( this, SIGNAL(activated(const QModelIndex&)), this, SLOT(indexActivated(const QModelIndex&)) );

    connect( &m_contentSet, SIGNAL(changed()), this, SIGNAL(documentsChanged()));

    m_contentModel = new QContentSetModel( &m_contentSet, this );
    m_proxyModel = new NewDocumentProxyModel( newIcon, this );

    m_proxyModel->setSourceModel( m_contentModel );

    setModel( m_proxyModel );
}

DocumentView::~DocumentView()
{
}

QContentFilter DocumentView::baseFilter() const
{
    return m_baseFilter;
}

void DocumentView::setBaseFilter( const QContentFilter &filter )
{
    m_baseFilter = filter;

    if( m_typeDialog )
        m_typeDialog->setFilter( filter );

    if( m_categoryDialog )
        m_categoryDialog->setFilter( filter );

    m_typeFilter = QContentFilter();

    filterDefaultCategories();

    setCombinedFilter();
}

void DocumentView::setSortMode( QDocumentSelector::SortMode mode )
{
    if( mode != m_sortMode )
    {
        m_sortMode = mode;

        switch( mode )
        {
        case QDocumentSelector::Alphabetical:
            m_contentSet.setSortOrder( QStringList() << QLatin1String( "name" ) );
            break;
        case QDocumentSelector::ReverseAlphabetical:
            m_contentSet.setSortOrder( QStringList() << QLatin1String( "name desc" ) );
            break;
        case QDocumentSelector::Chronological:
            m_contentSet.setSortOrder( QStringList() << QLatin1String( "time" ) );
            break;
        case QDocumentSelector::ReverseChronological:
            m_contentSet.setSortOrder( QStringList() << QLatin1String( "time desc" ) );
            break;
        }
    }
}

QDocumentSelector::SortMode DocumentView::sortMode() const
{
    return m_sortMode;
}


QDocumentSelector::Options DocumentView::options() const
{
    return m_options;
}

void DocumentView::setOptions( QDocumentSelector::Options options )
{
    QDocumentSelector::Options changes = m_options ^ options;

    if( m_typeDialog && changes & QDocumentSelector::NestTypes )
    {
        if( options & QDocumentSelector::NestTypes )
            m_typeDialog->setModelTemplate( typeTemplate() );
        else
            m_typeDialog->setModelTemplate(
                    QContentFilter::MimeType,
                    QString(),
                    QContentFilterModel::CheckList | QContentFilterModel::SelectAll );

        m_typeFilter = QContentFilter();

        setCombinedFilter();
    }

    if( changes & QDocumentSelector::NewDocument )
    {
        m_proxyModel->setNewEnabled( options & QDocumentSelector::NewDocument );
        m_newAction->setVisible( options & QDocumentSelector::NewDocument );
    }

    if( changes & QDocumentSelector::TypeSelector )
    {
        m_typeAction->setVisible( options & QDocumentSelector::TypeSelector );
    }
#ifdef QTOPIA_KEYPAD_NAVIGATION
    if( changes & QDocumentSelector::ContextMenu )
    {
        if( options & QDocumentSelector::ContextMenu )
            QSoftMenuBar::addMenuTo( this, m_softMenu );
        else
            QSoftMenuBar::removeMenuFrom( this, m_softMenu );
    }
#endif

    m_options = options;
}

void DocumentView::setDefaultCategories( const QStringList &categories )
{
    m_defaultCategories = categories;

    filterDefaultCategories();

    setCombinedFilter();
}

QStringList DocumentView::defaultCategories() const
{
    return m_defaultCategories;
}

void DocumentView::setSelectPermission( QDrmRights::Permission permission )
{
    m_contentModel->setSelectPermission( permission );
}

QDrmRights::Permission DocumentView::selectPermission() const
{
    return m_contentModel->selectPermission();
};

void DocumentView::setMandatoryPermissions( QDrmRights::Permissions permissions )
{
    m_contentModel->setMandatoryPermissions( permissions );
}

QDrmRights::Permissions DocumentView::mandatoryPermissions() const
{
    return m_contentModel->mandatoryPermissions();
}

QContent DocumentView::currentDocument() const
{
    return m_contentModel->rowCount()
            ? m_contentModel->content( m_proxyModel->mapToSource( currentIndex() ) )
            : QContent();
}

bool DocumentView::newCurrent() const
{
    return m_options & QDocumentSelector::NewDocument && currentIndex().row() == 0;
}

const QContentSet &DocumentView::documents() const
{
    return m_contentSet;
}

void DocumentView::indexActivated( const QModelIndex &index )
{
    if( m_options & QDocumentSelector::NewDocument && index.row() == 0 )
        emit newSelected();
    else if( !(m_contentModel->rowCount() == 0 && index.row() == 0) && index.flags() & Qt::ItemIsSelectable )
    {
        QContent content = m_contentModel->content( m_proxyModel->mapToSource( index ) );

        if( DrmContentPrivate::activate( content, m_contentModel->selectPermission(), this ) )
            emit documentSelected( content );
    }
}

#ifdef QTOPIA_KEYPAD_NAVIGATION
void DocumentView::currentChanged( const QModelIndex &current, const QModelIndex &previous )
{
    QListView::currentChanged( current, previous );

    int minRow = m_options & QDocumentSelector::NewDocument ? 1 : 0;

    if( previous.row() < minRow && current.row() >= minRow )
    {
        m_deleteAction->setVisible( true );
        m_propertiesAction->setVisible( true );
    }
    else if( previous.row() >= minRow && current.row() < minRow )
    {
        m_deleteAction->setVisible( false );
        m_propertiesAction->setVisible( false );
    }
}
#endif

void DocumentView::focusInEvent( QFocusEvent *event )
{
    QListView::focusInEvent( event );

    setCurrentIndex( currentIndex() );
}

void DocumentView::selectTypeFilter()
{
    if( !m_typeDialog )
    {
        m_typeDialog = new QContentFilterDialog( this );

        if( m_options & QDocumentSelector::NestTypes )
            m_typeDialog->setModelTemplate( typeTemplate() );
        else
            m_typeDialog->setModelTemplate(
                    QContentFilter::MimeType,
        QString(),
        QContentFilterModel::CheckList | QContentFilterModel::SelectAll );

        m_typeDialog->setFilter( m_baseFilter );
    }

    QtopiaApplication::execDialog( m_typeDialog );

    m_typeFilter = m_typeDialog->checkedFilter();

    setCombinedFilter();
}

void DocumentView::selectCategoryFilter()
{
    if( m_defaultCategoriesDirty && m_categoryDialog )
    {
        delete m_categoryDialog;

        m_categoryDialog = 0;
    }

    if( !m_categoryDialog )
    {
        m_defaultCategoriesDirty = false;

        QContentFilterModel::Template categoryPage;

        categoryPage.setOptions( QContentFilterModel::CheckList | QContentFilterModel::SelectAll );

        categoryPage.addList( QContentFilter::Category, QString(), m_filteredDefaultCategories );
        categoryPage.addList( QContentFilter::Category, QLatin1String( "Documents" ), m_filteredDefaultCategories );

        m_categoryDialog = new QContentFilterDialog( categoryPage, this );

        m_categoryDialog->setFilter( m_baseFilter );
    }

    QtopiaApplication::execDialog( m_categoryDialog );

    m_categoryFilter = m_categoryDialog->checkedFilter();

    setCombinedFilter();
}

void DocumentView::deleteCurrent()
{
    QContent content = currentDocument();

    if( content.id() != QContent::InvalidId && Qtopia::confirmDelete( this, tr("Delete"), content.name() ) )
        content.removeFiles();
}

void DocumentView::showProperties()
{
    QContent content = currentDocument();

    if( content.id() != QContent::InvalidId && content.isValid() )
    {
        if( m_propertiesDialog )
            delete m_propertiesDialog;

        m_propertiesDialog = new QDocumentPropertiesDialog( content, this );
        m_propertiesDialog->setObjectName( QLatin1String( "document-properties" ) );
        QtopiaApplication::showDialog( m_propertiesDialog );
        m_propertiesDialog->setWindowState( m_propertiesDialog->windowState() | Qt::WindowMaximized );
    }
}

void DocumentView::setCombinedFilter()
{
    m_contentSet.setCriteria( m_baseFilter & m_typeFilter & m_categoryFilter );

    emit documentsChanged();
}

void DocumentView::filterDefaultCategories()
{
    if( m_defaultCategories.isEmpty() )
    {
        m_categoryFilter = QContentFilter();

        return;
    }

    QContentFilter filter;

    foreach( QString category, m_defaultCategories )
        filter |= QContentFilter( QContentFilter::Category, category );

    filter &= m_baseFilter;

    m_filteredDefaultCategories
            = filter.argumentMatches( QContentFilter::Category, QString() )
            + filter.argumentMatches( QContentFilter::Category, QLatin1String( "Documents" ) );

    m_defaultCategoriesDirty = true;

    m_categoryFilter = QContentFilter();

    foreach( QString category, m_filteredDefaultCategories )
        m_categoryFilter |= QContentFilter( QContentFilter::Category, category );
}

QContentFilterModel::Template DocumentView::typeTemplate()
{
    QContentFilterModel::Template subTypePage(
            QContentFilter::MimeType,
            QString(),
            QContentFilterModel::CheckList );

    QContentFilterModel::Template typePage;

    typePage.setOptions( QContentFilterModel::CheckList | QContentFilterModel::SelectAll );

    typePage.addLabel( subTypePage, tr( "Audio" ), QContentFilter( QContentFilter::MimeType, QLatin1String( "audio/*" ) ) );
    typePage.addLabel( subTypePage, tr( "Image" ), QContentFilter( QContentFilter::MimeType, QLatin1String( "image/*" ) ) );
    typePage.addLabel( subTypePage, tr( "Text"  ), QContentFilter( QContentFilter::MimeType, QLatin1String( "text/*"  ) ) );
    typePage.addLabel( subTypePage, tr( "Video" ), QContentFilter( QContentFilter::MimeType, QLatin1String( "video/*" ) ) );
    typePage.addList( ~( QContentFilter( QContentFilter::MimeType, QLatin1String( "audio/*" ) )
                       | QContentFilter( QContentFilter::MimeType, QLatin1String( "image/*" ) )
                       | QContentFilter( QContentFilter::MimeType, QLatin1String( "text/*"  ) )
                       | QContentFilter( QContentFilter::MimeType, QLatin1String( "video/*" ) ) ),
                      QContentFilter::MimeType );

    return typePage;
}

class QDocumentSelectorPrivate : public DocumentView
{
public:
    QDocumentSelectorPrivate( QWidget *parent = 0 )
    : DocumentView( parent )
    {
    }

    virtual ~QDocumentSelectorPrivate()
    {
    }
};

/*!
    \class QDocumentSelector
    \brief The QDocumentSelector widget allows the user to select documents from
            a list of documents available on the device.

    The QDocumentSelector widget builds the list of documents by using a supplied content filter.  If no
    filter is set a default filter which searches for all documents on the device is used.

    The following functionality is provided:
    \list
        \o \c setFilter() : filter the list of documents using a content filter
        \o \c filter() : retrieve the current content filter
        \o \c setSortMode() : control the sorting of the list of documents
        \o \c QDocumentSelector::SortMode : provide a list of supported sort modes
        \o \c sortMode() : retrieve the current sort mode
        \o \c currentDocument() : retrieve the current selected document
        \o \c newCurrent() : indicates whether the new document list item is selected
        \o \c QDocumentSelector::Option : provides a list of document selector configuration options.
        \o \c options : retrieve the enabled document selector options.
        \o \c setOptions() : set the enabled document selector options.
        \o \c enableOptions() : enable one or more document selector options.
        \o \c disableOptions() : disable one or more document selector options.
    \endlist

    When the \c NewDocument options is enabled QDocumentSelector adds a new document icon to the selection widget.
    When the icon is selected the newDocument() signal is emitted.

    When a document is selected, QDocumentSelector emits a
    documentSelected() signal and a QContent for the selected document is passed with
    the signal and the selected document is retrieved using selectedDocument().

    The following code is an example of how to:
    \list
        \o select a document from the chronologically ordered list on the device
        \o create new documents
    \endlist

    \code
    QDocumentSelector *selector = new QDocumentSelector( this );
    selector->setSortMode( QDocumentSelector::Chronological );
    selector->enableOption( QDocumentSelector::NewDocument );

    connect( selector, SIGNAL(newSelected()), this, SLOT(newDocument()) );
    connect( selector, SIGNAL(documentSelected(const QContent&)),
    this, SLOT(openDocument(const QContent&)) );
    \endcode

    QDocumentSelector is often the first widget seen in a \l {Qtopia - Main Document Widget}{document-oriented application }. Used in combination with
    \l {QStackedWidget}, a typical application allows
    selecton of a document using the selector before revealing the document
    viewer or editor.

    \ingroup qtopiaemb
    \sa QImageDocumentSelector
*/
/*!
    \enum QDocumentSelector::Option
    Options for configuring a QDocumentSelector.

    \value None No special configuration options.
    \value NewDocument The first item in the content list selects a new document.
    \value TypeSelector A 'Select Type' menu item is available in the context menu which allows the user to restrict the visible content types.
    \value NestTypes The type selector widget common mime types together as a single type. Individual types are accessible through a sub menu.
    \value ContextMenu The selector has a context menu.
 */

/*!
    \enum QDocumentSelector::SortMode

    This enum specifies the sort order of the documents.
    \value Alphabetical
    \value ReverseAlphabetical
    \value Chronological
    \value ReverseChronological
 */

/*!
    \enum QDocumentSelector::Selection

    This enum indicates the result of diplaying a document selector dialog.

    \value NewSelected The user selected the new document option.
    \value DocumentSelected The user selected an existing document.
    \value Cancelled The user cancelled the dialog.
*/

/*!
    \typedef QDocumentSelector::Options

    Synonym for \c QFlags<QDocumentSelector::Option>
 */

/*!
    Constructs a new content selector widget with the given \a parent.
 */
QDocumentSelector::QDocumentSelector( QWidget *parent )
    : QWidget( parent )
{
    QVBoxLayout *layout = new QVBoxLayout( this );

    d = new QDocumentSelectorPrivate( this );

    layout->setMargin( 0 );
    layout->setSpacing( 0 );

    layout->addWidget( d );

    connect( d, SIGNAL(documentSelected(const QContent&)), this, SIGNAL(documentSelected(const QContent&)) );
    connect( d, SIGNAL(newSelected()), this, SIGNAL(newSelected()) );
    connect( d, SIGNAL(documentsChanged()), this, SIGNAL(documentsChanged()) );

    setFocusProxy( d );
}

/*!
    Destroys the selector.
 */
QDocumentSelector::~QDocumentSelector()
{
}

/*!
    Returns the filter which defines the subset of content on the device the user can select from.

    \sa QContentSet::filter(), setFilter()
 */
QContentFilter QDocumentSelector::filter() const
{
    return d->baseFilter();
}

/*!
    Sets the \a filter which defines the subset of content on the device the user can select from.

    \sa filter()
 */
void QDocumentSelector::setFilter( const QContentFilter &filter )
{
    d->setBaseFilter( filter );
}

/*!
  Sets the document sort mode to \a mode.

  The default mode is QDocumentSelector::Alphabetical.

  \sa sortMode()
 */
void QDocumentSelector::setSortMode( SortMode mode )
{
    d->setSortMode( mode );
}

/*!
  Returns the current document sort mode.

  \sa setSortMode()
 */
QDocumentSelector::SortMode QDocumentSelector::sortMode() const
{
    return d->sortMode();
}

/*!
    Returns the enabled document selector options.
 */
QDocumentSelector::Options QDocumentSelector::options() const
{
    return d->options();
}

/*!
    Sets the enabled selector \a options.
 */
void QDocumentSelector::setOptions( Options options )
{
    d->setOptions( options );
}

/*!
    Enables the document selector options \a options.
*/
void QDocumentSelector::enableOptions( Options options )
{
    d->setOptions( d->options() | options );
}

/*!
    Disables the document selector options \a options.
 */
void QDocumentSelector::disableOptions( Options options )
{
    d->setOptions( d->options() & ~options );
}

/*!
    Sets the \a categories selected by default in the document selector's category filter dialog.

    If a default category does not appear in the category filter dialog then the content of the document selector will
    not be filtered on that category.  If a default category is added to the list after the category selector has been
    initialized it will not be automatically selected.

    \sa defaultCategories()
*/
void QDocumentSelector::setDefaultCategories( const QStringList &categories )
{
    d->setDefaultCategories( categories );
}

/*!
    Returns the categories selected by default in the document selector's category filter dialog.

    \sa setDefaultCategories()
*/
QStringList QDocumentSelector::defaultCategories() const
{
    return d->defaultCategories();
}

/*!
    Sets the intended usage of the selected document.  If the document does not have the \a permission the document selector
    will attempt to activate before the selection succeeds.  If the document cannot be activated with that permission, it
    will not be selectable.

    If the permission is QDrmRights::InvalidPermission the default permission for the content is used.
*/
void QDocumentSelector::setSelectPermission( QDrmRights::Permission permission )
{
    d->setSelectPermission( permission );
}

/*!
    Returns the intended usage of the selected document.
*/
QDrmRights::Permission QDocumentSelector::selectPermission() const
{
    return d->selectPermission();
};

/*!
    Sets the \a permissions a document must have in order to be selectable in the document selector.

    Unlike the select permission if a document is missing a mandatory permission it is simply unselectable and can not
    be activated.

    \sa setSelectPermission()
*/
void QDocumentSelector::setMandatoryPermissions( QDrmRights::Permissions permissions )
{
    d->setMandatoryPermissions( permissions );
}

/*!
    Returns the permissions a document must have in order to be selectable in the document selector.
*/
QDrmRights::Permissions QDocumentSelector::mandatoryPermissions() const
{
    return d->mandatoryPermissions();
}

/*!
    Returns the content currently selected in the view.
 */
QContent QDocumentSelector::currentDocument() const
{
    return d->currentDocument();
}

/*!
    Returns true if the new document option is currently selected.
 */
bool QDocumentSelector::newCurrent() const
{
    return d->newCurrent();
}

/*!
    Returns the content set being displayed by the selector.
*/
const QContentSet &QDocumentSelector::documents() const
{
    return d->documents();
}

/*!
    \fn QDocumentSelector::documentSelected( const QContent &content )

    Signals that the user has selected the QContent \a content.
 */

/*!
    \fn QDocumentSelector::newSelected()

    Signals that the user selected the new document option.
 */

/*!
    \fn QDocumentSelector::documentsChanged()

    Signals that the documents visible in the selector have changed.
*/

/*!
    Displays a document selector dialog with the parent \a parent and window title \a title, configured with
    the standard selector options \a filter, \a options and \a sortMode.  Selecting a document will assign the document
    to \a content if it can be rendered with the given \a permission.
*/
QDocumentSelector::Selection QDocumentSelector::select( QWidget *parent, QContent *content, QDrmRights::Permission permission, const QString &title, const QContentFilter &filter, Options options, SortMode sortMode )
{
    QDocumentSelectorDialog dialog( parent );

    dialog.setWindowTitle( title );
    dialog.setFilter( filter );
    dialog.setSortMode( sortMode );
    dialog.setOptions( options );
    dialog.setSelectPermission( permission );

    if( QtopiaApplication::execDialog( &dialog ) == QDialog::Accepted )
    {
        if( dialog.newSelected() )
        {
            *content = QContent();
            return NewSelected;
        }
        else
        {
            *content = dialog.selectedDocument();
            return DocumentSelected;
        }
    }
    else
        return Cancelled;
}

/*!
    Displays a document selector dialog with the parent \a parent and window title \a title, configured with
    the standard selector options \a filter, \a options and \a sortMode.  Selecting a document will assign the
    document to \a content.
 */
QDocumentSelector::Selection QDocumentSelector::select( QWidget *parent, QContent *content, const QString &title, const QContentFilter &filter, Options options, SortMode sortMode )
{
    QDocumentSelectorDialog dialog( parent );

    dialog.setWindowTitle( title );
    dialog.setFilter( filter );
    dialog.setSortMode( sortMode );
    dialog.setOptions( options );

    if( QtopiaApplication::execDialog( &dialog ) == QDialog::Accepted )
    {
        if( dialog.newSelected() )
        {
            *content = QContent();

            return NewSelected;
        }
        else
        {
            *content = dialog.selectedDocument();

            return DocumentSelected;
        }
    }
    else
        return Cancelled;
}


class QDocumentSelectorDialogPrivate : public DocumentView
{
public:
    QDocumentSelectorDialogPrivate( QWidget *parent = 0 )
        : DocumentView( parent )
    {
    }

    virtual ~QDocumentSelectorDialogPrivate()
    {
    }
};

/*!
    \class QDocumentSelectorDialog
    \brief The QDocumentSelectorDialog widget allows the user to select documents from
            a list of documents available on the device.

    A QDocumentSelectorDialog is internally based on a QContentSet
    The QContent selected in the dialog is retrieved using the \c selectedDocument() method.
 */

/*!
    Constructs a new content selector dialog with the given \a parent.
 */
QDocumentSelectorDialog::QDocumentSelectorDialog( QWidget *parent )
    : QDialog( parent )
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    QtopiaApplication::setMenuLike( this, true );
#endif
    QVBoxLayout *layout = new QVBoxLayout( this );

    d = new QDocumentSelectorDialogPrivate( this );

    layout->setMargin( 0 );
    layout->setSpacing( 0 );

    layout->addWidget( d );

    connect( d, SIGNAL(documentSelected(const QContent&)), this, SLOT(accept()) );
    connect( d, SIGNAL(newSelected()), this, SLOT(accept()) );
}

/*!
    Destroys the dialog.
 */
QDocumentSelectorDialog::~QDocumentSelectorDialog()
{
}

/*!
    Returns the filter which defines the subset of content on the device the user can select from.

    \sa QContentSet::filter(), setFilter()
 */
QContentFilter QDocumentSelectorDialog::filter() const
{
    return d->baseFilter();
}

/*!
    Sets the \a filter which defines the subset of content on the device the user can select from.

    \sa filter()
 */
void QDocumentSelectorDialog::setFilter( const QContentFilter &filter )
{
    d->setBaseFilter( filter );
}

/*!
  Sets the document sort mode to \a mode.

  The default mode is QDocumentSelector::Alphabetical.

  \sa sortMode()
 */
void QDocumentSelectorDialog::setSortMode( QDocumentSelector::SortMode mode )
{
    d->setSortMode( mode );
}

/*!
  Returns the current document sort mode.

  \sa setSortMode()
 */
QDocumentSelector::SortMode QDocumentSelectorDialog::sortMode() const
{
    return d->sortMode();
}

/*!
    Returns the enabled document selector options.
 */
QDocumentSelector::Options QDocumentSelectorDialog::options() const
{
    return d->options();
}

/*!
    Sets the enabled selector \a options.
 */
void QDocumentSelectorDialog::setOptions( QDocumentSelector::Options options )
{
    d->setOptions( options );
}

/*!
    Enables the document selector options \a options.
 */
void QDocumentSelectorDialog::enableOptions( QDocumentSelector::Options options )
{
    d->setOptions( d->options() | options );
}

/*!
    Disables the document selector options \a options.
 */
void QDocumentSelectorDialog::disableOptions( QDocumentSelector::Options options )
{
    d->setOptions( d->options() & ~options );
}

/*!
    Sets the \a categories selected by default in the document selector's category filter dialog.

    If a default category does not appear in the category filter dialog then the content of the document selector will
    not be filtered on that category.  If a default category is added to the list after the category selector has been
    initialized it will not be automatically selected.

    \sa defaultCategories()
 */
void QDocumentSelectorDialog::setDefaultCategories( const QStringList &categories )
{
    d->setDefaultCategories( categories );
}

/*!
    Returns the categories selected by default in the document selector's category filter dialog.

    \sa setDefaultCategories()
 */
QStringList QDocumentSelectorDialog::defaultCategories() const
{
    return d->defaultCategories();
}

/*!
    Sets the intended usage of the selected document.  If the document does not have the \a permission the document selector
    will attempt to activate before the selection succeeds.  If the document cannot be activated with that permission, it
    will not be selectable.

    If the permission is QDrmRights::InvalidPermission the default permission for the content is used.
 */
void QDocumentSelectorDialog::setSelectPermission( QDrmRights::Permission permission )
{
    d->setSelectPermission( permission );
}

/*!
    Returns the intended usage of the selected document.
 */
QDrmRights::Permission QDocumentSelectorDialog::selectPermission() const
{
    return d->selectPermission();
};

/*!
    Sets the \a permissions a document must have in order to be selectable in the document selector.

    Unlike the select permission if a document is missing a mandatory permission it is simply unselectable and can not
    be activated.

    \sa setSelectPermission()
 */
void QDocumentSelectorDialog::setMandatoryPermissions( QDrmRights::Permissions permissions )
{
    d->setMandatoryPermissions( permissions );
}

/*!
    Returns the permissions a document must have in order to be selectable in the document selector.
 */
QDrmRights::Permissions QDocumentSelectorDialog::mandatoryPermissions() const
{
    return d->mandatoryPermissions();
}

/*!
    Returns the content item selected.  If the user selected to create a new document this will be a
    null QContent.
 */
QContent QDocumentSelectorDialog::selectedDocument() const
{
    return d->currentDocument();
}

/*!
    Returns true if the user selected the new document option.
 */
bool QDocumentSelectorDialog::newSelected() const
{
    return d->newCurrent();
}

/*!
    Returns the content set being displayed by the selector.
 */
const QContentSet &QDocumentSelectorDialog::documents() const
{
    return d->documents();
}

#include "qdocumentselector.moc"

