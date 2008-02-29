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

#include "qpimsqlio_p.h"
#include "qannotator_p.h"
#include "qpimsource.h"

#include <qtopiasql.h>
#include <qtimezone.h>
#include <qtopialog.h>
#include <QSqlResult>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QTimer>
#include <QDebug>

//#define USE_TEMPORARY_TABLE

QString concat(const char *a, const char *b, const char *c, const char *d = 0, const char *e = 0){
    QString s;
    s += a;
    s += b;
    s += c;
    s += d;
    s += e;
    return s;
}

/*
  \internal
  \class QPimSqlIO
  \module qpepim
  \ingroup qpepim
  \brief The QPimSqlIO class provides generalized access to pimrecord based
  sql tables.
*/

/*!
  \fn void QPimSqlIO::bindFields(const QPimRecord &r, QSqlQuery &q) const = 0
  \internal

  This function is called to bind fields in the query \a q to those
  in the record \a r.

  The record \a r will be the same object passed to updateRecord().
*/

/*!
  \internal
  Constructs a PimSqlObject with context scope \a context,
  primary table \a table, \a categoryTable, customFields table, text for
  \a updateText and text for \a insertText.

  The string fields are used to generate a set of cached sql statements.
*/
QPimSqlIO::QPimSqlIO(const QUuid &scope, const char *table, const char *categoryTable,
        const char *customTable, const char *updateText, const char *insertText) :
    model(table, categoryTable), appScope(scope),
    tableText(table),
    updateRecordText(concat("UPDATE ", table, " SET ", updateText, " WHERE recid = :i")),
    selectCustomText(concat("SELECT count(*) FROM ", customTable, " WHERE recid = :i")),
    deleteCustomText(concat("DELETE FROM ", customTable, " WHERE recid = :i")),
    insertCustomText(concat("INSERT INTO ", customTable, " (recid, fieldname, fieldvalue) VALUES (:i, :n, :v)")),
    retrieveCustomText(concat("SELECT fieldname, fieldvalue FROM ", customTable, " WHERE recid = :i")),
    selectCategoriesText(concat("SELECT count(*) FROM ", categoryTable, " WHERE recid = :i")),
    deleteCategoriesText(concat("DELETE FROM ", categoryTable, " WHERE recid = :i")),
    insertCategoriesText(concat("INSERT INTO ", categoryTable, " (recid, categoryid) VALUES (:i, :v)")),
    retrieveCategoriesText(concat("SELECT categoryid FROM ", categoryTable, " WHERE recid=:id")),
    // not done for extra tables, uknown number of.
    deleteRecordText(concat("DELETE FROM ", table, " WHERE recid = :i")),
    insertRecordText(concat("INSERT INTO ", table, insertText))
{
}

/*!
  \internal
  Destroys the QPimSqlIO object.
*/
QPimSqlIO::~QPimSqlIO()
{
}

/*!
  \internal
  Updates the table rows for record \a r to the values in record \a r.
  */
