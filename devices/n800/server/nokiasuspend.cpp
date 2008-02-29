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
#include <QProcess>

#include "systemsuspend.h"

#include <qvaluespace.h>

class NokiaSuspend : public SystemSuspendHandler
{
public:
    NokiaSuspend();
    virtual bool canSuspend() const;
    virtual bool suspend();
    virtual bool wake();
};

QTOPIA_DEMAND_TASK(NokiaSuspend, NokiaSuspend);
QTOPIA_TASK_PROVIDES(NokiaSuspend, SystemSuspendHandler);

NokiaSuspend::NokiaSuspend()
{
}

bool NokiaSuspend::canSuspend() const
{
    QPowerSource src( QLatin1String("DefaultBattery") );
    return !src.charging();
}

bool NokiaSuspend::suspend()
{
    QProcess apm;
    apm.start("apm", QStringList() << "--suspend");
    apm.waitForFinished(-1);
    return true;
}

bool NokiaSuspend::wake()
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

