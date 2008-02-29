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

#define SoftKey 0x8000

static int buzzer_fd()
{
    static int fd = -1;
    if (fd < 0)
        fd = ::open( "/dev/sharp_buz", O_RDWR|O_NONBLOCK );
    return fd;
}

extern int qtopia_muted;
