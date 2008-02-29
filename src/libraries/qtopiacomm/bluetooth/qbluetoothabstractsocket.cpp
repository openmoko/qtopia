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

#include <private/qringbuffer_p.h>
#include <QSocketNotifier>
#include <QTimer>
#include <QTime>
#include <QDebug>
#include <QtAlgorithms>

#include <qbluetoothabstractsocket.h>
#include <qbluetoothaddress.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>

static const int QBLUETOOTHRFCOMMSOCKET_DEFAULT_READBUFFERSIZE = 1024;
static const int QBLUETOOTHRFCOMMSOCKET_DEFAULT_WRITEBUFFERSIZE = 4096;

class QBluetoothAbstractSocketPrivate : public QObject
{
    Q_OBJECT

public:
    QBluetoothAbstractSocketPrivate(QBluetoothAbstractSocket *parent);
    ~QBluetoothAbstractSocketPrivate();

    bool initiateConnect(int socket, struct sockaddr *addr, int size);
    bool initiateDisconnect();

    bool flush();

    qint64 writeToSocket(const char *data, qint64 len);
    qint64 readFromSocket(char *data, qint64 len);
    bool readData();

    void resetNotifiers();
    void setupNotifiers();

public slots:
    void testConnected();
    void abortConnectionAttempt();
    bool readActivated();
    bool writeActivated();

public:
    QBluetoothAbstractSocket *m_parent;
    QBluetoothAbstractSocket::SocketError m_error;
    QBluetoothAbstractSocket::SocketState m_state;
    int m_fd;
    QSocketNotifier *m_readNotifier;
    QSocketNotifier *m_writeNotifier;
    QTimer *m_timer;

    QRingBuffer m_writeBuffer;
    QRingBuffer m_readBuffer;

    bool m_readSocketNotifierCalled;
    bool m_readSocketNotifierState;
    bool m_readSocketNotifierStateSet;
    bool m_emittedReadyRead;
    bool m_closeCalled;
    bool m_emittedBytesWritten;
    qint64 m_readBufferCapacity;
};

static const int QT_BLUETOOTH_CONNECT_TIMEOUT = 15000;

QBluetoothAbstractSocketPrivate::QBluetoothAbstractSocketPrivate(QBluetoothAbstractSocket *parent)
    : m_writeBuffer(QBLUETOOTHRFCOMMSOCKET_DEFAULT_WRITEBUFFERSIZE),
    m_readBuffer(QBLUETOOTHRFCOMMSOCKET_DEFAULT_READBUFFERSIZE)
{
    m_parent = parent;
    m_error = QBluetoothAbstractSocket::NoError;
    m_state = QBluetoothAbstractSocket::UnconnectedState;
    m_fd = -1;
    m_readNotifier = 0;
    m_writeNotifier = 0;
    m_timer = 0;
    m_readBufferCapacity = 0;
    m_closeCalled = false;

    m_readSocketNotifierCalled = false;
    m_readSocketNotifierState = false;
    m_readSocketNotifierStateSet = false;
    m_emittedReadyRead = false;
    m_emittedBytesWritten = false;
}

QBluetoothAbstractSocketPrivate::~QBluetoothAbstractSocketPrivate()
{
    if (m_readNotifier)
        delete m_readNotifier;

    if (m_writeNotifier)
        delete m_writeNotifier;

    if (m_timer) {
        m_timer->stop();
        delete m_timer;
        m_timer = 0;
    }
}

void QBluetoothAbstractSocketPrivate::setupNotifiers()
{
    if (!m_readNotifier) {
        m_readNotifier = new QSocketNotifier(m_fd,
                                             QSocketNotifier::Read,
                                             this);
    }

    m_readNotifier->setEnabled(true);
    QObject::connect(m_readNotifier, SIGNAL(activated(int)),
                     this, SLOT(readActivated()));

    if (!m_writeNotifier) {
        m_writeNotifier = new QSocketNotifier(m_fd,
                                              QSocketNotifier::Write,
                                              this);
    }

    m_writeNotifier->setEnabled(false);
    QObject::connect(m_writeNotifier, SIGNAL(activated(int)),
                     this, SLOT(writeActivated()));
}

void QBluetoothAbstractSocketPrivate::resetNotifiers()
{

    if (m_readNotifier) {
        delete m_readNotifier;
        m_readNotifier = 0;
    }

    if (m_writeNotifier) {
        delete m_writeNotifier;
        m_writeNotifier = 0;
    }

    if (m_timer) {
        m_timer->stop();
        delete m_timer;
        m_timer = 0;
    }

    m_readSocketNotifierCalled = false;
    m_readSocketNotifierState = false;
    m_readSocketNotifierStateSet = false;
    m_emittedReadyRead = false;
    m_emittedBytesWritten = false;
}