bool QPimSqlIO::updateRecord(const QPimRecord& r)
{
    if (mSyncTime.isNull()) QSqlDatabase::database().transaction();
    QLocalUniqueId uid = r.uid();
    QByteArray uiddata = uid.toByteArray();

    // update last_modified as well, unless change tracking is off.

    {
        QSqlQuery q;
        q.prepare(updateRecordText);

        q.bindValue(":i", uiddata);
        bindFields(r, q);

        QtopiaSql::logQuery(q);
        if (!q.exec()) {
            qWarning("failed main table update: %s", (const char *)q.lastError().text().toLocal8Bit());
            if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
            return false;
        }

        // custom
        QMap<QString, QString> cmap = r.customFields();
        // could do a check to see whats there or not, or ... XXX
        q.prepare(selectCustomText);
        q.bindValue(":i", uiddata);
        QtopiaSql::logQuery(q);
        if(!q.exec())
        {
            qWarning("selectCustomText failed: %s", (const char *)q.lastError().text().toLocal8Bit());
            if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
            return false;
        }

        if(q.next() && q.value(0).toInt() > 0) {
            q.prepare(deleteCustomText);
            q.bindValue(":i", uiddata);
            QtopiaSql::logQuery(q);
            if (!q.exec()) {
                qWarning("failed clean up custom fields: %s", (const char *)q.lastError().text().toLocal8Bit());
                if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
                return false;
            }
        }

        if (cmap.count()) {
            q.prepare(insertCustomText);
            q.bindValue(":i", uiddata);
            QMap<QString, QString>::ConstIterator it;
            for (it = cmap.begin(); it != cmap.end(); ++it) {
                q.bindValue(":n", it.key());
                q.bindValue(":v", it.value());
                QtopiaSql::logQuery(q);
                if (!q.exec()) {
                    qWarning("failed custom insert: %s", (const char *)q.lastError().text().toLocal8Bit());
                    if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
                    return false;
                }
            }
        }

        QList<QString> cats = r.categories();
        q.prepare(selectCategoriesText);
        q.bindValue(":i", uiddata);
        q.exec();
        if(q.next() && q.value(0).toInt() > 0) {
            q.prepare(deleteCategoriesText);
            q.bindValue(":i", uiddata);
            QtopiaSql::logQuery(q);
            if (!q.exec()) {
                qWarning("failed clean up: %s", (const char *)q.lastError().text().toLocal8Bit());
                if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
                return false;
            }
        }
        if (cats.count()) {
            q.prepare(insertCategoriesText);
            q.bindValue(":i", uiddata);
            foreach(QString v, cats) {
                q.bindValue(":v", v);
                QtopiaSql::logQuery(q);
                if (!q.exec()) {
                    qWarning("failed category table insert: %s", (const char *)q.lastError().text().toLocal8Bit());
                    if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
                    return false;
                }
            }
        }

        if (!updateExtraTables(uiddata, r)) {
            qWarning("failed to update extra tables: %s", (const char *)QSqlDatabase::database().lastError().text().toLocal8Bit());
            if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
            return false;
        }

        if (!q.prepare("UPDATE changelog SET modified = :ls WHERE recid = :id"))
            qWarning("Failed to prepare query %s: %s",
                    q.lastQuery().toLocal8Bit().constData(), q.lastError().text().toLocal8Bit().constData());
        QDateTime syncTime = mSyncTime.isNull() ? QTimeZone::current().toUtc(QDateTime::currentDateTime()) : mSyncTime;
        q.bindValue(":ls", syncTime);
        q.bindValue(":id", uiddata);
        if (!q.exec()) {
            qWarning("Failed to update record sync time: %s",
                    q.lastError().text().toLocal8Bit().constData());
            if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
            return false;
        }
    }
    if (mSyncTime.isNull() && !QSqlDatabase::database().commit()) {
        qWarning("Could not commit update of record: %s", (const char *)QSqlDatabase::database().lastError().text().toLocal8Bit());
        QSqlDatabase::database().rollback();
        return false;
    }

    /* insert notes */
    QAnnotator bag;
    if (r.notes().simplified().isEmpty() && bag.contains(uid)) {
        bag.remove(uid);
    } else {
        QByteArray ba;
        {
            QDataStream ts(&ba, QIODevice::WriteOnly);
            ts << r.notes();
        }
        bag.set(uid, ba, "text/html");
    }

    invalidateCache();
    return true;

}

/*!
  \internal
  Remove record at \a row in the sorted query.
*/
bool QPimSqlIO::removeRecord(int row)
{
    // may be better way... but not needed.  Doesn't need to be
    // efficient.
    return removeRecord(recordId(row));
}

/*!
  \internal
  Remove record identified by \a id from the tables.
*/
bool QPimSqlIO::removeRecord(const QUniqueId & id)
{
    if (mSyncTime.isNull()) QSqlDatabase::database().transaction();

    const QLocalUniqueId &lid = (const QLocalUniqueId &)id;
    QByteArray uiddata = lid.toByteArray();

    if (!removeExtraTables(uiddata)) {
        if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
        qWarning ("Failed to remove extra tables for %s", lid.toString().toLatin1().constData());
        return false;
    }
    QSqlQuery q;

    q.prepare(deleteCustomText);
    q.bindValue(":i", uiddata);
    QtopiaSql::logQuery(q);
    if (!q.exec()) {
        qWarning("failed clean up custom fields: %s", (const char *)q.lastError().text().toLocal8Bit());
        if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
        return false;
    }

    q.prepare(deleteRecordText);
    q.bindValue(":i", uiddata);
    QtopiaSql::logQuery(q);
    if( !q.exec()) {
        qWarning("failed to delete record: %s", (const char *)q.lastError().text().toLocal8Bit());
        if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
        return false;
    }

    if (!q.prepare("UPDATE changelog SET removed = :ls WHERE recid = :id"))
        qWarning("Failed to prepare query %s: %s",
                q.lastQuery().toLocal8Bit().constData(), q.lastError().text().toLocal8Bit().constData());
    QDateTime syncTime = mSyncTime.isNull() ? QTimeZone::current().toUtc(QDateTime::currentDateTime()) : mSyncTime;
    q.bindValue(":ls", syncTime);
    q.bindValue(":id", uiddata);
    if (!q.exec()) {
        qWarning("Failed to update record remove time: %s",
                q.lastError().text().toLocal8Bit().constData());
        if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
        return false;
    }

    if (mSyncTime.isNull() && !QSqlDatabase::database().commit()) {
        qWarning("Could not commit removal of record: %s", (const char *)QSqlDatabase::database().lastError().text().toLocal8Bit());
        QSqlDatabase::database().rollback();
        return false;
    }

    /* remove notes */
    QAnnotator bag;
    bag.remove(id);

    invalidateCache();
    return true;
}

