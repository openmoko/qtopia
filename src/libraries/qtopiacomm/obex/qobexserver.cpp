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
#include <QList>

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
    \mainclass
    \brief The QObexServer class provides an abstract class for OBEX servers.

    This class makes it possible to accept incoming OBEX client connections.
    The class does not implement an OBEX server for any particular transport; 
    it provides an abstract interface for OBEX servers, and subclasses must 
    provide the implementation for their own specific OBEX transports (such
    as Bluetooth or IrDA).

    When a client connects, the newConnection() signal is emitted. The 
    nextPendingConnection() method can then be called to retrieve the new
    connection as a QObexSocket object. This socket object can then be 
    used to run an OBEX client or service, such as QObexPushClient and
    QObexPushService.

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
    QList<obex_t *> m_pending;
    QObexServer *m_parent;
};

QObexServer_Private::QObexServer_Private(QObexServer *obj, QObject *parent)
    : QObject(parent)
{
    m_parent = obj;
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

    obex_t *handle = OBEX_ServerAccept(m_self, NULL, NULL);
    if (!handle) {
        qLog(Obex) << "QObexServer error: accept() returned null obex handle";
        return;
    }

    ::fcntl(OBEX_GetFD(handle), F_SETFD, FD_CLOEXEC);
    m_pending.append(handle);

    emit newConnection();
}

void QObexServer_Private::close()
{
    while (m_pending.size() > 0) {
        obex_t *handle = m_pending.takeLast();
        if (handle)
            OBEX_Cleanup(handle);
    }

    if (m_self) {
        OBEX_Cleanup(m_self);
        m_self = 0;
    }
}

obex_t *QObexServer_Private::spawnReceiver()
{
    if (m_pending.isEmpty())
        return 0;

    return m_pending.takeLast();
}

/*!
    Constructs an OBEX Server with the given \a parent.

    \sa listen()
 */
QObexServer::QObexServer(QObject *parent)
: QObject(parent)
{
    m_data = new QObexServer_Private(this);
    connect(m_data, SIGNAL(newConnection()), SIGNAL(newConnection()));
}

/*!
    Destroys the server.
 */
QObexServer::~QObexServer()
{
    if (m_data)
        delete m_data;
}

/*!
    Returns true if the server has a pending connection; otherwise returns 
    false.

    \sa nextPendingConnection()
 */
bool QObexServer::hasPendingConnections() const
{
    return (!m_data->m_pending.isEmpty());
}

/*!
    Returns the next pending connection as a QObexSocket object.

    The socket is created as a child of the server, which means that it is
    automatically deleted when the server object is destroyed. It is still a
    good idea to delete the object explicitly when you are done with it, to
    avoid wasting memory.

    Returns 0 if there are no pending connections.

    \sa hasPendingConnections()
 */
QObexSocket *QObexServer::nextPendingConnection()
{
    return 0;
}

/*!
    Returns an implementation dependent handle for the next pending 
    connection. 

    Returns 0 if there are no pending connections.
*/
void *QObexServer::spawnReceiver()
{
    return m_data->spawnReceiver();
}

/*!
    Closes the server. The server will no longer listen for incoming 
    connections.
 */
void QObexServer::close()
{
    m_data->close();
}

/*!
    Returns whether the server is currently listening for connections.

    \sa listen()
*/
bool QObexServer::isListening() const
{
    return m_data->m_self != 0;
}

/*!
    Tells the server to listen for incoming connections.

    Returns true on success; otherwise returns false.

    \sa isListening()
*/
bool QObexServer::listen()
{
    return m_data->listen();
}

/*!
    \fn void *QObexServer::registerServer()

    Sets up the server and returns an implementation dependent handle to a server
    socket. Returns 0 if the server could not be set up appropriately.

    Subclasses must implement this function according to their own specific 
    transport protocols.
*/

/*!
    \fn void QObexServer::newConnection()

    This signal is emitted whenever a new client has connected to the server.

    \sa hasPendingConnections(), nextPendingConnection()
 */

#include "qobexserver.moc"
