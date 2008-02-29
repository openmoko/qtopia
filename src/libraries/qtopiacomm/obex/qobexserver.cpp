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

#include <qobexserver.h>
#include <qobexsocket.h>
#include <qtopialog.h>

#include <QSocketNotifier>
#include <QString>
#include <QTimer>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <openobex/obex.h>

/*!
    \class QObexServer
    \brief The QObexServer class is an abstract class that listens for OBEX client connections.

The QObexServer class can be used to listen for new OBEX client connections.  This class
does not implement an actual transport (e.g. Bluetooth, IRDA, HTTP, etc.)  Instead it is used
as an abstract interface class for other classes that will impelement transport specific
functionality. Once a client is connected, server reports a new connection, which should then be handled by the individual service implementation, e.g. OBEX Push Service, OBEX FTP Service, etc.

    \ingroup qtopiaobex
 */

static void qobex_server_callback(obex_t *handle, obex_object_t * object, int mode, int event, int obex_cmd, int obex_rsp);

class QObexServer_Private : public QObject {
    Q_OBJECT

    friend void qobex_server_callback(obex_t *handle, obex_object_t *obj,
                                      int mode, int event, int obex_cmd, int obex_rsp);

public slots:
    void processInput();

signals:
    void newConnection();

public:
    QObexServer_Private(QObexServer *obj, QObject *parent = 0);
    ~QObexServer_Private();

    void close();
    obex_t * spawnReceiver();
    void newConnectionHint();
    bool listen();

    obex_t *m_self;
    int m_pending;
    QObexServer *m_parent;
};

QObexServer_Private::QObexServer_Private(QObexServer *obj, QObject *parent)
    : QObject(parent)
{
    m_parent = obj;
    m_pending = 0;
    m_self = 0;
}

QObexServer_Private::~QObexServer_Private()
{
    close();
}

bool QObexServer_Private::listen()
{
    qLog(Obex) << "Registering server...";
    m_self = static_cast<obex_t *>(m_parent->registerServer());

    if (!m_self)
        return false;

    qLog(Obex) << "Setting user callback and data";
    OBEX_SetUserCallBack(m_self, qobex_server_callback, 0);
    OBEX_SetUserData( m_self, this );

    qLog(Obex) << "QObexServer setting socket options on server socket";
    ::fcntl(OBEX_GetFD(m_self), F_SETFD, FD_CLOEXEC);

    qLog(Obex) << "Setting up SocketNotifier";
    QSocketNotifier *sn = new QSocketNotifier( OBEX_GetFD( m_self ),
            QSocketNotifier::Read,
            this );

    connect( sn, SIGNAL(activated(int)), this, SLOT(processInput()) );

    return true;
}

static void qobex_server_callback(obex_t *handle, obex_object_t * object, int mode, int event, int obex_cmd, int obex_rsp)
{

    qLog(Obex) << "qobex_server_callback called";

    Q_UNUSED(object);
    Q_UNUSED(obex_cmd);
    Q_UNUSED(obex_rsp);
    Q_UNUSED(mode);

    QObexServer_Private* server =
            static_cast<QObexServer_Private *>(OBEX_GetUserData(handle));

    switch (event) {
        case OBEX_EV_ACCEPTHINT:
        {
            server->newConnectionHint();
            break;
        }
        default:
            qWarning( " Unexpected event in server" );
            break;
    }
}

void QObexServer_Private::processInput()
{
    if (!m_self)
        return;

    OBEX_HandleInput( m_self, 0 );
}

void QObexServer_Private::newConnectionHint()
{
    qLog(Obex) << "New Connection Hint called";
    m_pending++;
    emit newConnection();
}

void QObexServer_Private::close()
{
    OBEX_Cleanup(m_self);
    m_self = 0;
}

obex_t *QObexServer_Private::spawnReceiver()
{
    if (m_pending == 0)
        return NULL;

    m_pending--;

    obex_t *handle = OBEX_ServerAccept( m_self, NULL, NULL );

    qLog(Obex) << "QObexServer setting socket options on spawned socket";
    ::fcntl(OBEX_GetFD(handle), F_SETFD, FD_CLOEXEC);
    return handle;
}

/*!
    Constructs a new Bluetooth OBEX Server.  The \a parent parameter specifies
    the \c QObject parent of the server.
 */
QObexServer::QObexServer(QObject *parent)
: QObject(parent)
{
    m_data = new QObexServer_Private(this);
    connect(m_data, SIGNAL(newConnection()), SIGNAL(newConnection()));
}

/*!
    Deconstructs an OBEX Server.
 */
QObexServer::~QObexServer()
{
    if (m_data)
        delete m_data;
}

/*!
    Does the server have pending connections?
 */
bool QObexServer::hasPendingConnections() const
{
    return m_data->m_pending != 0;
}

/*!
    Returns the next pending connection if there are pending connections, otherwise
    returns NULL value.
 */
QObexSocket *QObexServer::nextPendingConnection()
{
    return NULL;
}

/*!
    Returns NULL if there are no pending connections.  Otherwise returns an
    implementation dependent handle.  The current implementation returns an OpenOBEX handle
    of \c obex_t
*/
void *QObexServer::spawnReceiver()
{
    return m_data->spawnReceiver();
}

/*!
    Close the server socket
 */
void QObexServer::close()
{
    m_data->close();
}

/*!
    Returns whether the server is currently listening for connections.
*/
bool QObexServer::isListening() const
{
    return m_data->m_self != 0;
}

/*!
    Attempts to register a local server socket in order to listen for connections.
    Internally this function calls the registerServer() function.  Subclasses must
    provide an implementation of this function for the specific transport protocol.

    \sa registerServer()
*/
bool QObexServer::listen()
{
    return m_data->listen();
}

/*!
    \fn void *QObexServer::registerServer()

    Abstract function that provides an implementation dependent OBEX handle to a server
    socket.  Subclasses of the QObexServer class must implement this function.  If the
    socket could not be bound, the function must return a NULL value.
*/

/*!
    \fn void QObexServer::newConnection()

    This signal is emitted whenever a new client has connected to the server.
 */

#include "qobexserver.moc"
