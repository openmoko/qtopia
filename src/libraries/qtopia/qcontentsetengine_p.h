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
#ifndef QCONTENTSETENGINE_P_H
#define QCONTENTSETENGINE_P_H

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

#include <QStringList>
#include <QContentFilter>
#include <QContentSortCriteria>
#include <QContentSet>
#include <qtopia/private/qsparselist_p.h>

class QStringList;
class QContent;

class QContentSetEngine : public QObject, protected QSparseList< QContent, 20, 5 >
{
    Q_OBJECT
public:
    QContentSetEngine( const QContentFilter &filter, const QContentSortCriteria &sort, QContentSet::UpdateMode mode )
        : m_filter( filter )
        , m_sortCriteria( sort )
        , m_updateMode( mode )
        , m_updateInProgress( false )
    {
    }

    virtual ~QContentSetEngine();

    virtual QContent content( int index ) const;
    virtual QContentId contentId( int index ) const;
    virtual int count() const;
    virtual bool isEmpty() const;

    void setSortCriteria( const QContentSortCriteria &sort );
    void setFilter( const QContentFilter &filter );

    void setSortOrder( const QStringList &sortOrder );

    virtual void insertContent( const QContent &content ) = 0;
    virtual void removeContent( const QContent &content ) = 0;

    virtual void clear() = 0;

    virtual bool contains( const QContent &content ) const = 0;

    QStringList sortOrder() const{ return m_sortOrder; }
    QContentSortCriteria sortCriteria() const{ return m_sortCriteria; }
    QContentFilter filter() const{ return m_filter; }

    QContentSet::UpdateMode updateMode() const{ return m_updateMode; }

    static QContentSortCriteria convertSortOrder( const QStringList &sortOrder );

    bool updateInProgress() const{ return m_updateInProgress; }

signals:
    void contentAboutToBeRemoved( int start, int end );
    void contentAboutToBeInserted( int start, int end );
    void contentInserted();
    void contentRemoved();
    void contentChanged( int start, int end );
    void contentChanged( const QContentIdList &ids, QContent::ChangeType type );
    void contentChanged();

    void updateStarted();
    void updateFinished();

    void reset();

protected:

    virtual void sortCriteriaChanged( const QContentSortCriteria &sort ) = 0;
    virtual void filterChanged( const QContentFilter &filter ) = 0;

protected slots:
    void startUpdate();
    void finishUpdate();

private:
    QContentFilter m_filter;
    QStringList m_sortOrder;
    QContentSortCriteria m_sortCriteria;
    QContentSet::UpdateMode m_updateMode;
    bool m_updateInProgress;
};

#endif
