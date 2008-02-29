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
#include <QDebug>
#include <QtAlgorithms>

#include <qtopia/comm/qbluetoothabstractsocket.h>
#include <qtopia/comm/qbluetoothabstractserver.h>
#include <qtopiacomm/private/qbluetoothnamespace_p.h>
#include <qtopia/comm/qbluetoothaddress.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <bluetooth/sco.h>
#include <fcntl.h>

class QBluetoothAbstractServerPrivate : public QObject
{
    Q_OBJECT

public:
    QBluetoothAbstractServerPrivate(QBluetoothAbstractServer *parent);
    ~QBluetoothAbstractServerPrivate();

    int m_fd;

    bool m_isListening;
    QBluetoothAbstractServer::ServerError m_error;
    QString m_errorString;
    int m_maxConnections;
    QList<QBluetoothAbstractSocket *> m_pendingConnections;

    QBluetoothAbstractServer *m_parent;
    QSocketNotifier *m_readNotifier;

private slots:
    void incomingConnection();
};

QBluetoothAbstractServerPrivate::QBluetoothAbstractServerPrivate(QBluetoothAbstractServer *parent)
{
    m_parent = parent;
    m_isListening = false;
    m_error = QBluetoothAbstractServer::NoError;
    m_errorString = QString();
    m_maxConnections = 1;
    m_readNotifier = 0;
}

QBluetoothAbstractServerPrivate::~QBluetoothAbstractServerPrivate()
{
    if (m_readNotifier)
        delete m_readNotifier;
}

void QBluetoothAbstractServerPrivate::incomingConnection()
{
    while (true) {
        if (m_pendingConnections.count() > m_maxConnections) {
            qWarning("QBluetoothAbstractServer::incomingConnection() too many connections");
            if (m_readNotifier->isEnabled())
                m_readNotifier->setEnabled(false);
            return;
        }

        int fd = ::accept(m_fd, 0, 0);
        if (fd  < 0) {
            qWarning("QBluetoothAbstractServerPrivate::incomingConnection Accept failed...");
            return;
        }

        ::fcntl(fd, F_SETFD, FD_CLOEXEC);

        QBluetoothAbstractSocket *socket = m_parent->createSocket();
        socket->setSocketDescriptor(fd, QBluetoothAbstractSocket::ConnectedState,
                                    QIODevice::ReadWrite);
        m_pendingConnections.append(socket);

        QPointer<QBluetoothAbstractServer> that = m_parent;
        emit m_parent->newConnection();
        if (!that || !m_parent->isListening())
            return;
    }
}

/*!
    \class QBluetoothAbstractServer
    \mainclass
    \brief The QBluetoothAbstractServer class represents an abstract Bluetooth server socket.

    This class is a common base class for all Bluetooth server
    socket implementations and makes it possible to accept incoming
    Bluetooth connections.  The users should not use this class
    directly, instead they should use concrete implementations.

    The subclasses will implement a \c{listen()} implementation in order
    to create a listening socket for each particular Bluetooth socket
    type.  In particular, QBluetoothL2CapServer, QBluetoothRfcommServer,
    and QBluetoothScoServer will create L2CAP, RFCOMM and SCO sockets
    respectively.

    Services that provide RFCOMM based profiles should use the
    QBluetoothRfcommServer class.  L2CAP based services should use
    the QBluetoothL2CapServer class.  Headset and Handsfree profile
    implementations will require the use of QBluetoothScoServer
    class.

    The typical use of this class is to call the \c{listen()} implementation
    in the subclass, and then hook onto the newConnection() signal to
    get an indication of an incoming connection. The newConnection()
    signal is emitted each time a client has connected to the server.
    Call nextPendingConnection() to accept the pending
    connection as a connected QBluetoothAbstractSocket.  The actual socket
    type depends on the type of the server.  E.g. the
    QBluetoothRfcommServer socket will return QBluetoothRfcommSocket type
    sockets.

    If an error occurs, error() returns the type of error that has
    occurred.

    Calling close() makes the QBluetoothAbstractServer stop
    listening for incoming connections and deletes all pending
    connections.

    \sa QBluetoothAbstractSocket

    \ingroup qtopiabluetooth
 */

/*!
    \enum QBluetoothAbstractServer::ServerError
    \brief The error that has occurred on the server.

    \value NoError No error.
    \value UnknownError An unknown error has occurred.
    \value ResourceError The system has run out of sockets.
    \value ListenError The socket could not be listened on.
    \value BindError The socket could not be bound to the address provided.
 */

