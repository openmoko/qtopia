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

#ifndef QSQLPIMTABLEMODEL_PRIVATE_H
#define QSQLPIMTABLEMODEL_PRIVATE_H

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

#include "qpreparedquery_p.h"

#include <qcategorymanager.h>
#include <quniqueid.h>

#include <QCache>
#include <QSet>
#include <QBasicTimer>

class QFile;
class QTimer;
class QPreparedSqlQuery;

class QPimQueryCache
{
public:
    QPimQueryCache() {}
    virtual ~QPimQueryCache() {}

    virtual void setMaxCost(int) = 0;

    virtual QString fields() const = 0;
    virtual void cacheRow(int row, const QPreparedSqlQuery &q) = 0;
    virtual void clear() = 0;
};

/* read only */
class QSqlPimTableModel : public QObject {
    Q_OBJECT
public:
    QSqlPimTableModel(const QString &table, const QString &categoryTable);
    virtual ~QSqlPimTableModel();

    void setCategoryFilter(const QCategoryFilter &f);
    QCategoryFilter categoryFilter() const;

    void setContextFilter(const QSet<int> &, bool);
    QSet<int> contextFilter() const;
    bool contextFilterExcludes() const;

    QUniqueId recordId(int row) const;

    QList<QVariant> orderKey(int row) const
    { return orderKey(recordId(row)); }

    QList<QVariant> orderKey(const QUniqueId &) const;

    int row(const QUniqueId & tid) const;
    bool contains(const QUniqueId &id) const;
    int predictedRow(const QList<QVariant> &k) const;
    int count() const;

    void setFilter(const QString &filter)
    {
        QStringList sl;
        sl << filter;
        setFilters(sl);
    }
    void setFilters(const QStringList &);
    QStringList filters() const { return mFilters; }

    void setJoins(const QStringList &);
    QStringList joins() const { return mJoins; }

    void setOrderBy(const QStringList &);
    QStringList orderBy() const { return mOrderBy; }

    void reset();

    QString selectText(const QStringList & = QStringList()) const;
    QString selectText(const QString &, const QStringList & = QStringList(), const QStringList & = QStringList()) const;

    void setSimpleQueryCache(QPimQueryCache *c);

protected:
    void timerEvent(QTimerEvent *event);
private:
    /* gets the row you want, returns, schedules remainder for loop */
    void buildCache(int) const;
    void cacheRows(QPreparedSqlQuery &, int, int, int, uint = 0) const;
    void prepareRowQueries() const;

    void invalidateQueries();
    void invalidateCache();

    QString sortColumn() const;

    QCategoryFilter cCatFilter;
    QSet<int> mContextFilter;
    bool mExcludeContexts;
    QStringList mFilters;
    QStringList mJoins;
    QStringList mOrderBy;

    mutable int cachedCount;
    mutable QCache<int, QUniqueId> cachedIndexes;
    mutable QCache<int, QVariant> cachedKeyValues;
    mutable QCache<int, QUniqueId> cachedKeys;
    mutable QPimQueryCache *mSimpleCache;

    const QString tableText;

    // join parts
    const QString catUnfiledText;
    const QString catSelectedText;

    mutable QPreparedSqlQuery idByRowQuery;
    mutable QPreparedSqlQuery idByJumpedRowQuery;
    mutable bool idByRowValid;

    mutable QBasicTimer cacheTimer;
    mutable int cacheRow;
    mutable int cacheTimerTarget;
    mutable int lastCachedRow;

};

#endif // QSQLPIMTABLEMODEL_PRIVATE_H
