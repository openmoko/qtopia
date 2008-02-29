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
    \brief The QObexSocket class is an abstract base class for OBEX sockets.

    This class provides a common base class for OBEX sockets. Subclasses
    such as QBluetoothObexSocket and QIrObexSocket provide implementations
    specific to their own types of transport.

    \sa QObexPushClient, QObexPushService

    \ingroup qtopiaobex
 */

/*!
    Constructs a QObexSocket with parent object \a parent.
 */
QObexSocket::QObexSocket(QObject *parent)
    : QObject(parent)
{
    m_handle = 0;
}

/*!
    Destroys a QObexSocket.
 */
QObexSocket::~QObexSocket()
{
    OBEX_Cleanup(static_cast<obex_t *>(m_handle));
    m_handle = NULL;
}

/*!
    Connects the underlying transport connection. Returns true if the socket
    was connected successfully.
 */
bool QObexSocket::connect()
{
    return false;
}

/*!
    Closes the socket and disconnects the transport connection. Returns true
    if the socket was able to be closed.
 */
void QObexSocket::close()
{
    OBEX_TransportDisconnect(static_cast<obex_t *>(m_handle));
    m_handle = NULL;
}

/*!
    Returns whether the transport for this socket is connected.
 */
bool QObexSocket::isConnected() const
{
    return m_handle != NULL;
}

/*!
    \fn void QObexSocket::setHandle(void *handle)

    Sets the underlying implementation handle to \a handle.
 */

/*!
    \fn void *QObexSocket::handle()

    Returns the underlying implementation handle.
 */
