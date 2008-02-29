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

#include <qbluetoothabstractservice.h>

#include <qtopiacomm/private/qbluetoothservicecontrolserver_p.h>

#include <qtopialog.h>

/*!
    \class QBluetoothServiceControlServer
    \brief The QBluetoothServiceControlServer class provides a control interface for a QBluetoothAbstractService using Qtopia IPC mechanisms.

    A QBluetoothServiceControlServer provides a control interface for a
    QBluetoothAbstractService using the IPC mechanisms in the Qtopia
    framework. A service control server can be added as an IPC interface for
    a QAbstractIpcInterfaceGroup object.

    Normally this class will not need to be used directly; this is used by
    QBluetoothAbstractService, so any Bluetooth service that subclasses
    QBluetoothAbstractService will automatically have a
    QBluetoothServiceControlServer object included as an IPC interface.

    If you wish to have a Bluetooth service class that can be controlled
    through this interface (i.e. make it registered and accessible as a
    system Bluetooth service) but you don't want to subclass
    QBluetoothAbstractService, simply subclass QAbstractIpcInterfaceGroup
    and call QAbstractIpcInterfaceGroup::addInterface() with an instance of
    QBluetoothServiceControlServer inside the initialize() method. See the
    QAbstractIpcInterfaceGroup documentation for more details.

    \ingroup qtopiabluetooth
    \sa QBluetoothAbstractService, QAbstractIpcInterfaceGroup, QAbstractIpcInterface
 */

 /*!
    Constructs a QBluetoothServiceControlServer that will control the service
    \a service. The \a parent parameter specifies the QObject parent.
 */
QBluetoothServiceControlServer::QBluetoothServiceControlServer(QBluetoothAbstractService *service, QObject *parent)
    : QBluetoothServiceControl(service->groupName(), parent, QCommInterface::Server),
      m_service(service)
{
    connect(service, SIGNAL(started(QBluetooth::ServiceError, const QString &)),
        SIGNAL(started(QBluetooth::ServiceError, const QString &)));
    connect(service, SIGNAL(stopped(QBluetooth::ServiceError, const QString &)),
        SIGNAL(stopped(QBluetooth::ServiceError, const QString &)));
    connect(service, SIGNAL(error(QBluetooth::ServiceError, const QString &)),
        SIGNAL(error(QBluetooth::ServiceError, const QString &)));

    // must use QCommInterface::Immediate to avoid flushing event loop,
    // so that the BluetoothServiceManager will be able to get the display
    // name straight away (by calling translatableDisplayName() here) as
    // soon as the service is created
    setValue(KEY_DISPLAY_NAME, service->translatableDisplayName(),
             QCommInterface::Immediate);
}

/*!
    Deconstructs a QBluetoothServiceControlServer.
 */
QBluetoothServiceControlServer::~QBluetoothServiceControlServer()
{
}

/*!
    Starts the bluetooth service controlled by this server on \a channel.
 */
void QBluetoothServiceControlServer::start(int channel)
{
    m_service->start(channel);
}

/*!
    Stops the bluetooth service controlled by this server.
 */
void QBluetoothServiceControlServer::stop()
{
    m_service->stop();
}

/*!
    Sets the security options \a options for the bluetooth service controlled by
    this server.
 */
void QBluetoothServiceControlServer::setSecurityOptions(QBluetooth::SecurityOptions options)
{
    m_service->setSecurityOptions(options);
}
