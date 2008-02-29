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

#ifndef TODO_SQLIO_PRIVATE_H
#define TODO_SQLIO_PRIVATE_H

#include <qtopia/pim/qtask.h>
#include <qtopiasql.h>
#include <qsqlquery.h>
#include "qpimsource.h"
#include "qtaskio_p.h"
#include "qpimsqlio_p.h"

class QTaskSqlIO;
class QTaskDefaultContext : public QTaskContext
{
    Q_OBJECT
public:
    // could have constructor protected/private with friends class.
    QTaskDefaultContext(QObject *parent, QObject *access);

    QIcon icon() const; // default empty
    QString description() const;

    using QTaskContext::title;
    QString title() const;

    // better to be flags ?
    using QTaskContext::editable;
    bool editable() const; // default true

    QPimSource defaultSource() const;
    void setVisibleSources(const QSet<QPimSource> &);
    QSet<QPimSource> visibleSources() const;
    QSet<QPimSource> sources() const;
    QUuid id() const;

    using QTaskContext::exists;
    bool exists(const QUniqueId &) const;
    QPimSource source(const QUniqueId &) const;

    bool updateTask(const QTask &);
    bool removeTask(const QUniqueId &);
    QUniqueId addTask(const QTask &, const QPimSource &);

private:
    QTaskSqlIO *mAccess;
};

class QTaskSqlIO : public QTaskIO, public QPimSqlIO {

    Q_OBJECT

 public:
     explicit QTaskSqlIO(QObject *parent, const QString &name = QString());
  ~QTaskSqlIO();

  QUuid contextId() const;
  int count() const { return QPimSqlIO::count(); }

  bool editableByRow() const { return true; }
  bool editableByField() const { return true; }

  void setSortKey(QTaskModel::Field k);
  QTaskModel::Field sortKey() const;

  void setCategoryFilter(const QCategoryFilter &f);
  QCategoryFilter categoryFilter() const { return QPimSqlIO::categoryFilter(); }

  void setContextFilter(const QSet<int> &);
  QSet<int> contextFilter() const;

  bool startSyncTransaction(const QSet<QPimSource> &sources, const QDateTime &syncTime) { return QPimSqlIO::startSync(sources, syncTime); }
  bool abortSyncTransaction() { return QPimSqlIO::abortSync(); }
  bool commitSyncTransaction() { return QPimSqlIO::commitSync(); }

  QList<QUniqueId> removed(const QSet<QPimSource> &sources, const QDateTime &timestamp) const
  { return QPimSqlIO::removed(sources, timestamp); }

  QList<QUniqueId> added(const QSet<QPimSource> &sources, const QDateTime &timestamp) const
  { return QPimSqlIO::added(sources, timestamp); }

  QList<QUniqueId> modified(const QSet<QPimSource> &sources, const QDateTime &timestamp) const
  { return QPimSqlIO::modified(sources, timestamp); }

  bool completedFilter() const;
  void setCompletedFilter(bool);

  QTask task(const QUniqueId &) const;
  QTask task(int row) const;
  QUniqueId taskId(int row) const { return QPimSqlIO::recordId(row); }
  int row(const QUniqueId & id) const { return QPimSqlIO::row(id); }
  QUniqueId id(int row) const { return QPimSqlIO::recordId(row); }
  QVariant key(int row) const;

  bool removeTask(int row);
  bool removeTask(const QUniqueId & id);
  bool removeTask(const QTask &);
  bool removeTasks(const QList<int> &rows);
  bool removeTasks(const QList<QUniqueId> &ids);

  bool updateTask(const QTask &t);
  QUniqueId addTask(const QTask &t, const QPimSource &s)
  { return addTask(t, s, true); }
  QUniqueId addTask(const QTask &, const QPimSource &, bool);

  bool exists(const QUniqueId & id) const { return !task(id).uid().isNull(); }
  bool contains(const QUniqueId & id) const { return row(id) != -1; }

#ifdef SUPPORT_SYNCML
  bool canProvideDiff() const { return false; }
  void clearJournal() {}
  QList<QUniqueId> addedTasks() const { return QList<QUniqueId>(); }
  QList<QUniqueId> modifiedTasks() const { return QList<QUniqueId>(); }
  QList<QUniqueId> deletedTasks() const { return QList<QUniqueId>(); }
#endif

  // searching.  row based.
  // pda based
  int startSearch(const QString &) { return -1; }
  int nextSearchItem() { return -1; }
  void clearSearch() { }

  void checkAdded(const QUniqueId &) { invalidateCache(); }
  void checkRemoved(const QUniqueId &) { invalidateCache(); }
  void checkRemoved(const QList<QUniqueId> &) { invalidateCache(); }
  void checkUpdated(const QUniqueId &) { invalidateCache(); }
 protected:
  void bindFields(const QPimRecord &, QSqlQuery &) const;
  void invalidateCache();
  QStringList sortColumns() const;
  QStringList otherFilters() const;

 private:
  bool cCompFilter;
  QTaskModel::Field cSort;

  mutable bool taskByRowValid;
  mutable QTask lastTask;
};

#endif
