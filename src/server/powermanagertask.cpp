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

#include "powermanagertask.h"
#include "qtopiaserverapplication.h"

#include <QPowerStatus>
#include <QPowerStatusManager>

// FIXME
// define PowerManagerTask
/*!
  \class PowerManagerTask
  \ingroup QtopiaServer::Task
  \brief The PowerManagerTask class implements the device's power management functionality.

  The PowerManagerTask class provides the \c {PowerManager} task.
  Qtopia's Light-and-power application configures the functionality implemented by the \c {PowerManager} task.
  The PowerManagerTask monitors QPowerStatusManager and calls powerStatusChanged() whenever the power status changes.

  \sa QtopiaPowerManager, {Hardware Configuration}
 */

/*!
  Construct a PowerManagerTask instance with the specified \a parent.
 */
PowerManagerTask::PowerManagerTask(QObject *parent)
: QObject(parent)
{
    // Create the screen saver and the associated service.
    QPowerStatusManager *manager = new QPowerStatusManager(this);
    QObject::connect(manager, SIGNAL(powerStatusChanged(const QPowerStatus &)),
                     this, SLOT(powerStatusChanged(const QPowerStatus &)));
    QPowerStatus status = manager->readStatus();
    powerStatusChanged(status); // force an update
}

/*!
Called when the power status of the device changes.
\a ps contains the new power status.
This slot is automatically connected to QPowerStatusManager::powerStatusChanged(). */
void PowerManagerTask::powerStatusChanged(const QPowerStatus &ps)
{
    Q_UNUSED(ps);
}

QTOPIA_TASK(PowerManagerTask, PowerManagerTask)
