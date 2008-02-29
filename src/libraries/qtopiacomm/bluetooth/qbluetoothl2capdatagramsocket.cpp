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

#include <QSocketNotifier>

#include <qtopia/comm/qbluetoothl2capdatagramsocket.h>
#include <qtopiacomm/private/qbluetoothnamespace_p.h>
#include <qtopiacomm/private/qbluetoothabstractsocket_p.h>
#include <bluetooth/l2cap.h>
#include <qbluetoothaddress.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

class QBluetoothL2CapDatagramSocketPrivate : public QBluetoothAbstractSocketPrivate
{
public:
    QBluetoothL2CapDatagramSocketPrivate();

    QBluetoothAddress m_local;
    QBluetoothAddress m_remote;
    int m_remotePsm;
    int m_localPsm;
    int m_imtu;
    int m_omtu;
    QBluetooth::SecurityOptions m_options;
};

QBluetoothL2CapDatagramSocketPrivate::QBluetoothL2CapDatagramSocketPrivate()
    : QBluetoothAbstractSocketPrivate(false)
{
    m_remotePsm = -1;
    m_localPsm = -1;
    m_imtu = L2CAP_DEFAULT_MTU;
    m_omtu = L2CAP_DEFAULT_MTU;
    m_options = 0;
}

/*!
    \class QBluetoothL2CapDatagramSocket
    \mainclass
    \brief The QBluetoothL2CapDatagramSocket class represents an L2CAP Datagram Socket.

    The Bluetooth L2CAP protocol provides reliable connection-oriented
    and unreliable connectionless data services.  It is a lower
    layer protocol than RFCOMM.

    PSM stands for Port and Service Multiplexer.  The PSM value can be
    any \bold{odd} value the range of 1-32765.  For more information
    please see Bluetooth Specification Version 2.0 + EDR [vol 4]
    page 45.

    The implementation of L2CAP Datagram protocol under Linux uses sockets,
    and is in general very similar to UDP socket programming.

    The most common way to use this class is to call connect() and then
    write() and read() to write and read the data.  If you only wish
    to receive data, call bind() and use the readDatagram() function.

    Note that unlike UDP, the L2Cap Datagram socket must be connected
    first by using connect().

    Note that special attention should be paid to the incoming and outgoing
    MTU sizes.  These determine in what size chunks the data should be
    read / written.

    \bold{NOTE:} The socket is not buffered.  All reads and writes happen
    immediately.  The user must be prepared to deal with conditions where
    no more data could be read/written to the socket.

    \ingroup qtopiabluetooth
    \sa QBluetoothAbstractSocket
 */

/*!
    Constructs a new QBluetoothL2CapDatagramSocket object.
    The \a parent parameter is passed to the QObject constructor.
 */
QBluetoothL2CapDatagramSocket::QBluetoothL2CapDatagramSocket(QObject *parent)
    : QBluetoothAbstractSocket(new QBluetoothL2CapDatagramSocketPrivate, parent)
{
}

/*!
    Destroys the socket.
 */
QBluetoothL2CapDatagramSocket::~QBluetoothL2CapDatagramSocket()
{
}

/*!
    Returns the address of the remote device.  If the socket is not currently
    connected, returns QBluetoothAddress::invalid.

    \sa remotePsm()
 */
QBluetoothAddress QBluetoothL2CapDatagramSocket::remoteAddress() const
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);
    return m_data->m_remote;
}

/*!
    Returns the address of the local device.  If the socket is not currently
    connected or bound, returns QBluetoothAddress::invalid.

    \sa localPsm()
 */
QBluetoothAddress QBluetoothL2CapDatagramSocket::localAddress() const
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);
    return m_data->m_local;
}

/*!
    Returns the PSM of the remote device.  If the socket is not
    currently connected, returns -1.

    \sa remoteAddress()
 */
int QBluetoothL2CapDatagramSocket::remotePsm() const
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);
    return m_data->m_remotePsm;
}

/*!
    Returns the PSM that the socket is bound to on the local device.
    If the socket is not bound, returns -1

    \sa localAddress()
*/
int QBluetoothL2CapDatagramSocket::localPsm() const
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);
    return m_data->m_localPsm;
}

/*!
    Returns true if the socket is encrypted.

    \sa securityOptions(), isAuthenticated()
 */
bool QBluetoothL2CapDatagramSocket::isEncrypted() const
{
    return securityOptions() & QBluetooth::Encrypted;
}

/*!
    Returns true if the socket is authenticated.

    \sa securityOptions(), isEncrypted()
 */
bool QBluetoothL2CapDatagramSocket::isAuthenticated() const
{
    return securityOptions() & QBluetooth::Authenticated;
}

/*!
    Returns the security options currently active for the socket.

    \sa isAuthenticated(), isEncrypted()
 */
QBluetooth::SecurityOptions QBluetoothL2CapDatagramSocket::securityOptions() const
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);
    if (state() == QBluetoothL2CapDatagramSocket::UnconnectedState)
        return m_data->m_options;

    if (_q_getL2CapSecurityOptions(socketDescriptor(), m_data->m_options))
        return m_data->m_options;

    return 0;
}

