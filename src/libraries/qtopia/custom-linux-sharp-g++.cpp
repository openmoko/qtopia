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
#include <qtopia/custom.h>

#include <qfile.h>

#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

int qpe_sysBrightnessSteps()
{
    return 4;
}


void qpe_setBrightness(int bright)
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

void PowerStatusManager::getStatus()
{
    int ac, bs, bf, pc, sec;
    ps->percentAccurate = TRUE; // not for long...

    if ( getProcApmStatus( ac, bs, bf, pc, sec ) ) {
	// special case
	if ( bs == 0x7f )
	    ps->bs = PowerStatus::VeryLow;
	pc = -1; // fake percentage
	if ( pc < 0 ) {
	    switch ( bs ) {
		case 0x00: ps->percentRemain = 100; break; // High
		case 0x01: ps->percentRemain = 30; break; // Low
		case 0x7f: ps->percentRemain = 10; break; // Very Low
		case 0x02: ps->percentRemain = 5; break; // Critical
		case 0x03: ps->percentRemain = -1; break; // Charging
	    }
	    ps->percentAccurate = FALSE;
	}
    }

    char *device = "/dev/apm_bios";
    int fd = ::open (device, O_WRONLY);
    if ( fd >= 0 ) {
	int bbat_status = ioctl( fd, APM_IOC_BATTERY_BACK_CHK, 0 );
	switch ( bbat_status ) {
	    case 0x00:
		ps->bbs = PowerStatus::High;
		break;
	    case 0x01:
		ps->bbs = PowerStatus::Low;
		break;
	    case 0x7f:
		ps->bbs = PowerStatus::VeryLow;
		break;
	    case 0x02:
		ps->bbs = PowerStatus::Critical;
		break;
	    case 0x03:
		ps->bbs = PowerStatus::Charging;
		break;
	    case 0x04:
		ps->bbs = PowerStatus::NotPresent;
		break;
	}
	::close(fd);
    }
}

#define SoftKey	0x8000

static int buzzer_fd()
{
    static int fd = -1;
    if (fd < 0)
    	fd = ::open( "/dev/sharp_buz", O_RDWR|O_NONBLOCK );
    return fd;
}

extern int qtopia_muted;

void qtopia_buz_touch(bool press)
{
    static bool isPenTouch = FALSE;
    int fd = buzzer_fd();
    if (press) {
	if (!isPenTouch) {
	    if (!qtopia_muted && fd >= 0)
		ioctl(fd, SHARP_BUZZER_MAKESOUND, SHARP_BUZ_TOUCHSOUND);
	    isPenTouch = TRUE;
	}
    } else
	isPenTouch = FALSE;
}

void qtopia_buz_key(int keycode, bool press, bool repeat)
{
    if (press) {
	if (repeat && keycode == Qt::Key_F33)
	    return;
	if (keycode != Qt::Key_F34 &&
	    keycode != Qt::Key_F22 &&
	    keycode != Qt::Key_Shift &&
	    keycode != Qt::Key_Meta)
	{
	    int fd = buzzer_fd();
	    if (fd >= 0 && !qtopia_muted) {
		//if (ke->simpleData.modifiers & SoftKey)
		    //ioctl(fd, SHARP_BUZZER_MAKESOUND, SHARP_BUZ_TOUCHSOUND);
		//else
		    ioctl(fd, SHARP_BUZZER_MAKESOUND, SHARP_BUZ_KEYSOUND);
	    }
	}
    }
}


