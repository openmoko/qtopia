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

#include <qbluetoothrfcommserver.h>
#include <qbluetoothrfcommsocket.h>
#include <qtopiacomm/private/qbluetoothnamespace_p.h>
#include <qbluetoothaddress.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <bluetooth/rfcomm.h>
#include <fcntl.h>

class QBluetoothRfcommServerPrivate
{
public:
    int m_channel;
    QBluetoothAddress m_address;
    QBluetooth::SecurityOptions m_options;
};

/*!
    \class QBluetoothRfcommServer
    \brief The QBluetoothRfcommServer class represents an RFCOMM server socket.

    This class makes it possible to accept incoming RFCOMM connections.
    You can specify the address and the RFCOMM channel to listen on.
*/

/*!
    Constructs a new QBluetoothRfcommServer with parent \a parent.
    The server is in the UnconnectedState.
*/
QBluetoothRfcommServer::QBluetoothRfcommServer(QObject *parent)
    : QBluetoothAbstractServer(parent)
{
    m_data = new QBluetoothRfcommServerPrivate();
    m_data->m_channel = -1;
    m_data->m_options = 0;
}

/*!
    Destructor.
*/
QBluetoothRfcommServer::~QBluetoothRfcommServer()
{
    if (m_data)
        delete m_data;
}

/*!
    Tells the server to listen for incoming connections on address \a local
    and channel \a channel.

    Returns true on success; otherwise returns false.

    \sa isListening()
*/
bool QBluetoothRfcommServer::listen(const QBluetoothAddress &local, int channel)
{
    if (isListening()) {
        return false;
    }

    int fd = socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    if (fd < 0) {
        setError(QBluetoothRfcommServer::ResourceError);
        return false;
    }

    struct sockaddr_rc addr;
    bdaddr_t localBdaddr;

    str2bdaddr(local.toString(), &localBdaddr);

    memset(&addr, 0, sizeof(addr));
    addr.rc_family = AF_BLUETOOTH;
    memcpy(&addr.rc_bdaddr, &localBdaddr, sizeof(bdaddr_t));
    addr.rc_channel = channel;

    _q_setSecurityOptions(fd, m_data->m_options);

    bool ret = initiateListen(fd, (struct sockaddr *) &addr, sizeof(addr));

    if (ret) {
        m_data->m_channel = channel;
        m_data->m_address = local;

        struct sockaddr_rc addr;
        socklen_t len = sizeof(addr);
        memset(&addr, 0, sizeof(addr));

        if (::getsockname(fd, (struct sockaddr *) &addr, &len) == 0) {
            bdaddr_t localBdaddr;
            memcpy(&localBdaddr, &addr.rc_bdaddr, sizeof(bdaddr_t));
            QString str = bdaddr2str(&localBdaddr);
            m_data->m_address = QBluetoothAddress(str);
            m_data->m_channel = channel;
        }

    }

    return ret;
}

/*!
    \reimp
*/
void QBluetoothRfcommServer::close()
{
    QBluetoothAbstractServer::close();

    m_data->m_address = QBluetoothAddress::invalid;
    m_data->m_channel = -1;
}

/*!
    Returns the RFCOMM channel the server is currently listening on.  If the server
    is not listening, returns -1.
*/
int QBluetoothRfcommServer::serverChannel() const
{
    return m_data->m_channel;
}

/*!
    Returns the address the server is currently listening on.  If the server
    is not listening, returns QBluetoothAddress::invalid.
 */
QBluetoothAddress QBluetoothRfcommServer::serverAddress() const
{
    return m_data->m_address;
}

/*!
    Returns true if the socket is encrypted.

    \sa securityOptions(), isAuthenticated()
 */
bool QBluetoothRfcommServer::isEncrypted() const
{
    return securityOptions() & QBluetooth::Encrypted;
}

/*!
    Returns true if the socket is authenticated.

    \sa securityOptions(), isEncrypted()
 */
bool QBluetoothRfcommServer::isAuthenticated() const
{
    return securityOptions() & QBluetooth::Authenticated;
}

/*!
    Returns the security options currently active for the socket.

    \sa setSecurityOptions()
 */
QBluetooth::SecurityOptions QBluetoothRfcommServer::securityOptions() const
{
    if (!isListening()) {
        return m_data->m_options;
    }

    if (_q_getSecurityOptions(socketDescriptor(), m_data->m_options))
        return m_data->m_options;

    return 0;
}

/*!
    Sets the security options of the socket to be \a options.

    \sa securityOptions()
 */
bool QBluetoothRfcommServer::setSecurityOptions(QBluetooth::SecurityOptions options)
{
    m_data->m_options = options;

    if (!isListening()) {
        return true;
    }

    return _q_setSecurityOptions(socketDescriptor(), m_data->m_options);
}

/*!
    \reimp
 */
QBluetoothAbstractSocket * QBluetoothRfcommServer::createSocket()
{
    return new QBluetoothRfcommSocket(this);
}