void QBluetoothAbstractSocketPrivate::abortConnectionAttempt()
{
    if (m_writeNotifier)
        disconnect(m_writeNotifier, SIGNAL(activated(int)),
                   this, SLOT(testConnected()));
    ::close(m_fd);
    m_fd = -1;
    m_state = QBluetoothAbstractSocket::UnconnectedState;
    m_error = QBluetoothAbstractSocket::TimedoutError;
    QPointer<QBluetoothAbstractSocket> that = m_parent;
    emit m_parent->error(m_error);
    if (that)
        emit m_parent->stateChanged(m_state);
}

void QBluetoothAbstractSocketPrivate::testConnected()
{
    if (m_writeNotifier)
        disconnect(m_writeNotifier, SIGNAL(activated(int)),
                   this, SLOT(testConnected()));

    if (m_timer) {
        m_timer->stop();
        delete m_timer;
        m_timer = 0;
    }

    int error = 0;
    socklen_t len = sizeof(error);

    if (getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        m_error = QBluetoothAbstractSocket::UnknownError;
        m_state = QBluetoothAbstractSocket::UnconnectedState;
            ::close(m_fd);
            QPointer<QBluetoothAbstractSocket> that = m_parent;
            emit m_parent->error(m_error);
            if (that)
                emit m_parent->stateChanged(m_state);
            return;
    }
    else {
        if (error) {
            m_state = QBluetoothAbstractSocket::UnconnectedState;

            switch (error) {
                case EADDRINUSE:
                    m_error = QBluetoothAbstractSocket::NetworkError;
                    break;
                case ETIMEDOUT:
                    m_error = QBluetoothAbstractSocket::TimedoutError;
                    break;
                case EINVAL:
                    m_error = QBluetoothAbstractSocket::UnknownError;
                    break;
                case EHOSTDOWN:
                    m_error = QBluetoothAbstractSocket::HostDownError;
                    break;
                case ECONNREFUSED:
                    m_error = QBluetoothAbstractSocket::ConnectionRefused;
                    break;
                case EAGAIN:
                    m_error = QBluetoothAbstractSocket::UnknownError;
                    break;
                case EACCES:
                case EPERM:
                    m_error = QBluetoothAbstractSocket::AccessError;
                    break;
                default:
                    m_error = QBluetoothAbstractSocket::UnknownError;
                    break;
            };

            QPointer<QBluetoothAbstractSocket> that = m_parent;
            emit m_parent->error(m_error);
            if (that)
                emit m_parent->stateChanged(m_state);
            return;
        }
    }

    m_parent->readSocketParameters(m_fd);
    setupNotifiers();

    m_state = QBluetoothAbstractSocket::ConnectedState;
    m_parent->setOpenMode(QIODevice::ReadWrite);
    QPointer<QBluetoothAbstractSocket> that = m_parent;
    emit m_parent->connected();
    if (that)
        emit m_parent->stateChanged(m_state);
}

qint64 QBluetoothAbstractSocketPrivate::writeToSocket(const char *data, qint64 len)
{
    ssize_t writtenBytes;
    do {
        writtenBytes = ::write(m_fd, data, len);
    } while (writtenBytes < 0 && errno == EINTR);

    if (writtenBytes < 0) {
    switch (errno) {
        case EPIPE:
        case ECONNRESET:
            writtenBytes = -1;
            m_error = QBluetoothAbstractSocket::RemoteHostClosedError;
            m_parent->close();
            break;
        case EAGAIN:
            writtenBytes = 0;
            break;
        case EMSGSIZE:
            m_error = QBluetoothAbstractSocket::NetworkError;
            break;
        default:
            break;
    }
    }

    return qint64(writtenBytes);
}

qint64 QBluetoothAbstractSocketPrivate::readFromSocket(char *data, qint64 maxSize)
{
    ssize_t r = 0;
    do {
        r = ::read(m_fd, data, maxSize);
    } while (r == -1 && errno == EINTR);

    if (r < 0) {
        r = -1;
        switch (errno) {
            case EAGAIN:
                r = -2;
                break;
            case EBADF:
            case EINVAL:
            case EIO:
                m_error = QBluetoothAbstractSocket::NetworkError;
                break;
            case ECONNRESET:
                r = 0;
                break;
            default:
                break;
        }
    }

    return qint64(r);
}

