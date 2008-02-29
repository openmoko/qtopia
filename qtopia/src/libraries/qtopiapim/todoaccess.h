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

#ifndef TODO_ACCESS_H
#define TODO_ACCESS_H

#include <qobject.h>
#include <qtopia/pim/task.h>

class TodoAccessPrivate;

class TaskIO;
class TodoAccess;
class TaskIteratorMachine;

class QTOPIAPIM_EXPORT TodoIterator
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

class QTOPIAPIM_EXPORT TodoAccess : public QObject {
  Q_OBJECT

      friend class TodoIterator;
  public:

  TodoAccess();
  
  ~TodoAccess();

  bool editSupported() const;
  
  void updateTask(const PimTask& task);

  void removeTask(const PimTask& task);
  
  void addTask(const PimTask& task);

  PimTask taskForId(const QUuid &, bool *ok = 0) const;

 signals:
  void todoListUpdated();

 private:
  TaskIO* taskio;
};


#endif

