/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef TODO_ACCESS_H
#define TODO_ACCESS_H

#include <qobject.h>
#include "task.h"

class TodoAccessPrivate;

class TaskIO;
class TodoAccess;
class TaskIteratorMachine;

class TodoIterator
{
public:
    // internal constructors
    TodoIterator(const TaskIO &);
    TodoIterator(TaskIteratorMachine *);

    // public constructors.
    TodoIterator(const TodoAccess &);
    TodoIterator(const TodoIterator &);

    TodoIterator &operator=(const TodoIterator &);
    ~TodoIterator();

    bool atFirst() const;
    bool atLast() const;
    const PimTask *toFirst();
    const PimTask *toLast();

    const PimTask *operator++();
    const PimTask *operator*() const;

    const PimTask *current() const;

private:
    TaskIteratorMachine *machine;
};

class TodoAccess : public QObject {
  Q_OBJECT

      friend class TodoIterator;
  public:

  TodoAccess();
  
  ~TodoAccess();

  bool editSupported() const;
  
  void updateTask(const PimTask& task);

  void removeTask(const PimTask& task);
  
  void addTask(const PimTask& task);

 signals:
  void todoListUpdated();

 private:
  TaskIO* taskio;
};


#endif