/*!
  Returns the list of identifiers for records in the list of \a rows.
  This function should not be used for large lists.
*/
QList<QUniqueId> QPimSqlIO::recordIds(const QList<int> &rows) const
{
    QList<QUniqueId> ids;
    foreach(int r, rows) {
        QUniqueId i = recordId(r);
        if (!i.isNull())
            ids.append(i);
    }
    return ids;
}

/*!
  \internal
  Removes the list of records identified by \a ids.  Returns true if sucessful,
  otherwise returns false.
*/
bool QPimSqlIO::removeRecords(const QList<QUniqueId> &ids)
{
    if (mSyncTime.isNull()) QSqlDatabase::database().transaction();

    foreach(QUniqueId id, ids) {
        if (!removeRecord(id)) {
            if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
            return false;
        }
    }
    if (mSyncTime.isNull() && !QSqlDatabase::database().commit()) {
        QSqlDatabase::database().rollback();
        return false;
    }
    return true;
}

/*!
  \internal
  Adds the \a record to the appropriate sql tables.
*/
QUniqueId QPimSqlIO::addRecord(const QPimRecord &record, const QPimSource &source, bool createuid)
{
    int context = sourceContext(source);
    if (context < 1)
        return QUniqueId();
    return addRecord(record, context, createuid);
}

QUniqueId QPimSqlIO::addRecord(const QPimRecord &record, int context, bool createuid)
{
    if (mSyncTime.isNull()) QSqlDatabase::database().transaction();

    // update created and last_modified as well, unless change tracking is off.

    QUniqueIdGenerator g(appScope); // later, same scop method as xml
    QLocalUniqueId u;
    if (createuid || record.uid().isNull())
        u = g.createUniqueId();
    else
        u = record.uid();
    QByteArray ub = u.toByteArray();

    QSqlQuery q;
    q.prepare(insertRecordText);

    q.bindValue(":i", ub);
    q.bindValue(":context", context);

    bindFields(record, q);

    QtopiaSql::logQuery(q);
    if (!q.exec()) {
        qWarning("failed main table insert: %s", (const char *)q.lastError().text().toLocal8Bit());
        if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
        return QUniqueId();
    }

    // custom
    QMap<QString, QString> cmap = record.customFields();
    if (cmap.count()) {
        q.prepare(insertCustomText);
        q.bindValue(":i", ub);
        QMap<QString, QString>::ConstIterator it;
        for (it = cmap.begin(); it != cmap.end(); ++it) {
            q.bindValue(":n", it.key());
            q.bindValue(":v", it.value());
            QtopiaSql::logQuery(q);
            if (!q.exec()) {
                qWarning("failed custom insert: %s", (const char *)q.lastError().text().toLocal8Bit());
                if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
                return QUniqueId();
            }
        }
    }

    QList<QString> cats = record.categories();
    if (cats.count()) {
        q.prepare(insertCategoriesText);
        q.bindValue(":i", ub);
        foreach(QString v, cats) {
            q.bindValue(":v", v);
            QtopiaSql::logQuery(q);
            if (!q.exec()) {
                qWarning("failed category table insert: %s", (const char *)q.lastError().text().toLocal8Bit());
                if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
                return QUniqueId();
            }
        }
    }

    insertExtraTables(ub, record);

    q.prepare("SELECT recid FROM changelog WHERE recid = :r");
    q.bindValue(":r", ub);
    q.exec();
    if (q.next()) {
        if (!q.prepare("UPDATE changelog SET modified = :ls, removed = NULL WHERE recid = :id"))
            qWarning("Failed to prepare query %s: %s",
                    q.lastQuery().toLocal8Bit().constData(), q.lastError().text().toLocal8Bit().constData());
        QDateTime syncTime = mSyncTime.isNull() ? QTimeZone::current().toUtc(QDateTime::currentDateTime()) : mSyncTime;
        q.bindValue(":ls", syncTime);
        q.bindValue(":id", ub);
        if (!q.exec()) {
            qWarning("Failed to update record sync time: %s",
                    q.lastError().text().toLocal8Bit().constData());
            if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
            return QUniqueId();
        }
    } else {
        if (!q.prepare("INSERT INTO changelog (recid, created, modified) VALUES (:id, :ct, :mt)"))
            qWarning("Failed to prepare query %s: %s",
                    q.lastQuery().toLocal8Bit().constData(), q.lastError().text().toLocal8Bit().constData());
        QDateTime syncTime = mSyncTime.isNull() ? QTimeZone::current().toUtc(QDateTime::currentDateTime()) : mSyncTime;
        q.bindValue(":id", ub);
        q.bindValue(":ct", syncTime);
        q.bindValue(":mt", syncTime);
        if (!q.exec()) {
            qWarning("Failed to insert record sync time: %s",
                    q.lastError().text().toLocal8Bit().constData());
            if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
            return QUniqueId();
        }
    }

    if (mSyncTime.isNull() && !QSqlDatabase::database().commit()) {
        qWarning("failed to commit: %s", (const char *)QSqlDatabase::database().lastError().text().toLocal8Bit());
        QSqlDatabase::database().rollback();
        return QUniqueId();
    }

    /* insert notes */
    QAnnotator bag;
    if (record.notes().simplified().isEmpty() && bag.contains(u)) {
        bag.remove(u);
    } else {
        QByteArray ba;
        {
            QDataStream ts(&ba, QIODevice::WriteOnly);
            ts << record.notes();
        }
        bag.set(u, ba, "text/html");
    }

    invalidateCache();
    return u;
}

