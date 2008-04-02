/***************************************************************************
**
** Copyright (C) 2008 Openmoko Inc.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2 or at your option any later version.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************/

// Parts copied from neosuspend.cpp
/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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


#ifdef QT_ILLUME_LAUNCHER

#include "apmsuspendobserver.h"

#include <qtopiaserverapplication.h>
#include <qlog.h>

#include <QtopiaIpcAdaptor>
#include <QtopiaIpcEnvelope>

#include <QSocketNotifier>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>


QTOPIA_DEMAND_TASK(NeoSuspend, APMSuspendObserver);

APMSuspendObserver::APMSuspendObserver()
    : m_apmBiosNotifier(0)
    , m_apmBiosFd(-1)
{
    m_apmBiosFd = open("/dev/apm_bios", O_RDWR);

    if (m_apmBiosFd < 0) {
        perror("apm_bios");
        return;
    }

    m_apmBiosNotifier = new QSocketNotifier(m_apmBiosFd, QSocketNotifier::Read, this);
    connect(m_apmBiosNotifier, SIGNAL(activated(int)), SLOT(readApmEvents()));

    QtopiaIpcAdaptor* modemSuspend = new QtopiaIpcAdaptor("QPE/ModemSuspend", this);
    QtopiaIpcAdaptor::connect(modemSuspend, MESSAGE(suspendDone()),
                              this, SLOT(modemSuspendDone()));
    
}


/*
 * Only suspend and treat all three suspend reasons equally
 */
void APMSuspendObserver::readApmEvents()
{
    int retval = read(m_apmBiosFd, (void*)&m_apmEvents, sizeof(m_apmEvents));

    if (retval < 0) {
        perror("Failed to read");
        return;
    }

    for (unsigned i = 0; i < retval/sizeof(apm_event_t); ++i) {
        qLog(PowerManagement) << "APM Event" << m_apmEvents[i];
        switch(m_apmEvents[i]) {
        case APM_SYS_SUSPEND:
        case APM_CRITICAL_SUSPEND:
        case APM_USER_SUSPEND:
            qLog(PowerManagement) << "Suspend event";
            handleSuspendEvent();
            break;

        case APM_NORMAL_RESUME:
        case APM_CRITICAL_RESUME:
            qLog(PowerManagement) << "Resume event";
            handleResumeEvent();
            break;

        case APM_SYS_STANDBY:
        case APM_LOW_BATTERY:
        case APM_POWER_STATUS_CHANGE:
        case APM_UPDATE_TIME:
        case APM_USER_STANDBY:
        case APM_STANDBY_RESUME:
        case APM_CAPABILITY_CHANGE:
        default:
            qLog(PowerManagement) << "Unhandled event";
            break;
        }
    }
}

void APMSuspendObserver::modemSuspendDone()
{
    ackSuspend();
}

void APMSuspendObserver::handleSuspendEvent()
{
    QtopiaIpcEnvelope amp("QPE/AudioVolumeManager/Ficgta01VolumeService", "setAmpMode(bool)");
    amp << false;

    qLog(PowerManagement) << "Asking the modem to suspend, waiting for ack";
    QtopiaIpcEnvelope modemSuspend("QPE/ModemSuspend", "suspend()");
}

void APMSuspendObserver::handleResumeEvent()
{
    QtopiaIpcEnvelope modemWake("QPE/ModemSuspend", "wake()");

    QtopiaIpcEnvelope amp("QPE/AudioVolumeManager/Ficgta01VolumeService", "setAmpMode(bool)");
    amp << true;

    // The state of media and networking might have changed
    QtopiaIpcEnvelope("QPE/Card", "mtabChanged()" );
    QtopiaIpcEnvelope("QPE/NetworkState", "updateNetwork()");
}

void APMSuspendObserver::ackSuspend()
{
    qLog(PowerManagement) << "Acking suspend";
    sync();
    int retval = ioctl(m_apmBiosFd, APM_IOC_SUSPEND, NULL);
    qLog(PowerManagement) << "Acked suspend" << retval;
}


#endif
