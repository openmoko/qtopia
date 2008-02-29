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
#ifndef QDOCUMENTSERVERCONTENTSETENGINE_P_H
#define QDOCUMENTSERVERCONTENTSETENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtopia/private/qcontentsetengine_p.h>
#include <QCache>

class QDocumentServerContentStore;
class QDocumentServerContentStorePrivate;

class QDocumentServerContentSetEngine : public QContentSetEngine
{
    Q_OBJECT
public:
    QDocumentServerContentSetEngine( const QContentFilter &filter, const QContentSortCriteria &sortOrder, QContentSet::UpdateMode mode, int setId, QDocumentServerContentStore *store );

    virtual ~QDocumentServerContentSetEngine();

    virtual QContent content( int index ) const;
    virtual int count() const;
    virtual bool isEmpty() const;

    virtual void insertContent( const QContent &content );
    virtual void removeContent( const QContent &content );

    virtual void clear();

    virtual bool contains( const QContent &content ) const;

    void insertContent( int start, int end );
    void removeContent( int start, int end );
    void refreshContent( int start, int end );
    void emitContentChanged( const QContentIdList &contentIds, QContent::ChangeType change );
    void emitContentChanged();
    void emitReset();

signals:
    void releaseContentSet( int setId );

protected:
    virtual void filterChanged( const QContentFilter &filter );
    virtual void sortCriteriaChanged( const QContentSortCriteria &sort );

private slots:
    void performRefresh();

private:
    virtual QContentList values( int index, int count );
    virtual int valueCount() const;

    void refresh( bool reset );

    int m_setId;

    QDocumentServerContentStore *m_store;

    int m_count;
    bool m_refreshPending;
    bool m_filterDirty;
    bool m_sortDirty;

    friend class QDocumentServerContentStorePrivate;
};

#endif
