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

#ifndef BLUETOOTH_DIALUP_DEVICE_H
#define BLUETOOTH_DIALUP_DEVICE_H
#include <QByteArray>
#include <QObject>
#include <QStringList>

#include <QBluetoothAddress>
#include <QBluetoothLocalDevice>
#include <QBluetoothSdpQuery>
#include <QBluetoothRfcommSerialPort>

class QBluetoothLocalDeviceManager;
class QBluetoothRfcommSocket;

class BluetoothDialupDevice : public QObject
{
    Q_OBJECT
public:
    BluetoothDialupDevice( QObject* parent = 0 );
    ~BluetoothDialupDevice();

    QString name() const;
    bool isAvailable( const QString& devName );
    void connectToDUNService( const QBluetoothAddress& remote );
    void releaseDUNConnection();
    bool hasActiveConnection();
    QByteArray rfcommDevice() const;

signals:
    void deviceStateChanged();
    void connectionEstablished();

private slots:
    void devAdded( const QString& devName );
    void devRemoved( const QString& devName );
    void deviceStateChanged( QBluetoothLocalDevice::State state );
    void searchComplete( const QBluetoothSdpQueryResult& result );
    void serialPortConnected( const QString& boundDevice );
    void serialPortError(QBluetoothRfcommSerialPort::Error error);

private:
    void reconnectDevice();

private:
    QStringList knownDevices;
    QBluetoothLocalDeviceManager* btManager;
    QBluetoothLocalDevice* btDevice;
    QBluetoothAddress remoteAddress;
    QBluetoothRfcommSerialPort* serialPort;
    QBluetoothRfcommSocket *socket;
    QString btDeviceName;
    QBluetoothSdpQuery m_sdap;
};

#endif //BLUETOOTH_DIALUP_DEVICE_H
