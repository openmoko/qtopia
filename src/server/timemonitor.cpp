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

#include "timemonitor.h"
#include <QtopiaApplication>
#include <QtopiaIpcEnvelope>
#include <QtopiaService>
#include "qtopiaserverapplication.h"

/*!
  \class TimeMonitorTask
  \brief The TimeMonitorTask class is required for the TimeMonitor service to be supported.
  \ingroup QtopiaServer::Task

  The TimeMonitorTask provides a Qtopia Server Task.  Qtopia Server Tasks are
  documented in full in the QtopiaServerApplication class documentation.

  \table
  \row \o Task Name \o TimeMonitor
  \row \o Interfaces \o None
  \row \o Services \o TimeMonitor
  \endtable

  The TimeMonitorTask class calls the TimeMonitor::timeChanged(QString) service
  method for each application that supports it whenever the system time changes.
  This allows applications to update themselves appropriately.
 */

/*! \internal */
TimeMonitorTask::TimeMonitorTask()
{
    QObject::connect(QtopiaApplication::instance(), SIGNAL(timeChanged()),
                     this, SLOT(pokeTimeMonitors()));
}

/*! \internal */
void TimeMonitorTask::pokeTimeMonitors()
{
    // inform all TimeMonitors
    QStringList tms = QtopiaService::channels("TimeMonitor");
    foreach (QString ch, tms) {
        QString t = getenv("TZ");
        QtopiaIpcEnvelope e(ch, "TimeMonitor::timeChange(QString)");
        e << t;
    }
}

QTOPIA_TASK(TimeMonitor, TimeMonitorTask);
