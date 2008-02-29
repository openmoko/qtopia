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

#include "qtask.h"
#include "qtaskmodel.h"
#include "qpimsource.h"
#include <qtopiaipcenvelope.h>
#include "qtasksqlio_p.h"
#ifdef Q_OS_WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

// both... until I can get all the required widgets sorted out.
#include <qcategorymanager.h>

#include <QString>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimer>

static const char *contextsource = "default";
QStringList QTaskSqlIO::sortColumns() const
{
    switch(cSort) {
        default:
        case QTaskModel::Invalid:
        case QTaskModel::Description:
            return QStringList() << "description"; // no tr
        case QTaskModel::Priority:
            return QStringList() << "priority"; // no tr
        case QTaskModel::Completed:
        case QTaskModel::PercentCompleted:
            return QStringList() << "percentcompleted"; // no tr
        case QTaskModel::Status:
            return QStringList() << "status"; // no tr
        case QTaskModel::StartedDate:
            return QStringList() << "started"; // no tr
        case QTaskModel::DueDate:
            return QStringList() << "due"; // no tr
        case QTaskModel::CompletedDate:
            return QStringList() << "completed"; // no tr
    }
}

QStringList QTaskSqlIO::otherFilters() const
{
    QStringList l;
    if (cCompFilter)
        l << "percentcompleted != 100";
    return l;
}

void QTaskSqlIO::setContextFilter(const QSet<int> &list)
{
    if (list != contextFilter()) {
        QPimSqlIO::setContextFilter(list, ExcludeContexts);
        invalidateCache();
        emit recordsUpdated();
    }
}

QSet<int> QTaskSqlIO::contextFilter() const
{
    return QPimSqlIO::contextFilter();
}

void QTaskSqlIO::setCategoryFilter(const QCategoryFilter &f)
{
    if (f != categoryFilter()) {
        QPimSqlIO::setCategoryFilter(f);
        emit recordsUpdated();
    }
}

QTaskSqlIO::QTaskSqlIO(QObject *parent, const QString &)
    : QTaskIO(parent),
    QPimSqlIO(contextId(), "tasks", "taskcategories", "taskcustom",
            "description = :description, status = :status, priority = :priority, "
            "percentcompleted = :pc, due = :due, started = :started, completed = :completed",
            " (recid, context, description, status, priority, percentcompleted, due, started, completed"
            ") VALUES (:i, :context, :description, :status, :priority, :pc, :due, :started, "
            ":completed)"),
    cCompFilter(false), cSort(QTaskModel::Description), taskByRowValid(false)
{
    QStringList tables;
    tables << "sqlsources";
    tables << "changelog";
    tables << "tasks";
    tables << "categories";
    tables << "taskcategories";
    tables << "taskcustom";
    QtopiaSql::ensureSchema(tables, QtopiaSql::systemDatabase());


    QStringList sortColumns;
    sortColumns << "completed";
    sortColumns << "priority";
    sortColumns << "description";
    QPimSqlIO::setOrderBy(sortColumns);
}

QTaskSqlIO::~QTaskSqlIO()
{
}

QUuid QTaskSqlIO::contextId() const
{
    // generated with uuidgen
    static QUuid u("10b16464-b37b-4e0b-9f15-7a5d895b70c6");
    return u;
}

void QTaskSqlIO::bindFields(const QPimRecord& r, QSqlQuery &q) const
{
    const QTask &t = (const QTask &)r;
    q.bindValue(":description", t.description());
    q.bindValue(":status", t.status());
    q.bindValue(":priority", t.priority());
    q.bindValue(":pc", t.percentCompleted());
    q.bindValue(":due", t.dueDate());
    q.bindValue(":started", t.startedDate());
    q.bindValue(":completed", t.completedDate());
}

// by uid doesn't neeed caching... always fast and unlikely to be in order?
QTask QTaskSqlIO::task( const QUniqueId & u ) const
{
    if (taskByRowValid && u == lastTask.uid())
        return lastTask;
    const QLocalUniqueId &lid = (const QLocalUniqueId &)u;
    QByteArray uid = lid.toByteArray();

    QSqlQuery q;
    q.prepare("SELECT recid, description, priority, status, percentcompleted, due, started, completed from tasks where recid = :i");
    q.bindValue(":i", uid);

    QTask t;
    retrieveRecord(uid, t);
    q.setForwardOnly(true);
    if (!q.exec()) {
        qWarning("failed to select task: %s", (const char *)q.lastError().text().toLocal8Bit());
        taskByRowValid = false;
        return t;
    }

    if ( q.next() ) {
        t.setUid(QUniqueId(q.value(0).toByteArray()));
        t.setDescription(q.value(1).toString());
        t.setPriority((QTask::Priority)q.value(2).toInt());
        t.setStatus((QTask::Status)q.value(3).toInt());
        t.setPercentCompleted(q.value(4).toInt());
        if (!q.isNull(5))
            t.setDueDate(q.value(5).toDate());
        else
            t.clearDueDate();
        if (!q.isNull(6))
            t.setStartedDate(q.value(6).toDate());
        if (!q.isNull(7))
            t.setCompletedDate(q.value(7).toDate());

        lastTask = t;
        taskByRowValid = true;
    } else {
        taskByRowValid = false;
        qWarning("Did not find selected task");
    }
    return t;
}

