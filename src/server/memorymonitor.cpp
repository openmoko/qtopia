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

#include "memorymonitor.h"

/*!
  \class MemoryMonitor
  \ingroup QtopiaServer::Task::Interfaces
  \brief The MemoryMonitor class interface measures the available memory level
         in the system.

  The MemoryMonitor class provides an abstract interface for querying and
  monitoring the system memory condition.  As a task interface it is never
  instantiated directly, but rather through a qtopiaTask<MemoryMonitor>()
  call.  The MemoryMonitor interface is an optional task interface, so consumers
  must be prepared for a request for this interface to fail.
 */

/*!
  \fn MemState MemoryMonitor::memoryState() const

  Returns the current memory level.
 */

/*!
  \fn unsigned int MemoryMonitor::timeInState() const

  Returns the amount of time, in seconds, since the last memory state change.
  That is the amount of time the memory state has been equal to the current
  memory state.
 */

/*!
  \enum MemoryMonitor::MemState

  The MemState enumeration represents the memory level in the system.

  \value MemUnknown Memory level is unknown or unavailable.
  \value MemCritical Memory level is critically low.  Drastic measures should be taken to ensure the system's stability.
  \value MemVeryLow Memory level is very low.  The system should perform any measure that is very likely to recover memory.  User noticable actions are allowed.
  \value MemLow Memory level is low.  The system should attempt to recover memory in a way that does not interfere with the user.
  \value MemNormal Memory level is normal.  No action is necessary.
 */

/*!
  \fn void MemoryMonitor::memoryStateChanged(MemState newState);

  Emitted whenever the memory state changes.  \a newState will be set to the
  new memory state.
 */
