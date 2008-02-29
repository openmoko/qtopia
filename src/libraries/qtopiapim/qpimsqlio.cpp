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

#include "qpimsqlio_p.h"
#include "qannotator_p.h"
#include "qpimsource.h"

#include <qtopiasql.h>
#include <qtimezone.h>
#include <qtopialog.h>
#include <QSqlResult>
#include <QSqlError>
#include <QVariant>
#include <QTimer>
#include <QDebug>

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
  \mainclass
  \module qpepim
  \ingroup pim
  \brief The QPimSqlIO class provides generalized access to QPimRecord based
  sql tables.
*/

/*!
  \fn void QPimSqlIO::bindFields(const QPimRecord &r, QPreparedSqlQuery &q) const = 0
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
    model(table, categoryTable), idGenerator(scope),
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
    retrieveCustomQuery(concat("SELECT fieldname, fieldvalue FROM ", customTable, " WHERE recid = :id")),
    changeLogInsert("INSERT INTO changelog (recid, context, created, modified) VALUES (:id, :context, :ct, :mt)"),
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

static QSqlDatabase *pimdb = 0;

/*!
  Returns the database object specific to the pim sql data.
*/
QSqlDatabase QPimSqlIO::database()
{
    if (!pimdb)
        pimdb = new QSqlDatabase( QtopiaSql::instance()->systemDatabase() );
    if (!pimdb->isOpen())
        qWarning() << "Failed to open pim database";
    return *pimdb;
}

/*!
  \internal
  Updates the table rows for record \a r to the values in record \a r.
  */
