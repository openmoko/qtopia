/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qtopia/power.h"

#include <qwindowsystem_qws.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <qfile.h>

int qpe_sysBrightnessSteps()
{
    return 255;
}


void qpe_setBrightness(int bright)
{
    if ( QFile::exists("/usr/bin/bl") ) {
	QString cmd = "/usr/bin/bl 1 ";
	cmd += bright<=0 ? "0 " : "1 ";
	cmd += QString::number(bright);
	system(cmd.latin1());
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

void PowerStatusManager::getStatus()
{
    bool usedApm = FALSE;

    ps->percentAccurate = TRUE;

    // Some iPAQ kernel builds don't have APM. If this is not the case we
    // save ourselves an ioctl by testing if /proc/apm exists in the
    // constructor and we use /proc/apm instead

    int ac, bs, bf, pc, sec;
    if ( getProcApmStatus( ac, bs, bf, pc, sec ) ) {
	ps->percentRemain = pc;
	ps->secsRemain = sec;
	ps->percentAccurate = TRUE;
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
	ps->percentAccurate = FALSE;
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
   { 0x7a,   {   Qt::Key_Call,	    0xffff, 0xffff, 0xffff  } },
   { 0x7b,   {   Qt::Key_Context1,  0xffff, 0xffff, 0xffff  } },
   { 0x7c,   {   Qt::Key_Back,	    0xffff, 0xffff, 0xffff  } },
   { 0x7d,   {   Qt::Key_Hangup,    0xffff, 0xffff, 0xffff  } },
#else
   { 0x7a,   {   Qt::Key_F9,	    0xffff, 0xffff, 0xffff  } },
   { 0x7b,   {   Qt::Key_F10,	    0xffff, 0xffff, 0xffff  } },
   { 0x7c,   {   Qt::Key_F11,	    0xffff, 0xffff, 0xffff  } },
   { 0x7d,   {   Qt::Key_F12,	    0xffff, 0xffff, 0xffff  } },
#endif
   { 0,	     {   0,		    0xffff, 0xffff, 0xffff  } }
};

const KeyOverride* qtopia_override_keys()
{
    return deviceKeys;
}
