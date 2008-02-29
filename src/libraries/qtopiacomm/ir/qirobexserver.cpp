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

#include <qirobexserver.h>
#include <qirobexsocket.h>
#include <qtopialog.h>
#include <qtopiacomm/private/qirnamespace_p.h>

#include <QSocketNotifier>
#include <QString>
#include <QTimer>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/irda.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>

#include <openobex/obex.h>

/*!
    \class QIrObexServer
    \brief The QIrObexServer class listens for OBEX client connections.

The QIrObexServer class can be used to listen for new OBEX client connections.
The object binds to an Infrared OBEX service and listens for any client connections.  Once a
client is connected, server reports a new connection, which should then be handled
by the individual service implementation, e.g. IR Xfer service, IR OBEX service, etc.

    \ingroup qtopiair
 */

class QIrObexServer_Private : public QObject {
public:
    QIrObexServer_Private(QObject *parent = 0);
    bool listen(const QString &service);

    QIr::DeviceClasses m_classes;
    QString m_service;
    obex_t *m_self;
};

// Need this to keep OpenOBEX happy, it will be reset as soon as the handle is passed
// to an actual service
static void qobex_dummy_callback(obex_t *, obex_object_t *, int, int, int, int)
{
}

QIrObexServer_Private::QIrObexServer_Private(QObject *parent)
    : QObject(parent)
{
    m_self = OBEX_Init( OBEX_TRANS_IRDA, qobex_dummy_callback,
                      OBEX_FL_KEEPSERVER );
}

bool QIrObexServer_Private::listen(const QString &service)
{
    qLog(Infrared) << "QIrObexServer binding to: " << service;

    if (IrOBEX_ServerRegister(m_self, service.toAscii().constData()) < 0) {
        perror("Error reported is: ");
        qLog(Infrared) << "ServerRegister for service: " << service << " failed";
        return false;
    }

    // Try to set the hint bits, this should almost always work.
    unsigned char hints[4];

    hints[0] = 0;
    hints[1] = 0;
    hints[2] = 0;
    hints[3] = 0;

    convert_to_hints(m_classes, hints);

    qLog(Infrared) << "Setting the hint bits to: " << m_classes;
    int status = setsockopt(OBEX_GetFD(m_self),
                            SOL_IRLMP, IRLMP_HINTS_SET, hints, sizeof(hints));
    qLog(Infrared) << "Success: " << (status == 0);

    qLog(Infrared) << "QIrObexServer for service " << service << " configured";

    return true;
}

/*!
    Constructs a new Infrared OBEX Server. The \a service parameter specifies
    the Information Access Service (IAS) entry to use.  The IAS entry will be
    created automatically.  The \a classes parameter specifies what major
    device classes this service should advertise.  The remote devices
    will see a combination of all device classes advertised by all running
    services on the local device.  The \a parent parameter
    specifies the \c QObject parent of the server.
*/
QIrObexServer::QIrObexServer(const QString &service,
                             QIr::DeviceClasses classes,
                             QObject *parent)
    : QObexServer(parent)
{
    m_data = new QIrObexServer_Private(this);
    m_data->m_service = service;
    m_data->m_classes = classes;
}

/*!
    Deconstructs an Infrared OBEX Server.
*/
QIrObexServer::~QIrObexServer()
{
    if (m_data)
        delete m_data;
}

/*!
    This function starts attempts to open a server socket and register itself with
    the OBEX protocol handler.

    Returns a valid QObexHandle if the registration was completed successfully,
    and an invalid one otherwise.
*/
void *QIrObexServer::registerServer()
{
    bool ret = m_data->listen(m_data->m_service);

    if (!ret)
        return NULL;

    return m_data->m_self;
}

/*!
    Returns the IAS service name the server is listening on.  If the server is not listening,
    a null string will be returned.

    \sa listen()
*/
const QString &QIrObexServer::service() const
{
    return m_data->m_service;
}

/*!
    Returns the DeviceClasses that this service will advertise.  The union of
    all actively listening services' device classes will be advertised to remote
    devices.  E.g. if there are two running services, one with OBEX and Computer
    hints set, and another with Telephony hint set, then the remote devices will
    discover the local device with OBEX, Computer and Telephony bits set.
*/
QIr::DeviceClasses QIrObexServer::deviceClasses() const
{
    return m_data->m_classes;
}

/*!
    Returns the next pending connection if there are pending connections, otherwise returns
    NULL value.  It is responsibility of the caller to make sure the socket is deleted.
*/
QObexSocket *QIrObexServer::nextPendingConnection()
{
    qLog(Infrared) << "nextPendingConnection for service " << m_data->m_service << " called.";
    void *handle = spawnReceiver();

    if (!handle)
        return NULL;

    qLog(Infrared) << "Creating new QIrObexSocket";
    QIrObexSocket *socket = new QIrObexSocket();
    socket->setHandle(handle);

    return socket;
}
