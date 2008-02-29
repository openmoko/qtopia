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
    m_error = QBluetoothAbstractServer::UnknownError;
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
    \brief The QBluetoothAbstractServer class represents an abstract Bluetooth server socket.

    This class makes it possible to accept incoming Bluetooth connections.

    Call the subclass listen() implementation to have the server listen
    for incoming connections.  The newConnection() signal is emitted each time
    a client connects to the server.

    Call nextPendingConnection() to accept the pending connection
    as a connected QBluetoothAbstractSocket.  The actual socket type depends on the
    type of the server.  E.g. the QBluetoothRfcommServer class will return
    QBluetoothRfcommSocket sockets.  The function returns a pointer to a
    QBluetoothAbstractSocket in QBluetoothAbstractSocket::ConnectedState that you can
    use for communicating with the client.

    If an error occurs, lastError() returns the type of error that has occurred.

    Calling close() makes the QBluetoothAbstractServer stop listening for incoming
    connections.
 */

/*!
    \enum QBluetoothAbstractServer::ServerError
    \brief The error that has occurred on the server.

    \value NoError No error.
    \value ResourceError The system has run out of sockets.
    \value BindError The socket could not be bound to the address provided.
    \value ListenError The socket could not be listened on.
    \value UnknownError An unknown error has occurred.
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
    Destructor.
 */
QBluetoothAbstractServer::~QBluetoothAbstractServer()
{
    close();

    if (m_data)
        delete m_data;
}

/*! \fn void QBluetoothAbstractServer::newConnection()

    This signal is emitted every time a new connection is available.

    \sa hasPendingConnections(), nextPendingConnection()
 */

/*!
    Subclasses of this class should call this function and hand off the
    server socket that will be used for accepting connections.  The
    \a socket parameter contains the socket file descriptor, the
    \a addr parameter contains the sockaddr structure of the address
    to bind and listen on.  The \a len contains the length of
    the \a addr structure.

    \sa isListening()
 */
bool QBluetoothAbstractServer::initiateListen(int socket, sockaddr *addr, int len)
{
    m_data->m_fd = socket;

    int flags = fcntl(m_data->m_fd, F_GETFL, 0);
    fcntl(m_data->m_fd, F_SETFL, flags | O_NONBLOCK);

    ::fcntl(m_data->m_fd, F_SETFD, FD_CLOEXEC);

    if (bind(m_data->m_fd, addr, len) < 0) {
        qWarning("QBluetoothScoServer::listen couldn't bind server...");
        ::close(m_data->m_fd);
        m_data->m_error = QBluetoothAbstractServer::BindError;
        return false;
    }

    if (::listen(m_data->m_fd, 1) < 0) {
        qWarning("QBluetoothScoServer::listen Couldn't listen... %d %s", errno, strerror(errno));
        ::close(m_data->m_fd);
        m_data->m_error = QBluetoothAbstractServer::ListenError;
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
 */
bool QBluetoothAbstractServer::isListening() const
{
    return m_data->m_isListening;
}

/*!
    Closes the server. The server will no longer listen for incoming
    connections.
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
 */
QBluetoothAbstractServer::ServerError QBluetoothAbstractServer::lastError() const
{
    return m_data->m_error;
}

/*!
    Returns the maximum number of pending accepted connections. The
    default is 7.

    \sa setMaxPendingConnections(), hasPendingConnections()
 */
int QBluetoothAbstractServer::maxPendingConnections() const
{
    return m_data->m_maxConnections;
}

/*!
    Sets the maximum number of pending accepted connections to \a
    numConnections. QBluetoothScoServer will accept no more than \a
    numConnections incoming connections before
    nextPendingConnection() is called. By default, the limit is 1
    pending connection.
 */
void QBluetoothAbstractServer::setMaxPendingConnections(int numConnections)
{
    m_data->m_maxConnections = numConnections;
}

/*!
    Returns true if the server has a pending connection; otherwise
    returns false.

    \sa nextPendingConnection(), setMaxPendingConnections()
 */
bool QBluetoothAbstractServer::hasPendingConnections() const
{
    return !m_data->m_pendingConnections.isEmpty();
}

/*!
    Returns the next pending connection as a connected QBluetoothRfcommSocket
    object.

    The socket is created as a child of the server, which means that
    it is automatically deleted when the QBluetoothScoServer object is
    destroyed. It is still a good idea to delete the object
    explicitly when you are done with it, to avoid wasting memory.

    0 is returned if this function is called when there are no pending
    connections.

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

    This is a blocking function call. Its use is disadvised in a
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
                m_data->m_error = QBluetoothAbstractServer::UnknownError;
                close();
                return false;
        }
    }
}

/*!
    Sets the last error that has occurred to \a error.
*/
void QBluetoothAbstractServer::setError(const QBluetoothAbstractServer::ServerError &error)
{
    m_data->m_error = error;
}

/*!
    Returns the socket descriptor the server is currently listening on.  If the
    server is not listening, then -1 is returned.
*/
int QBluetoothAbstractServer::socketDescriptor() const
{
    return m_data->m_fd;
}

/*!
    \fn QBluetoothAbstractSocket * QBluetoothAbstractServer::createSocket();

    Clients of this class should override this function to provide sockets specific
    to the protocol that the client server implements.
*/

#include "qbluetoothabstractserver.moc"
