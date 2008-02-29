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

#include "genericmemorymonitor.h"
#include <QFile>
#include <QTimer>
#include <QTextStream>
#include <QString>
#include <sys/types.h>
#include <unistd.h>
static const int MIN_MEM_LIMIT = 10000;

/*!
  \class GenericMemoryMonitorTask
  \ingroup QtopiaServer::Task
  \brief The GenericMemoryMonitorTask class implements a simple page-fault
         driven memory monitor task.

  The GenericMemoryMonitorTask provides a Qtopia Server Task.  Qtopia Server 
  Tasks are documented in full in the QtopiaServerApplication class 
  documentation.

  \table
  \row \o Task Name \o GenericMemoryMonitory
  \row \o Interfaces \o MemoryMonitor
  \row \o Services \o None
  \endtable

  The GenericMemoryMonitorTask uses a simple page-fault rate driven algorithm to
  approximate the memory "pressure" on the device.

  The GenericMemoryMonitorTask task exports the following informational value 
  space items.

  \table
  \header \o Item \o Description
  \row \o \c {/ServerTasks/GenericMemoryMonitorTask/MemoryLevel} \o Set to the current memory level.  Either "Unknown", "Critical", "VeryLow", "Low" or "Normal".
  \row \o \c {/ServerTasks/GenericMemoryMonitorTask/ChangedTime} \o The date and time at which the memory level last changed.
  \endtable

  As polling is used to sample memory usage information from the /c {/proc} 
  filesystem, the GenericMemoryMonitorTask should generally be replaced by 
  system integrators with a more efficient, system specific mechanism.
 */

/*!  \internal */
GenericMemoryMonitorTask::GenericMemoryMonitorTask()
: m_vmStatType(VMUnknown), m_slowVMMonitor(true), m_count(0), m_prevFaults(0),
  m_memstate(MemUnknown), m_vso(0)
{
    m_pgFaults.fill(0, 10);

    QFile vmstat("/proc/vmstat"); // kernel 2.6+
    if(vmstat.exists() && vmstat.open(QIODevice::ReadOnly)) {
        qLog(QtopiaServer) << "GenericMemoryMonitorTask: Detected Linux 2.6.x kernel";
        m_vmStatType  = VMLinux_2_6;
        vmstat.close();
    } else {
        qLog(QtopiaServer) << "GenericMemoryMonitorTask: Detected Linux 2.4.x kernel";
        m_vmStatType = VMLinux_2_4;
    }

    if(m_vmStatType != VMUnknown) {
        m_vmMonitor = new QTimer(this);
        connect(m_vmMonitor, SIGNAL(timeout()), this, SLOT(memoryMonitor()));

        int freeMem = -1;
        int pcUsed = -1;
        readMemInfo(&freeMem, &pcUsed);

        if (freeMem > MIN_MEM_LIMIT) {
            // plenty of mem available => reduce number of checks
            m_vmMonitor->start( 10000 );
            m_slowVMMonitor = true;
        }
        else {
            m_vmMonitor->start( 1000 );
            m_slowVMMonitor = false;
        }
    }

    QTimer *timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()), this, SLOT(evalMemory()));
    timer->start( 10000 );

    evalMemory();
}

/*!  \internal */
void GenericMemoryMonitorTask::readMemInfo(int *freeMem, int *pcUsed)
{
    QFile file( "/proc/meminfo" );
    *freeMem = -1;
    *pcUsed = -1;
    if ( file.open( QIODevice::ReadOnly ) ) {
        int total, memfree, buffers, cached;
        QTextStream t( &file );

        // structure of meminfo changed in kernel 2.6
        QString word;

        while (!t.atEnd()) {
            t>>word;
            if (word == "MemTotal:") {
                t >> total;
            } else if (word == "MemFree:") {
                t >> memfree;
            } else if (word == "Buffers:") {
                t >> buffers;
            } else if (word == "Cached:") {
                t >> cached;
                break; //last entry to read
            }
        }
        *freeMem = buffers + cached + memfree;
        int realUsed = total - ( *freeMem );
        *pcUsed = 100*realUsed/total;
    }
}