bool QBluetoothAbstractSocketPrivate::readData()
{
    do {
        qint64 bytesToRead = 4096;

        if (m_readBufferCapacity &&
            (bytesToRead > (m_readBufferCapacity - m_readBuffer.size())))
            bytesToRead = m_readBufferCapacity - m_readBuffer.size();

        if (bytesToRead == 0)
            break;

        char *ptr = m_readBuffer.reserve(bytesToRead);
        qint64 readBytes = readFromSocket(ptr, bytesToRead);

        if (readBytes == -2) {
            m_readBuffer.chop(bytesToRead);
            return true;
        }
        else if (readBytes == -1) {
            m_readBuffer.chop(bytesToRead);
            emit m_parent->error(m_error);
            return false;
        }

        m_readBuffer.chop(int(bytesToRead - (readBytes < 0 ? qint64(0) : readBytes)));

        if (readBytes == 0)
            return false;
    } while (1);

    return true;
}

bool QBluetoothAbstractSocketPrivate::initiateConnect(int socket,
        struct sockaddr *addr, int size)
{
    m_closeCalled = false;
    m_writeBuffer.clear();
    m_readBuffer.clear();

    m_fd = socket;

    int flags = fcntl(m_fd, F_GETFL, 0);
    fcntl(m_fd, F_SETFL, flags | O_NONBLOCK);
    ::fcntl(m_fd, F_SETFD, FD_CLOEXEC);

    if (::connect(m_fd, addr, size) < 0) {
        qWarning("QBluetoothAbstractSocketPrivate::initiateConnect: error during connect: %d, %s",
                 errno, strerror(errno));
        switch(errno) {
            case EINVAL:
                m_error = QBluetoothAbstractSocket::UnknownError;
                break;
            case EISCONN:
                m_state = QBluetoothAbstractSocket::ConnectedState;
                break;
            case ECONNREFUSED:
                m_error = QBluetoothAbstractSocket::ConnectionRefused;
                break;
            case ETIMEDOUT:
            case EADDRINUSE:
                m_error = QBluetoothAbstractSocket::NetworkError;
                break;
            case EINPROGRESS:
            case EALREADY:
                m_state = QBluetoothAbstractSocket::ConnectingState;
                break;
            case EAGAIN:
                // Seems that bluetooth stack uses this instead of EINPROGRESS
                m_state = QBluetoothAbstractSocket::ConnectingState;
                break;
            case EACCES:
            case EPERM:
                m_error = QBluetoothAbstractSocket::AccessError;
                break;
            default:
                m_error = QBluetoothAbstractSocket::UnknownError;
                break;
        }
    }
    else {
        m_state = QBluetoothAbstractSocket::ConnectedState;
    }

    if (m_state == QBluetoothAbstractSocket::ConnectingState) {
        if (!m_writeNotifier)
            m_writeNotifier = new QSocketNotifier(m_fd, QSocketNotifier::Write);
        connect(m_writeNotifier, SIGNAL(activated(int)),
                this, SLOT(testConnected()));
        if (!m_timer) {
            m_timer = new QTimer(this);
            QObject::connect(m_timer, SIGNAL(timeout()),
                             this, SLOT(abortConnectionAttempt()));
        }
        m_timer->start(QT_BLUETOOTH_CONNECT_TIMEOUT);

        return true;
    }

    return (m_state == QBluetoothAbstractSocket::ConnectedState);
}

bool QBluetoothAbstractSocketPrivate::initiateDisconnect()
{
    if (m_readNotifier)
        m_readNotifier->setEnabled(false);
    if (m_state != QBluetoothAbstractSocket::ClosingState) {
        m_state = QBluetoothAbstractSocket::ClosingState;
        emit m_parent->stateChanged(m_state);
    }

    if (m_writeBuffer.size() > 0) {
        m_writeNotifier->setEnabled(true);
        return true;
    }

    resetNotifiers();
    m_state = QBluetoothAbstractSocket::UnconnectedState;
    close(m_fd);
    m_fd = -1;

    QPointer<QBluetoothAbstractSocket> that = m_parent;
    emit m_parent->disconnected();

    if (!that)
        return true;

    emit m_parent->stateChanged(m_state);

    if (!that)
        return true;

    m_parent->resetSocketParameters();

    if (m_closeCalled) {
        m_readBuffer.clear();
        m_writeBuffer.clear();
        m_parent->QIODevice::close();
    }

    return true;
}

