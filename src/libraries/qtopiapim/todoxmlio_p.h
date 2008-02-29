/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
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
#include <qtopia/pim/private/taskio_p.h>
#include <qtopia/pim/private/xmlio_p.h>

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
  bool saveData(bool force = FALSE);

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

  // back over from sql code.
  PrTask taskForId( const int, bool *ok = 0) const;

  PimTask filteredItem(int pos);
  int filteredPos(const int id);
  int filteredCount();

  QValueList<int> filteredSearch(const QString &);

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
