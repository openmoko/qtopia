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

#include <qbluetoothscosocket.h>
#include <qbluetoothaddress.h>
#include <qbluetoothnamespace.h>
#include "qbluetoothnamespace_p.h"
#include "qbluetoothabstractsocket_p.h"
#include "qbluetoothsocketengine_p.h"

class QBluetoothScoSocketPrivate : public QBluetoothAbstractSocketPrivate
{
public:
    QBluetoothScoSocketPrivate();

    QBluetoothAddress m_local;
    QBluetoothAddress m_remote;
    int m_mtu;
};

QBluetoothScoSocketPrivate::QBluetoothScoSocketPrivate()
    : QBluetoothAbstractSocketPrivate(true)
{
    m_mtu = 0;
}

/*!
    \class QBluetoothScoSocket
    \mainclass
    \brief The QBluetoothScoSocket class represents an SCO client socket.

    The Bluetooth SCO protocol provides Synchronous Connection Oriented sockets.
    These sockets are mainly used for audio data transmission, and should not be used
    for reliable data transmission, as the underlying transport mechanisms might
    be using lossy compression techniques.

    The address of the connected peer is fetched by calling remoteAddress().
    The localAddress() returns the address of the local socket.

    \ingroup qtopiabluetooth
    \sa QBluetoothScoServer, QBluetoothAbstractSocket
 */

/*!
    Constructs the QBluetoothScoSocket object.  The QObject parent is given by
    \a parent.
*/
QBluetoothScoSocket::QBluetoothScoSocket(QObject *parent)
    : QBluetoothAbstractSocket(new QBluetoothScoSocketPrivate, parent)
{
}

/*!
    Destroys the socket. 
*/
QBluetoothScoSocket::~QBluetoothScoSocket()
{
}

/*!
    Attempts to open a SCO connection between the local device with
    address \a local and the remote device with address \a remote.
    This function should generally return immediately, and the socket
    will enter into the \c ConnectingState.

    The function returns true if the connection could be started,
    and false otherwise.

    Note that the connection could still fail, the state of the socket
    will be sent in the stateChanged() signal.

    \sa state(), connected(), waitForConnected()
 */
bool QBluetoothScoSocket::connect(const QBluetoothAddress &local,
                                  const QBluetoothAddress &remote)
{
    if (state() != QBluetoothAbstractSocket::UnconnectedState)
        return false;

    resetSocketParameters();

    int sockfd = m_data->m_engine->scoSocket();

    if (sockfd < 0) {
        setError(m_data->m_engine->error());
        return false;
    }

    m_data->m_engine->setSocketOption(sockfd, QBluetoothSocketEngine::NonBlockingOption);

    QBluetoothAbstractSocket::SocketState connectState =
            m_data->m_engine->connectSco(sockfd, local, remote);

    return handleConnect(sockfd, connectState);
}

/*!
    Returns the remote address of the opened SCO socket.  If the socket
    is currently not connected, or the information could not be obtained,
    an invalid QBluetoothAddress is returned.

    \sa localAddress()
*/
QBluetoothAddress QBluetoothScoSocket::remoteAddress() const
{
    SOCKET_DATA(QBluetoothScoSocket);
    return m_data->m_remote;
}

/*!
    Returns the local address of the opened SCO socket.  If the socket
    is currently not connected, or the information could not be obtained,
    an invalid QBluetoothAddress is returned.

    \sa remoteAddress()
*/
QBluetoothAddress QBluetoothScoSocket::localAddress() const
{
    SOCKET_DATA(QBluetoothScoSocket);
    return m_data->m_local;
}

/*!
    This method returns the MTU of the opened SCO socket.  If the socket
    is currently not connected, a 0 is returned.
*/
int QBluetoothScoSocket::mtu() const
{
    SOCKET_DATA(QBluetoothScoSocket);
    return m_data->m_mtu;
}

/*!
    \reimp
*/
bool QBluetoothScoSocket::readSocketParameters(int sockfd)
{
    SOCKET_DATA(QBluetoothScoSocket);

    m_data->m_engine->getsocknameSco(sockfd, &m_data->m_local);
    m_data->m_engine->getpeernameSco(sockfd, &m_data->m_remote);

    if (!m_data->m_engine->getScoMtu(sockfd, &m_data->m_mtu))
        return false;

    setReadMtu(m_data->m_mtu);
    setWriteMtu(m_data->m_mtu);

    return true;
}

/*!
    \reimp
*/
void QBluetoothScoSocket::resetSocketParameters()
{
    SOCKET_DATA(QBluetoothScoSocket);

    m_data->m_local = QBluetoothAddress::invalid;
    m_data->m_remote = QBluetoothAddress::invalid;
    m_data->m_mtu = 0;
}