bool QPimSqlIO::updateRecord(const QPimRecord& r)
{
    if (mSyncTime.isNull()) database().transaction();
    QUniqueId uid = r.uid();

    // update last_modified as well, unless change tracking is off.

    {
        QPreparedSqlQuery q;
        q.prepare(updateRecordText);

        q.bindValue(":i", uid.toUInt());
        bindFields(r, q);

        if (!q.exec()) {
            if (mSyncTime.isNull()) database().rollback();
            return false;
        }

        // custom
        QMap<QString, QString> cmap = r.customFields();
        // could do a check to see whats there or not, or ... XXX
        q.prepare(selectCustomText);
        q.bindValue(":i", uid.toUInt());
        if(!q.exec())
        {
            if (mSyncTime.isNull()) database().rollback();
            return false;
        }

        if(q.next() && q.value(0).toInt() > 0) {
            q.prepare(deleteCustomText);
            q.bindValue(":i", uid.toUInt());
            if (!q.exec()) {
                if (mSyncTime.isNull()) database().rollback();
                return false;
            }
        }

        if (cmap.count()) {
            q.prepare(insertCustomText);
            q.bindValue(":i", uid.toUInt());
            QMap<QString, QString>::ConstIterator it;
            for (it = cmap.begin(); it != cmap.end(); ++it) {
                q.bindValue(":n", it.key());
                q.bindValue(":v", it.value());
                if (!q.exec()) {
                    if (mSyncTime.isNull()) database().rollback();
                    return false;
                }
            }
        }

        QSet<QString> cats = r.categories().toSet();
        q.prepare(selectCategoriesText);
        q.bindValue(":i", uid.toUInt());
        q.exec();
        if(q.next() && q.value(0).toInt() > 0) {
            q.prepare(deleteCategoriesText);
            q.bindValue(":i", uid.toUInt());
            if (!q.exec()) {
                if (mSyncTime.isNull()) database().rollback();
                return false;
            }
        }
        if (cats.count()) {
            q.prepare(insertCategoriesText);
            q.bindValue(":i", uid.toUInt());
            foreach(QString v, cats) {
                q.bindValue(":v", v);
                if (!q.exec()) {
                    if (mSyncTime.isNull()) database().rollback();
                    return false;
                }
            }
        }

        if (!updateExtraTables(uid.toUInt(), r)) {
            qWarning("failed to update extra tables: %s", (const char *)database().lastError().text().toLocal8Bit());
            if (mSyncTime.isNull()) database().rollback();
            return false;
        }

        q.prepare("UPDATE changelog SET modified = :ls WHERE recid = :id");
        QDateTime syncTime = mSyncTime.isNull() ? QTimeZone::current().toUtc(QDateTime::currentDateTime()) : mSyncTime;
        q.bindValue(":ls", syncTime);
        q.bindValue(":id", uid.toUInt());
        if (!q.exec()) {
            if (mSyncTime.isNull()) database().rollback();
            return false;
        }
    }
    if (mSyncTime.isNull() && !database().commit()) {
        qWarning("Could not commit update of record: %s", (const char *)database().lastError().text().toLocal8Bit());
        database().rollback();
        return false;
    }

    /* insert notes */
    QAnnotator bag;
    if (r.notes().simplified().isEmpty()) {
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
    if (mSyncTime.isNull()) database().transaction();

    if (!removeExtraTables(id.toUInt())) {
        if (mSyncTime.isNull()) database().rollback();
        qWarning ("Failed to remove extra tables for %s", id.toString().toLatin1().constData());
        return false;
    }
    QPreparedSqlQuery q(database());

    q.prepare(deleteCustomText);
    q.bindValue(":i", id.toUInt());
    if (!q.exec()) {
        qWarning("failed clean up custom fields: %s", (const char *)q.lastError().text().toLocal8Bit());
        if (mSyncTime.isNull()) database().rollback();
        return false;
    }

    q.prepare(deleteRecordText);
    q.bindValue(":i", id.toUInt());
    if( !q.exec()) {
        if (mSyncTime.isNull()) database().rollback();
        return false;
    }

    q.prepare("UPDATE changelog SET removed = :ls WHERE recid = :id");
    QDateTime syncTime = mSyncTime.isNull() ? QTimeZone::current().toUtc(QDateTime::currentDateTime()) : mSyncTime;
    q.bindValue(":ls", syncTime);
    q.bindValue(":id", id.toUInt());
    if (!q.exec()) {
        if (mSyncTime.isNull()) database().rollback();
        return false;
    }

    if (mSyncTime.isNull() && !database().commit()) {
        qWarning("Could not commit removal of record: %s", (const char *)database().lastError().text().toLocal8Bit());
        database().rollback();
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
    if (mSyncTime.isNull()) database().transaction();

    foreach(QUniqueId id, ids) {
        if (!removeRecord(id)) {
            if (mSyncTime.isNull()) database().rollback();
            return false;
        }
    }
    if (mSyncTime.isNull() && !database().commit()) {
        database().rollback();
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
    if (mSyncTime.isNull()) database().transaction();

    // update created and last_modified as well, unless change tracking is off.

    QUniqueId u;
    if (createuid || record.uid().isNull())
        u = idGenerator.createUniqueId();
    else
        u = record.uid();

    addRecordQuery.prepare();

    addRecordQuery.bindValue(":i", u.toUInt());
    addRecordQuery.bindValue(":context", context);

    bindFields(record, addRecordQuery);

    if (!addRecordQuery.exec()) {
        if (mSyncTime.isNull()) database().rollback();
        return QUniqueId();
    }

    addRecordQuery.reset();

    // custom
    QMap<QString, QString> cmap = record.customFields();
    if (cmap.count()) {
        QPreparedSqlQuery q(database());
        q.prepare(insertCustomText);
        q.bindValue(":i", u.toUInt());
        QMap<QString, QString>::ConstIterator it;
        for (it = cmap.begin(); it != cmap.end(); ++it) {
            q.bindValue(":n", it.key());
            q.bindValue(":v", it.value());
            if (!q.exec()) {
                if (mSyncTime.isNull()) database().rollback();
                return QUniqueId();
            }
        }
    }

    QSet<QString> cats = record.categories().toSet();
    if (cats.count()) {
        QPreparedSqlQuery q(database());
        q.prepare(insertCategoriesText);
        q.bindValue(":i", u.toUInt());
        foreach(QString v, cats) {
            q.bindValue(":v", v);
            if (!q.exec()) {
                if (mSyncTime.isNull()) database().rollback();
                return QUniqueId();
            }
        }
    }

    if (!insertExtraTables(u.toUInt(), record))
    {
        if (mSyncTime.isNull()) database().rollback();
        return QUniqueId();
    }

    changeLogQuery.prepare();
    changeLogQuery.bindValue(":r", u.toUInt());
    changeLogQuery.exec();
    if (changeLogQuery.next()) {
        changeLogQuery.reset();
        changeLogUpdate.prepare();
        QDateTime syncTime = mSyncTime.isNull() ? QTimeZone::current().toUtc(QDateTime::currentDateTime()) : mSyncTime;
        changeLogUpdate.bindValue(":ls", syncTime);
        changeLogUpdate.bindValue(":id", u.toUInt());
        if (!changeLogUpdate.exec()) {
            if (mSyncTime.isNull()) database().rollback();
            changeLogUpdate.reset();
            return QUniqueId();
        }
        changeLogUpdate.reset();
    } else {
        changeLogQuery.reset();
        changeLogInsert.prepare();
        QDateTime syncTime = mSyncTime.isNull() ? QTimeZone::current().toUtc(QDateTime::currentDateTime()) : mSyncTime;
        changeLogInsert.bindValue(":id", u.toUInt());
        changeLogInsert.bindValue(":context", context);
        changeLogInsert.bindValue(":ct", syncTime);
        changeLogInsert.bindValue(":mt", syncTime);
        if (!changeLogInsert.exec()) {
            if (mSyncTime.isNull()) database().rollback();
            changeLogInsert.reset();
            return QUniqueId();
        }
        changeLogInsert.reset();
    }

    if (mSyncTime.isNull() && !database().commit()) {
        qWarning("failed to commit: %s", (const char *)database().lastError().text().toLocal8Bit());
        database().rollback();
        return QUniqueId();
    }

    /* insert notes */
    QAnnotator bag;
    if (record.notes().simplified().isEmpty()) {
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
    contextQuery.prepare();

    contextQuery.bindValue(":i", id.toUInt());

    int rv = -1;
    
    if (contextQuery.exec() && contextQuery.next()) 
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
void QPimSqlIO::retrieveRecord(uint id, QPimRecord &record) const
{
    retrieveCategoriesQuery.prepare();
    retrieveCustomQuery.prepare();

    retrieveCategoriesQuery.bindValue(":id", id);
    retrieveCategoriesQuery.exec();

    QList<QString> tlist;
    while(retrieveCategoriesQuery.next())
        tlist.append(retrieveCategoriesQuery.value(0).toString());

    record.setCategories(tlist);

    retrieveCategoriesQuery.reset();

    retrieveCustomQuery.bindValue(":id", id);
    retrieveCustomQuery.exec();

    QMap<QString, QString> cmap;
    while(retrieveCustomQuery.next())
        cmap.insert(retrieveCustomQuery.value(0).toString(), retrieveCustomQuery.value(1).toString());
    record.setCustomFields(cmap);

    retrieveCustomQuery.reset();

    QAnnotator bag;

    QUniqueId lid(QUniqueId::fromUInt(id));

    QByteArray ba = bag.blob(lid);
    if (ba.size() > 0) {
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
bool QPimSqlIO::updateExtraTables(uint, const QPimRecord &) { return true; }
/*!
  \internal
  Perform any additional inserts on any additional tables.  Return true if successful.
*/
bool QPimSqlIO::insertExtraTables(uint, const QPimRecord &) { return true; }
/*!
  \internal
  Perform any additional removes on any additional tables.  Return true if successful.
*/
bool QPimSqlIO::removeExtraTables(uint) { return true; }

int QPimSqlIO::sourceContext(const QPimSource &source)
{
    QUuid id = source.context;
    QString name = source.identity;

    static QPreparedSqlQuery sourceContextQuery;
    sourceContextQuery.prepare("SELECT condensedid FROM sqlsources WHERE contextid = :id AND subsource = :source");

    sourceContextQuery.bindValue(":id", id.toString());
    sourceContextQuery.bindValue(":source", name);

    if (!sourceContextQuery.exec()) {
        return -1;
    }

    if (sourceContextQuery.next()) {
        int rv = sourceContextQuery.value(0).toInt();
        sourceContextQuery.reset();
        return rv;
    }
    sourceContextQuery.reset();

    QPreparedSqlQuery q(database());
    q.prepare("INSERT INTO sqlsources (contextid, subsource) VALUES (:id, :source)");
    q.bindValue(":id", id.toString());
    q.bindValue(":source", name);
    q.exec();
    return sourceContext(source);

}

QDateTime QPimSqlIO::lastSyncTime(const QPimSource &source)
{
    QPreparedSqlQuery q(database());
    q.prepare("SELECT last_sync FROM sqlsources WHERE contextid = :id AND subsource = :source");
    q.bindValue(":id", source.context.toString());
    q.bindValue(":source", source.identity);
    q.exec();
    if (q.next() && !q.value(0).isNull())
        return q.value(0).toDateTime();
    return QDateTime();
}

bool QPimSqlIO::setLastSyncTime(const QPimSource &source, const QDateTime &time)
{
    QPreparedSqlQuery q(database());
    q.prepare("UPDATE sqlsources SET last_sync = :ls WHERE contextid = :id AND subsource = :source");
    q.bindValue(":id", source.context.toString());
    q.bindValue(":source", source.identity);
    q.bindValue(":ls", time);
    return q.exec();
}

bool QPimSqlIO::startSync(const QPimSource &source, const QDateTime &syncTime)
{
    QSet<QPimSource> sources;
    sources.insert(source);
    return startSync(sources, syncTime);
}

static QString contextString(const QSet<QPimSource> &list)
{
    QString contextset;
    bool first = true;
    foreach(QPimSource source, list) {
        int c = QPimSqlIO::sourceContext(source);
        if (c == -1)
            continue;
        if (first)
            first = false;
        else
            contextset.append(QLatin1String(", "));
        contextset.append(QString::number(QPimSqlIO::sourceContext(source)));
    }
    return contextset;
}

bool QPimSqlIO::startSync(const QSet<QPimSource> &sources, const QDateTime &syncTime)
{
    QString contextset = contextString(sources);
    Q_ASSERT(syncTime.isValid());
    if (database().transaction()) {
        QString querytext = QLatin1String("UPDATE sqlsources SET last_sync = :ls WHERE condensedid in (");
        querytext.append(contextset);
        querytext.append(QLatin1String(")"));
        QPreparedSqlQuery query(database());
        query.prepare(querytext);
        query.bindValue(":ls", syncTime);
        if (!query.exec()) {
            database().rollback();
            return false;
        }
        mSyncTime = syncTime;
    }
    return true;
}

bool QPimSqlIO::abortSync()
{
    mSyncTime = QDateTime();
    return database().rollback();
}

bool QPimSqlIO::commitSync()
{
    if (database().commit()) {
        mSyncTime = QDateTime();
        return true;
    }
    return false;
}

QList<QUniqueId> QPimSqlIO::removed(const QSet<QPimSource> &sources, const QDateTime &timestamp) const
{
    QList<QUniqueId> result;
    if (!timestamp.isValid())
        return result;
    QString contextset = contextString(sources);
    QPreparedSqlQuery q(database());
    q.prepare("SELECT recid FROM changelog WHERE removed >= :ts AND CREATED < :ts2 AND context IN ("
            + contextset + ")");
    q.bindValue(":ts", timestamp);
    q.bindValue(":ts2", timestamp);
    q.exec();
    while(q.next()) {
        result.append(QUniqueId::fromUInt(q.value(0).toInt()));
    }
    return result;
}

QList<QUniqueId> QPimSqlIO::added(const QSet<QPimSource> &sources, const QDateTime &timestamp) const
{
    QList<QUniqueId> result;
    QString contextset = contextString(sources);
    QPreparedSqlQuery q(database());
    if(timestamp.isValid()) {
        q.prepare("SELECT recid FROM changelog WHERE created >= :ts AND removed IS NULL AND context IN ("
                + contextset + ")");
        q.bindValue(":ts", timestamp);
    } else {
        q.prepare("SELECT recid FROM changelog WHERE removed IS NULL AND context IN ("
                + contextset + ")");
        
    }
    q.exec();
    while(q.next()) {
        result.append(QUniqueId::fromUInt(q.value(0).toInt()));
    }
    return result;
}

QList<QUniqueId> QPimSqlIO::modified(const QSet<QPimSource> &sources, const QDateTime &timestamp) const
{
    QList<QUniqueId> result;
    if (!timestamp.isValid())
        return result;
    QString contextset = contextString(sources);
    QPreparedSqlQuery q(database());
    q.prepare("SELECT recid FROM changelog WHERE modified >= :ts AND created < :ts2 AND removed IS NULL AND context IN ("
            + contextset + ")");
    q.bindValue(":ts", timestamp);
    q.bindValue(":ts2", timestamp);
    q.exec();
    while(q.next()) {
        result.append(QUniqueId::fromUInt(q.value(0).toInt()));
    }
    return result;
}

