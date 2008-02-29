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

#ifndef PIMSQLIO_PRIVATE_H
#define PIMSQLIO_PRIVATE_H

#include <qcategorymanager.h>
#include <qtopia/pim/qpimrecord.h>
#include <QSqlQuery>
#include <QDateTime>

#include <QCache>

#include "qsqlpimtablemodel_p.h"

class QFile;
class QTimer;

class QPimSource;
class QPimSqlIO {
public:
    QPimSqlIO(const QUuid &scope, const char *table, const char *catTable,
            const char *customTable, const char *updateText, const char *insertText);
    virtual ~QPimSqlIO();

    static int sourceContext(const QPimSource &);
    static QDateTime lastSyncTime(const QPimSource &);
    static bool setLastSyncTime(const QPimSource &, const QDateTime &);

    int context(const QUniqueId &) const;

    bool startSync(const QPimSource &, const QDateTime &syncTime);
    bool abortSync();
    bool commitSync();

    enum ContextFilterType {
        ExcludeContexts,
        RestrictToContexts
    };
    QSet<int> contextFilter() const;
    ContextFilterType contextFilterType() const;

protected:
    void setCategoryFilter(const QCategoryFilter &f);
    QCategoryFilter categoryFilter() const;

    QUniqueId recordId(int row) const;
    QList<QUniqueId> recordIds(const QList<int> &rows) const;
    int row(const QUniqueId & tid) const;
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

    virtual void invalidateCache();

    bool updateRecord(const QPimRecord& t);
    bool removeRecord(int row);
    bool removeRecord(const QUniqueId & id);
    bool removeRecords(const QList<QUniqueId> &ids);
    QUniqueId addRecord(const QPimRecord &, const QPimSource &, bool = true);
    QUniqueId addRecord(const QPimRecord &, int, bool = true);


protected:
    QSqlPimTableModel model;
    QDateTime mSyncTime;

    virtual bool updateExtraTables(const QByteArray &, const QPimRecord &);
    virtual bool insertExtraTables(const QByteArray &, const QPimRecord &);
    virtual bool removeExtraTables(const QByteArray &);

    virtual void bindFields(const QPimRecord &r, QSqlQuery &) const = 0;

    void retrieveRecord(const QByteArray &, QPimRecord &) const;

    bool updateNotes(const QByteArray &, const QString &);
    bool addNotes(const QByteArray &, const QString &);
    bool removeNotes(const QByteArray &);
    QString notes(const QByteArray &) const;

    // yes, I do mean const.
    const QUuid appScope;
    const QString tableText;
    const QString updateRecordText;
    const QString selectCustomText;
    const QString deleteCustomText;
    const QString insertCustomText;
    const QString retrieveCustomText;
    const QString selectCategoriesText;
    const QString deleteCategoriesText;
    const QString insertCategoriesText;
    const QString retrieveCategoriesText;
    // not done for extra tables, uknown number of.
    const QString deleteRecordText;
    const QString insertRecordText;
};

#endif
