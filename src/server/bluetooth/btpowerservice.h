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

#ifndef __BTPOWERSERVICE_H__
#define __BTPOWERSERVICE_H__

#include "qabstractdevicemanager.h"
#include <qbluetoothlocaldevice.h>


class BtPowerServicePrivate;
class BtPowerService : public QAbstractCommDeviceManager
{
    Q_OBJECT

public:
    BtPowerService(const QByteArray &serverPath,
                    const QByteArray &devId,
                    QObject *parent = 0);
    ~BtPowerService();
    void initialize();

    virtual void bringUp();
    virtual void bringDown();
    virtual bool isUp() const;
    virtual bool shouldBringDown(QUnixSocket *) const;

private slots:
    void stateChanged(QBluetoothLocalDevice::State state);
    void error(QBluetoothLocalDevice::Error error, const QString& msg);
    void planeModeChanged(bool enabled);

private:
    BtPowerServicePrivate *m_data;
};

class BtPowerServiceTask : public QObject
{
    Q_OBJECT

public:
    BtPowerServiceTask(QObject *parent = 0);
    ~BtPowerServiceTask();

private slots:
    void deviceAdded(const QString &devName);
    void deviceRemoved(const QString &devName);
    void startService();

private:
    BtPowerService *m_btPower;
};

#endif
