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
#ifndef QSQLCONTENTSETENGINE_P_H
#define QSQLCONTENTSETENGINE_P_H

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
#include <QContent>
#include <QCache>
#include <QMutex>
#include <QWaitCondition>
#include <QVector>

class QSqlContentStore;
class QSqlDatabaseContentSet;
class QSqlContentSetIterator;
class QSqlContentSetUpdateThread;
class QSqlContentSetUpdateProxy;

class QSqlContentSetEngine : public QContentSetEngine
{
    Q_OBJECT
public:
    QSqlContentSetEngine( const QContentFilter &filter, const QContentSortCriteria &order, QContentSet::UpdateMode mode, QSqlContentStore *store );
    virtual ~QSqlContentSetEngine();
    virtual int count() const;

    virtual void insertContent( const QContent &content );
    virtual void removeContent( const QContent &content );

    virtual void clear();

    virtual bool contains( const QContent &content ) const;

signals:
    void performRefresh();

protected:

    virtual void filterChanged( const QContentFilter &filter );
    virtual void sortCriteriaChanged( const QContentSortCriteria &sort );

private slots:
    void performUpdate();
    void performReset();
    void refresh( bool reset = false );
    void contentChangedEvent( const QContentIdList &ids, QContent::ChangeType type );

    void updateInsert( int index, int count, int primaryIndex, int secondaryIndex );
    void updateRemove( int index, int count, int primaryIndex, int secondaryIndex );
    void updateRefresh( int index, int count );
    void updateFinish();

private:
    virtual int valueCount() const;
    virtual QList< QContent > values( int index, int count );
    int expectedIndexOf( const QContentSortCriteria &sort, const QContent &content ) const;
    QContent contentFromId( QContentId contentId ) const;
    QContent explicitContent( quint64 id ) const;

    bool update();

    int minimumSetIndex( const QContentSortCriteria &sort, const QVector< QContent > &content ) const;
    int maximumSetIndex( const QContentSortCriteria &sort, const QVector< QContent > &content ) const;

    void synchronizeSets( const QContentFilter &criteria, const QContentSortCriteria &sort, const QList< QPair< quint64, QContent > > &explicits, QSqlContentSetUpdateProxy *updateProxy );
    void synchronizeSingleSet( QSqlContentSetUpdateProxy *updateProxy );
    void synchronizeMultipleSets( const QContentSortCriteria &sort, const QList< QContentIdList > &contentIds, const QList< QPair< quint64, QContent > > &explicits, QSqlContentSetUpdateProxy *updateProxy );

    QContentIdList sortIds( const QContentSortCriteria &sort, const QList< QContentIdList > &contentIdLists, const QList< QPair< quint64, QContent > > &explicits ) const;

    QSqlContentStore *m_store;

    QList< QContentId > m_primaryIds;
    QList< QContentId > m_secondaryIds;
    QList< QPair< quint64, QContent > > m_explicit;
    QMutex m_databaseSetMutex;
    QMutex m_syncMutex;
    QWaitCondition m_syncCondition;
    QSqlContentSetUpdateThread *m_updateThread;

    int m_count;
    int m_primaryOffset;
    int m_secondaryCutoff;

    bool m_refreshPending;
    bool m_resetPending;
    bool m_deletePending;

    quint64 m_explicitIdSource;

    friend class QSqlContentSetUpdateThread;
};

#endif
