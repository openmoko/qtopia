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

#include "modememulatorservice.h"
#include "atsessionmanager.h"
#include <qvaluespace.h>
#include <qtopialog.h>

/*!
    \service ModemEmulatorService ModemEmulator
    \brief Provides the Qtopia ModemEmulator service.

    The \i ModemEmulator service enables applications to register
    serial devices and TCP ports with the modem emulator to allow
    external devices to access Qtopia via AT commands.
*/

/*!
    \internal
*/
ModemEmulatorService::ModemEmulatorService( AtSessionManager *parent )
    : QtopiaAbstractService( "ModemEmulator", parent )
{
    publishAll();
    sessions = parent;
    info = new QValueSpaceObject( "/Communications/ModemEmulator", this );
    connect( parent, SIGNAL(devicesChanged()), this, SLOT(updateValueSpace()) );

    pendingTcpPort = 0;
}

/*!
    \internal
*/
ModemEmulatorService::~ModemEmulatorService()
{
}

/*!
    Add \a deviceName to the list of serial ports that are being
    monitored for AT sessions.  The device name will be something
    like \c{/dev/ttyS1:115200}, indicating the kernel device and
    baud rate to use.  Returns false if the device could not be opened.

    \sa removeSerialPort()
*/
void ModemEmulatorService::addSerialPort( const QString& deviceName )
{
    qLog(ModemEmulator) << "add serial port" << deviceName;

    pendingSerialPort = deviceName;
    updateValueSpace();

    sessions->addSerialPort( deviceName );
    pendingSerialPort.clear();
    updateValueSpace();
}

/*!
    Add \a deviceName to the list of serial ports that are being
    monitored for AT sessions.  The \a options parameter specifies
    additional options to configure the initial AT command settings.

    \sa removeSerialPort()
*/
void ModemEmulatorService::addSerialPort
        ( const QString& deviceName, const QString& options)
{
    qLog(ModemEmulator) << "add serial port" << deviceName
                        << "options =" << options;

    pendingSerialPort = deviceName;
    updateValueSpace();

    sessions->addSerialPort( deviceName, options );
    pendingSerialPort.clear();
    updateValueSpace();
}

/*!
    Add \a tcpPort to the list of TCP ports that are being monitored
    for incoming AT sessions.  If \a localHostOnly is true, then
    only allow processes on the local host to connect.  Returns false
    if the TCP port could not be bound.

    This is typically used for debugging the AT interface.  A program
    such as \c telnet can be used to connect to the port for issuing
    AT commands manually.

    Applications can use this for non-debugging access if they connect
    to the port using QSerialSocket.

    \sa removeTcpPort()
*/
void ModemEmulatorService::addTcpPort( int tcpPort, bool localHostOnly )
{
    qLog(ModemEmulator) << "add tcp port" << tcpPort
                        << "localHostOnly =" << localHostOnly;

    pendingTcpPort = tcpPort;
    updateValueSpace();

    sessions->addTcpPort( tcpPort, localHostOnly );
    pendingTcpPort = 0;
    updateValueSpace();
}

/*!
    Add \a tcpPort to the list of TCP ports that are being monitored
    for incoming AT sessions.  If \a localHostOnly is true, then
    only allow processes on the local host to connect.  Returns false
    if the TCP port could not be bound.    The \a options parameter specifies
    additional options to configure the initial AT command settings.

    \sa removeTcpPort()
*/
void ModemEmulatorService::addTcpPort
        ( int tcpPort, bool localHostOnly, const QString& options )
{
    qLog(ModemEmulator) << "add tcp port" << tcpPort
                        << "localHostOnly =" << localHostOnly
                        << "options =" << options;
    pendingTcpPort = tcpPort;
    updateValueSpace();

    sessions->addTcpPort( tcpPort, localHostOnly, options );
    pendingTcpPort = 0;
    updateValueSpace();
}

/*!
    Remove \a deviceName from the list of serial ports that are being
    monitored for AT sessions.  All active sessions associated with
    the device will be destroyed.

    \sa addSerialPort()
*/
void ModemEmulatorService::removeSerialPort( const QString& deviceName )
{
    qLog(ModemEmulator) << "remove serial port" << deviceName;
    sessions->removeSerialPort( deviceName );
    updateValueSpace();
}

/*!
    Remove \a tcpPort from the list of TCP ports that are being
    monitored for incoming AT sessions.  If there are active sessions,
    they will continue until the peer closes the connection.

    \sa addTcpPort()
*/
void ModemEmulatorService::removeTcpPort( int tcpPort )
{
    qLog(ModemEmulator) << "remove tcp port" << tcpPort;
    sessions->removeTcpPort( tcpPort );
    updateValueSpace();
}

void ModemEmulatorService::updateValueSpace()
{
    // This does some trickery with pendingSerialPort / pendingTcpPort
    // Reason is that BluetoothSerialPortService / HF service depend
    // on the ValueSpace entry with all the serial ports to be modified
    // whenever the serial port has gone away.  This can lead to a
    // strange situation where the HF/Serial service could successfully
    // create the serial port, but the modem emulator could not open it
    // e.g. due to security reasons.  This leads to a situation
    // where the hf / serial service are clueless that the serial port
    // has gone away.

    // There is still a race condition, but it shouldn't be too bad:
    // 1. serial service adds port
    // 2. hf service adds port
    // 3. modem emulator processes #1
    // 4. hf service thinks that the port wasn't added, disconnects
    // 5. modem emulator processes #2
    // 6. modem emulator processes #4
    // no way to fix unless this is converted to message response semantics

    QStringList serialPorts = sessions->serialPorts();
    if (!pendingSerialPort.isEmpty())
        serialPorts.append(pendingSerialPort);
    info->setAttribute( "serialPorts", serialPorts );

    QList<int> ports = sessions->tcpPorts();
    QStringList strports;
    foreach ( int port, ports )
        strports += QString::number( port );
    if (pendingTcpPort != 0)
        strports += QString::number( pendingTcpPort );
    info->setAttribute( "tcpPorts", strports );

    QValueSpaceObject::sync();
}
