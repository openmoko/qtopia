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

#include <qbluetoothrfcommsocket.h>
#include "qbluetoothnamespace_p.h"
#include "qbluetoothabstractsocket_p.h"
#include <qbluetoothaddress.h>
#include "qbluetoothsocketengine_p.h"

class QBluetoothRfcommSocketPrivate : public QBluetoothAbstractSocketPrivate
{
public:
    QBluetoothRfcommSocketPrivate();
    QBluetoothAddress m_local;
    QBluetoothAddress m_remote;
    int m_remoteChannel;
};

QBluetoothRfcommSocketPrivate::QBluetoothRfcommSocketPrivate()
    : QBluetoothAbstractSocketPrivate(true)
{
    m_remoteChannel = -1;
}

/*!
    \class QBluetoothRfcommSocket
    \mainclass
    \brief The QBluetoothRfcommSocket class represents an RFCOMM client socket.

    The Bluetooth RFCOMM protocol provides emulation of serial ports over the
    lower-level protocols of Bluetooth, namely L2CAP.  The implementation of RFCOMM
    protocol under Linux uses sockets, and is in general very similar to TCP
    socket programming.  QBluetoothRfcommSockets are thus stream oriented network
    sockets and share many of the interface aspects of the Qt TCP sockets.

    The channel and address of the connected peer are fetched by calling
    remoteChannel() and remoteAddress().  localAddress() return
    address of the local socket.

    \ingroup qtopiabluetooth
    \sa QBluetoothRfcommServer, QBluetoothAbstractSocket
*/

/*!
    Constructs a new QBluetoothRfcommSocket object.  The \a parent parameter
    is passed to the QObject constructor.
*/
QBluetoothRfcommSocket::QBluetoothRfcommSocket(QObject *parent)
    : QBluetoothAbstractSocket(new QBluetoothRfcommSocketPrivate, parent)
{
}

/*!
    Destroys the socket.
*/
QBluetoothRfcommSocket::~QBluetoothRfcommSocket()
{
}

/*!
    Returns the address of the remote device.  If the socket is not currently
    connected, returns QBluetoothAddress::invalid.

    \sa localAddress(), remoteChannel()
 */
QBluetoothAddress QBluetoothRfcommSocket::remoteAddress() const
{
    SOCKET_DATA(QBluetoothRfcommSocket);

    return m_data->m_remote;
}

/*!
    Returns the address of the local device.  If the socket is not currently
    connected, returns QBluetoothAddress::invalid.

    \sa remoteAddress(), remoteChannel()
 */
QBluetoothAddress QBluetoothRfcommSocket::localAddress() const
{
    SOCKET_DATA(QBluetoothRfcommSocket);

    return m_data->m_local;
}

/*!
    Returns the RFCOMM channel of the remote device.  If the socket is not
    currently connected, returns -1.

    \sa remoteAddress(), localAddress()
 */
int QBluetoothRfcommSocket::remoteChannel() const
{
    SOCKET_DATA(QBluetoothRfcommSocket);

    return m_data->m_remoteChannel;
}

/*!
    Returns true if the socket is encrypted.

    \sa securityOptions(), isAuthenticated()
 */
bool QBluetoothRfcommSocket::isEncrypted() const
{
    return securityOptions() & QBluetooth::Encrypted;
}

/*!
    Returns true if the socket is authenticated.

    \sa securityOptions(), isEncrypted()
 */
bool QBluetoothRfcommSocket::isAuthenticated() const
{
    return securityOptions() & QBluetooth::Authenticated;
}

/*!
    Returns the security options currently active for the socket.

    \sa isAuthenticated(), isEncrypted()
 */
QBluetooth::SecurityOptions QBluetoothRfcommSocket::securityOptions() const
{
    if (state() == QBluetoothRfcommSocket::UnconnectedState)
        return 0;

    QBluetooth::SecurityOptions options;

    if (_q_getSecurityOptions(socketDescriptor(), options))
        return options;

    return 0;
}

/*!
    \reimp
*/
bool QBluetoothRfcommSocket::readSocketParameters(int socket)
{
    SOCKET_DATA(QBluetoothRfcommSocket);

    int dummy;
    m_data->m_engine->getsocknameRfcomm(socket, &m_data->m_local, &dummy);
    m_data->m_engine->getpeernameRfcomm(socket, &m_data->m_remote,
                                        &m_data->m_remoteChannel);

    return true;
}

/*!
    Attempts to open a RFCOMM connection between the local device with
    address \a local and the remote device with address \a remote.  The
    RFCOMM channel to use is given by \a channel.  This function should
    generally return immediately, and the socket will enter into the
    \c ConnectingState.

    Optionally the client can request that the connection be secured
    by specifying the \a options parameter.  \bold NOTE: This feature
    might not work under some systems.  The options will be set
    but might be ignored by the implementation.

    The function returns true if the connection process could be started,
    and false otherwise.

    Note that the connection could still fail, the state of the socket
    will be sent in the stateChanged() signal.

    \sa state(), connected(), waitForConnected()
 */
bool QBluetoothRfcommSocket::connect(const QBluetoothAddress &local,
                                       const QBluetoothAddress &remote,
                                       int channel,
                                       QBluetooth::SecurityOptions options)
{
    if (state() != QBluetoothAbstractSocket::UnconnectedState)
        return false;

    resetSocketParameters();

    int sockfd = m_data->m_engine->rfcommSocket();

    if (sockfd < 0) {
        setError(m_data->m_engine->error());
        return false;
    }

    m_data->m_engine->setSocketOption(sockfd, QBluetoothSocketEngine::NonBlockingOption);

    if (!_q_setSecurityOptions(sockfd, options))
        qWarning("Cannot set security options for RFCOMM socket %d", sockfd);

    QBluetoothAbstractSocket::SocketState connectState =
            m_data->m_engine->connectRfcomm(sockfd, local, remote, channel);

    return handleConnect(sockfd, connectState);
}

/*!
    \reimp
*/
void QBluetoothRfcommSocket::resetSocketParameters()
{
    SOCKET_DATA(QBluetoothRfcommSocket);

    m_data->m_local = QBluetoothAddress::invalid;
    m_data->m_remote = QBluetoothAddress::invalid;
    m_data->m_remoteChannel = -1;
}
