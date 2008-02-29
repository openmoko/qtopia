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
#include <qtopia/private/qdocumentservercontentsetengine_p.h>
#include <qtopia/private/qdocumentservercontentstore_p.h>
#include <QTimer>
#include <QtDebug>


/*!
    \class QDocumentServerContentSetEngine
    \mainclass
    \brief The QDocumentServerContentSetEngine is an implementation of the QContentSetEngine interface for content stored in QDocumentServerContentStore.

    \internal
 */

/*!
    Constructs a new sql content set engine for content in the given content \a store with the filtering
    criteria \a filter, and sort order \a order.
 */
QDocumentServerContentSetEngine::QDocumentServerContentSetEngine( const QContentFilter &filter, const QContentSortCriteria &order, QContentSet::UpdateMode mode, int setId, QDocumentServerContentStore *store )
    : QContentSetEngine( filter, order, mode )
    , m_setId( setId )
    , m_store( store )
    , m_count( 0 )
    , m_refreshPending( false )
    , m_filterDirty( true )
    , m_sortDirty( true )
{
    refresh( false );
}

QDocumentServerContentSetEngine::~QDocumentServerContentSetEngine()
{
    emit releaseContentSet( m_setId );
}

QContent QDocumentServerContentSetEngine::content( int index ) const
{
    if( m_refreshPending )
        const_cast< QDocumentServerContentSetEngine * >( this )->performRefresh();

    return QContentSetEngine::content( index );
}

/*!
    \reimp
 */
int QDocumentServerContentSetEngine::count() const
{
    if( m_refreshPending )
        const_cast< QDocumentServerContentSetEngine * >( this )->performRefresh();

    return m_count;
}

/*!
    \reimp
 */
bool QDocumentServerContentSetEngine::isEmpty() const
{
    return count() == 0;
}

/*!
    \reimp
 */
void QDocumentServerContentSetEngine::filterChanged( const QContentFilter &filter )
{
    Q_UNUSED( filter );

    m_filterDirty = true;

    refresh( true );
}

/*!
    \reimp
 */
void QDocumentServerContentSetEngine::sortCriteriaChanged( const QContentSortCriteria &order )
{
    Q_UNUSED( order );

    m_sortDirty = true;

    refresh( true );
}

/*!
    \reimp
 */
void QDocumentServerContentSetEngine::insertContent( const QContent &content )
{
    m_store->insertContentIntoSet( m_setId, content );

    refresh( false );
}

/*!
    \reimp
 */
void QDocumentServerContentSetEngine::removeContent( const QContent &content )
{
    m_store->removeContentFromSet( m_setId, content );

    refresh( false );
}

/*!
    \reimp
 */
void QDocumentServerContentSetEngine::clear()
{
    setFilter( QContentFilter() );
    setSortCriteria( QContentSortCriteria() );

    if( updateMode() == QContentSet::Synchronous )
    {
        m_count = 0;
        clearCache();

        emit reset();
    }
}

/*!
    \reimp
 */
bool QDocumentServerContentSetEngine::contains( const QContent &content ) const
{
    return m_store->contentSetContains( m_setId, content );
}

void QDocumentServerContentSetEngine::insertContent( int start, int end )
{
    emit contentAboutToBeInserted( start, end );

    int count = end - start + 1;

    insertRange( start, count );

    m_count += count;

    emit contentInserted();
}

void QDocumentServerContentSetEngine::removeContent( int start, int end )
{
    emit contentAboutToBeRemoved( start, end );

    int count = end - start + 1;

    removeRange( start, count );

    m_count -= count;

    emit contentRemoved();
}

void QDocumentServerContentSetEngine::refreshContent( int start, int end )
{
    int count = end - start + 1;

    refreshRange( start, count );

    emit contentChanged( start, end );
}

void QDocumentServerContentSetEngine::emitContentChanged( const QContentIdList &contentIds, QContent::ChangeType change )
{
    emit contentChanged( contentIds, change );
}

void QDocumentServerContentSetEngine::emitContentChanged()
{
    emit contentChanged();
}

void QDocumentServerContentSetEngine::emitReset()
{
    m_count = 0;
    clearCache();

    m_refreshPending = updateMode() == QContentSet::Synchronous;

    emit reset();
}

void QDocumentServerContentSetEngine::performRefresh()
{
    if( m_refreshPending )
    {
        m_refreshPending = false;

        if( m_filterDirty && m_sortDirty )
            m_store->setContentSetCriteria( m_setId, filter(), sortCriteria() );
        else if( m_filterDirty )
            m_store->setContentSetFilter( m_setId, filter() );
        else if( m_sortDirty )
            m_store->setContentSetSortOrder( m_setId, sortCriteria() );

        if( updateMode() == QContentSet::Synchronous )
        {
            m_count = m_store->contentSetCount( m_setId );
            clearCache();
        }
    }
}

/*!
    Requeries the content of the set.
 */
void QDocumentServerContentSetEngine::refresh( bool reset )
{
    if( !m_refreshPending )
    {
        m_refreshPending = true;

        QTimer::singleShot( 0, this, SLOT(performRefresh()) );
    }

    if( reset && updateMode() == QContentSet::Synchronous )
    {
        emit this->reset();
    }
}

QContentList QDocumentServerContentSetEngine::values( int index, int count )
{
    return m_store->contentSetFrame( m_setId, index, count );
}

int QDocumentServerContentSetEngine::valueCount() const
{
    return m_count;
}
