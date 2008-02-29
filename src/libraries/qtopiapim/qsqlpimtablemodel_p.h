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

#ifndef QSQLPIMTABLEMODEL_PRIVATE_H
#define QSQLPIMTABLEMODEL_PRIVATE_H

#include <qcategorymanager.h>
#include <quniqueid.h>

#include <QCache>
#include <QSet>

class QFile;
class QTimer;

/* read only */
class QSqlPimTableModel {
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

private:
    void invalidateQueries();
    void invalidateCache();

    QString sortColumn() const;

    QCategoryFilter cCatFilter;
    QSet<int> mContextFilter;
    bool mExcludeContexts;
    QStringList mFilters;
    QStringList mJoins;
    QStringList mOrderBy;

    static const int rowStep;

    mutable int cachedCount;
    mutable QCache<int, QUniqueId> cachedIndexes;
    mutable QCache<int, QUniqueId> cachedKeys;

    const QString tableText;

    // join parts
    const QString catUnfiledText;
    const QString catSelectedText;

    // used only for the view now.
    QString selectText(const QStringList & = QStringList()) const;
    QString selectText(const QString &, const QStringList & = QStringList()) const;
};

#endif // QSQLPIMTABLEMODEL_PRIVATE_H
