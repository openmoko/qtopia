/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include "timemonitor.h"
#include <QtopiaApplication>
#include <QtopiaIpcEnvelope>
#include <QtopiaService>
#include "qtopiaserverapplication.h"

/*!
  \class TimeMonitorTask
  \brief The TimeMonitorTask class is required for the TimeMonitor service to be supported.
  \ingroup QtopiaServer::Task
  This class is part of the Qtopia server and cannot be used by other Qtopia applications.
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
        QString t = Qtopia::currentTimeZone();
        QtopiaIpcEnvelope e(ch, "TimeMonitor::timeChange(QString)");
        e << t;
    }
}

QTOPIA_TASK(TimeMonitor, TimeMonitorTask);