/*!
    Sets the security options on the socket to \a options.  Returns true if the
    options could be set successfully and false otherwise.

    \sa securityOptions(), isAuthenticated(), isEncrypted()
*/
bool QBluetoothL2CapDatagramSocket::setSecurityOptions(QBluetooth::SecurityOptions options)
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);
    m_data->m_options = options;

    if (state() == QBluetoothL2CapDatagramSocket::UnconnectedState)
        return true;

    return _q_setL2CapSecurityOptions(socketDescriptor(), m_data->m_options);
}

/*!
    Returns the MTU for incoming data.  The underlying implementation will accept
    data packets of size no bigger than the incoming MTU.  The MTU information is
    provided so that the read buffer size can be set appropriately.

    \sa outgoingMtu()
 */
int QBluetoothL2CapDatagramSocket::incomingMtu() const
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);
    return m_data->m_imtu;
}

/*!
    Returns the MTU for outgoing data.  The underlying implementation will accept
    outgoing data packets of size no bigger than the outgoing MTU.

    \sa incomingMtu()
 */
int QBluetoothL2CapDatagramSocket::outgoingMtu() const
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);
    return m_data->m_omtu;
}

static void decode_sockaddr_l2(struct sockaddr_l2 *addr, QBluetoothAddress *ret, int *psm)
{
    if (ret) {
        bdaddr_t remoteBdaddr;
        memcpy(&remoteBdaddr, &addr->l2_bdaddr, sizeof(bdaddr_t));
        QString str = bdaddr2str(&remoteBdaddr);
        *ret = QBluetoothAddress(str);
    }

    if (psm) {
        *psm = btohs(addr->l2_psm);
    }
}

static bool readLocalSocketParameters(QBluetoothL2CapDatagramSocketPrivate *m_data, int socket)
{
    struct sockaddr_l2 addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, sizeof(addr));

    if (::getsockname(socket, (struct sockaddr *) &addr, &len) == 0) {
        decode_sockaddr_l2(&addr, &m_data->m_local, &m_data->m_localPsm);
        return true;
    }

    return false;
}

static bool readRemoteSocketParameters(int socket, QBluetoothAddress *remote, int *remotePsm)
{
    struct sockaddr_l2 addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, sizeof(addr));

    if (::getpeername(socket, (struct sockaddr *) &addr, &len) == 0) {
        decode_sockaddr_l2(&addr, remote, remotePsm);
        return true;
    }

    return false;
}

/*!
    \internal

    \reimp
*/
bool QBluetoothL2CapDatagramSocket::readSocketParameters(int socket)
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);

    bool ret = readLocalSocketParameters(m_data, socket);

    if (!ret)
        return ret;

    ret = readRemoteSocketParameters(socket, &m_data->m_remote, &m_data->m_remotePsm);
    if (!ret)
        return ret;

    struct l2cap_options opts;
    memset(&opts, 0, sizeof(opts));
    socklen_t optlen = sizeof(opts);

    if (::getsockopt(socket, SOL_L2CAP, L2CAP_OPTIONS, &opts, &optlen) == 0) {
        m_data->m_imtu = opts.imtu;
        m_data->m_omtu = opts.omtu;
    }

    return true;
}

/*!
    Attempts to open a L2CAP connection between the local device with
    address \a local and the remote device with address \a remote.  The
    L2CAP PSM to use is given by \a psm.  This function should
    generally return immediately, and the socket will enter into the
    \c ConnectingState.

    The \a incomingMtu and \a outgoingMtu represent the MTU sizes to use
    for incoming and outgoing data respectively.

    The function returns true if the connection could be started,
    and false otherwise.

    Note that the connection could still fail, the state of the socket
    will be sent in the stateChanged() signal.

    \sa connected(), stateChanged(), waitForConnected()
 */
bool QBluetoothL2CapDatagramSocket::connect(const QBluetoothAddress &local,
                                            const QBluetoothAddress &remote,
                                            int psm,
                                            int incomingMtu,
                                            int outgoingMtu)
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);

    if (state() != QBluetoothAbstractSocket::UnconnectedState)
        return false;

    resetSocketParameters();

    int sockfd = socket(PF_BLUETOOTH, SOCK_DGRAM, BTPROTO_L2CAP);

    if (sockfd < 0) {
        setError(QBluetoothAbstractSocket::ResourceError);
        return false;
    }

    struct sockaddr_l2 addr;
    bdaddr_t localBdaddr;

    str2bdaddr(local.toString(), &localBdaddr);

    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    memcpy(&addr.l2_bdaddr, &localBdaddr, sizeof(bdaddr_t));

    if (::bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        ::close(sockfd);
        setError(QBluetoothAbstractSocket::BindError);
        return false;
    }

    _q_setL2CapSecurityOptions(sockfd, m_data->m_options);

    struct l2cap_options opts;
    socklen_t optlen;

    memset(&opts, 0, sizeof(opts));
    optlen = sizeof(opts);

    if (getsockopt(sockfd, SOL_L2CAP, L2CAP_OPTIONS, &opts, &optlen) < 0) {
        ::close(sockfd);
        setError(QBluetoothAbstractSocket::UnknownError);
    }

    opts.omtu = outgoingMtu;
    opts.imtu = incomingMtu;

    if (setsockopt(sockfd, SOL_L2CAP, L2CAP_OPTIONS, &opts, sizeof(opts)) < 0) {
        ::close(sockfd);
        setError(QBluetoothAbstractSocket::UnknownError);
    }

    bdaddr_t remoteBdaddr;
    str2bdaddr(remote.toString(), &remoteBdaddr);

    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    memcpy(&addr.l2_bdaddr, &remoteBdaddr, sizeof(bdaddr_t));
    addr.l2_psm = htobs(psm);

    return initiateConnect(sockfd, (struct sockaddr *) &addr, sizeof(addr));
}

