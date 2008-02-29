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

#include <QValueSpaceItem>
#include <qtopianamespace.h>
#include <qpowerstatus.h>
#include <qwindowsystem_qws.h>
#include "custom.h"

#include "include/ipmc.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define _LCDCTRL_IOCTL_BRIGHTNESS 4
#define PM_DISPLAY_ON           10
#define PM_DISPLAY_OFF          11
#define KPBL_ON                 1
#define KPBL_OFF                2


QTOPIA_EXPORT int qpe_sysBrightnessSteps()
{
    return 11;
}

QTOPIA_EXPORT void qpe_setBrightness(int b)
{
    int lcdFd = ::open("/dev/lcdctrl", O_RDWR);
    if(lcdFd >= 0) {
        ::ioctl(lcdFd, _LCDCTRL_IOCTL_BRIGHTNESS, 10*b/255);
        ::close(lcdFd);
    }  

    int ipmcFd = ::open("/dev/ipmc", O_RDWR);
    if (ipmcFd >= 0) {
        if (b == 0) {
            ::ioctl(ipmcFd, IPMC_IOCTL_SEND_PMCOMM, PM_DISPLAY_OFF);
        } else if (b == 1) {
            ::ioctl(ipmcFd, IPMC_IOCTL_SEND_PMCOMM, PM_DISPLAY_ON);

            int kpblFd = ::open("/dev/omega_kpbl", O_RDWR);
            if (kpblFd >= 0) {
                ::ioctl(kpblFd, KPBL_OFF, 0);
                ::close(kpblFd);
            }
        } else {
            ::ioctl(ipmcFd, IPMC_IOCTL_SEND_PMCOMM, PM_DISPLAY_ON);
        }

        ::close(ipmcFd);
    }
}

void QPowerStatusManager::getStatus()
{
    // ac = AC line status
    // bs = battery status
    // percentRemain = remaining battery life as percent
    // secsRemain = remaining battery life in time units


    QValueSpaceItem battery("/Accessories/Battery");
    bool charging = battery.value("Charging", false).toBool();
    int batteryLevel = battery.value("Charge", -1).toInt();

    if (batteryLevel >= 0) {
        ps->percentRemain = batteryLevel;

        if (charging)
            ps->ac = QPowerStatus::Online;
        else
            ps->ac = QPowerStatus::Offline;

        if (charging && batteryLevel < 4)
            ps->bs = QPowerStatus::Charging;
        else if (batteryLevel == 0)
            ps->bs = QPowerStatus::Critical;
        else if (batteryLevel == 1)
            ps->bs = QPowerStatus::VeryLow;
        else if (batteryLevel == 2)
            ps->bs = QPowerStatus::Low;
        else if (batteryLevel == 3)
            ps->bs = QPowerStatus::High;
        else if (batteryLevel >= 4)
            ps->bs = QPowerStatus::High;
    } else {
        ps->bs = QPowerStatus::NotPresent;
        ps->percentRemain = -1;
        ps->secsRemain = -1;
        ps->ac = QPowerStatus::Unknown;
        ps->percentAccurate = false;
    }
}

