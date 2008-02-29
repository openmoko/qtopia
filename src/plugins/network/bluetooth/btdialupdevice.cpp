/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "btdialupdevice.h"

#include <QDebug>
#include <QTimer>

#include <qtopialog.h>

#include <qbluetoothlocaldevicemanager.h>
#include <qbluetoothrfcommserialport.h>

#include <QBluetoothRemoteDevice>
#include <qsdpservice.h>

BluetoothDialupDevice::BluetoothDialupDevice( QObject* parent )
    : QObject( parent ), btDevice( 0 ), serialPort( 0 )
{
    btManager = new QBluetoothLocalDeviceManager( this );
    connect( btManager, SIGNAL(deviceAdded(const QString&)),
                this, SLOT(devAdded(const QString&)) );
    connect( btManager, SIGNAL(deviceRemoved(const QString&)),
                this, SLOT(devRemoved(const QString&)) );
    knownDevices = btManager->devices();

    reconnectDevice();

    connect( &m_sdap, SIGNAL(searchComplete(const QSDAPSearchResult&)),
            this, SLOT(searchComplete(const QSDAPSearchResult&)) );
    remoteAddress = QBluetoothAddress::invalid;
}

BluetoothDialupDevice::~BluetoothDialupDevice()
{
}

QString BluetoothDialupDevice::name() const
{
    return btDeviceName;
}

QByteArray BluetoothDialupDevice::rfcommDevice() const
{
    if ( serialPort )
        return serialPort->boundDevice().toAscii();
    else
        return QByteArray();
}

bool BluetoothDialupDevice::isAvailable( const QString& devName )
{
    if ( devName.isEmpty() )
        return false;

    if ( btDeviceName == devName
            && btDevice->isValid()
            && btDevice->isUp()
            && knownDevices.contains( devName ) ) {
            return true;
    }

    return false;
}

void BluetoothDialupDevice::devAdded( const QString& devName )
{
    bool wasEmpty = knownDevices.isEmpty();
    if ( !knownDevices.contains( devName ) )
        knownDevices.append( devName );

    if ( wasEmpty ) {
        reconnectDevice();
    }
    if ( knownDevices.count() == 1 && wasEmpty )
        emit deviceStateChanged();
}

void BluetoothDialupDevice::devRemoved( const QString& devName )
{
    if ( knownDevices.isEmpty() )
        return;
    if ( knownDevices.contains( devName ) ) {
        knownDevices.removeAll( devName );
    }

   if ( btDeviceName == devName ) {
        delete btDevice;
        btDevice = 0;
        if ( knownDevices.isEmpty() ) {
            btDeviceName = "";
        } else {
            reconnectDevice();
        }
        emit deviceStateChanged();
    } else {
        //do nothing
    }
}

void BluetoothDialupDevice::deviceStateChanged( QBluetoothLocalDevice::State /*state*/ )
{
    emit deviceStateChanged();
}

void BluetoothDialupDevice::reconnectDevice()
{
    if ( btDevice ) {
        delete btDevice;
        btDevice = 0;
    }
    if ( serialPort ) {
        delete serialPort;
        serialPort = 0;
    }
    remoteAddress = QBluetoothAddress::invalid;

    btDeviceName = btManager->defaultDevice();
    btDevice = new QBluetoothLocalDevice( btDeviceName, this );
    if ( btDevice->isValid() ) {
        connect( btDevice, SIGNAL(stateChanged(QBluetoothLocalDevice::State)),
            this, SLOT(deviceStateChanged(QBluetoothLocalDevice::State)) );
    } else {
        delete btDevice;
        btDevice = 0;
        btDeviceName = "";
    }
}

void BluetoothDialupDevice::connectToDUNService( const QBluetoothAddress& remote )
{
    if ( !btDevice ) {
        qLog(Network) << "Cannot search for Dialup Service due to missing Bluetooth device";
        return;
    }
    qLog(Network) << "Searching for Dialup Networking Profile";
    remoteAddress = remote;
    m_sdap.cancelSearch();
    m_sdap.searchServices( remote, *btDevice, QBluetooth::DialupNetworkingProfile  );
}

void BluetoothDialupDevice::releaseDUNConnection()
{
    if ( serialPort ) {
        serialPort->releaseTty();
        delete serialPort;
        serialPort = 0;
        remoteAddress = QBluetoothAddress::invalid;
    }
}

bool BluetoothDialupDevice::hasActiveConnection()
{
    if ( serialPort
            && !serialPort->boundDevice().isEmpty()
            && remoteAddress.valid() )
        return true;
    return false;
}

void BluetoothDialupDevice::searchComplete( const QSDAPSearchResult& result )
{
    qLog(Network) << "Search for remote Bluetooth Dialup Networking Service complete";
    foreach( QSDPService service, result.services() ) {
        if ( QSDPService::isInstance( service, QBluetooth::DialupNetworkingProfile ) ) {
            int channel = QSDPService::rfcommChannel( service );
            if ( serialPort )
                delete serialPort;

            serialPort = new QBluetoothRfcommSerialPort( this );
            QByteArray tty = serialPort->createTty( btDevice->address(), remoteAddress, channel ).toLatin1().constData();

            if ( tty.isEmpty() ) {
                qLog(Network) << "Cannot create serial device for DUN";
                delete serialPort;
                serialPort = 0;
                remoteAddress = QBluetoothAddress::invalid;
            }

            //we have to submit this even though tty might be empty
            //the receiver interprets an empty string as error

            //sleep for 1s. creation of the rfcomm tty port takes a bit of time
            QTimer::singleShot( 1000, this, SIGNAL(connectionEstablished()) );
            return;
        }
    }
    //we couldn't find such a service
    //the receiver of this signal will discover that the rfcomm device is invalid
    QTimer::singleShot( 0, this, SIGNAL(connectionEstablished()) );
}
