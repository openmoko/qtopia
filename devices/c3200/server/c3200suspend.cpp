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

#include <QtopiaIpcAdaptor>
#include <QtopiaIpcEnvelope>
#include <QtopiaServiceRequest>
#include <QPowerSource>

#include "systemsuspend.h"

#include <qvaluespace.h>

class C3200Suspend : public SystemSuspendHandler
{
public:
    C3200Suspend();
    virtual bool canSuspend() const;
    virtual bool suspend();
    virtual bool wake();
};

QTOPIA_DEMAND_TASK(C3200Suspend, C3200Suspend);
QTOPIA_TASK_PROVIDES(C3200Suspend, SystemSuspendHandler);

C3200Suspend::C3200Suspend()
{
}

bool C3200Suspend::canSuspend() const
{
    QPowerSource src( QLatin1String("DefaultBattery") );
    return !src.charging();
}

bool C3200Suspend::suspend()
{
    system("apm --suspend");
    return true;
}

bool C3200Suspend::wake()
{
    QWSServer::screenSaverActivate( false );
    {
        QtopiaIpcEnvelope("QPE/Card", "mtabChanged()" ); // might have changed while asleep
        QtopiaServiceRequest e("QtopiaPowerManager", "setBacklight(int)");
        e << -3; // Force on
        e.send();
    }

    return true;
}

