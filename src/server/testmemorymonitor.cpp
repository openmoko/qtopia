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

#include "testmemorymonitor.h"
#include "qtopiaserverapplication.h"
#include <QValueSpaceObject>

/*!
  \class TestMemoryMonitor
  \ingroup QtopiaServer::Task
  \brief The TestMemoryMonitor class provides an instrumented implementation of MemoryMonitor for testing.

  The TestMemoryMonitorTask provides a Qtopia Server Task.  Qtopia Server 
  Tasks are documented in full in the QtopiaServerApplication class 
  documentation.

  \table
  \row \o Task Name \o TestMemoryMonitory
  \row \o Interfaces \o MemoryMonitor
  \row \o Services \o None
  \endtable

  The TestMemoryMonitor allows low memory conditions on the device to be 
  simulated.  The TestMemoryMonitor class exposes the following value space 
  items.

  \table
  \header \o Item \o Permissions \o Description
  \row \o \c {/ServerTasks/TestMemoryMonitor/MemoryLevel} \o rw \o Current memory level.  Valid values are "Unknown", "Critical", "VeryLow", "Low" and "Normal".
  \row \o \c {/ServerTasks/TestMemoryMonitor/ChangedTime} \o ro \o The date and time at which the memory level last changed.
  \endtable

  By setting the \c {MemoryLevel} items, test scripts can simulate changes in
  the system's memory pressure.  Only the \c {MemoryLevel} item is writable.
  The \c {ChangedTime} item will be updated accordingly each time the memory
  level changes.

  To ensure that the TestMemoryMonitor task is selected as the MemoryMonitor
  provider, testers should ensure that all other MemoryMonitor providers are
  either disabled, or that the TestMemoryMonitor is manually started at an
  appropriately early time.

  The TestMemoryMonitor task will fail if it is started before the value space
  is initialized.
 */

/*! \internal */
TestMemoryMonitor::TestMemoryMonitor()
: m_vso(0), m_memstate(MemNormal), m_lastDateTime(QDateTime::currentDateTime())
{
    QByteArray vsoPath = QtopiaServerApplication::taskValueSpaceObject("TestMemoryMonitor");
    Q_ASSERT(!vsoPath.isEmpty());

    m_vso = new QValueSpaceObject(vsoPath, this);
    QObject::connect(m_vso,
                     SIGNAL(itemSetValue(const QByteArray &, const QVariant &)),
                     this,
                     SLOT(setValue(const QByteArray &, const QVariant &)));

    refresh();
}

/*! \internal */
TestMemoryMonitor::MemState TestMemoryMonitor::memoryState() const
{
    return m_memstate;
}

/*! \internal */
unsigned int TestMemoryMonitor::timeInState() const
{
    return m_lastDateTime.secsTo(QDateTime::currentDateTime());
}

/*! \internal */
void TestMemoryMonitor::setValue(const QByteArray &name, const QVariant &value)
{
    if("/MemoryLevel" == name) {
        QByteArray state = value.toByteArray();
        MemState newState = m_memstate;
        if("Unknown" == state) newState = MemUnknown;
        else if("Critical" == state) newState = MemCritical;
        else if("VeryLow" == state) newState = MemVeryLow;
        else if("Low" == state) newState = MemLow;
        else if("Normal" == state) newState = MemNormal;

        if(newState != m_memstate) {
            m_memstate = newState;
            m_lastDateTime = QDateTime::currentDateTime();
            refresh();
            emit memoryStateChanged(m_memstate);
        }
    }
}

/*! \internal */
void TestMemoryMonitor::refresh()
{
    QByteArray state;
    switch(m_memstate) {
        case MemUnknown: state = "Unknown"; break;
        case MemCritical: state = "Critical"; break;
        case MemVeryLow: state = "VeryLow"; break;
        case MemLow: state = "Low"; break;
        case MemNormal: state = "Normal"; break;
    };

    m_vso->setAttribute("MemoryLevel", state);
    m_vso->setAttribute("ChangedTime", m_lastDateTime);
}

QTOPIA_TASK(TestMemoryMonitor, TestMemoryMonitor);
QTOPIA_TASK_PROVIDES(TestMemoryMonitor, MemoryMonitor);