bool QBluetoothAbstractSocketPrivate::readActivated()
{
    if (m_readSocketNotifierCalled) {
        if (!m_readSocketNotifierStateSet) {
            m_readSocketNotifierStateSet = true;
            m_readSocketNotifierState = m_readNotifier->isEnabled();
            m_readNotifier->setEnabled(false);
        }
    }
    m_readSocketNotifierCalled = true;

    if (m_readBufferCapacity && m_readBuffer.size() >= m_readBufferCapacity) {
        m_readSocketNotifierCalled = false;
        return false;
    }

    qint64 newBytes = m_readBuffer.size();

    if (!readData()) {
        m_readSocketNotifierCalled = false;
        initiateDisconnect();
        return false;
    }

    newBytes = m_readBuffer.size() - newBytes;

    // If read buffer is full, disable the read notifier
    if (m_readBufferCapacity &&
        m_readBuffer.size() == m_readBufferCapacity) {
        m_readNotifier->setEnabled(false);
    }


    if (!m_emittedReadyRead && newBytes) {
        m_emittedReadyRead = true;
        emit m_parent->readyRead();
        m_emittedReadyRead = false;
    }

    if ((m_state == QBluetoothAbstractSocket::UnconnectedState) ||
            (m_state == QBluetoothAbstractSocket::ClosingState)) {
        m_readSocketNotifierCalled = false;
        return true;
    }

    if (m_readSocketNotifierStateSet &&
        m_readSocketNotifierState != m_readNotifier->isEnabled()) {
        m_readNotifier->setEnabled(m_readSocketNotifierState);
        m_readSocketNotifierStateSet = false;
    }

    m_readSocketNotifierCalled = false;
    return true;
}

bool QBluetoothAbstractSocketPrivate::writeActivated()
{
    int tmp = m_writeBuffer.size();
    flush();

    if (m_writeBuffer.isEmpty())
        m_writeNotifier->setEnabled(false);

    return m_writeBuffer.size() < tmp;
}

bool QBluetoothAbstractSocketPrivate::flush()
{
    if (m_writeBuffer.isEmpty())
        return false;

    int nextSize = m_writeBuffer.nextDataBlockSize();
    const char *ptr = m_writeBuffer.readPointer();

    // Attempt to write it all in one chunk.
    qint64 written = writeToSocket(ptr, nextSize);
    if (written < 0) {
        emit m_parent->error(m_error);
        // an unexpected error so close the socket.
        qWarning("QBluetoothAbstractSocket::flush: unexpected error");
        m_parent->abort();
        return false;
    }

    m_writeBuffer.free(written);
    if (written > 0) {
        if (!m_emittedBytesWritten) {
            m_emittedBytesWritten = true;
            emit m_parent->bytesWritten(written);
            m_emittedBytesWritten = false;
        }
    }

    if (m_writeBuffer.isEmpty() && m_writeNotifier->isEnabled()) {
        m_writeNotifier->setEnabled(false);
    }

    if (m_state == QBluetoothAbstractSocket::ClosingState) {
        m_parent->close();
    }

    return true;
}

/*!
    \class QBluetoothAbstractSocket
    \brief The QBluetoothAbstractSocket class represents an abstract Bluetooth client socket.

    At any time, the QBluetoothAbstractSocket has a state (returned by
    state()). The initial state is QBluetoothAbstractSocket::UnconnectedState. After
    calling connect(), the socket enters QBluetoothAbstractSocket::ConnectingState.
    If connection is established, the socket enters
    QBluetoothAbstractSocket::ConnectedState and emits connected(). If an error
    occurs at any stage, error() is emitted. Whenever the state changes,
    stateChanged() is emitted.
    For convenience, isValid() returns true if the socket is ready for
    reading and writing.

    Read or write data by calling read() or write(), or use the
    convenience functions readLine() and readAll(). QBluetoothAbstractSocket
    also inherits getChar(), putChar(), and ungetChar() from
    QIODevice, which work on single bytes. For every chunk of data
    that has been written to the socket, the bytesWritten() signal is
    emitted.

    The readyRead() signal is emitted every time a new chunk of data
    has arrived. bytesAvailable() then returns the number of bytes
    that are available for reading. Typically, you would connect the
    readyRead() signal to a slot and read all available data there.
    If you don't read all the data at once, the remaining data will
    still be available later, and any new incoming data will be
    appended to QBluetoothAbstractSocket's internal read buffer. To limit the
    size of the read buffer, call setReadBufferSize().

    To close the socket, call disconnect(). QBluetoothAbstractSocket enters
    QBluetoothAbstractSocket::ClosingState, then emits closing(). After all
    pending data has been written to the socket, QBluetoothAbstractSocket actually
    closes the socket, enters the QBluetoothAbstractSocket::ClosedState,
    and emits disconnected(). If no data is pending when disconnect() is
    called, the connection is disconnected immediately.  If you want to
    abort a connection immediately, discarding all pending data, call
    abort() instead.

    QBluetoothAbstractSocket provides a set of functions that suspend the
    calling thread until certain signals are emitted. These functions
    can be used to implement blocking sockets:

    \list
        \o waitForConnected() blocks until a connection has been established.

        \o waitForReadyRead() blocks until new data is available for
           reading.

        \o waitForBytesWritten() blocks until one payload of data has been
           written to the socket.

        \o waitForDisconnected() blocks until the connection has closed.
    \endlist

    \ingroup qtopiabluetooth
 */

