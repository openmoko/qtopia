/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <qfile.h>

QTOPIABASE_EXPORT int qpe_sysBrightnessSteps()
{
    return 255;
}


QTOPIABASE_EXPORT void qpe_setBrightness(int bright)
{
    if ( QFile::exists("/usr/bin/bl") ) {
        QString cmd = "/usr/bin/bl 1 ";
        cmd += bright<=0 ? "0 " : "1 ";
        cmd += QString::number(bright);
        system(cmd.toLatin1());
    } else if ( QFile::exists("/dev/ts") || QFile::exists("/dev/h3600_ts") ) {
        typedef struct {
            unsigned char mode;
            unsigned char pwr;
            unsigned char brightness;
        } FLITE_IN;
# ifndef FLITE_ON
#  ifndef _LINUX_IOCTL_H
#   include <linux/ioctl.h>
#  endif
#  define FLITE_ON                _IOW('f', 7, FLITE_IN)
# endif
        int fd;
        if ( QFile::exists("/dev/ts") )
            fd = open("/dev/ts", O_WRONLY);
        else
            fd = open("/dev/h3600_ts", O_WRONLY);
        if (fd >= 0 ) {
            FLITE_IN bl;
            bl.mode = 1;
            bl.pwr = bright ? 1 : 0;
            bl.brightness = bright;
            ioctl(fd, FLITE_ON, &bl);
            close(fd);
        }
    }
}

QTOPIABASE_EXPORT void QPowerStatusManager::getStatus()
{
    bool usedApm = false;

    ps->percentAccurate = true;

    // Some iPAQ kernel builds don't have APM. If this is not the case we
    // save ourselves an ioctl by testing if /proc/apm exists in the
    // constructor and we use /proc/apm instead

    int ac, bs, bf, pc, sec;
    if ( getProcApmStatus( ac, bs, bf, pc, sec ) ) {
        ps->percentRemain = pc;
        ps->secsRemain = sec;
        ps->percentAccurate = true;
    } else {
#ifdef QT_QWS_IPAQ_NO_APM
        int fd;
        int err;
        struct bat_dev batt_info;

        memset(&batt_info, 0, sizeof(batt_info));

        fd = ::open("/dev/ts",O_RDONLY);
        if( fd < 0 )
            return;

        ioctl(fd, GET_BATTERY_STATUS, &batt_info);
        ac_status = batt_info.ac_status;
        ps->percentRemain = ( 425 * batt_info.batt1_voltage ) / 1000 - 298; // from h3600_ts.c
        ps->secsRemain = -1; // seconds is bogus on iPAQ
        ::close (fd);
#else
        ps->percentRemain = 100;
        ps->secsRemain = -1;
        ps->percentAccurate = false;
#endif
    }
}

typedef struct KeyOverride {
    ushort scan_code;
    QWSServer::KeyMap map;
};

static const KeyOverride deviceKeys[] = {
   { 0x79,   {   Qt::Key_F34,    0xffff, 0xffff, 0xffff  } }, // power
#if defined(QT_KEYPAD_MODE)
   { 0x7a,   {   Qt::Key_Call,      0xffff, 0xffff, 0xffff  } },
   { 0x7b,   {   Qt::Key_Context1,  0xffff, 0xffff, 0xffff  } },
   { 0x7c,   {   Qt::Key_Back,      0xffff, 0xffff, 0xffff  } },
   { 0x7d,   {   Qt::Key_Hangup,    0xffff, 0xffff, 0xffff  } },
#else
   { 0x7a,   {   Qt::Key_F9,        0xffff, 0xffff, 0xffff  } },
   { 0x7b,   {   Qt::Key_F10,       0xffff, 0xffff, 0xffff  } },
   { 0x7c,   {   Qt::Key_F11,       0xffff, 0xffff, 0xffff  } },
   { 0x7d,   {   Qt::Key_F12,       0xffff, 0xffff, 0xffff  } },
#endif
   { 0,      {   0,                 0xffff, 0xffff, 0xffff  } }
};

QTOPIABASE_EXPORT const KeyOverride* qtopia_override_keys()
{
    return deviceKeys;
}
