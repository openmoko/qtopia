/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include <qirobexsocket.h>
#include <qtopialog.h>

#include <openobex/obex.h>
#include <linux/types.h>
#include <linux/irda.h>
#include <sys/socket.h>
#include <fcntl.h>

/*!
    Constructs an new (invalid) Infrared OBEX socket.
 */
QIrObexSocket::QIrObexSocket() : QObexSocket()
{
    m_service = QString();
    m_addr = 0;
}

/*!
    Constructs a new Infrared OBEX socket.  The \a service specifies
    the service to connect to.  The \a addr specifies the address
    of the remote device to connect to. If the address is 0, all
    devices are searched and the first one found will be connected to.
*/
QIrObexSocket::QIrObexSocket(const QString &service, uint addr) : QObexSocket()
{
    m_service = service;
    m_addr = addr;
}

/*!
    Destructor.
*/
QIrObexSocket::~QIrObexSocket()
{

}

/*!
    Returns the service name on the remote device to connect
    to.
*/
const QString &QIrObexSocket::service() const
{
    return m_service;
}

/*!
    Sets the service name to connect to on the remote device.
*/
void QIrObexSocket::setService(const QString &service)
{
    m_service = service;
}

/*!
    Returns the address of the remote device to connect to.
*/
uint QIrObexSocket::address() const
{
    return m_addr;
}

/*!
    Sets the \a addr address of the remote device to connect to, if the address
    is 0, all devices are searched and the first one found will be connected to.
*/
void QIrObexSocket::setAddress(uint addr)
{
    m_addr = addr;
}

// Need this to keep OpenOBEX happy, it will be reset as soon as the handle is passed
// to an actual service
static void qobex_dummy_callback(obex_t *, obex_object_t *, int, int, int, int)
{
}

/*!
    Tries to establish a connection to the remote device.  Returns true
    if the connection was established successfully, and false otherwise
*/
bool QIrObexSocket::connect()
{
    if (m_service.isNull())
        return false;

    obex_t *self;
    self = OBEX_Init( OBEX_TRANS_IRDA, qobex_dummy_callback, OBEX_FL_KEEPSERVER );

    int retc = -1;
    if (m_addr == 0) {
        // The obex implementation will pick a device for us.
        retc = IrOBEX_TransportConnect(self, m_service.toAscii().constData());
    }
    else {
        struct sockaddr_irda sock;
        sock.sir_family = AF_IRDA;
        sock.sir_addr = m_addr;
        strncpy(sock.sir_name, m_service.toAscii().constData(), 25);
        retc = OBEX_TransportConnect(self,
                                     reinterpret_cast<sockaddr *>(&sock),
                                     sizeof(sock));
    }

    if ( retc < 0 ) {
        qLog(Infrared) << "Connection failed to service " << m_service;
        return false;
    }

    ::fcntl(OBEX_GetFD(self), F_SETFD, FD_CLOEXEC);

    m_handle = self;
    return true;
}
