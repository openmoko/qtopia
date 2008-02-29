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

#include "qsqlpimtablemodel_p.h"
#include <qtopialog.h>
#include <QSqlResult>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QTimer>
#include <QDebug>
#include <QVector>
#include <QtopiaSql>

#include "qpimsqlio_p.h"

/* there may be an argument to make this more dynamic */
const int QSqlPimTableModel::rowStep(25);

QSqlPimTableModel::QSqlPimTableModel(const QString &table, const QString &categoryTable)
    : cCatFilter(QCategoryFilter::All), mExcludeContexts(true), cachedCount(-1),
    cachedIndexes(2*rowStep), cachedKeys(100),
    tableText(table),
    catUnfiledText(" LEFT JOIN " + categoryTable + " AS cat ON t1.recid = cat.recid"),
    catSelectedText(categoryTable)
{
    mOrderBy << "recid";
}

QSqlPimTableModel::~QSqlPimTableModel()
{
}

QList<QVariant> QSqlPimTableModel::orderKey(const QUniqueId &id) const
{
    QSqlQuery keyQuery(QPimSqlIO::database());
    if (!keyQuery.prepare("SELECT " + sortColumn()
                + ", t1.recid FROM "
                + tableText + " AS t1 WHERE t1.recid = :id")) {
        qWarning("QSqlPimTableModel::orderKey() - Could not prepare query: %s",
                keyQuery.lastError().text().toLocal8Bit().constData());
        qLog(Sql) << keyQuery.lastQuery();
    }
    keyQuery.bindValue(":id", id.toUInt());

    QList<QVariant> list;
    QtopiaSql::logQuery(keyQuery);
    if (keyQuery.exec()) {
        if (keyQuery.next()) {
            for (int i = 0; i < mOrderBy.count() + 1; i++)
                list << keyQuery.value(i);
        }
    } else {
        qWarning("QSqlPimTableModel::orderKey() - Could not execute query: %s",
                keyQuery.lastError().text().toLocal8Bit().constData());
        qLog(Sql) << keyQuery.executedQuery();
    }

    return list;
}

QString QSqlPimTableModel::selectText(const QStringList &temporaryFilters) const
{
    return selectText("distinct t1.recid", temporaryFilters);
}

QString QSqlPimTableModel::selectText(const QString &retrieve, const QStringList &temporaryFilters) const
{
    /*
       select
            ( id | count(id) | * )
       from records as t1
            join records as t2 on (record = :id)
            where t1.key > t2.key etc.
            [, records as t2]

            -- multiple categories --
            inner join contactcategories as cat1 on (cat1.recid = t1.recid and cat1.categoryid='Personal')
            inner join contactcategories as cat2 on (cat2.recid = t1.recid and cat2.categoryid='Business')

            -- unfiled --
            | left join recordcategories as cat1 on t1.id = cat1.recid )]
            where cat.recid is null

       where

            [t1.sortcolumn <= t2.sortcolumn and t2.id = id and t1.id < id]
            [recid is null]
            [order by sortcolumn, id]
     */

    QString qtext = "SELECT " + retrieve + " from " + tableText + " as t1";

    QStringList j = joins();
    foreach(QString join, j) {
        if (join == "simcardidmap")
            qtext += " JOIN simcardidmap ON (t1.recid = simcardidmap.sqlid) ";
        else
            qtext += " JOIN " + join + " ON (t1.recid = " + join + ".recid) ";
    }

    /* THE JOINS */
    if (cCatFilter.acceptUnfiledOnly())
        qtext += catUnfiledText;
    else if (!cCatFilter.acceptAll()) {
        QStringList cats = cCatFilter.requiredCategories();
        QString cname("c%1");
        int cid = 1;
        foreach (QString ctj, cats) {
            qtext += " JOIN " + catSelectedText + " AS " + cname.arg(cid) + " ON (t1.recid = "
                + cname.arg(cid) + ".recid and "
                + cname.arg(cid) + ".categoryid='" + ctj + "') ";

            cid++;
        }
    }

    /* THE CONDITIONS */
    bool isFirstCondition = true;
    // sort out category filtering later
    if (cCatFilter.acceptUnfiledOnly()) {
        qtext += " WHERE cat.recid IS NULL";
        isFirstCondition = false;
    }

    /*
       Exclusion filter.  e.g. if its in the list, don't show it
       makes it easier for different contexts to exclude or include themselves,
       don't have to know what the entire set of numbers is.
     */
    if (!mContextFilter.isEmpty()) {
        if (isFirstCondition) {
            qtext += " WHERE"; // No tr
            isFirstCondition=false;
        } else {
            qtext += " AND"; // No tr
        }
        if (mExcludeContexts)
            qtext += " t1.context NOT IN (";
        else
            qtext += " t1.context IN (";
        bool first = true;
        foreach(int item, mContextFilter) {
            if (item >= 0) {
                if (!first) qtext += ", ";
                qtext += QString::number(item);
                first = false;
            }
        }
        qtext += ")";
    }

    QStringList ofs = filters() + temporaryFilters;
    foreach (QString of, ofs) {
        if (isFirstCondition) {
            qtext += " WHERE "; // No tr
            isFirstCondition=false;
        } else {
            qtext += " AND "; // No tr
        }
        qtext += of;
        qtext += " ";
    }

    return qtext;
}