/*!  \internal */
void GenericMemoryMonitorTask::memoryMonitor()
{
    int freemem = -1;
    int pcUsed = -1;
    readMemInfo( &freemem, &pcUsed );
    if(freemem < 0)
        freemem = MIN_MEM_LIMIT - 1;

    if(freemem > MIN_MEM_LIMIT) {
        if(!m_slowVMMonitor) {
            m_vmMonitor->start(10000);
            m_slowVMMonitor = true;
        }
        return;
    } else {
        if(m_slowVMMonitor) {
            m_vmMonitor->start(1000);
            m_slowVMMonitor = false;
        }
    }

    if(m_vmStatType == VMLinux_2_6) {

        QFile vmstat("/proc/vmstat"); //kernel 2.6+
        if(vmstat.open(QIODevice::ReadOnly)) {
            QTextStream t ( &vmstat );
            QString nLine, majfaults;
            nLine = t.readLine();
            while (!nLine.isNull()) {
                //only interested in major page faults
                if (nLine.contains("pgmajfault"))  {
                    majfaults = nLine.mid(nLine.indexOf(' '));
                    break;
                }
                nLine = t.readLine();
            }

            bool ok;
            long newFaults = majfaults.toInt(&ok);
            if (m_prevFaults != 0  && ok) {
                m_pgFaults[m_count] = newFaults - m_prevFaults;
            }
            else
                m_pgFaults[m_count] = 0;

            m_count = (++m_count) % m_pgFaults.size();
            m_prevFaults = newFaults;
            vmstat.close();
        }

    } else if(m_vmStatType == VMLinux_2_4) {

        QFile vmstat("/proc/"+QString::number(::getpid())+"/stat");
        if (vmstat.open( QIODevice::ReadOnly )) {
            QTextStream t(&vmstat);
            QString majFaults;
            for (int i = 0; i < 12; i++)
                t >> majFaults;
            vmstat.close();
            bool ok;
            long newFaults = majFaults.toLong(&ok);
            if (m_prevFaults != 0 && ok) {
                m_pgFaults[m_count] = newFaults - m_prevFaults;
            } else {
                m_pgFaults[m_count] = 0;
            }

            m_count = (++m_count) % m_pgFaults.size();
            m_prevFaults = newFaults;
        }
    }
}

/*!  \internal */
void GenericMemoryMonitorTask::evalMemory()
{
    int sum = 0;
    for (int i=0; i < (int)m_pgFaults.size(); i++)
        sum += m_pgFaults[i];

    int avg = sum/m_pgFaults.size();

    if (m_vmStatType == VMLinux_2_6 ) {
        if (avg > 250)
            setMemState(MemCritical);
        else if (avg > 120)
            setMemState(MemVeryLow);
        else if (avg > 60)
            setMemState(MemLow);
        else
            setMemState(MemNormal);

    } else if (m_vmStatType == VMLinux_2_4) {
        if ( avg > 50 )
            setMemState(MemCritical);
        else if ( avg > 25 )
            setMemState(MemVeryLow);
        else if ( avg > 20 )
            setMemState(MemLow);
        else
            setMemState(MemNormal);
    }

}

/*!  \internal */
void GenericMemoryMonitorTask::setMemState(MemState newState)
{
    if(newState == m_memstate)
        return;

    m_memstate = newState;
    m_lastChanged = QDateTime::currentDateTime();

    if(!m_vso) {
        QByteArray obj = QtopiaServerApplication::taskValueSpaceObject("GenericMemoryMonitorTask");
        if(!obj.isNull())
            m_vso = new QValueSpaceObject(obj, this);
    }

    if(m_vso) {
        m_vso->setAttribute("ChangedTime", m_lastChanged);
        QByteArray stateName;
        switch(m_memstate) {
            case MemUnknown: stateName = "Unknown"; break;
            case MemCritical: stateName = "Critical"; break;
            case MemVeryLow: stateName = "VeryLow"; break;
            case MemLow: stateName = "Low"; break;
            case MemNormal: stateName = "Normal"; break;
        };
        m_vso->setAttribute("MemoryLevel", stateName);
    }

    emit memoryStateChanged(m_memstate);
}

/*!  \internal */
GenericMemoryMonitorTask::MemState GenericMemoryMonitorTask::memoryState() const
{
    return m_memstate;
}

/*!  \internal */
unsigned int GenericMemoryMonitorTask::timeInState() const
{
    return m_lastChanged.secsTo(QDateTime::currentDateTime());
}

QTOPIA_TASK(GenericMemoryMonitor, GenericMemoryMonitorTask);
QTOPIA_TASK_PROVIDES(GenericMemoryMonitor, MemoryMonitor);