/*!
    Constructs a new QBluetoothAbstractServer with parent \a parent.
    The server is in the UnconnectedState.
 */
QBluetoothAbstractServer::QBluetoothAbstractServer(QObject *parent)
    : QObject(parent)
{
    m_data = new QBluetoothAbstractServerPrivate(this);
}

/*!
    Destroys the server.
 */
QBluetoothAbstractServer::~QBluetoothAbstractServer()
{
    close();

    if (m_data)
        delete m_data;
}

/*! \fn void QBluetoothAbstractServer::newConnection()

    This signal is emitted every time a new connection is available.
    Call nextPendingConnection() in order to accept the connection.

    \sa hasPendingConnections(), nextPendingConnection()
 */

/*!
    \internal

    Subclasses of this class should call this function and hand off the
    server socket that will be used for accepting connections.  The
    \a socket parameter contains the socket file descriptor, the
    \a addr parameter contains the sockaddr structure of the address
    to bind and listen on and the \a len parameter contains the length of
    the \a addr structure.

    Returns true on successful completion of the request;
    otherwise returns false.

    \sa isListening()
 */
bool QBluetoothAbstractServer::initiateListen(int socket, sockaddr *addr, int len)
{
    m_data->m_fd = socket;

    int flags = fcntl(m_data->m_fd, F_GETFL, 0);
    fcntl(m_data->m_fd, F_SETFL, flags | O_NONBLOCK);

    ::fcntl(m_data->m_fd, F_SETFD, FD_CLOEXEC);

    if (bind(m_data->m_fd, addr, len) < 0) {
        qWarning("QBluetoothAbstractServer::listen couldn't bind server...");
        ::close(m_data->m_fd);
        m_data->m_fd = -1;
        setError(QBluetoothAbstractServer::BindError);
        return false;
    }

    if (::listen(m_data->m_fd, 1) < 0) {
        qWarning("QBluetoothAbstractServer::listen Couldn't listen... %d %s", errno, strerror(errno));
        ::close(m_data->m_fd);
        m_data->m_fd = -1;
        setError(QBluetoothAbstractServer::ListenError);
        return false;
    }

    if (!m_data->m_readNotifier)
        m_data->m_readNotifier = new QSocketNotifier(m_data->m_fd, QSocketNotifier::Read);

    m_data->m_readNotifier->setEnabled(true);

    connect(m_data->m_readNotifier, SIGNAL(activated(int)),
            m_data, SLOT(incomingConnection()));

    m_data->m_isListening = true;

    return true;
}

/*!
    Returns true if the server is currently listening for remote connections,
    and false otherwise.

    \sa close(), socketDescriptor()
 */
bool QBluetoothAbstractServer::isListening() const
{
    return m_data->m_isListening;
}

/*!
    Closes the server. The server will no longer listen for incoming
    connections and all pending connections will be closed.

    \sa isListening(), socketDescriptor()
 */
void QBluetoothAbstractServer::close()
{
    qDeleteAll(m_data->m_pendingConnections);
    m_data->m_pendingConnections.clear();

    if (m_data->m_readNotifier) {
        delete m_data->m_readNotifier;
        m_data->m_readNotifier = 0;
    }

    ::close(m_data->m_fd);
    m_data->m_fd = -1;

    m_data->m_isListening = false;
}

/*!
    Returns the last error that has occurred.

    \sa errorString()
 */
QBluetoothAbstractServer::ServerError QBluetoothAbstractServer::error() const
{
    return m_data->m_error;
}

/*!
    Returns a human-readable description of the last device error that occurred.

    \sa error()
*/
QString QBluetoothAbstractServer::errorString() const
{
    return m_data->m_errorString;
}

/*!
    Returns the maximum number of pending accepted connections. The
    default is 1.

    \sa setMaxPendingConnections(), hasPendingConnections()
 */
int QBluetoothAbstractServer::maxPendingConnections() const
{
    return m_data->m_maxConnections;
}

/*!
    Sets the maximum number of pending accepted connections to \a
    numConnections. QBluetoothAbstractServer will accept no more than \a
    numConnections incoming connections before
    nextPendingConnection() is called. By default, the limit is 1
    pending connection.

    \sa maxPendingConnections(), hasPendingConnections()
 */
void QBluetoothAbstractServer::setMaxPendingConnections(int numConnections)
{
    m_data->m_maxConnections = numConnections;
}

/*!
    Returns true if the server has a pending connection(s); otherwise
    returns false.

    \sa nextPendingConnection(), setMaxPendingConnections()
 */
bool QBluetoothAbstractServer::hasPendingConnections() const
{
    return !m_data->m_pendingConnections.isEmpty();
}