/*!
  \internal
  Returns number of records shown in filtered view of data.
*/
int QSqlPimTableModel::count() const
{
    if (cachedCount != -1)
        return cachedCount;

    qLog(Sql) << " QSqlPimTable::count() - not cached";
    // may not be sqlite compatible...
    QSqlQuery countQuery(QPimSqlIO::database());
    countQuery.setForwardOnly(true);
    countQuery.prepare(selectText("count(distinct t1.recid)"));

    QtopiaSql::logQuery(countQuery);
    if (!countQuery.exec()) {
        qWarning("QSqlPimTableModel::count() - Could not execute query: %s",
                countQuery.lastError().text().toLocal8Bit().constData());
        qLog(Sql) << countQuery.executedQuery();
        return 0;
    }
    if (countQuery.next() && !countQuery.isNull(0)) {
        cachedCount = countQuery.value(0).toInt();
        qLog(Sql) << "QSqlPimTable::count() - result:" << cachedCount;
        return cachedCount;
    }
    qLog(Sql) << "QSqlPimTable::count() - failed";
    return 0;
}

/*!
  \internal
  Sets the category filter used to filter records to \a f.
  Records that are not accepted by the category filter \a f will not be shown.
*/
void QSqlPimTableModel::setCategoryFilter(const QCategoryFilter &f)
{
    if (cCatFilter != f) {
        invalidateQueries();
        cCatFilter = f;
    }
}

/*!
  \internal
  Returns the category filter used to filter records.
*/
QCategoryFilter QSqlPimTableModel::categoryFilter() const
{
    return cCatFilter;
}

/*!
  \internal
  Sets the context filter used to filter records to \a f.
  Records in the context list \a f will not be shown in the list of records.
*/
void QSqlPimTableModel::setContextFilter(const QSet<int> &f, bool b)
{
    if (mContextFilter != f || b != mExcludeContexts) {
        invalidateQueries();
        mContextFilter = f;
        mExcludeContexts = b;
    }
}

/*!
  \internal
  Returns the context filter used to filter records.
*/
QSet<int> QSqlPimTableModel::contextFilter() const
{
    return mContextFilter;
}

/*!
  \internal
  Returns whether the context filter excludes or restricts to set
*/
bool QSqlPimTableModel::contextFilterExcludes() const
{
    return mExcludeContexts;
}