/*!
    \internal

    \reimp
*/
void QBluetoothL2CapDatagramSocket::resetSocketParameters()
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);

    m_data->m_local = QBluetoothAddress::invalid;
    m_data->m_remote = QBluetoothAddress::invalid;
    m_data->m_remotePsm = -1;
    m_data->m_localPsm = -1;
    m_data->m_omtu = 0;
    m_data->m_imtu = 0;
    m_data->m_options = 0;
}

/*!
    Binds an L2CAP socket to a specific \a local address and \a psm, returning true if
    successful; otherwise returns false. The \a mtu specifies the MTU to use.

    \sa 
*/
bool QBluetoothL2CapDatagramSocket::bind(const QBluetoothAddress &local, int psm, int mtu)
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);

    if (state() != QBluetoothAbstractSocket::UnconnectedState)
        return false;

    int fd = socket(PF_BLUETOOTH, SOCK_DGRAM, BTPROTO_L2CAP);

    if (fd < 0) {
        setError(QBluetoothAbstractSocket::ResourceError);
        return false;
    }

    struct sockaddr_l2 addr;
    bdaddr_t localBdaddr;

    str2bdaddr(local.toString(), &localBdaddr);

    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    memcpy(&addr.l2_bdaddr, &localBdaddr, sizeof(bdaddr_t));
    addr.l2_psm = htobs(psm);

    struct l2cap_options opts;
    memset(&opts, 0, sizeof(opts));
    socklen_t optlen = sizeof(opts);

    if (::getsockopt(fd, SOL_L2CAP, L2CAP_OPTIONS, &opts, &optlen) == -1) {
        setError(QBluetoothAbstractSocket::BindError);
        ::close(fd);
        return false;
    }

    opts.imtu = mtu;

    if (setsockopt(fd, SOL_L2CAP, L2CAP_OPTIONS, &opts, sizeof(opts)) == -1) {
        setError(QBluetoothAbstractSocket::BindError);
        ::close(fd);
        return false;
    }

    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    ::fcntl(fd, F_SETFD, FD_CLOEXEC);

    if (::bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        qWarning("QBluetoothL2CapDatagramSocket::bind couldn't bind server...");
        ::close(fd);
        setError(QBluetoothAbstractSocket::BindError);
        return false;
    }

    m_data->m_fd = fd;

    readLocalSocketParameters(m_data, fd);
    m_data->setupNotifiers();

    m_data->m_state = QBluetoothAbstractSocket::BoundState;
    emit stateChanged(m_data->m_state);

    return true;
}

/*!
    Reads a datagram from the socket.  The \a data specifies the pointer
    to a buffer of at least \a maxSize.  The \a address and \a psm parameters
    specify where to store the address and psm of the remote device
    that sent the datagram.  The result is discarded if address
    or psm are NULL.

    If the \a data buffer \a maxSize is smaller than the incoming MTU,
    the data that does not fit will be discarded.

    Note: Some Linux implementations do not currently return
    remote address information.

    \sa incomingMtu()
*/
qint64 QBluetoothL2CapDatagramSocket::readDatagram(char * data, qint64 maxSize,
                                                   QBluetoothAddress *address,
                                                   int *psm)
{
    SOCKET_DATA(QBluetoothL2CapDatagramSocket);

    if (m_data->m_state != QBluetoothAbstractSocket::BoundState)
        return -1;

    struct sockaddr_l2 cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    socklen_t len = sizeof(cliaddr);

    ssize_t r = 0;
    do {
        r = ::recvfrom(m_data->m_fd, data, maxSize, 0, (struct sockaddr *)&cliaddr, &len);
    } while (r == -1 && errno == EINTR);

    if (r < 0) {
        setError(QBluetoothAbstractSocket::NetworkError);
        emit error(m_data->m_error);
    } else if (address || psm) {
        decode_sockaddr_l2(&cliaddr, address, psm);
    }

    m_data->m_readNotifier->setEnabled(true);

    return qint64(r);
}