/*!
  \internal
  Returns number of records shown in filtered view of data.
*/
int QPimSqlIO::count() const
{
    return model.count();
}

/*!
  \internal
  Sets the category filter used to filter records to \a f.
*/
void QPimSqlIO::setCategoryFilter(const QCategoryFilter &f)
{
    model.setCategoryFilter(f);
}

/*!
  \internal
  Returns the category filter used to filter records.
*/
QCategoryFilter QPimSqlIO::categoryFilter() const
{
    return model.categoryFilter();
}

void QPimSqlIO::setContextFilter(const QSet<int> &list, ContextFilterType type)
{
    model.setContextFilter(list, type == ExcludeContexts);
}

QSet<int> QPimSqlIO::contextFilter() const
{
    return model.contextFilter();
}

QPimSqlIO::ContextFilterType QPimSqlIO::contextFilterType() const
{
    return model.contextFilterExcludes() ? ExcludeContexts : RestrictToContexts;
}

/*!
  \internal
  Returns the identifier for the record at \a row in the filtered records.
*/
QUniqueId QPimSqlIO::recordId(int row) const
{
    return model.recordId(row);
}

/*!
  \internal
  Returns the row for the record with identifier \a id in the filtered records.
  returns -1 if the record isn't in the filtered records.
*/
int QPimSqlIO::row(const QUniqueId & tid) const
{
    return model.row(tid);
}

/*!
  \internal
  Returns the context for the record with identifier \a id.
*/
int QPimSqlIO::context(const QUniqueId &id) const
{
    const QLocalUniqueId &lid = (const QLocalUniqueId &)id;
    QSqlQuery q;
    q.prepare("SELECT context FROM " + tableText + " WHERE recid = :i");
    q.bindValue(":i", lid.toByteArray());
    if (q.exec() && q.next()) {
        return q.value(0).toInt();
    }
    return -1;
}

/*!
  \internal
  Invalidates cached data and queries.
*/
void QPimSqlIO::invalidateCache()
{
    model.reset();
}

/*!
  \internal
  Retrieves the category and custom filed information for the \a record
  identified by the \a id.
*/
void QPimSqlIO::retrieveRecord(const QByteArray &id, QPimRecord &record) const
{
    QSqlQuery q;
    qLog(Sql) << "Read categories";
    if (!q.prepare(retrieveCategoriesText))
        qWarning("Failed to prepare select categories: %s", q.lastError().text().toLocal8Bit().constData());
    q.bindValue(":id", id);
    if (!q.exec())
        qWarning("select categoryid failed: %s", (const char *)q.lastError().text().toLocal8Bit());

    QList<QString> tlist;
    while(q.next())
        tlist.append(q.value(0).toString());
    record.setCategories(tlist);

    if (!q.prepare(retrieveCustomText))
        qWarning("Failed to prepare select custom fields: %s", q.lastError().text().toLocal8Bit().constData());
    q.bindValue(":id", id);
    if (!q.exec())
        qWarning("select custom field failed: %s", (const char *)q.lastError().text().toLocal8Bit());

    QMap<QString, QString> cmap;
    while(q.next())
        cmap.insert(q.value(0).toString(), q.value(1).toString());
    record.setCustomFields(cmap);

    QAnnotator bag;
    QLocalUniqueId lid(id);
    if (bag.contains(lid)) {
        QByteArray ba = bag.blob(lid);
        QString n;
        {
            QDataStream ts(ba);
            ts >> n;
        }
        record.setNotes(n);
    }
}

