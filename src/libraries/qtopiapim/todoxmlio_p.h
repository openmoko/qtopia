/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info\@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef TODO_XMLIO_PRIVATE_H
#define TODO_XMLIO_PRIVATE_H

#include <qvector.h>
#include <qasciidict.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qtopia/pim/task.h>
#include <qtopia/pim/qtopiapim.h>
#include "taskio_p.h"
#include "xmlio_p.h"

#ifdef QTOPIAPIM_TEMPLATEDLL
//MOC_SKIP_BEGIN
template class QTOPIAPIM_EXPORT QList<class PrTask>; 
template class QTOPIAPIM_EXPORT QListIterator<class PrTask>; 
//MOC_SKIP_END
#endif

class QTOPIAPIM_EXPORT SortedTasks : public SortedRecords<PimTask>
{
public:
    SortedTasks();
    SortedTasks(uint size);

    ~SortedTasks();

    int compareItems(QCollection::Item d1, QCollection::Item d2);

private:
    int compareTaskField(int f, PrTask *t1, PrTask *t2);
};

class QTOPIAPIM_EXPORT TodoXmlIterator : public TaskIteratorMachine
{
public:
    TodoXmlIterator(const QList<PrTask>&list) : it(list) {}

    ~TodoXmlIterator() {}

    TodoXmlIterator &operator=(const TodoXmlIterator &o) {
	it = o.it;
	return *this;
    }

    bool atFirst() const { return it.atFirst(); }
    bool atLast() const { return it.atLast(); }
    const PrTask *toFirst() { return it.toFirst(); }
    const PrTask *toLast() { return it.toLast(); }

    const PrTask *next() { return ++it; }
    const PrTask *prev() { return --it; }
    const PrTask *current() const { return it.current(); }

private:
    QListIterator<PrTask>it;
};

class QTOPIAPIM_EXPORT TodoXmlIO : public TaskIO, public PimXmlIO {

    Q_OBJECT

 public:
  TodoXmlIO(AccessMode m,
	    const QString &file = QString::null,
	    const QString &journal = QString::null );

  ~TodoXmlIO();

  TaskIteratorMachine *begin() const;

  /**
   * Returns the full task list.  This is guaranteed
   * to be current against what is stored by other apps.
   */
  const QList<PrTask>& tasks();

  const SortedTasks &sortedTasks();

  PrTask taskForId( const QUuid &, bool *ok ) const;

  /**
   * Loads the task data into the internal list
   */
  bool loadData();

  /**
   * Saves the current task data.  Returns true if
   * successful.
   */
  bool saveData();

  void clear();

  // external methods.. use PimTask
  void updateTask(const PimTask& task);
  void removeTask(const PimTask& task);
  QUuid addTask(const PimTask& task, bool assignUid = TRUE);
     
  int sortKey() const;
  bool sortAcending() const;
  void setSorting(int key, bool ascending = FALSE);

  int filter() const;
  void setFilter(int);

  bool completedFilter() const;
  void setCompletedFilter(bool);

  void ensureDataCurrent(bool = FALSE);

protected:
  const char *recordStart() const;
  const char *listStart() const;
  const char *listEnd() const;

  virtual QString recordToXml(const PimRecord *);
  
  PimRecord *createRecord() const;

  bool internalAddRecord(PimRecord *);
  bool internalRemoveRecord(PimRecord *);
  bool internalUpdateRecord(PimRecord *);

  bool select(const PrTask &) const;

protected slots:
  void pimMessage(const QCString &, const QByteArray &);

private:

  QList<PrTask> m_Tasks;
  SortedTasks m_Filtered;
  int cFilter;
  bool cCompFilter;
  bool needsSave;
};

#endif