/*!
    \enum QBluetoothAbstractSocket::SocketState
    \brief State of the rfcomm socket.

    \value UnconnectedState The socket is not connected.
    \value ConnectingState The socket is being connected.
    \value ConnectedState The socket is connected.
    \value ClosingState The socket is being closed.
 */

/*!
    \enum QBluetoothAbstractSocket::SocketError
    \brief Error that last occurred on the rfcomm socket.

    \value NoError No error has occurred.
    \value AccessError The client has inadequate permissions to access the socket.
    \value ResourceError The kernel has run out of sockets.
    \value BindError The socket could not be bound to a particular address.
    \value ConnectionRefused The remote host has refused a connection.
    \value HostDownError The remote host could not be contacted.
    \value NetworkError A network error has occurred, e.g. device moved out of range.
    \value TimedoutError Operation has timed out.
    \value RemoteHostClosedError Remote host has closed the connection.
    \value UnknownError Unknown error has occurred.
 */

/*!
    Constructs a new QBluetoothAbstractSocket with \a parent. The socket is not
    connected.
 */
QBluetoothAbstractSocket::QBluetoothAbstractSocket(QObject *parent)
    : QIODevice(parent)
{
    setOpenMode(QIODevice::NotOpen);

    m_data = new QBluetoothAbstractSocketPrivate(this);
}

/*!
    Deconstructs a QBluetoothAbstractSocket.
 */
QBluetoothAbstractSocket::~QBluetoothAbstractSocket()
{
    if (m_data->m_state != QBluetoothAbstractSocket::UnconnectedState)
        abort();

    if (m_data)
        delete m_data;
}

/*!
    Returns the socket descriptor for the socket if the socket is currently
    active (e.g. not in UnconnectedState).  Otherwise returns -1.
 */
int QBluetoothAbstractSocket::socketDescriptor() const
{
    return m_data->m_fd;
}

/*!
    Initializes the QBluetoothAbstractSocket with the native descriptor
    \a socketDescriptor.  The socket is put into the \a state and
    opened in \a openMode.
 */
bool QBluetoothAbstractSocket::setSocketDescriptor(int socketDescriptor,
        QBluetoothAbstractSocket::SocketState state,
        QIODevice::OpenMode openMode)
{
    m_data->resetNotifiers();
    m_data->m_fd = socketDescriptor;
    m_data->m_closeCalled = false;

    // Update the local, remote and channel
    bool ret = readSocketParameters(socketDescriptor);

    if (!ret) {
        m_data->m_fd = -1;
        return false;
    }

    // Set socket non-blocking
    int flags = fcntl(m_data->m_fd, F_GETFL, 0);
    if (!(flags & O_NONBLOCK)) {
        fcntl(m_data->m_fd, F_SETFL, flags | O_NONBLOCK);
    }

    m_data->setupNotifiers();
    setOpenMode(openMode);

    if (m_data->m_state != state) {
        m_data->m_state = state;
        emit stateChanged(m_data->m_state);
    }

    return true;
}

/*!
    Closes the socket.

    \sa abort(), disconnect()
 */
void QBluetoothAbstractSocket::close()
{
    QIODevice::close();
    if (m_data->m_state != QBluetoothAbstractSocket::UnconnectedState) {
        m_data->m_closeCalled = true;
        m_data->initiateDisconnect();
    }
}

/*!
    Returns the last error that has occurred.
 */
QBluetoothAbstractSocket::SocketError QBluetoothAbstractSocket::lastError() const
{
    return m_data->m_error;
}

/*!
    Returns the state of the socket.
 */
