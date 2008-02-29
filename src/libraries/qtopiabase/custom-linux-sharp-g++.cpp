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

#include <qpowerstatus.h>
#include <qwindowsystem_qws.h>

#include <custom.h>

#include <qfile.h>

#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

QTOPIABASE_EXPORT int qpe_sysBrightnessSteps()
{
    return 4;
}


QTOPIABASE_EXPORT void qpe_setBrightness(int bright)
{
    if ( QFile::exists("/dev/fl") ) {
#define FL_IOCTL_STEP_CONTRAST    100
        int fd = open("/dev/fl", O_WRONLY);
        if (fd >= 0 ) {
            int steps = qpe_sysBrightnessSteps();
            int bl = ( bright * steps + 127 ) / 255;
            if ( bright && !bl ) bl = 1;
            bl = ioctl(fd, FL_IOCTL_STEP_CONTRAST, bl);
            close(fd);
        }
    }
}

QTOPIABASE_EXPORT void QPowerStatusManager::getStatus()
{
    int ac, bs, bf, pc, sec;
    ps->percentAccurate = true; // not for long...

    if ( getProcApmStatus( ac, bs, bf, pc, sec ) ) {
        // special case
        if ( bs == 0x7f )
            ps->bs = QPowerStatus::VeryLow;
        pc = -1; // fake percentage
        if ( pc < 0 ) {
            switch ( bs ) {
                case 0x00: ps->percentRemain = 100; break; // High
                case 0x01: ps->percentRemain = 30; break; // Low
                case 0x7f: ps->percentRemain = 10; break; // Very Low
                case 0x02: ps->percentRemain = 5; break; // Critical
                case 0x03: ps->percentRemain = -1; break; // Charging
            }
            ps->percentAccurate = false;
        }
    }

    char *device = "/dev/apm_bios";
    int fd = ::open (device, O_WRONLY);
    if ( fd >= 0 ) {
        int bbat_status = ioctl( fd, APM_IOC_BATTERY_BACK_CHK, 0 );
        switch ( bbat_status ) {
            case 0x00:
                ps->bbs = QPowerStatus::High;
                break;
            case 0x01:
                ps->bbs = QPowerStatus::Low;
                break;
            case 0x7f:
                ps->bbs = QPowerStatus::VeryLow;
                break;
            case 0x02:
                ps->bbs = QPowerStatus::Critical;
                break;
            case 0x03:
                ps->bbs = QPowerStatus::Charging;
                break;
            case 0x04:
                ps->bbs = QPowerStatus::NotPresent;
                break;
        }
        ::close(fd);
    }
}

#define SoftKey 0x8000

static int buzzer_fd()
{
    static int fd = -1;
    if (fd < 0)
        fd = ::open( "/dev/sharp_buz", O_RDWR|O_NONBLOCK );
    return fd;
}

extern int qtopia_muted;
