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

#include "task.h"
#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#ifndef QT_NO_COP
#include <qtopia/services.h>
#endif
#include "todoaccess.h"
#include "todoxmlio_p.h"

#include <sys/types.h>
#ifndef Q_WS_WIN32
#include <unistd.h>
#endif
#include <stdlib.h>

/*! \class TodoAccess
  \module qpepim
  \ingroup qpepim
  \brief The TodoAccess class provides a safe API for accessing PimTasks.

  TodoAccess provides a safe API for accessing PimTasks stored by
  Qtopia's Todo application.  TodoAccess tries to keep the Todo
  application in sync with modifications, and alerts the user of
  the library when modifications have been made by other applications.
*/


/*!
 Constructor.
*/
TodoAccess::TodoAccess()
: taskio(0L) {
  taskio = new TodoXmlIO(TaskIO::ReadOnly);

  connect( taskio, SIGNAL(tasksUpdated()),
	  this, SIGNAL(todoListUpdated()) );
}

/*!
  Cleans up the the Addressbook access.
*/
TodoAccess::~TodoAccess()
{
  delete taskio;
}

/*!
  If there exists an task with the uid \a u, return 
  that task.  Returns a null task if otherwise.

  If \a ok is non-null, *ok is set to TRUE if the task is
  found, and FALSE if the task is not found.
*/
PimTask TodoAccess::taskForId(const QUuid &u, bool *ok) const
{
    return taskio->taskForId(u, ok);
}


/*!
  If supported will update \a task in the pim data.

  Updating tasks requires the Tasks service to be available.
*/
void TodoAccess::updateTask(const PimTask& task)
{
#ifndef QT_NO_COP
    QCopEnvelope e(Service::channel("Tasks"),
	    "updateTask(PimTask)");
    e << task;
#else
    Q_UNUSED( task ); 
#endif
}


/*!
  If supported will remove \a task from the pim data;

  Removing tasks requires the Tasks service to be available.
 */
void TodoAccess::removeTask(const PimTask& task)
{
#ifndef QT_NO_COP
    QCopEnvelope e(Service::channel("Tasks"),
	    "removeTask(PimTask)");
    e << task;
#else
    Q_UNUSED( task );
#endif
}

/*!
 If supported will assign a new unique ID to \a task and add the task to the
 pim data.

 Adding tasks requires the Tasks service to be available.

 If the task is added correctly, an addedTask(int,PimTask) message will be 
 sent to the QPE/PIM channel.  The task in this message can be used to determine
 the assigned ID.
 */
void TodoAccess::addTask(const PimTask& task)
{
#ifndef QT_NO_COP
    QCopEnvelope e(Service::channel("Tasks"),
	    "addTask(PimTask)");
    e << task;
#else
    Q_UNUSED( task ); 
#endif
}

/*!
  Returns TRUE if it is possible to add, remove and update tasks in
  the pim data.  Otherwise returns FALSE.

  Adding, removing and tasks contacts requires the Tasks service to be
  available
*/
bool TodoAccess::editSupported() const
{
#ifdef Q_WS_QWS
    return Service::list().contains("Tasks"); // No tr
#else
    return FALSE;
#endif
}


/*!
 \fn void TodoAccess::todoListUpdated()
 Emitted whenever the todo list is updated.
*/

/*! \class TodoIterator
  \module qpepim
  \ingroup qpepim
  \brief The TodoIterator class provides iterators of TodoAccess.

  The only way to traverse the data of an TodoAccess is with an
  TodoIterator.
*/

/*!
  \internal
*/
TodoIterator::TodoIterator(TaskIteratorMachine *m) : machine(m) {}

/*!
  Constructs a copy of the iterator \a it.
*/
TodoIterator::TodoIterator(const TodoIterator &it) : machine(it.machine)
{
    if (machine) machine->ref();
}

/*!
  Constructs a new TodoIterator pointing to the start of the data in \a taskaccess.
*/
TodoIterator::TodoIterator(const TodoAccess &taskaccess)
{
    machine = taskaccess.taskio ? taskaccess.taskio->begin() : 0;
}

/*!
  \internal
*/
TodoIterator::TodoIterator(const TaskIO &tio)
{
    machine = tio.begin();
}

/*!
  Sets the iterator to be a copy of \a other.
*/
TodoIterator &TodoIterator::operator=(const TodoIterator &other)
{
    if (other.machine) other.machine->ref();
    if (machine && machine->deref() ) delete machine;
    machine = other.machine;
    return *this;
}

/*!
  Destroys the iterator
*/
TodoIterator::~TodoIterator()
{
    if ( machine && machine->deref() ) delete machine;
}

/*!
  Returns TRUE if the iterator is at the first item of the data.
  Otherwise returns FALSE.
*/
bool TodoIterator::atFirst() const
{
    return machine ? machine->atFirst() : FALSE;
}

/*!
  Returns TRUE if the iterator is at the last item of the data.
  Otherwise returns FALSE.
*/
bool TodoIterator::atLast() const
{
    return machine ? machine->atLast() : FALSE;
}

/*!
  Sets the iterator to the first item of the data.
  If a PimTask exists in the data will return a const pointer to the
  PimTask.  Otherwise returns 0.
*/
const PimTask *TodoIterator::toFirst()
{
    return machine ? machine->toFirst() : 0;
}

/*!
  Sets the iterator to the last item of the data.
  If a PimTask exists in the data will return a const pointer to the
  PimTask.  Otherwise returns 0.
*/
const PimTask *TodoIterator::toLast()
{
    return machine ? machine->toLast() : 0;
}

/*!
  Sets the iterator to the next item of the data.
  If the iterator is moved pass the end of the list returns 0.
  Otherwise returns a const pointer to the current PimTask.
*/
const PimTask *TodoIterator::operator++()
{
    return machine ? machine->next() : 0;
}

/*!
  If the iterator is at a valid PimTask returns a const pointer to
  the current PimTask.  Otherwise returns 0.
*/
const PimTask* TodoIterator::operator*() const
{
    return machine ? machine->current() : 0;
}

/*!
  If the iterator is at a valid PimTask returns a const pointer to
  the current PimTask.  Otherwise returns 0.
*/
const PimTask* TodoIterator::current() const
{
    return machine ? machine->current() : 0;
}
