/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifdef QTOPIA_BLUETOOTH

#include <qwindowsystem_qws.h>

#include <QtopiaIpcAdaptor>
#include <QProcess>

#include "systemsuspend.h"

#include <qbluetoothlocaldevice.h>
#include <qvaluespace.h>
#include <qtopialog.h>

class BluetoothSuspend : public SystemSuspendHandler
{
public:
    BluetoothSuspend();
    ~BluetoothSuspend();

    virtual bool canSuspend() const;
    virtual bool suspend();
    virtual bool wake();

private:
    QProcess m_process;
};

QTOPIA_TASK(BluetoothSuspend, BluetoothSuspend)
QTOPIA_TASK_PROVIDES(BluetoothSuspend, SystemSuspendHandler)

BluetoothSuspend::BluetoothSuspend()
{
    QObject::connect(&m_process, SIGNAL(finished(int)), this, SIGNAL(operationCompleted()));
}

BluetoothSuspend::~BluetoothSuspend()
{
}

bool BluetoothSuspend::canSuspend() const
{
    QByteArray path("/Hardware/Devices/");
    QBluetoothLocalDevice dev;

    path.append(dev.deviceName());
    QValueSpaceItem activeSessions(path);

    bool ret = !activeSessions.value("ActiveSessions", false).toBool();

    qLog(Bluetooth) << "Bluetooth::canSuspend returning: " << ret;

    return ret;
}

bool BluetoothSuspend::suspend()
{
    qLog(Bluetooth) << "Bluetooth Suspend called";
    return true;
}

bool BluetoothSuspend::wake()
{
    qLog(Bluetooth) << "Wake called!!!";

    m_process.start("bluetooth-wakeup.sh");

    qLog(Bluetooth) << "Successfully woken up the device";

    return false;
}

#endif
