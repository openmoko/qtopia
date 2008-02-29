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

#include <qobexsocket.h>

#include <openobex/obex.h>

/*!
    \class QObexSocket
    \mainclass
    \brief The QObexSocket class is an abstract base class for OBEX sockets.

    This class provides a common base class for OBEX sockets. It is used to
    implement different types of transport connections for the OBEX protocol.
    For example, QBluetoothObexSocket and QIrObexSocket subclass QObexSocket
    to provide OBEX connections over Bluetooth and IrDA, respectively.

    QObexSocket is abstract and cannot be instantiated, but it is common to
    use the interface it defines to provide transport-independent OBEX
    features. For example, QObexPushClient and QObexPushService are
    constructed using QObexSocket pointers, allowing them to be used with
    various types of transport connections (such as Bluetooth or IrDA).

    Call connect() to connect the transport connection. You can use
    isConnected() to check whether the socket is connected, and
    socketDescriptor() to get the socket's file descriptor.

    \sa QBluetoothObexSocket, QIrObexSocket

    \ingroup qtopiaobex
 */

/*!
    Constructs an OBEX socket with the given \a parent.
 */
QObexSocket::QObexSocket(QObject *parent)
    : QObject(parent)
{
    m_handle = 0;
}

/*!
    Destroys the socket.
 */
QObexSocket::~QObexSocket()
{
    if (m_handle) {
        OBEX_Cleanup(static_cast<obex_t *>(m_handle));
        m_handle = NULL;
    }
}

/*!
    Connects the underlying transport connection. Returns true if the socket
    was connected successfully.

    \sa isConnected()
 */
bool QObexSocket::connect()
{
    return false;
}

/*!
    Closes the socket and disconnects the transport connection.
 */
void QObexSocket::close()
{
    OBEX_TransportDisconnect(static_cast<obex_t *>(m_handle));
    m_handle = NULL;
}

/*!
    Returns whether the socket is connected.

    \sa connect()
 */
bool QObexSocket::isConnected() const
{
    return m_handle != NULL;
}

/*!
    Returns the socket descriptor associated with this OBEX Socket.

    \bold {Note:} This implies that transports which are not socket based cannot
    be used with this class.
*/
int QObexSocket::socketDescriptor() const
{
    return OBEX_GetFD(static_cast<obex_t *>(const_cast<void *>(m_handle)));
}

/*!
    \internal
    \fn void QObexSocket::setHandle(void *handle)

    Sets the implementation-dependent handle to \a handle.
 */

/*!
    \fn void *QObexSocket::handle()

    Returns the implementation-dependent handle.
 */