QBluetoothAbstractSocket::SocketState QBluetoothAbstractSocket::state() const
{
    return m_data->m_state;
}

/*!
    Returns the number of bytes that are waiting to be read.

    \sa bytesToWrite(), read()
 */
qint64 QBluetoothAbstractSocket::bytesAvailable() const
{
    qint64 available = QIODevice::bytesAvailable();

    available += qint64(m_data->m_readBuffer.size());

    return available;
}

/*!
    Returns the number of bytes which are pending to be written.

    \sa bytesAvailable(), write()
 */
qint64 QBluetoothAbstractSocket::bytesToWrite() const
{
    return qint64(m_data->m_writeBuffer.size());
}

/*!
    Returns the size of the internal read buffer.  This limits the
    amount of data that the client can receive before you call read()
    or readAll().

    A read buffer size of 0 (the default) means that the buffer has no
    size limit, ensuring that no data is lost.

    \sa setReadBufferSize(), read()
 */
qint64 QBluetoothAbstractSocket::readBufferSize() const
{
    return m_data->m_readBufferCapacity;
}

/*!
    Sets the size of QBluetoothAbstractSocket's internal read buffer to be
    \a size bytes.  A size of 0 means that the buffer is infinite.  This is
    the default.
 */
void QBluetoothAbstractSocket::setReadBufferSize(qint64 size)
{
    Q_ASSERT(size > 0);
    m_data->m_readBufferCapacity = size;
}

/*!
    Returns true if a line of data can be read from the socket;
    otherwise returns false.

    \sa readLine()
 */
bool QBluetoothAbstractSocket::canReadLine() const
{
    bool hasLine = m_data->m_readBuffer.canReadLine();

    return hasLine || QIODevice::canReadLine();
}

/*!
    \reimp
 */
bool QBluetoothAbstractSocket::isSequential() const
{
    return true;
}

/*!
    \reimp
 */
bool QBluetoothAbstractSocket::waitForReadyRead(int msecs)
{
    if (m_data->m_state == QBluetoothAbstractSocket::UnconnectedState)
        return false;

    QTime stopWatch;
    stopWatch.start();

    if (m_data->m_state == QBluetoothAbstractSocket::ConnectingState) {
        if (!waitForConnected(msecs))
            return false;
    }

    while (true) {
        if (m_data->m_state != QBluetoothAbstractSocket::ConnectedState)
            return false;

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

        int rv = ::select(m_data->m_fd + 1, &fdread, 0, 0, tvptr);

        switch (rv) {
            case 0:
                // Timeout
                return false;
            case 1:
                // Read notification, depends on
                // whether we actually read anything
                if (m_data->readActivated())
                    return true;
                break;
            default:
                // Interrupt, retry
                if (rv == EINTR)
                    continue;
                // Otherwise, close the socket
                m_data->m_error = QBluetoothAbstractSocket::UnknownError;
                emit error(m_data->m_error);
                close();
                return false;
        }
    }

    return false;
}

/*!
    \reimp
 */
bool QBluetoothAbstractSocket::waitForBytesWritten(int msecs)
{
    if (m_data->m_state == QBluetoothAbstractSocket::UnconnectedState)
        return false;

    QTime stopWatch;
    stopWatch.start();

    if (m_data->m_state == QBluetoothAbstractSocket::ConnectingState) {
        if (!waitForConnected(msecs))
            return false;
    }

    while (true) {
        if (m_data->m_state != QBluetoothAbstractSocket::ConnectedState) {
            qWarning("QBluetoothAbstractSocket::waitForBytesWritten - Socket no longer in connected state...");
            return false;
        }

        fd_set fdwrite;
        FD_ZERO(&fdwrite);
        FD_SET(m_data->m_fd, &fdwrite);

        int timeout = msecs < 0 ? 0 : msecs - stopWatch.elapsed();
        struct timeval tv;
        struct timeval *tvptr;

        if (msecs != -1) {
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = (timeout % 1000) * 1000;
            tvptr = &tv;
        }

        int rv = ::select(m_data->m_fd + 1, 0, &fdwrite, 0, &tv);

        switch (rv) {
            case 0:
                return false;
            case 1:
                m_data->writeActivated();
                return true;
            default:
                if (rv == EINTR)
                    continue;
                // Otherwise, close the socket
                m_data->m_error = QBluetoothAbstractSocket::UnknownError;
                emit error(m_data->m_error);
                close();
                return false;
        }
    }

    return false;
}

/*!
    \reimp
 */
