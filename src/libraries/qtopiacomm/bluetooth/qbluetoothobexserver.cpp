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

#include <qbluetoothobexserver.h>
#include <qbluetoothobexsocket.h>
#include <qtopialog.h>

#include <QSocketNotifier>
#include <QString>
#include <QTimer>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>

#include <qtopiacomm/private/qbluetoothnamespace_p.h>

#include <openobex/obex.h>
#include <bluetooth/rfcomm.h>

/*!
    \class QBluetoothObexServer
    \brief The QBluetoothObexServer class listens for OBEX client connections.

    The QBluetoothObexServer class can be used to listen for
    new OBEX client connections.  The object binds to an rfcomm
    channel and listens for any client connections.  Once a client
    is connected, server reports a new connection, which should
    then be handled by the individual service implementation,
    e.g. OBEX Push Service, OBEX FTP Service, etc.

    \ingroup qtopiabluetooth
 */

class QBluetoothObexServer_Private : public QObject {
public:
    QBluetoothObexServer_Private(QObject *parent = 0);
    bool listen(quint8 channel, const QBluetoothAddress &addr);

    QBluetoothAddress m_addr;
    obex_t *m_self;
    qint16 m_channel;
};

// Need this to keep OpenOBEX happy, it will be reset as soon as the handle is passed
// to an actual service
static void qobex_dummy_callback(obex_t *, obex_object_t *, int, int, int, int)
{
}

QBluetoothObexServer_Private::QBluetoothObexServer_Private(QObject *parent)
    : QObject(parent)
{
    m_self = OBEX_Init( OBEX_TRANS_BLUETOOTH, qobex_dummy_callback,
                      OBEX_FL_KEEPSERVER );
}

bool QBluetoothObexServer_Private::listen(quint8 channel, const QBluetoothAddress &addr)
{
    bdaddr_t bdaddr;
    str2bdaddr(addr.toString(), &bdaddr);

    qLog(Bluetooth) << "QBluetoothObexServer binding to: " << addr.toString() <<
            " on channel: " << channel;

    if (BtOBEX_ServerRegister(m_self, &bdaddr, channel) < 0) {
        perror("Error reported is: ");
        qLog(Bluetooth) << "ServerRegister failed";
        return false;
    }

    qLog(Bluetooth) << "QBluetoothObexServer configured";

    return true;
}

/*!
    Constructs a new Bluetooth OBEX Server. The \a channel
    parameter specifies the rfcomm channel to listen on.
    If a parameter \a local is provided, then the server
    will only listen on requests addressed to a specific device.
    Otherwise it will listen for requests on all devices.
    The \a parent parameter specifies the \c QObject
    parent of the server.
*/
QBluetoothObexServer::QBluetoothObexServer(quint8 channel,
                                           const QBluetoothAddress &local,
                                           QObject *parent)
    : QObexServer(parent)
{
    m_data = new QBluetoothObexServer_Private(this);
    m_data->m_channel = channel;
    m_data->m_addr = local;
}

/*!
    Deconstructs a Bluetooth OBEX Server.
*/
QBluetoothObexServer::~QBluetoothObexServer()
{
    if (m_data)
        delete m_data;
}

/*!
    This function starts attempts to open a server socket and
    register itself with the OBEX protocol handler.

    Returns a valid QObexHandle if the registration was
    completed successfully, and an invalid one otherwise.
*/
void *QBluetoothObexServer::registerServer()
{
    bool ret = m_data->listen(m_data->m_channel, m_data->m_addr);

    if (!ret)
        return NULL;

    return m_data->m_self;
}

/*!
    Returns the channel the server is currently listening on.  If the server
    is not listening, returns -1
*/
qint16 QBluetoothObexServer::serverChannel() const
{
    return m_data->m_channel;
}

/*!
    Returns the address the server is listening on.  If the server is not listening
    it will return an invalid Bluetooth address.

    \sa listen()
*/
const QBluetoothAddress & QBluetoothObexServer::serverAddress() const
{
    return m_data->m_addr;
}

/*!
    Returns the next pending connection if there are pending connections, otherwise returns
    NULL value.  It is responsibility of the caller to make sure the socket is deleted.
*/
QObexSocket *QBluetoothObexServer::nextPendingConnection()
{
    void *handle = spawnReceiver();

    if (!handle)
        return NULL;

    QBluetoothObexSocket *socket = new QBluetoothObexSocket();
    socket->setHandle(handle);

    qLog(Bluetooth) << "Socket encryption:" << socket->encrypted() <<
            "auth:" << socket->authenticated();

    return socket;
}

/*!
    Returns true if the server authentication security option is set.
    This method will always return false if the server is not
    actively listening for connections.
 */
bool QBluetoothObexServer::authenticated() const
{
    if (!isListening())
        return false;

    return securityOptions() & QBluetooth::Authenticated;
}

/*!
    Returns true if the server encryption security option is set.
    This method will always return false if the server is not
    actively listening for connections.
 */
bool QBluetoothObexServer::encrypted() const
{
    if (!isListening())
        return false;

    return securityOptions() & QBluetooth::Encrypted;
}

/*!
    Sets the security \a options of the server.  This method
    can be called at any time after the server has entered
    the listening state.

    The security options specify whether the server should mandate
    to the remote   clients whether the connection will be
    authenticated, encrypted or both.
 */
bool QBluetoothObexServer::setSecurityOptions(QBluetooth::SecurityOptions options)
{
    if (!isListening())
        return false;

    return _q_setSecurityOptions(OBEX_GetFD(m_data->m_self), options);
}

/*!
    Returns the security options of the server.  If the server
    is not listening, 0 is returned.
 */
QBluetooth::SecurityOptions QBluetoothObexServer::securityOptions() const
{
    QBluetooth::SecurityOptions options = 0;
    if (!isListening())
        return options;

    int fd = OBEX_GetFD(m_data->m_self);

    if (_q_getSecurityOptions(fd, options))
        return options;

    return 0;
}
