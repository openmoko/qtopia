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

QTOPIA_EXPORT int qpe_sysBrightnessSteps()
{
    return 255;
}


QTOPIA_EXPORT void qpe_setBrightness(int)
{
}

QTOPIA_EXPORT void QPowerStatusManager::getStatus()
{
#ifdef Q_WS_QWS
    int ac, bs, bf, pc, sec;
    if ( getProcApmStatus( ac, bs, bf, pc, sec ) ) {
	ps->percentRemain = pc;
	ps->secsRemain = sec;
	ps->percentAccurate = true;
    } else {
	ps->percentRemain = 100;
	ps->secsRemain = -1;
	ps->percentAccurate = false;
    }
#endif
}