qint64 QBluetoothAbstractSocket::readData(char *data, qint64 maxsize)
{
    if (m_data->m_readBuffer.isEmpty())
        return qint64(0);

    if (!m_data->m_readNotifier->isEnabled()) {
        m_data->m_readNotifier->setEnabled(true);
    }

    if (maxsize == 1) {
        *data = m_data->m_readBuffer.getChar();
        return 1;
    }

    qint64 bytesToRead = qMin(qint64(m_data->m_readBuffer.size()), maxsize);
    qint64 readSoFar = 0;
    while (readSoFar < bytesToRead) {
        const char *ptr = m_data->m_readBuffer.readPointer();
        int bytesToReadFromThisBlock = qMin(int(bytesToRead - readSoFar),
                                            m_data->m_readBuffer.nextDataBlockSize());
        memcpy(data + readSoFar, ptr, bytesToReadFromThisBlock);
        readSoFar += bytesToReadFromThisBlock;
        m_data->m_readBuffer.free(bytesToReadFromThisBlock);
    }

    return readSoFar;
}

/*!
    \reimp
 */
qint64 QBluetoothAbstractSocket::readLineData(char *data, qint64 maxsize)
{
    return QIODevice::readLineData(data, maxsize);
}

/*!
    \reimp
 */
qint64 QBluetoothAbstractSocket::writeData(const char *data, qint64 size)
{
    if (m_data->m_state == QBluetoothAbstractSocket::UnconnectedState) {
        m_data->m_error = QBluetoothAbstractSocket::UnknownError;
        setErrorString(tr("Socket is not connected"));
        return -1;
    }

    char *ptr = m_data->m_writeBuffer.reserve(size);
    if (size == 1)
        *ptr = *data;
    else
        memcpy(ptr, data, size);

    if (!m_data->m_writeBuffer.isEmpty()) {
        m_data->m_writeNotifier->setEnabled(true);
    }

    return size;
}

/*!
    Waits until the socket is connected, up to \a msecs milliseconds.  If
    the connection has been established, this function returns true; otherwise
    returns false.  In the case where it returns false, you can call lastError()
    to determine the cause of the error.

    \sa connect(), connected()
 */
bool QBluetoothAbstractSocket::waitForConnected(int msecs)
{
    if (m_data->m_state == QBluetoothAbstractSocket::UnconnectedState)
        return false;

    if (m_data->m_state == QBluetoothAbstractSocket::ConnectedState)
        return true;

    QTime stopWatch;
    stopWatch.start();

    while (true) {
        fd_set fdwrite;
        FD_ZERO(&fdwrite);
        FD_SET(m_data->m_fd, &fdwrite);

        int timeout = msecs < 0 ? 0 : msecs - stopWatch.elapsed();
        struct timeval tv;
        struct timeval *tvptr;

        if (msecs != -1) {
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = (timeout % 1000) * 1000;
            tvptr = &tv;
        }

        int rv = ::select(m_data->m_fd + 1, 0, &fdwrite, 0, &tv);

        switch (rv) {
            case 0:
                return false;
            case 1:
                m_data->testConnected();
                return m_data->m_state == QBluetoothAbstractSocket::ConnectedState;
            default:
                if (rv == EINTR)
                    continue;
                // Otherwise, close the socket
                m_data->m_error = QBluetoothAbstractSocket::UnknownError;
                emit error(m_data->m_error);
                close();
                return false;
        }
    }
}

/*!
    Waits until the socket is disconnected, up to \a msecs milliseconds.  If
    the connection has been terminated, this function returns true; otherwise
    returns false.  In the case where it returns false, you can call lastError()
    to determine the cause of the error.

    \sa disconnect(), close(), disconnected()
 */
bool QBluetoothAbstractSocket::waitForDisconnected(int msecs)
{
    if (m_data->m_state == QBluetoothAbstractSocket::UnconnectedState)
        return false;

    QTime stopWatch;
    stopWatch.start();

    if (m_data->m_state == QBluetoothAbstractSocket::ConnectingState) {
        if (!waitForConnected(msecs))
            return false;
    }

    while (true) {
        fd_set fdwrite;
        FD_ZERO(&fdwrite);
        FD_SET(m_data->m_fd, &fdwrite);

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

        int rv = ::select(m_data->m_fd + 1, &fdread, &fdwrite, 0, &tv);

        if (rv == 0) {
            return false;
        }
        else if (rv < 0) {
            if (rv == EINTR)
                continue;
                // Otherwise, close the socket
            m_data->m_error = QBluetoothAbstractSocket::UnknownError;
            emit error(m_data->m_error);
            close();
            return false;
        }
        else {
            if (FD_ISSET(m_data->m_fd, &fdread)) {
                m_data->readActivated();
            }

            if (FD_ISSET(m_data->m_fd, &fdwrite)) {
                m_data->writeActivated();
            }
        }

        if (m_data->m_state == QBluetoothAbstractSocket::UnconnectedState)
            return true;
    }

    return false;
}

