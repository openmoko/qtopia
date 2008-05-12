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

#include <QValueSpaceItem>
#include <qtopianamespace.h>
#include <qwindowsystem_qws.h>
#include "custom.h"

QTOPIA_EXPORT int qpe_sysBrightnessSteps()
{
    return 255;
}

QTOPIA_EXPORT void qpe_setBrightness(int b)
{
    char cmd[80];
    if(b==1) b=10;
    else b=b/10;
    sprintf(cmd,"backlight.sh %d",b);
    system(cmd);
}