void QTaskSqlIO::setCompletedFilter(bool b)
{
    if (cCompFilter != b) {
        invalidateCache();
        cCompFilter = b;
    }
}

void QTaskSqlIO::setSortKey(QTaskModel::Field s)
{
    if (cSort != s) {
        invalidateCache();
        cSort = s;
        emit recordsUpdated();
    }
}

bool QTaskSqlIO::completedFilter() const
{
    return cCompFilter;
}


QTaskModel::Field QTaskSqlIO::sortKey() const
{
    return cSort;
}

void QTaskSqlIO::invalidateCache()
{
    QPimSqlIO::invalidateCache();
    taskByRowValid = false;
}

QTask QTaskSqlIO::task(int row) const
{
    // caching belongs in task lookup.
    return task(recordId(row));
}

bool QTaskSqlIO::removeTask(int row)
{
    QUniqueId u = recordId(row);
    return removeTask(u);
}

bool QTaskSqlIO::removeTask(const QUniqueId & id)
{
    if (id.isNull())
        return false;

    if (QPimSqlIO::removeRecord(id)) {
        notifyRemoved(id);
        emit recordsUpdated();
        return true;
    }
    return false;
}

bool QTaskSqlIO::removeTask(const QTask &t)
{
    return removeTask(t.uid());
}

bool QTaskSqlIO::removeTasks(const QList<int> &rows)
{
    QList<QUniqueId> ids = recordIds(rows);
    return removeTasks(ids);
}

bool QTaskSqlIO::removeTasks(const QList<QUniqueId> &ids)
{
    if (QPimSqlIO::removeRecords(ids)) {
        notifyRemoved(ids);
        emit recordsUpdated();
        return true;
    }
    return false;
}

bool QTaskSqlIO::updateTask(const QTask &t)
{
    if (QPimSqlIO::updateRecord(t)) {
        notifyUpdated(t);
        emit recordsUpdated();
        return true;
    }
    return false;
}

QUniqueId QTaskSqlIO::addTask(const QTask &task, const QPimSource &source, bool createuid)
{
    QPimSource s;
    s.identity = contextsource;
    s.context = contextId();
    QUniqueId i = addRecord(task, source.isNull() ? s : source, createuid);
    if (!i.isNull()) {
        QTask added = task;
        added.setUid(i);
        notifyAdded(added);
	emit recordsUpdated();
    }
    return i;
}

QVariant QTaskSqlIO::key(int row) const
{
    QList<QTaskModel::Field> mOrder;
    mOrder << QTaskModel::CompletedDate;
    mOrder << QTaskModel::Priority;
    mOrder << QTaskModel::Description;
    QStringList sc;
    foreach(QTaskModel::Field f, mOrder) {
        sc << taskField(row, f).toString();
    }
    return sc;
}

/***************
 * CONTEXT
 **************/
QTaskDefaultContext::QTaskDefaultContext(QObject *parent, QObject *access)
    : QTaskContext(parent)
{
    mAccess = qobject_cast<QTaskSqlIO *>(access);
    Q_ASSERT(mAccess);
}

QIcon QTaskDefaultContext::icon() const
{
    return QPimContext::icon(); // redundent, but will do for now.
}

QString QTaskDefaultContext::description() const
{
    return tr("Default task storage");
}

QString QTaskDefaultContext::title() const
{
    return tr("Tasks");
}

bool QTaskDefaultContext::editable() const
{
    return true;
}

QSet<QPimSource> QTaskDefaultContext::sources() const
{
    QSet<QPimSource> list;
    list.insert(defaultSource());
    return list;
}

QUuid QTaskDefaultContext::id() const
{
    return mAccess->contextId();
}

/* TODO set mapping to int */
void QTaskDefaultContext::setVisibleSources(const QSet<QPimSource> &set)
{
    int context = QPimSqlIO::sourceContext(defaultSource());

    QSet<int> filter = mAccess->contextFilter();

    if (set.contains(defaultSource()))
        filter.remove(context);
    else
        filter.insert(context);

    mAccess->setContextFilter(filter);
}

QSet<QPimSource> QTaskDefaultContext::visibleSources() const
{
    int context = QPimSqlIO::sourceContext(defaultSource());

    QSet<int> filter = mAccess->contextFilter();
    if (!filter.contains(context))
        return sources();
    return QSet<QPimSource>();
}

bool QTaskDefaultContext::exists(const QUniqueId &id) const
{
    int context = QPimSqlIO::sourceContext(defaultSource());
    return mAccess->exists(id) && mAccess->context(id) == context;
}

QPimSource QTaskDefaultContext::defaultSource() const
{
    QPimSource s;
    s.context = mAccess->contextId();
    s.identity = contextsource;
    return s;
}

QPimSource QTaskDefaultContext::source(const QUniqueId &id) const
{
    if (exists(id))
        return defaultSource();
    return QPimSource();
}

bool QTaskDefaultContext::updateTask(const QTask &task)
{
    return mAccess->updateTask(task);
}

bool QTaskDefaultContext::removeTask(const QUniqueId &id)
{
    return mAccess->removeTask(id);
}

QUniqueId QTaskDefaultContext::addTask(const QTask &task, const QPimSource &source)
{
    if (source.isNull() || source == defaultSource())
        return mAccess->addTask(task, defaultSource());
    return QUniqueId();
}

