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

#include "ui/criticalmemorypopup.h"
#include "qtopiaserverapplication.h"

CriticalMemoryPopup::CriticalMemoryPopup()
: QMessageBox(tr("Memory Status"),
              tr("<qt>Critical Memory Shortage. "
                 "Please end applications as soon as possible.</qt>"),
              QMessageBox::Critical, QMessageBox::Ok|QMessageBox::Default,
              QMessageBox::NoButton, QMessageBox::NoButton, 0)
{

    m_monitor = qtopiaTask<MemoryMonitor>();
    if(!m_monitor) return;
    QObject::connect(m_monitor,
                     SIGNAL(memoryStateChanged(MemoryMonitor::MemState)),
                     this, SLOT(memoryStateChanged(MemoryMonitor::MemState)));

    connect(&m_delayedVMInfo, SIGNAL(timeout()), this, SLOT(showToUser()));

    //force dialog to be loaded and generated to ensure quick response
    //if system is short of memory
    show();
    hide();
}

void CriticalMemoryPopup::memoryStateChanged(MemoryMonitor::MemState state)
{
    switch(state) {
        case MemoryMonitor::MemVeryLow:
            //delay dialog box because it increases
            //the page faults and thus causing a crash in a tight
            //situation
            m_delayedVMInfo.start(20000);
            break;

        case MemoryMonitor::MemCritical:
            //dont make it worse by poping up a 2nd dialog
            if (m_delayedVMInfo.isActive())
                m_delayedVMInfo.stop();
            break;

        default:
            break;
    }
}

void CriticalMemoryPopup::showToUser()
{
    if(isVisible())
        return; // don't show a second message while still on first

    if (m_monitor->memoryState() == MemoryMonitor::MemVeryLow ||
        m_monitor->memoryState()  == MemoryMonitor::MemCritical) {
        m_delayedVMInfo.start(10000); //system too busy atm
        return;
    }

    exec();
}

