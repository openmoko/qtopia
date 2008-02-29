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

#include <qtopia/comm/qbluetoothscosocket.h>
#include <qtopia/comm/qbluetoothaddress.h>
#include <qtopia/comm/qbluetoothnamespace.h>
#include <qtopiacomm/private/qbluetoothnamespace_p.h>
#include <qtopiacomm/private/qbluetoothabstractsocket_p.h>

#include <bluetooth/bluetooth.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <bluetooth/sco.h>
#include <unistd.h>

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

    int sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);
    if (sk < 0) {
        setError(QBluetoothAbstractSocket::ResourceError);
        return false;
    }

    struct sockaddr_sco addr;

    bdaddr_t remoteBdaddr;
    str2bdaddr(remote.toString(), &remoteBdaddr);
    bdaddr_t localBdaddr;
    str2bdaddr(local.toString(), &localBdaddr);

    memset(&addr, 0, sizeof(addr));
    addr.sco_family = AF_BLUETOOTH;
    memcpy(&addr.sco_bdaddr, &localBdaddr, sizeof(bdaddr_t));

    if (::bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        ::close(sk);
        setError(QBluetoothAbstractSocket::BindError);
        return false;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sco_family = AF_BLUETOOTH;
    memcpy(&addr.sco_bdaddr, &remoteBdaddr, sizeof(bdaddr_t));

    return initiateConnect(sk, (struct sockaddr *) &addr, sizeof(addr));
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

    struct sockaddr_sco addr;
    socklen_t len = sizeof(addr);

    memset(&addr, 0, sizeof(addr));
    if (::getsockname(sockfd, (struct sockaddr *) &addr, &len) == 0) {

        if (addr.sco_family != AF_BLUETOOTH) {
            qWarning("rc_family doesn't match AF_BLUETOOTH!!");
            return false;
        }

        bdaddr_t localBdaddr;
        memcpy(&localBdaddr, &addr.sco_bdaddr, sizeof(bdaddr_t));
        QString str = bdaddr2str(&localBdaddr);
        m_data->m_local = QBluetoothAddress(str);
    }

    memset(&addr, 0, sizeof(addr));
    if (::getpeername(sockfd, (struct sockaddr *) &addr, &len) == 0) {
        bdaddr_t remoteBdaddr;
        memcpy(&remoteBdaddr, &addr.sco_bdaddr, sizeof(bdaddr_t));
        QString str = bdaddr2str(&remoteBdaddr);
        m_data->m_remote = QBluetoothAddress(str);
    }

    struct sco_options opts;
    len = sizeof(opts);
    if (::getsockopt(sockfd, SOL_SCO, SCO_OPTIONS, &opts, &len) == 0) {
        m_data->m_mtu = opts.mtu;
    }

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