/*!
  \internal
  Returns the identifier for the record at \a row in the filtered records.
*/
QUniqueId QSqlPimTableModel::recordId(int row) const
{
    qLog(Sql) << "QSqlPimTableModel::recordId(" << row << ")";
    if (cachedIndexes.contains(row))
        return *cachedIndexes[row];

    if (cachedCount != -1 && row >= cachedCount)
        return QUniqueId();

    qLog(Sql) << "QSqlPimTableModel::recordId() - not cached";

    int rowBlockStart = row / rowStep;
    int rowEnd = (rowBlockStart+1) * rowStep;


    QList<QVariant> keys;
    QStringList  keyJumpFilter;
    for (;rowBlockStart > 0; rowBlockStart--) {
        if (cachedKeys.contains(rowBlockStart)) {
            keys = orderKey(*cachedKeys[rowBlockStart]);
            QStringList sc = orderBy();
            QString k;
            QString filter;
            int i = 0;
            foreach(k, sc) {
                // If we have a NULL key, we have to construct a query
                // such that either other records have the same field as
                // NULL and the other fields less than (the sub sort is ok),
                // or that other fields have a non NULL key (which gets sorted
                // after a NULL key.
                if (!keys[i].isNull())
                    filter += "(t1." + k + " > ? or t1." + k + " = ? and ";
                else
                    filter += "(t1." + k + " IS NOT NULL or (t1." + k + " IS NULL and ";
                i++;
            }
            filter += "(t1.recid > ? or t1.recid = ?)";
            i = 0;
            foreach(k, sc) {
                if (!keys[i].isNull())
                    filter += ")";
                else
                    filter += "))";
                i++;
            }
            keyJumpFilter << filter;
            break;
        }
    }

    qLog(Sql) << "QSqlPimTableModel::recordId() - row block" << rowBlockStart;
    QString joinText;
    QSqlQuery idByRowQuery(QPimSqlIO::database());
    /* if adding a limit... needs to be row-rowBlockStart *rowStep  */
    QString sortColumnString = sortColumn();
    if (!idByRowQuery.prepare( selectText("distinct t1.recid", keyJumpFilter) +
            (!sortColumnString.isNull() ? " ORDER BY " + sortColumnString : QString::null) +
            ", t1.recid"))
    {
        qWarning("QSqlPimTableModel::recordId() - Could not prepare query: %s",
                idByRowQuery.lastError().text().toLocal8Bit().constData());
        qLog(Sql) << idByRowQuery.lastQuery();
        return QUniqueId();
    }

    if (rowBlockStart) {
        int pos = 0;
        foreach(QVariant v, keys) {
            if (!v.isNull()) {
                qLog(Sql) << "bind value" << v.toString();
                idByRowQuery.bindValue(pos++, v);
                idByRowQuery.bindValue(pos++, v);
            }
        }
    }

    qLog(Sql) << "QSqlPimTableModel::recordId() - statement: " << idByRowQuery.lastQuery();

    QtopiaSql::logQuery(idByRowQuery);
    if (!idByRowQuery.exec()) {
        qWarning("QSqlPimTableModel::recordId() - Could not execute query: %s",
                idByRowQuery.lastError().text().toLocal8Bit().constData());
        qLog(Sql) << idByRowQuery.executedQuery();
        return QUniqueId();
    }

    int jumpedRow = rowBlockStart * rowStep;
    while (idByRowQuery.next()) {
        qLog(Sql) << "QSqlPimTableModel::recordId() - check if should cache" << jumpedRow << rowEnd << rowStep;

        /* cache any block keys we come across */
        if (!(jumpedRow % rowStep) && !cachedKeys.contains(jumpedRow/rowStep)) {
            qLog(Sql) << "QSqlPimTableModel::recordId() - cache set point" << jumpedRow;
            cachedKeys.insert(jumpedRow/rowStep, new QUniqueId(QUniqueId::fromUInt(idByRowQuery.value(0).toUInt())));
        }

        if (jumpedRow >= rowEnd)
            break;

        if (jumpedRow > rowEnd-2*rowStep) {
            qLog(Sql) << "QSqlPimTableModel::recordId() - cache" << jumpedRow;
           cachedIndexes.insert(jumpedRow, new QUniqueId(QUniqueId::fromUInt(idByRowQuery.value(0).toUInt())));
        }
        ++jumpedRow;
    }
    if (cachedCount == -1 && !idByRowQuery.next())
        cachedCount = jumpedRow;

    if (cachedIndexes.contains(row)) {
        qLog(Sql) << "QSqlPimTable::recordId() - result:" << cachedIndexes[row]->toString();
        return *cachedIndexes[row];
    }
    qLog(Sql) << "QSqlPimTable::recordId() - no result, returning nil id";
    return QUniqueId();
}

