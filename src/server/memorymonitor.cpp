// -*-C++-*-
/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
  \ingroup QtopiaServer::Memory
  \brief The MemoryMonitor class interface determines the available amount of available memory.

  The MemoryMonitor class provides an abstract interface for
  querying and monitoring the amount of system memory available.
  As a task interface, it is never instantiated directly. It is
  instantiated by a qtopiaTask<MemoryMonitor>() call.

  The MemoryMonitor interface is an optional task interface, so
  consumers must be prepared for a request for this interface to
  fail.
  
  This class is part of the Qtopia server and cannot be used by other Qtopia applications.
 */

/*!
  \fn MemState MemoryMonitor::memoryState() const

  The subclass must implement this function. It must return
  the current memory state. The four memory states are
  Critical, Very Low, Low, and Normal.

 */

/*!
  \fn unsigned int MemoryMonitor::timeInState() const

  The subclass must implement this function. It must return
  the time, in seconds, since the last memory state change.
 */

/*!
  \enum MemoryMonitor::MemState

  The MemState enumeration represents the memory level in the system.

  \value MemUnknown Memory level is unknown or unavailable.
  
  \value MemCritical Memory level is critically low. Drastic
         measures should be taken to prevent system failure.

  \value MemVeryLow Memory level is very low. The system
         should perform any measure that is very likely to
	 recover memory. User noticable actions are allowed.
	 
  \value MemLow Memory level is low. The system should attempt
         to recover memory in a way that does not interfere with
	 the user.
	 
  \value MemNormal Memory level is normal. No action is required.
 */

/*!
  \fn void MemoryMonitor::memoryStateChanged(MemState newState);

  This signal is emitted whenever the memory state changes.
  \a newState will be set to the new memory state.
 */
