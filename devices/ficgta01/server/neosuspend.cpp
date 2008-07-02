/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#include <QtopiaIpcAdaptor>
#include <QtopiaIpcEnvelope>
#include <QtopiaServiceRequest>
#include <QPowerSource>
#include <QProcess>
#include <stdio.h>
#include <stdlib.h>

#include "systemsuspend.h"

#include <qvaluespace.h>

#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif

class NeoSuspend : public SystemSuspendHandler
{
public:

  NeoSuspend();
    virtual bool canSuspend() const;
    virtual bool suspend();
    virtual bool wake();
};

QTOPIA_DEMAND_TASK(NeoSuspend, NeoSuspend);
QTOPIA_TASK_PROVIDES(NeoSuspend, SystemSuspendHandler);

NeoSuspend::NeoSuspend()
{
}

bool NeoSuspend::canSuspend() const
{
/*    QPowerSource src( QLatin1String("DefaultBattery") );
    return !src.charging();
*/
    return true;
}

bool NeoSuspend::suspend()
{
    qLog(PowerManagement)<<"NeoSuspend::suspend()";

      QFile powerStateFile("/sys/power/state");
    if( !powerStateFile.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)) {
        qWarning()<<"File not opened";
    } else {
        QTextStream out(&powerStateFile);
        out << "mem";
        powerStateFile.close();
    }
    return true;
}

bool NeoSuspend::wake()
{
    qLog(PowerManagement)<<" NeoSuspend::wake()";

    QtopiaIpcEnvelope("QPE/Card", "mtabChanged()" ); // might have changed while asleep

   //screenSaverActivate uses QTimer which depends on hwclock update
    //when the device wakes up. The clock update is another event in the
    //Qt event loop (see qeventdispatcher_unix.cpp. We have to call
    //processEvents to give Qt a chance to sync its internal timer with
    //the hw clock
#ifdef Q_WS_QWS
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    QWSServer::screenSaverActivate( false );
    {
        QtopiaIpcEnvelope("QPE/Card", "mtabChanged()" ); // might have changed while asleep
        QtopiaServiceRequest e("QtopiaPowerManager", "setBacklight(int)");
        e << -3; // Force on
        e.send();
        QtopiaIpcEnvelope("QPE/NetworkState", "updateNetwork()"); //might have changed
    }
#endif

    return true;
}

