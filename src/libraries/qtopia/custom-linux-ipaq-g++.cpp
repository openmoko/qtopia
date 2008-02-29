/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "power.h"

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