/*!
    Aborts the current connection and resets the socket. Unlike
    disconnect(), this function immediately closes the socket, clearing
    any pending data in the write buffer.

    \sa disconnect(), close()
 */
void QBluetoothAbstractSocket::abort()
{
    if (m_data->m_state == QBluetoothAbstractSocket::UnconnectedState)
        return;

    if (m_data->m_timer) {
        m_data->m_timer->stop();
        delete m_data->m_timer;
        m_data->m_timer = 0;
    }

    m_data->m_writeBuffer.clear();
    close();
}

/*!
    Returns true if no more data is currently
    available for reading; otherwise returns false.
 */
bool QBluetoothAbstractSocket::atEnd() const
{
    return QIODevice::atEnd() && (!isOpen() || m_data->m_readBuffer.isEmpty());
}

/*!
    This function writes as much as possible from the internal write buffer to
    the underlying network socket, without blocking. If any data was written,
    this function returns true; otherwise false is returned.
 */
bool QBluetoothAbstractSocket::flush()
{
    return m_data->flush();
}

/*!
    Clients of this class can call this method to initiate connection
    procedures common to all Bluetooth socket types.  The \a socket parameter
    holds the socket file descriptor to use.  The \a addr structure holds the
    sockaddr structure of the remote peer to connect to and \a size holds the
    size of the sockaddr structure.
*/
bool QBluetoothAbstractSocket::initiateConnect(int socket,
        sockaddr *addr, int size)
{
    bool ret = m_data->initiateConnect(socket, addr, size);
    if (!ret) {
        emit error(m_data->m_error);
    }
    else {
        QPointer<QBluetoothAbstractSocket> that = this;
        emit stateChanged(m_data->m_state);
        if (that && (m_data->m_state == QBluetoothAbstractSocket::ConnectedState))
            emit connected();
    }

    return ret;
}


/*!
    Attempts to close the socket.  If there is pending data waiting to be
    written, the socket will enter ClosingState and wait until all data has
    been written.  Eventually it will enter UnconnectedState and emit
    the disconnected() signal.

    \sa close()
 */
bool QBluetoothAbstractSocket::disconnect()
{
    if (m_data->m_state == QBluetoothAbstractSocket::UnconnectedState) {
        return false;
    }

    return m_data->initiateDisconnect();
}

/*!
    Can be used by the clients of this class to set the \a error that might have occurred.
    This function is generally used from the specific socket connect implementation.
*/
void QBluetoothAbstractSocket::setError(const QBluetoothAbstractSocket::SocketError &error)
{
    m_data->m_error = error;
}

/*!
    The clients of this class can override this method to read specific socket
    parameters from the socket given by \a sockfd.  This method is called
    when the socket initially enters the connected state.

    If the parameters could not be obtained, the clients should return false,
    and should return true otherwise.

    The default implementation returns true.
*/
bool QBluetoothAbstractSocket::readSocketParameters(int sockfd)
{
    Q_UNUSED(sockfd);
    return true;
}

/*!
    The clients of this class should override this method to reset the
    specific socket parameters.  This method is called when a socket enters
    the disconnected state.
*/
void QBluetoothAbstractSocket::resetSocketParameters()
{

}

/*!
    \fn void QBluetoothAbstractSocket::connected()

    This signal is emitted once the connect() has been called
    and the rfcomm socket has been successfully connected.

    \sa connect(), disconnected()
 */

/*!
    \fn void QBluetoothAbstractSocket::disconnected()

    This signal is emitted when the socket has been disconnected.

    \sa connect(), disconnect(), abort()
 */

/*!
    \fn void QBluetoothAbstractSocket::error(QBluetoothAbstractSocket::SocketError error)

    This signal is emitted after an error occurred. the \a error parameter
    describes the type of error that has occurred.

    \sa lastError()
 */

/*!
    \fn void QBluetoothAbstractSocket::stateChanged(QBluetoothAbstractSocket::SocketState socketState)

    This signal is emitted when the state of the socket has changed.  The
    \a socketState parameter holds the new state.

    \sa state()
 */

#include "qbluetoothabstractsocket.moc"