/*!
    Returns the next pending connection as a connected
    QBluetoothAbstractSocket object.  The function returns a
    pointer to a QBluetoothAbstractSocket in
    QBluetoothAbstractSocket::ConnectedState that you can
    use for communicating with the client.

    The type of the socket returned will be based on the server
    socket type used.  You will need to use qobject_cast to convert
    the result into the concrete socket type required.

    \code
        QBluetoothAbstractSocket *sock = server->nextPendingConnection();
        if (sock) {
            QBluetoothRfcommSocket *rfcomm =
                qobject_cast<QBluetoothRfcommSocket *>(sock);
            if (rfcomm) {
                // use RFCOMM socket
            }
        }
    \endcode

    The socket is created as a child of the server, which means that
    it is automatically deleted when the QBluetoothAbstractServer object is
    destroyed. It is still a good idea to delete the object
    explicitly when you are done with it, to avoid wasting memory.

    A NULL pointer is returned if this function is called when
    there are no pending connections.

    \sa hasPendingConnections()
 */
QBluetoothAbstractSocket *QBluetoothAbstractServer::nextPendingConnection()
{
    if (m_data->m_pendingConnections.isEmpty())
        return 0;

    if (!m_data->m_readNotifier->isEnabled())
        m_data->m_readNotifier->setEnabled(true);

    return m_data->m_pendingConnections.takeFirst();
}

/*!
    Waits for at most \a msecs milliseconds or until an incoming
    connection is available. Returns true if a connection is
    available; otherwise returns false. If the operation timed out
    and \a timedOut is not 0, *\a timedOut will be set to true.

    This is a blocking function call. Its use is not advised in a
    single-threaded GUI application, since the whole application will
    stop responding until the function returns.
    waitForNewConnection() is mostly useful when there is no event
    loop available.

    The non-blocking alternative is to connect to the newConnection()
    signal.

    \sa hasPendingConnections(), nextPendingConnection()
 */
bool QBluetoothAbstractServer::waitForNewConnection(int msecs, bool *timedOut)
{
    if (!m_data->m_isListening)
        return false;

    QTime stopWatch;
    stopWatch.start();

    while (true) {
        fd_set fdread;
        FD_ZERO(&fdread);
        FD_SET(m_data->m_fd, &fdread);

        int timeout = msecs < 0 ? 0 : msecs - stopWatch.elapsed();
        struct timeval tv;
        struct timeval *tvptr;

        if (msecs != -1) {
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = (timeout % 1000) * 1000;
            tvptr = &tv;
        }

        int rv = ::select(m_data->m_fd + 1, &fdread, 0, 0, &tv);

        switch (rv) {
            case 0:
                if (timedOut) {
                    *timedOut = true;
                }
                return false;
            case 1:
                return true;
            default:
                if (rv == EINTR)
                    continue;
                // Otherwise, close the socket
                setError(QBluetoothAbstractServer::UnknownError);
                close();
                return false;
        }
    }
}

/*!
    \internal
    Sets the last error that has occurred to \a serverError.  Only
    subclasses should use this function.
*/
void QBluetoothAbstractServer::setError(QBluetoothAbstractServer::ServerError serverError)
{
    m_data->m_error = serverError;

    switch (serverError) {
        case NoError:
            m_data->m_errorString = QString();
            break;

        case ResourceError:
            m_data->m_errorString = QLatin1String(QT_TRANSLATE_NOOP("QBluetoothAbstractServer", "Out of resources"));
            break;

        case ListenError:
            m_data->m_errorString = QLatin1String(QT_TRANSLATE_NOOP("QBluetoothAbstractServer", "Could not listen on socket"));
            break;

        case BindError:
            m_data->m_errorString = QLatin1String(QT_TRANSLATE_NOOP("QBluetoothAbstractServer", "Could not bind socket"));
            break;

        default:
            m_data->m_errorString = QLatin1String(QT_TRANSLATE_NOOP("QBluetoothAbstractServer",
                    "Unknown error"));
            break;
    };
}

/*!
    Returns the socket descriptor the server is currently listening on.  If the
    server is not listening, then -1 is returned.

    \sa isListening()
*/
int QBluetoothAbstractServer::socketDescriptor() const
{
    return m_data->m_fd;
}

/*!
    \internal

    \fn QBluetoothAbstractSocket * QBluetoothAbstractServer::createSocket();

    Clients of this class should override this function to provide sockets specific
    to the protocol that the client server implements.
*/

#include "qbluetoothabstractserver.moc"
