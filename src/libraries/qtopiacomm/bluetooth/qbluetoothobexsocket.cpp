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

#include <qbluetoothobexsocket.h>
#include <qtopiacomm/private/qbluetoothnamespace_p.h>
#include <qtopialog.h>

#include <openobex/obex.h>
#include <bluetooth/rfcomm.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

class QBluetoothObexSocketPrivate {
public:
    qint16 m_channel;
    QBluetoothAddress m_remote;
    QBluetoothAddress m_local;
};

/*!
    Constructs a new invalid QBluetoothObexSocket.
*/
QBluetoothObexSocket::QBluetoothObexSocket() : QObexSocket()
{
    m_data = new QBluetoothObexSocketPrivate();
    m_data->m_channel = -1;
    m_data->m_remote = QBluetoothAddress::invalid;
    m_data->m_local = QBluetoothAddress::any;
}

/*!
    Constructs a new QBluetoothObexSocket.  The \a rfcomm_channel
    parameter specifies the RFCOMM channel to connect to.  The
    \a remote parameter specifies the remote address of the
    bluetooth device, and \a local parameter specifies what
    local bluetooth adapter to use.
*/
QBluetoothObexSocket::QBluetoothObexSocket(const QBluetoothAddress &remote,
                                           quint8 rfcomm_channel,
                                           const QBluetoothAddress &local ) : QObexSocket()
{
    m_data = new QBluetoothObexSocketPrivate();
    m_data->m_channel = rfcomm_channel;
    m_data->m_remote = remote;
    m_data->m_local = local;
}

/*!
    Destructor.
*/
QBluetoothObexSocket::~QBluetoothObexSocket()
{

}

/*!
    Returns the RFCOMM channel of the socket.
*/
qint16 QBluetoothObexSocket::channel() const
{
    return m_data->m_channel;
}

/*!
    Sets the RFCOMM channel of the socket.
*/
void QBluetoothObexSocket::setChannel(quint8 channel)
{
    m_data->m_channel = channel;
}

/*!
    Returns the address of the remote bluetooth device to connect
    to.
*/
const QBluetoothAddress &QBluetoothObexSocket::remoteAddress() const
{
    return m_data->m_remote;
}

/*!
    Sets the address of the remote device to connect to.
*/
void QBluetoothObexSocket::setRemoteAddress(const QBluetoothAddress &addr)
{
    m_data->m_remote = addr;
}

/*!
    Returns the address of the local adaptor to use.
*/
const QBluetoothAddress &QBluetoothObexSocket::localAddress() const
{
    return m_data->m_local;
}

/*!
    Sets the address of the local adaptor to use.
*/
void QBluetoothObexSocket::setLocalAddress(const QBluetoothAddress &address)
{
    m_data->m_local = address;
}

// Need this to keep OpenOBEX happy, it will be reset as soon as the handle is passed
// to an actual service
static void qobex_dummy_callback(obex_t *, obex_object_t *, int, int, int, int)
{
}

/*!
    \reimp

    Initiates a bluetooth connection.
*/
bool QBluetoothObexSocket::connect()
{
    if ((m_data->m_channel == -1) ||
        !m_data->m_remote.valid() || m_handle)
        return false;

    obex_t *self;
    self = OBEX_Init( OBEX_TRANS_BLUETOOTH, qobex_dummy_callback, OBEX_FL_KEEPSERVER );

    bdaddr_t local;
    bdaddr_t remote;

    str2bdaddr(m_data->m_local.toString(), &local);
    str2bdaddr(m_data->m_remote.toString(), &remote);

    int retc = BtOBEX_TransportConnect(self, &local, &remote,
                                       m_data->m_channel);

    if ( retc < 0 ) {
        qLog(Bluetooth) <<
                "Connection failed on channel " << m_data->m_channel;
        return false;
    }

    ::fcntl(OBEX_GetFD(self), F_SETFD, FD_CLOEXEC);

    m_handle = self;
    return true;
}

/*!
    Returns true if the socket is authenticated.  This method will always
    return false if the socket is not connected.
*/
bool QBluetoothObexSocket::authenticated() const
{
    if (!isConnected())
        return false;

    return securityOptions() & QBluetooth::Authenticated;
}

/*!
    Returns true if the socket is encrypted.  This method will always
    return false if the socket is not connected.
 */
bool QBluetoothObexSocket::encrypted() const
{
    if (!isConnected())
        return false;

    return securityOptions() & QBluetooth::Encrypted;
}

/*!
    Returns the security options of the socket.  If the socket
    is not connected, 0 is returned.
*/
QBluetooth::SecurityOptions QBluetoothObexSocket::securityOptions() const
{
    if (!isConnected())
        return 0;

    int fd = OBEX_GetFD(static_cast<obex_t *>(m_handle));
    QBluetooth::SecurityOptions options;

    if (_q_getSecurityOptions(fd, options))
        return options;

    return 0;
}
