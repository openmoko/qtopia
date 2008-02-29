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

#ifndef TODO_XMLIO_PRIVATE_H
#define TODO_XMLIO_PRIVATE_H

#include <qvector.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qcategorymanager.h>
#include <qtopia/pim/qtask.h>
#include <qtopia/pim/qtaskmodel.h> // for sort key
#include "qtaskio_p.h"
#include "qpimxmlio_p.h"

#ifdef QTOPIAPIM_TEMPLATEDLL
//MOC_SKIP_BEGIN
template class QList<class QTask *>;
template class QListIterator<class QTask *>;
//MOC_SKIP_END
#endif

class QTaskXmlIO : public QTaskIO, public QPimXmlIO {

    Q_OBJECT

 public:
        // internal fields.
    enum TaskFields {
        HasDate,
        Description,
        Priority,
        CompletedField,
        PercentCompleted,
        DueDateYear,
        DueDateMonth,
        DueDateDay,
        Status,
        StartedDate,
        CompletedDate,
        Notes
    };

    explicit QTaskXmlIO(AccessMode m,
            const QString &file = QString(),
            const QString &journal = QString() );

  ~QTaskXmlIO();

  bool editableByRow() const { return true; }
  bool editableByField() const { return true; }

  QTaskModel::Field field() const;
  void setField(QTaskModel::Field);

  QCategoryFilter categoryFilter() const;
  void setCategoryFilter(const QCategoryFilter &);

  bool completedFilter() const;
  void setCompletedFilter(bool);

  // external methods.. use QTask
  QUniqueId addTask(const QTask& task);
  bool updateTask(const QTask& task);

  bool removeTask(int row);
  bool removeTask(const QUniqueId & id);
  bool removeTask(const QTask &);
  bool removeTasks(const QList<int> &rows);
  bool removeTasks(const QList<QUniqueId> &ids);

  QTask task(const QUniqueId &) const;
  QTask task(int row) const;
  QUniqueId taskId(int row) const;
  int row(const QUniqueId & id) const;

  int count() const;
  bool exists(const QUniqueId & id) const { return row(id) != -1; }
  bool contains(const QUniqueId & id) const { return row(id) != -1; }

    // searching.  row based.
    // pda based
  int startSearch(const QString &);
#ifdef QTOPIA_PHONE
  // phone key based
  int startPhoneKeySearch(const QString &) { return -1; }
#endif
  int nextSearchItem();
  void clearSearch();

protected:
  const char *recordStart() const;
  const char *listStart() const;
  const char *listEnd() const;

  void ensureDataCurrent(bool = false);
  QList<QUniqueId> filteredSearch(const QString &) const;
  virtual void setFields(QPimRecord *, const QMap<QString, QString> &) const;
  virtual QMap<QString, QString> fields(const QPimRecord *) const;

  QPimRecord *createRecord() const;

  bool internalAddRecord(QPimRecord *);
  bool internalRemoveRecord(QPimRecord *);
  bool internalUpdateRecord(QPimRecord *);

  bool select(const QTask &) const;

protected slots:
  void pimMessage(const QString &, const QByteArray &);

private:
  const QList<QTask*> &tasks() const;
  const QList<QTask*> &sortedTasks() const;
  bool loadData();
  bool saveData(bool force = false);

  void clear();

  void sort();

  QList<QTask*> m_Tasks;
  QList<QTask*> m_Filtered;
  QCategoryFilter cFilter;
  bool cCompFilter;
  QTaskModel::Field cKey;
  bool needsSave;
};

#endif
