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
QPreparedSqlQuery::QPreparedSqlQuery()
    : mQuery(0), mHash(0)
{
}

// +1, hash being different from statement means not prepared.
QPreparedSqlQuery::QPreparedSqlQuery(const QString &statement)
    : mQuery(0), mHash(qHash(statement)+1), mStatement(statement)
{
}

QPreparedSqlQuery::~QPreparedSqlQuery()
{
    if (mQuery)
        delete mQuery;
}

bool QPreparedSqlQuery::isValid() const
{
    return mQuery && mHash == qHash(mStatement);
}

bool QPreparedSqlQuery::prepare()
{
    // there is a concern that a prepared statement might not survive a schema change.
    // relevant SQLite doc can be found by searching for PRAGMA schema_version and
    // SQLITE_SCHEMA error.

    // these next two lines don't properly leave the query in a prepared state.
    if (isValid()) {
        //mQuery->setForwardOnly(false);
        first();
        //mQuery->setForwardOnly(true);
        return true;
    }

    buildQuery();
    if (mQuery->prepare(mStatement)) {
        mHash = qHash(mStatement);
        return true;
    }
    return false;
}

bool QPreparedSqlQuery::prepare(const QString &statement)
{
    buildQuery();
    if (isValid() && qHash(statement) == mHash && statement == mStatement) {
        reset();
        return true;
    }

    mStatement = statement;
    mHash = qHash(statement)+1;
    return prepare();
}

void QPreparedSqlQuery::clear()
{
    if (isValid())
        mHash++;
    if (mQuery)
        mQuery->clear();
}

void QPreparedSqlQuery::reset()
{
    // works for sqlite.
    last();
}

