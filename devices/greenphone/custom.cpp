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

#include <QValueSpaceItem>
#include <qtopianamespace.h>
#include <qwindowsystem_qws.h>
#include <custom.h>

#include "include/ipmc.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define _LCDCTRL_IOCTL_BRIGHTNESS 4
#define PM_DISPLAY_ON           10
#define PM_DISPLAY_OFF          11
#define KPBL_ON                 1
#define KPBL_OFF                2


QTOPIABASE_EXPORT int qpe_sysBrightnessSteps()
{
    return 11;
}

QTOPIABASE_EXPORT void qpe_setBrightness(int b)
{
    if (b > 0) {
        int lcdFd = ::open("/dev/lcdctrl", O_RDWR);
        if (lcdFd >= 0) {
            ::ioctl(lcdFd, _LCDCTRL_IOCTL_BRIGHTNESS, b-1);
            ::close(lcdFd);
        }
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

