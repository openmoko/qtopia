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

#ifdef QT_QWS_CUSTOM
#include "custom.h"
#endif

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#ifndef Q_OS_WIN32
#include <sys/ioctl.h>
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
    if ( !powerManager )
	(void)new PowerStatusManager;

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
    }

    return ok;
}