void QPreparedSqlQuery::buildQuery()
{
    if (!mQuery) {
        mQuery = new QSqlQuery();
        //mQuery->setForwardOnly(true);
    }
}

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
  \ingroup pim
  \brief The QPimSqlIO class provides generalized access to QPimRecord based
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
    selectCategoriesText(concat("SELECT count(*) FROM ", categoryTable, " WHERE recid = :i")),
    deleteCategoriesText(concat("DELETE FROM ", categoryTable, " WHERE recid = :i")),
    insertCategoriesText(concat("INSERT INTO ", categoryTable, " (recid, categoryid) VALUES (:i, :v)")),
    // not done for extra tables, unknown number of.
    deleteRecordText(concat("DELETE FROM ", table, " WHERE recid = :i")),

    retrieveCategoriesQuery(concat("SELECT categoryid FROM ", categoryTable, " WHERE recid=:id")),
    retrieveCustomQuery(concat("SELECT fieldname, fieldvalue FROM ", customTable, " WHERE recid = :i")),
    changeLogInsert("INSERT INTO changelog (recid, created, modified) VALUES (:id, :ct, :mt)"),
    changeLogUpdate("UPDATE changelog SET modified = :ls, removed = NULL WHERE recid = :id"),
    changeLogQuery("SELECT recid FROM changelog WHERE recid = :r"),
    addRecordQuery(concat("INSERT INTO ", table, insertText)),
    contextQuery(concat("SELECT context FROM ", table, " WHERE recid = :i"))

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
  Removes the list of records identified by \a ids.  Returns true if successful,
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

    addRecordQuery.prepare();

    addRecordQuery.bindValue(":i", ub);
    addRecordQuery.bindValue(":context", context);

    bindFields(record, *addRecordQuery);

    QtopiaSql::logQuery(*addRecordQuery);
    if (!addRecordQuery.exec()) {
        qWarning("failed main table insert: %s", (const char *)addRecordQuery.lastError().text().toLocal8Bit());
        if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
        return QUniqueId();
    }

    addRecordQuery.reset();

    // custom
    QMap<QString, QString> cmap = record.customFields();
    if (cmap.count()) {
        QSqlQuery q;
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
        QSqlQuery q;
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

    changeLogQuery.prepare();
    changeLogQuery.bindValue(":r", ub);
    changeLogQuery.exec();
    if (changeLogQuery.next()) {
        changeLogQuery.reset();
        changeLogUpdate.prepare();
        if (!changeLogUpdate.isValid())
            qWarning("Failed to prepare query %s: %s",
                    changeLogUpdate.lastQuery().toLocal8Bit().constData(), changeLogUpdate.lastError().text().toLocal8Bit().constData());
        QDateTime syncTime = mSyncTime.isNull() ? QTimeZone::current().toUtc(QDateTime::currentDateTime()) : mSyncTime;
        changeLogUpdate.bindValue(":ls", syncTime);
        changeLogUpdate.bindValue(":id", ub);
        if (!changeLogUpdate.exec()) {
            qWarning("Failed to update record sync time: %s",
                    changeLogUpdate.lastError().text().toLocal8Bit().constData());
            if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
            changeLogUpdate.reset();
            return QUniqueId();
        }
        changeLogUpdate.reset();
    } else {
        changeLogQuery.reset();
        changeLogInsert.prepare();
        if (!changeLogInsert.isValid())
            qWarning("Failed to prepare query %s: %s",
                    changeLogInsert.lastQuery().toLocal8Bit().constData(), changeLogInsert.lastError().text().toLocal8Bit().constData());
        QDateTime syncTime = mSyncTime.isNull() ? QTimeZone::current().toUtc(QDateTime::currentDateTime()) : mSyncTime;
        changeLogInsert.bindValue(":id", ub);
        changeLogInsert.bindValue(":ct", syncTime);
        changeLogInsert.bindValue(":mt", syncTime);
        if (!changeLogInsert.exec()) {
            qWarning("Failed to insert record sync time: %s",
                    changeLogInsert.lastError().text().toLocal8Bit().constData());
            if (mSyncTime.isNull()) QSqlDatabase::database().rollback();
            changeLogInsert.reset();
            return QUniqueId();
        }
        changeLogInsert.reset();
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
int QPimSqlIO::row(const QUniqueId & id) const
{
    return model.row(id);
}

/*!
  \internal
  Returns true if the record with identifier \a id is contained in the
  filtered set of records. Otherwise returns false.
*/
bool QPimSqlIO::contains(const QUniqueId & id) const
{
    return model.contains(id);
}

/*!
  \internal
  Returns the context for the record with identifier \a id.
*/
int QPimSqlIO::context(const QUniqueId &id) const
{
    const QLocalUniqueId &lid = (const QLocalUniqueId &)id;
    contextQuery.prepare();

    contextQuery.bindValue(":i", lid.toByteArray());

    int rv = -1;
    
    if (contextQuery.exec())
        if (contextQuery.next()) 
            rv = contextQuery.value(0).toInt();

    contextQuery.reset();
    return rv;
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
    qLog(Sql) << "Read categories";

    retrieveCategoriesQuery.prepare();
    retrieveCustomQuery.prepare();
    if (!retrieveCategoriesQuery.isValid() || !retrieveCustomQuery.isValid())
        return;

    retrieveCategoriesQuery.bindValue(":id", id);
    if (!retrieveCategoriesQuery.exec())
        qWarning("select categoryid failed: %s", (const char *)retrieveCategoriesQuery.lastError().text().toLocal8Bit());

    QList<QString> tlist;
    while(retrieveCategoriesQuery.next())
        tlist.append(retrieveCategoriesQuery.value(0).toString());

    record.setCategories(tlist);

    retrieveCategoriesQuery.reset();

    retrieveCustomQuery.bindValue(":id", id);
    if (!retrieveCustomQuery.exec())
        qWarning("select custom field failed: %s", (const char *)retrieveCustomQuery.lastError().text().toLocal8Bit());

    QMap<QString, QString> cmap;
    while(retrieveCustomQuery.next())
        cmap.insert(retrieveCustomQuery.value(0).toString(), retrieveCustomQuery.value(1).toString());
    record.setCustomFields(cmap);

    retrieveCustomQuery.reset();

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
  Perform any additional updates on any additional tables.  Return true if successful.
*/
bool QPimSqlIO::updateExtraTables(const QByteArray &, const QPimRecord &) { return true; }
/*!
  \internal
  Perform any additional inserts on any additional tables.  Return true if successful.
*/
bool QPimSqlIO::insertExtraTables(const QByteArray &, const QPimRecord &) { return true; }
/*!
  \internal
  Perform any additional removes on any additional tables.  Return true if successful.
*/
bool QPimSqlIO::removeExtraTables(const QByteArray &) { return true; }

int QPimSqlIO::sourceContext(const QPimSource &source)
{
    QUuid id = source.context;
    QString name = source.identity;

    static QPreparedSqlQuery sourceContextQuery;
    sourceContextQuery.prepare("SELECT condensedid FROM sqlsources WHERE contextid = :id AND subsource = :source");

    sourceContextQuery.bindValue(":id", id.toString());
    sourceContextQuery.bindValue(":source", name);

    QtopiaSql::logQuery(*sourceContextQuery);

    if (!sourceContextQuery.exec()) {
        qWarning("Failed source context id lookup: %s", (const char *)sourceContextQuery.lastError().text().toLocal8Bit());
        return -1;
    }

    if (sourceContextQuery.next()) {
        int rv = sourceContextQuery.value(0).toInt();
        sourceContextQuery.last();
        return rv;
    }
    sourceContextQuery.reset();

    QSqlQuery q;
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
