/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef PIMSQLIO_PRIVATE_H
#define PIMSQLIO_PRIVATE_H

#include <qcategorymanager.h>
#include <qtopia/pim/qpimrecord.h>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDateTime>

#include <QCache>

#include "qsqlpimtablemodel_p.h"

class QFile;
class QTimer;

class QPreparedSqlQuery
{
public:
    QPreparedSqlQuery();
    QPreparedSqlQuery(const QString &statement);
    ~QPreparedSqlQuery();

    QSqlQuery & operator*() const { return *mQuery; }
    QSqlQuery * operator->() const { return mQuery; }
    operator QSqlQuery*() const { return mQuery; }

    bool isValid() const;

    bool prepare();
    bool prepare(const QString &);
    void reset();
    void clear();

    // other wrapped calls to QSqlQuery
    void addBindValue( const QVariant & val, QSql::ParamType paramType = QSql::In )
    { mQuery->addBindValue(val, paramType); }
    int at() const
    { return mQuery->at(); }
    void bindValue( const QString & placeholder, const QVariant & val, QSql::ParamType paramType = QSql::In )
    { mQuery->bindValue(placeholder, val, paramType); }
    void bindValue( int pos, const QVariant & val, QSql::ParamType paramType = QSql::In )
    { mQuery->bindValue(pos, val, paramType); }
    QVariant boundValue( const QString & placeholder ) const
    { return mQuery->boundValue(placeholder); }
    QVariant boundValue( int pos ) const
    { return mQuery->boundValue(pos); }
    QMap<QString, QVariant> boundValues() const
    { return mQuery->boundValues(); }
    const QSqlDriver * driver() const
    { return mQuery->driver(); }
    bool exec()
    { return mQuery->exec(); }
    QString executedQuery() const
    { return mQuery->executedQuery(); }
    bool first()
    { return mQuery->first(); }
    bool last()
    { return mQuery->last(); }
    bool isActive() const
    { return mQuery->isActive(); }
    bool isForwardOnly() const
    { return mQuery->isForwardOnly(); }
    bool isNull( int field ) const
    { return mQuery->isNull(field); }
    bool isSelect() const
    { return mQuery->isSelect(); }

    QSqlError lastError() const
    { return mQuery->lastError(); }
    QVariant lastInsertId() const
    { return mQuery->lastInsertId(); }
    QString lastQuery() const
    { return mQuery->lastQuery(); }
    bool next()
    { return mQuery->next(); }
    int numRowsAffected() const
    { return mQuery->numRowsAffected(); }
    bool previous()
    { return mQuery->previous(); }
    QSqlRecord record() const
    { return mQuery->record(); }
    const QSqlResult * result() const
    { return mQuery->result(); }
    bool seek( int index, bool relative = false )
    { return mQuery->seek(index, relative); }
    int size() const
    { return mQuery->size(); }
    QVariant value( int index ) const
    { return mQuery->value(index); }

private:
    void buildQuery();
    QSqlQuery *mQuery;
    uint mHash;
    QString mStatement;
};

class QPimSource;
class QPimSqlIO {
public:
    QPimSqlIO(const QUuid &scope, const char *table, const char *catTable,
            const char *customTable, const char *updateText, const char *insertText);
    virtual ~QPimSqlIO();

    static int sourceContext(const QPimSource &);
    static QDateTime lastSyncTime(const QPimSource &);
    static bool setLastSyncTime(const QPimSource &, const QDateTime &);
    static QSqlDatabase database();

    int context(const QUniqueId &) const;

    // leave to let old code work
    bool startSync(const QPimSource &, const QDateTime &syncTime);

    bool startSync(const QSet<QPimSource> &sources, const QDateTime &syncTime);
    bool abortSync();
    bool commitSync();

    QList<QUniqueId> removed(const QSet<QPimSource> &sources, const QDateTime &) const;
    QList<QUniqueId> added(const QSet<QPimSource> &sources, const QDateTime &) const;
    QList<QUniqueId> modified(const QSet<QPimSource> &sources, const QDateTime &) const;

    enum ContextFilterType {
        ExcludeContexts,
        RestrictToContexts
    };
    QSet<int> contextFilter() const;
    ContextFilterType contextFilterType() const;

    virtual void invalidateCache();

protected:
    void setCategoryFilter(const QCategoryFilter &f);
    QCategoryFilter categoryFilter() const;

    QUniqueId recordId(int row) const;
    QList<QUniqueId> recordIds(const QList<int> &rows) const;
    int row(const QUniqueId & tid) const;
    bool contains(const QUniqueId & tid) const;
    int count() const;

    void setContextFilter(const QSet<int> &, ContextFilterType);

    /* convenience functions, change sets to invalidate cache? */
    QStringList orderBy() const { return model.orderBy(); }
    void setOrderBy(const QStringList &list) { model.setOrderBy(list); invalidateCache();}

    void setFilter(const QString &filter) { model.setFilter(filter); invalidateCache();}
    void setFilters(const QStringList &list) { model.setFilters(list); invalidateCache();}
    QStringList filters() const { return model.filters(); }

    void setJoins(const QStringList &list) { model.setJoins(list); invalidateCache();}
    QStringList joins() const { return model.joins(); }

    bool updateRecord(const QPimRecord& t);
    bool removeRecord(int row);
    bool removeRecord(const QUniqueId & id);
    bool removeRecords(const QList<QUniqueId> &ids);
    QUniqueId addRecord(const QPimRecord &, const QPimSource &, bool = true);
    QUniqueId addRecord(const QPimRecord &, int, bool = true);


protected:
    QSqlPimTableModel model;
    QDateTime mSyncTime;

    virtual bool updateExtraTables(uint, const QPimRecord &);
    virtual bool insertExtraTables(uint, const QPimRecord &);
    virtual bool removeExtraTables(uint);

    virtual void bindFields(const QPimRecord &r, QSqlQuery &) const = 0;

    void retrieveRecord(uint, QPimRecord &) const;

    QUniqueIdGenerator idGenerator;

    // yes, I do mean const.
    const QString tableText;
    const QString updateRecordText;
    const QString selectCustomText;
    const QString deleteCustomText;
    const QString insertCustomText;
    const QString selectCategoriesText;
    const QString deleteCategoriesText;
    const QString insertCategoriesText;
    // not done for extra tables, uknown number of.
    const QString deleteRecordText;
    const QString insertRecordText;

    mutable QPreparedSqlQuery retrieveCategoriesQuery;
    mutable QPreparedSqlQuery retrieveCustomQuery;
    mutable QPreparedSqlQuery changeLogInsert;
    mutable QPreparedSqlQuery changeLogUpdate;
    mutable QPreparedSqlQuery changeLogQuery;
    mutable QPreparedSqlQuery addRecordQuery;
    mutable QPreparedSqlQuery contextQuery;
};

#endif
