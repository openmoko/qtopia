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

#ifndef BLUETOOTH_IMPL_H
#define BLUETOOTH_IMPL_H

#include <qtopianetworkinterface.h>
#include <scriptthread.h>

class QValueSpaceObject;
class BluetoothDialupDevice;
class QProcess;
class QTimerEvent;
class QCommDeviceSession;
class BluetoothImpl : public QtopiaNetworkInterface
{
    Q_OBJECT
public:
    BluetoothImpl( const QString& confFile );
    virtual ~BluetoothImpl();

    virtual Status status();

    virtual void initialize();
    virtual void cleanup();
    virtual bool start( const QVariant options = QVariant() );
    virtual bool stop();
    virtual QString device() const;
    virtual bool setDefaultGateway();

    virtual QtopiaNetwork::Type type() const;

    virtual QtopiaNetworkConfiguration * configuration();

    virtual void setProperties(
            const QtopiaNetworkProperties& properties);

protected:
    bool isAvailable() const;
    bool isActive() const;

    void timerEvent( QTimerEvent* ev);

private slots:
    void updateState();
    void serialPortConnected();

private:
    enum { Initialize, Connect, Monitoring, Disappearing } state;
    void updateTrigger( QtopiaNetworkInterface::Error e = QtopiaNetworkInterface::NoError, const QString& desc = QString() );

private:
    QtopiaNetworkConfiguration *configIface;
    Status ifaceStatus;
    mutable QString deviceName;
    QValueSpaceObject* netSpace;
    int trigger;

    BluetoothDialupDevice* dialupDev;
    QCommDeviceSession *session;
    ScriptThread thread;
    QByteArray pppIface;
    int tidStateUpdate;
    int logIndex;
};
#endif //BLUETOOTH_IMPL_H
