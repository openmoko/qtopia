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

#include "power.h"

#include "custom.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#if defined(Q_OS_LINUX) || defined(_OS_LINUX_)
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#ifdef QT_QWS_IPAQ_NO_APM
#include <linux/h3600_ts.h>
#endif

PowerStatusManager *PowerStatusManager::powerManager = 0;
PowerStatus *PowerStatusManager::ps = 0;

PowerStatusManager::PowerStatusManager()
{
    powerManager = this;
    ps = new PowerStatus;
}

const PowerStatus &PowerStatusManager::readStatus()
{
    if ( !powerManager ) {
	(void)new PowerStatusManager;
    }

    powerManager->getStatus();

    return *ps;
}

// Standard /proc/apm reader
bool PowerStatusManager::getProcApmStatus( int &ac, int &bs, int &bf, int &pc, int &sec )
{
    bool ok = false;

    ac = 0xff;
    bs = 0xff;
    bf = 0xff;
    pc = -1;
    sec = -1;

    FILE *f = fopen("/proc/apm", "r");
    if ( f ) {
	//I 1.13 1.2 0x02 0x00 0xff 0xff 49% 147 sec
	char u;
	fscanf(f, "%*[^ ] %*d.%*d 0x%*x 0x%x 0x%x 0x%x %d%% %i %c",
		&ac, &bs, &bf, &pc, &sec, &u);
	fclose(f);
	switch ( u ) {
	    case 'm': sec *= 60;
	    case 's': break; // ok
	    default: sec = -1; // unknown
	}

	// extract data
	switch ( bs ) {
	    case 0x00:
		ps->bs = PowerStatus::High;
		break;
	    case 0x01:
		ps->bs = PowerStatus::Low;
		break;
	    case 0x7f:
		ps->bs = PowerStatus::VeryLow;
		break;
	    case 0x02:
		ps->bs = PowerStatus::Critical;
		break;
	    case 0x03:
		ps->bs = PowerStatus::Charging;
		break;
	    case 0x04:
	    case 0xff: // 0xff is Unknown but we map to NotPresent
	    default:
		ps->bs = PowerStatus::NotPresent;
		break;
	}

	switch ( ac ) {
	    case 0x00:
		ps->ac = PowerStatus::Offline;
		break;
	    case 0x01:
		ps->ac = PowerStatus::Online;
		break;
	    case 0x02:
		ps->ac = PowerStatus::Backup;
		break;
	}

	if ( pc > 100 )
	    pc = -1;

	ps->percentRemain = pc;
	ps->secsRemain = sec;

	ok = true;
    } else {
       ps->bs = PowerStatus::NotPresent;
       pc = sec = -1;
       ps->percentRemain = -1;
       ps->secsRemain = -1;
    }
    

    return ok;
}

const bool PowerStatusManager::APMEnabled() 
{
    int apm_install_flags;
    FILE *f = fopen("/proc/apm", "r");
    if ( f ) {
	//I 1.13 1.2 0x02 0x00 0xff 0xff 49% 147 sec
	fscanf(f, "%*[^ ] %*d.%*d 0x%x 0x%*x 0x%*x 0x%*x %*d%% %*i %*c",
		&apm_install_flags);
	fclose(f);

        if (!(apm_install_flags & 0x08)) //!APM_BIOS_DISABLED
        {
            return TRUE;
        }
    }
    return FALSE;
}