/*!
  \internal
  Peform any additonal updates on any addtional tables.  Return true if successful.
*/
bool QPimSqlIO::updateExtraTables(const QByteArray &, const QPimRecord &) { return true; }
/*!
  \internal
  Peform any additonal inserts on any addtional tables.  Return true if successful.
*/
bool QPimSqlIO::insertExtraTables(const QByteArray &, const QPimRecord &) { return true; }
/*!
  \internal
  Peform any additonal removes on any addtional tables.  Return true if successful.
*/
bool QPimSqlIO::removeExtraTables(const QByteArray &) { return true; }


int QPimSqlIO::sourceContext(const QPimSource &source)
{
    QUuid id = source.context;
    QString name = source.identity;
    QSqlQuery q;
    if (!q.prepare("SELECT condensedid FROM sqlsources WHERE contextid = :id AND subsource = :source")) {
        qWarning("failed to prepare source context id lookup: %s",
                (const char *)q.lastError().text().toLocal8Bit());
        return -1;
    }
    q.bindValue(":id", id.toString());
    q.bindValue(":source", name);

    QtopiaSql::logQuery(q);
    if (!q.exec()) {
        qWarning("Failed source context id lookup: %s", (const char *)q.lastError().text().toLocal8Bit());
        return -1;
    }

    if (q.next())
        return q.value(0).toInt();

    if (!q.prepare("INSERT INTO sqlsources (contextid, subsource) VALUES (:id, :source)")) {
        qWarning("Failed to prepare source context id store: %s",
                (const char *)q.lastError().text().toLocal8Bit());
        return -1;
    }
    q.bindValue(":id", id.toString());
    q.bindValue(":source", name);
    if (!q.exec()) {
        qWarning("Failed source context id store: %s", (const char *)q.lastError().text().toLocal8Bit());
        return -1;
    }

    return sourceContext(source);

}

QDateTime QPimSqlIO::lastSyncTime(const QPimSource &source)
{
    QSqlQuery q;
    if (!q.prepare("SELECT last_sync FROM sqlsources WHERE contextid = :id AND subsource = :source")) {
        qWarning("Failed to prepare select last sync time: %s",
                (const char *)q.lastError().text().toLocal8Bit());
        return QDateTime();
}
    q.bindValue(":id", source.context.toString());
    q.bindValue(":source", source.identity);
    QtopiaSql::logQuery(q);
    if (!q.exec()) {
        qWarning("Failed to select last sync time: %s",
                (const char *)q.lastError().text().toLocal8Bit());
        return QDateTime();
    }
    if (q.next())
        return q.value(0).toDateTime();
    return QDateTime();
}

bool QPimSqlIO::setLastSyncTime(const QPimSource &source, const QDateTime &time)
{
    QSqlQuery q;
    if (!q.prepare("UPDATE sqlsources SET last_sync = :ls WHERE contextid = :id AND subsource = :source")) {
        qWarning("Failed to prepare update last sync time: %s",
                (const char *)q.lastError().text().toLocal8Bit());
        return false;
    }
    q.bindValue(":id", source.context.toString());
    q.bindValue(":source", source.identity);
    q.bindValue(":ls", time);
    QtopiaSql::logQuery(q);
    return q.exec();
}

bool QPimSqlIO::startSync(const QPimSource &source, const QDateTime &syncTime)
{
    Q_ASSERT(syncTime.isValid());
    if (QSqlDatabase::database().transaction()) {
        if (!QPimSqlIO::setLastSyncTime(source, syncTime)) {
            QSqlDatabase::database().rollback();
            return false;
        }
        mSyncTime = syncTime;
        return true;
    }
    return false;
}

bool QPimSqlIO::abortSync()
{
    mSyncTime = QDateTime();
    return QSqlDatabase::database().rollback();
}

bool QPimSqlIO::commitSync()
{
    if (QSqlDatabase::database().commit()) {
        mSyncTime = QDateTime();
        return true;
    }
    return false;
}
