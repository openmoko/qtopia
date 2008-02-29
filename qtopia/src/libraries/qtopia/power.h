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

#ifndef POWER_H
#define POWER_H

#include <qtopia/qpeglobal.h>
#include <qobject.h>

class QTOPIA_EXPORT PowerStatus
{
public:
    PowerStatus() {
	ac = Offline;
	bs = NotPresent;
	bbs = NotPresent;
	percentRemain = -1;
	secsRemain = -1;
    }

    enum ACStatus { Offline, Online, Backup };
    ACStatus acStatus() const { return ac; }

    enum BatteryStatus { High=0x01, Low=0x02, VeryLow=0x04, Critical=0x08,
			 Charging=0x10, NotPresent=0x20 };
    BatteryStatus batteryStatus() const { return bs; }
    BatteryStatus backupBatteryStatus() const { return bbs; }

    bool batteryPercentAccurate() const { return percentAccurate; }
    int batteryPercentRemaining() const { return percentRemain; }
    int batteryTimeRemaining() const { return secsRemain; }

    bool operator!=( const PowerStatus &ps ) {
	return  (ps.ac != ac) || (ps.bs != bs) || (ps.bbs != bbs) ||
		(ps.percentRemain != percentRemain) ||
		(ps.secsRemain != secsRemain );
    }

private:
    ACStatus ac;
    BatteryStatus bs;
    BatteryStatus bbs;
    int percentRemain;
    int secsRemain;
    bool percentAccurate;

    friend class PowerStatusManager;
};


class QTOPIA_EXPORT PowerStatusManager
{
public:
    PowerStatusManager();

    static const PowerStatus &readStatus();
    static const bool APMEnabled();

protected:
    bool getProcApmStatus( int &ac, int &bs, int &bf, int &pc, int &sec );
    void getStatus();

private:
    static PowerStatus *ps;
    static PowerStatusManager *powerManager;
};

QTOPIA_EXPORT void qpe_setBacklight(int bright); 
QTOPIA_EXPORT int qpe_sysBrightnessSteps();


#endif

