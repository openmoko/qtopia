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
#include <QTimer>
#include <QTime>
#include <QtAlgorithms>
#include <QDebug>

#include <qbluetoothl2capserver.h>
#include <qbluetoothl2capsocket.h>
#include <qtopiacomm/private/qbluetoothnamespace_p.h>
#include <qbluetoothaddress.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <bluetooth/l2cap.h>
#include <fcntl.h>

class QBluetoothL2CapServerPrivate
{
public:
    int m_psm;
    QBluetoothAddress m_address;
    QBluetooth::SecurityOptions m_options;
};

/*!
    \class QBluetoothL2CapServer
    \brief The QBluetoothL2CapServer class represents an L2CAP server socket.

    This class makes it possible to accept incoming L2CAP connections.
    You can specify the address and the L2CAP PSM to listen on.
 */

/*!
    Constructs a new QBluetoothL2CapServer with parent \a parent.
    The server is in the UnconnectedState.
 */
QBluetoothL2CapServer::QBluetoothL2CapServer(QObject *parent)
    : QBluetoothAbstractServer(parent)
{
    m_data = new QBluetoothL2CapServerPrivate();
    m_data->m_psm = -1;
}

/*!
    Destructor.
 */
QBluetoothL2CapServer::~QBluetoothL2CapServer()
{
    if (m_data)
        delete m_data;
}

/*!
    Tells the server to listen for incoming connections on address \a local
    and PSM \a psm.  The \a mtu parameter holds the MTU to use for the server.

    Returns true if the server successfully started listening; otherwise returns false.
    If the server was already listening, returns false.

    \sa isListening()
 */
bool QBluetoothL2CapServer::listen(const QBluetoothAddress &local, int psm, int mtu)
{
    if (isListening()) {
        return false;
    }

    int fd = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

    if (fd < 0) {
        setError(QBluetoothAbstractServer::ResourceError);
        qDebug() << "Couldn't create socket";
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
        qDebug() << "getsockopt error";
        perror("getsockopt failed");
        setError(QBluetoothAbstractServer::ListenError);
        ::close(fd);
        return false;
    }

    opts.imtu = mtu;

    if (setsockopt(fd, SOL_L2CAP, L2CAP_OPTIONS, &opts, sizeof(opts)) == -1) {
        qDebug() << "setsockopt error";
        perror("setsockopt failed");
        setError(QBluetoothAbstractServer::ListenError);
        ::close(fd);
        return false;
    }

    bool ret = initiateListen(fd, (struct sockaddr *) &addr, sizeof(addr));

    if (ret) {
        m_data->m_psm = psm;
        m_data->m_address = local;

        struct sockaddr_l2 addr;
        socklen_t len = sizeof(addr);
        memset(&addr, 0, sizeof(addr));

        if (::getsockname(fd, (struct sockaddr *) &addr, &len) == 0) {
            bdaddr_t localBdaddr;
            memcpy(&localBdaddr, &addr.l2_bdaddr, sizeof(bdaddr_t));
            QString str = bdaddr2str(&localBdaddr);
            m_data->m_address = QBluetoothAddress(str);
            m_data->m_psm = btohs(addr.l2_psm);
        }
    }

    return ret;
}

/*!
    \reimp
 */
void QBluetoothL2CapServer::close()
{
    QBluetoothAbstractServer::close();

    m_data->m_address = QBluetoothAddress::invalid;
    m_data->m_psm = -1;
}

/*!
    Returns the L2CAP PSM the server is currently listening on.  If the server
    is not listening, returns -1.
 */
int QBluetoothL2CapServer::serverPsm() const
{
    return m_data->m_psm;
}

/*!
    Returns the address the server is currently listening on.  If the server
    is not listening, returns QBluetoothAddress::invalid.
 */
QBluetoothAddress QBluetoothL2CapServer::serverAddress() const
{
    return m_data->m_address;
}

/*!
    Returns true if the socket is encrypted.

    \sa securityOptions(), isAuthenticated()
 */
bool QBluetoothL2CapServer::isEncrypted() const
{
    return securityOptions() & QBluetooth::Encrypted;
}

/*!
    Returns true if the socket is authenticated.

    \sa securityOptions(), isEncrypted()
 */
bool QBluetoothL2CapServer::isAuthenticated() const
{
    return securityOptions() & QBluetooth::Authenticated;
}

/*!
    Returns the security options currently active for the socket.

    \sa setSecurityOptions()
 */
QBluetooth::SecurityOptions QBluetoothL2CapServer::securityOptions() const
{
    if (!isListening())
        return m_data->m_options;

    if (_q_getL2CapSecurityOptions(socketDescriptor(), m_data->m_options))
        return m_data->m_options;

    return 0;
}

/*!
    Sets the security options of the socket to be \a options.  Note that under the current
    Linux implementation only new connections will be affected by the change in security
    options.  Existing and pending connections will not be affected.

    \sa securityOptions()
 */
bool QBluetoothL2CapServer::setSecurityOptions(QBluetooth::SecurityOptions options)
{
    m_data->m_options = options;

    if (!isListening())
        return true;

    return _q_setL2CapSecurityOptions(socketDescriptor(), m_data->m_options);
}

/*!
    \reimp
 */
QBluetoothAbstractSocket * QBluetoothL2CapServer::createSocket()
{
    return new QBluetoothL2CapSocket(this);
}

/*!
    Returns the MTU that should be used for this server.  By default the MTU
    is set to be 672 bytes.  If the socket is not listening, or an error occurred,
    returns -1.
*/
int QBluetoothL2CapServer::mtu() const
{
    if (!isListening()) {
        return -1;
    }

    struct l2cap_options opts;
    memset(&opts, 0, sizeof(opts));
    socklen_t optlen = sizeof(opts);

    if (::getsockopt(socketDescriptor(), SOL_L2CAP, L2CAP_OPTIONS, &opts, &optlen) == 0) {
        return opts.imtu;
    }

    return -1;
}