QString QSqlPimTableModel::sortColumn() const
{
    return "t1." + orderBy().join(", t1.");
}

/*!
  \internal
  Returns the row for the record with identifier \a id in the filtered records.
  returns -1 if the record isn't in the filtered records.
*/
int QSqlPimTableModel::row(const QUniqueId & tid) const
{
    if (tid.isNull())
        return -1;

    QList<QVariant> keys = orderKey(tid);
    int r = predictedRow(keys)-1;
    if (r > -1 && recordId(r) == tid)
        return r;
    return -1;
}

bool QSqlPimTableModel::contains(const QUniqueId &id) const
{
    QStringList filter;
    filter << "t1.recid = :id";
    QSqlQuery q(QPimSqlIO::database());
    q.prepare(selectText(filter));
    q.bindValue(":id", id.toUInt());
    QtopiaSql::logQuery(q);
    q.exec();
    return q.next();
}

int QSqlPimTableModel::predictedRow(const QList<QVariant> &keys) const
{
    QStringList sc = orderBy();
    QString k;
    QString filter;
    int i = 0;

    // We have a dependence on the recid being the last value in keys, by using i below.
    if (keys.count() == 0)
        return -1;

    foreach(k, sc) {
        if (i >= keys.count())
            break;
        // For tasks, we often have NULL fields (these get sorted to the top in ASC order, bottom in DESC)
        // so if we have a non null, key, try to include NULL keys in the output as well as non null keys
        // that would come before this key.  If it is null, we only include NULL keys since non null keys
        // would never get sorted before this one.
        if (!keys[i].isNull())
            filter += "(t1." + k + " IS NULL or t1." + k + " < ? or t1." + k + " = ? and ";
        else
            filter += "(t1." + k + " IS NULL and ";
        i++;
    }
    filter += "(t1.recid < ? or t1.recid = ?)";
    i = 0;
    foreach(k, sc) {
        if (i >= keys.count())
            break;

        filter += ")";
        i++;
    }
    QStringList filters;
    filters << filter;
    QString querytext = selectText("count(distinct t1.recid)", filters);

    QSqlQuery rowByIdQuery(QPimSqlIO::database());
    if (!rowByIdQuery.prepare(querytext)) {
        qWarning("QSqlPimTableModel::row() - Could not prepare Query: %s", rowByIdQuery.lastError().text().toLocal8Bit().constData());
        qLog(Sql) << rowByIdQuery.lastQuery();
        return -1;
    }

    int pos = 0;
    foreach(QVariant v, keys) {
        if (!v.isNull()) {
            qLog(Sql) << "bind value" << v.toString();
            rowByIdQuery.bindValue(pos++, v);
            rowByIdQuery.bindValue(pos++, v);
        }
    }


    QtopiaSql::logQuery(rowByIdQuery);
    if (!rowByIdQuery.exec()) {
        qWarning("QSqlPimTableModel::row() - Could not execute Query: %s", rowByIdQuery.lastError().text().toLocal8Bit().constData());
        qLog(Sql) << rowByIdQuery.executedQuery();
        return -1;
    }

    if (!rowByIdQuery.next())
        return -1; // record doesn't exists
    //qLog(Sql) << "SqlIO::row" << rowByIdQuery->value(0).toInt();
    return rowByIdQuery.value(0).toInt();
}

void QSqlPimTableModel::reset()
{
    invalidateQueries();
}

void QSqlPimTableModel::invalidateCache()
{
    cachedCount = -1;
    cachedIndexes.clear();
    cachedKeys.clear();
}

void QSqlPimTableModel::invalidateQueries()
{
    invalidateCache();
}

void QSqlPimTableModel::setFilters(const QStringList &f)
{
    if (f == mFilters)
        return;
    mFilters = f;
    invalidateQueries();
}

void QSqlPimTableModel::setJoins(const QStringList &j)
{
    if (j == mJoins)
        return;
    mJoins = j;
    invalidateQueries();
}

void QSqlPimTableModel::setOrderBy(const QStringList &sortorder)
{
    if (sortorder == mOrderBy)
        return;
    mOrderBy = sortorder;
    invalidateQueries();
}
