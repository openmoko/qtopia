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

#include <qtsslsocket.h>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtNetwork/QHostAddress>
#include <qtopianamespace.h>

// OpenSSL includes
#include <ssllibrary.h>

//#endif

const int SslReadBlockSize = 4096;

#define SSL_ERRORSTR() lib->ERR_error_string(lib->ERR_get_error(), NULL)

class QtRingBuffer
{
public:
    QtRingBuffer(int growth = 4096);

    int nextDataBlockSize() const;
    char *readPointer() const;
    void free(int bytes);
    char *reserve(int bytes);
    void truncate(int bytes);

    bool isEmpty() const;

    int getChar();
    void putChar(char c);
    void ungetChar(char c);

    int size() const;
    void clear();
    int indexOf(char c) const;
    int readLine(char *data, int maxLength);
    bool canReadLine() const;

private:
    QList<QByteArray> buffers;
    int head, tail;
    int tailBuffer;
    int basicBlockSize;
    int bufferSize;
};

/*! \internal

    Creates an empty ring buffer. The buffer will grow in steps of \a
    growth as data is written to it.
*/
QtRingBuffer::QtRingBuffer(int growth)
    : basicBlockSize(growth)
{
    buffers << QByteArray();
    clear();
}

/*! \internal

    Returns the number of bytes that can be read in one operation. The
    data is read from readPointer().
*/
int QtRingBuffer::nextDataBlockSize() const
{
    return (tailBuffer == 0 ? tail : buffers.at(0).size()) - head;
}

/*! \internal

    Returns a pointer to where no more than nextDataBlockSize() bytes
    of data can be read. Call free() to remove data after reading.
*/
char *QtRingBuffer::readPointer() const
{
    if (buffers.count() == 0)
        return 0;
    return const_cast<char *>(buffers[0].data()) + head;
}

/*! \internal

    Removes \a bytes bytes from the front of the buffer. If \a bytes
    is larger than the size of the buffer, the buffer is cleared.
*/
void QtRingBuffer::free(int bytes)
{
    bufferSize -= bytes;
    if (bufferSize < 0)
        bufferSize = 0;

    for (;;) {
        int nextBlockSize = nextDataBlockSize();
        if (bytes < nextBlockSize) {
            head += bytes;
            if (head == tail && tailBuffer == 0)
                head = tail = 0;
            return;
        }

        bytes -= nextBlockSize;
        if (buffers.count() == 1) {
            if (buffers.at(0).size() != basicBlockSize)
                buffers[0].resize(basicBlockSize);
            head = tail = 0;
            tailBuffer = 0;
            return;
        }

        buffers.removeAt(0);
        --tailBuffer;
        head = 0;
    }
}

/*! \internal

    Reserves space in the buffer for \a bytes new bytes, and returns a
    pointer to the first byte.
*/
char *QtRingBuffer::reserve(int bytes)
{
    bufferSize += bytes;

    // if there is already enough space, simply return.
    if (tail + bytes <= buffers.at(tailBuffer).size()) {
        char *writePtr = buffers[tailBuffer].data() + tail;
        tail += bytes;
        return writePtr;
    }

    // if our buffer isn't half full yet, simply resize it.
    if (tail < buffers.at(tailBuffer).size() / 2) {
        buffers[tailBuffer].resize(tail + bytes);
        char *writePtr = buffers[tailBuffer].data() + tail;
        tail += bytes;
        return writePtr;
    }

    // shrink this buffer to its current size
    buffers[tailBuffer].resize(tail);

    // create a new QByteArray with the right size
    buffers << QByteArray();
    ++tailBuffer;
    buffers[tailBuffer].resize(qMax(basicBlockSize, bytes));
    tail = bytes;
    return buffers[tailBuffer].data();
}

/*! \internal

    Removes \a bytes bytes from the end of the buffer. If \a bytes is
    larger than the buffer size, the buffer is cleared.
*/
void QtRingBuffer::truncate(int bytes)
{
    bufferSize -= bytes;
    if (bufferSize < 0)
        bufferSize = 0;

    for (;;) {
        // special case: head and tail are in the same buffer
        if (tailBuffer == 0) {
            tail -= bytes;
            if (tail <= head)
                tail = head = 0;
            return;
        }

        if (bytes <= tail) {
            tail -= bytes;
            return;
        }

        bytes -= tail;
        buffers.removeAt(tailBuffer);

        --tailBuffer;
        tail = buffers.at(tailBuffer).size();
    }
}

/*! \internal

    Returns and removes the first character in the buffer. Returns -1
    if the buffer is empty.
*/
int QtRingBuffer::getChar()
{
    if (isEmpty())
       return -1;
    char c = *readPointer();
    free(1);
    return c;
}

/*! \internal

    Appends the character \a c to the end of the buffer.
*/
void QtRingBuffer::putChar(char c)
{
    char *ptr = reserve(1);
    *ptr = c;
}

/*! \internal

    Prepends the character \a c to the front of the buffer.
*/
void QtRingBuffer::ungetChar(char c)
{
    --head;
    if (head < 0) {
        buffers.prepend(QByteArray());
        buffers[0].resize(basicBlockSize);
        head = basicBlockSize - 1;
        ++tailBuffer;
    }
    buffers[0][head] = c;
    ++bufferSize;
}

/*! \internal

    Returns the size of the buffer; e.g. the number of bytes
    currently in use.
*/
int QtRingBuffer::size() const
{
    return bufferSize;
}

/*! \internal

    Removes all data from the buffer and resets its size to 0.
*/
void QtRingBuffer::clear()
{
    QByteArray tmp = buffers[0];
    buffers.clear();
    buffers << tmp;

    if (buffers.at(0).size() != basicBlockSize)
        buffers[0].resize(basicBlockSize);

    head = tail = 0;
    tailBuffer = 0;
    bufferSize = 0;
}

/*! \internal

    Returns true if the buffer is empty; otherwise returns false.
*/
bool QtRingBuffer::isEmpty() const
{
    return tailBuffer == 0 && tail == 0;
}

/*! \internal

    Returns the index of the first occurrence of the character \a c in
    the buffer. In no such character is found, -1 is returned.
*/
int QtRingBuffer::indexOf(char c) const
{
    int index = 0;
    for (int i = 0; i < buffers.size(); ++i) {
        int start = 0;
        int end = buffers.at(i).size();

        if (i == 0)
            start = head;
        if (i == tailBuffer)
            end = tail;
        const char *ptr = buffers.at(i).data() + start;
        for (int j = start; j < end; ++j) {
            if (*ptr++ == c)
                return index;
            ++index;
        }
    }

    return -1;
}

/*! \internal

    Reads one line of data (all data up to and including the '\\n'
    character), no longer than \a maxSize - 1 bytes, and stores it in \a
    data. If the line is too long, maxSize bytes of the line are read.
    \a data is always terminated by a '\\0' byte.
*/
int QtRingBuffer::readLine(char *data, int maxSize)
{
    int index = indexOf('\n');
    if (index == -1 || maxSize <= 0)
        return -1;

    int readSoFar = 0;
    while (readSoFar < index && readSoFar < maxSize - 1) {
        int bytesToRead = qMin((index + 1) - readSoFar, nextDataBlockSize());
        bytesToRead = qMin(bytesToRead, (maxSize - 1) - readSoFar);
        memcpy(data + readSoFar, readPointer(), bytesToRead);
        readSoFar += bytesToRead;
        free(bytesToRead);
    }

    // Terminate it.
    data[readSoFar] = '\0';
    return readSoFar;
}

/*! \internal

    Returns true if a line can be read from the buffer; otherwise
    returns false.
*/
bool QtRingBuffer::canReadLine() const
{
    return indexOf('\n') != -1;
}

class QtSslSocketPrivate
{
public:
    QtSslSocketPrivate(QtSslSocket *qq);
    void initializeConnection(QTcpSocket *socket);

    QtRingBuffer readBuffer;
    QtRingBuffer writeBuffer;
    bool connectionSecured;
    QTcpSocket *socket;
    QtSslSocket::Mode mode;
    bool initialized;

    bool calledWriteToSocket;
    bool readyReadEmitted;

    // SSL context
    SSL *ssl;
    SSL_CTX *ctx;
    BIO *rbio;
    BIO *wbio;

    QString cert;
    QString certDir;
    QString key;
    QString cafile;
    QString cadir;
    QString ciph;

    QtSslSocket *q;
};

QtSslSocketPrivate::QtSslSocketPrivate(QtSslSocket *qq)
    : connectionSecured(false), socket(0), mode(QtSslSocket::Client),
      initialized(false), calledWriteToSocket(false),
      readyReadEmitted(false), ssl(0), ctx(0), rbio(0), wbio(0),
      q(qq)
{
}

void QtSslSocketPrivate::initializeConnection(QTcpSocket *socket)
{
    socket->disconnect();
    QObject::connect(socket, SIGNAL(connected()),
                     q, SLOT(sslConnect()));
    QObject::connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                     q, SLOT(handleSocketError(QAbstractSocket::SocketError)));
    QObject::connect(socket, SIGNAL(disconnected()),
                     q, SIGNAL(disconnected()));
    QObject::connect(socket, SIGNAL(readyRead()),
                     q, SLOT(readFromSocket()));
    QObject::connect(socket, SIGNAL(bytesWritten(qint64)),
                     q, SLOT(socketBytesWritten(qint64)));
}

/*!
    \class QtSslSocket
    \brief The QtSslSocket class provides a TCP socket with SSL
    encryption support.

    It provides some extra functionality which is needed for working
    with SSL, but otherwise there is no difference from
    QTcpSocket. Example:

    \code
    void MyClient::connectToHost(const QString &host, int port)
    {
        // Create a new SSL socket
        socket = new QtSslSocket();

        // Set the path to the CA certificates
        socket->setPathToCACertDir("/etc/ssl/certs/");

        // Connect the socket's signals to slots in our client
        // implementation.
        connect(socket, SIGNAL(connected()),
                this, SLOT(connectedToHost()));
        connect(socket, SIGNAL(readyRead()),
                this, SLOT(readData()));
        connect(socket, SIGNAL(disconnected()),
                this, SLOT(connectionClosed()));
        connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(inspectError(QAbstractSocket::SocketError)));
        connect(socket, SIGNAL(connectionVerificationDone(QtSslSocket::VerifyResult, bool, const QString &)),
                this, SLOT(checkCertError(QtSslSocket::VerifyResult, bool, const QString &)));

        // Initiate a connection.
        socket->connectToHost(host, port);
    }
    \endcode

    Use connectToHost() when connecting to an SSL server. QtSslSocket will
    eventually emit the connected() signal if the connection was
    established, or the error() signal if the connection failed.

    In the case of accepting incoming connections, one common approach is to
    create a subclass of QTcpServer and reimplement
    QTcpServer::incomingConnection(). In this function, create a QtSslSocket
    object and call setSocketDescriptor() passing the provided
    socketDescriptor. Connect QtSslSocket's signals to slots in the server
    class. The path to a file containing a PEM encoded certificate is set with
    setPathToCertificate(). The path to the private key file is set with
    setPathToPrivateKey(). Finally, call sslAccept() to establish the secure
    connection. When QtSslSocket emits \l connectionVerificationDone(), the
    SSL encrypted link is established. Example:

    \code
    void MyServer::incomingConnection(int socketDescriptor)
    {
        // Create a new socket and pass it a new QSocket
        socket = new QtSslSocket(QtSslSocket::Server, this);
        socket->setSocketDescriptor(socketDescriptor);

        // Set up the certificate and private key.
        socket->setPathToCertificate("/etc/ssl/servercert.pem");
        socket->setPathToPrivateKey("/etc/ssl/serverkey.pem");

        // Connect the socket's signals to slots in our server
        // implementation
        connect(socket, SIGNAL(disconnected()),
                this, SLOT(connectionClosed()));
        connect(socket, SIGNAL(readyRead()),
                this, SLOT(readData()));
        connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(error(QAbstractSocket::SocketError)));

        // Initiate SSL handshake. Eventually, QtSslSocket will emit
        // connectionVerificationDone.
        socket->sslAccept();
    }
    \endcode

    In order to convert an existing socket into an SSL-encrypted socket (i.e.,
    using the "STARTTLS" command or something similar), you can create a new
    QtSslSocket and then pass your existing socket to setSocket(). The
    original socket is assumed to be in ConnectedState.

    Most often clients, but sometimes also servers need a set of CA
    (Certificate Authority) certificates to verify the identity of their
    peer. Use setPathToCACertFile() and setPathToCACertDir() to point
    QtSslSocket to these certificates. CA certificates are usually bundled as
    part of any Unix distribution or Mac OS X as part of the OpenSSL
    package. On Windows, you can either use the bundle provided with OpenSSL,
    or export CA certificates from your web browser.

    In rare occasions, it also is useful to
    restrict the encryption ciphers available to the client or server with
    setCiphers(). This is mostly useful when debugging faulty connections.

    As mentioned, QtSslSocket emits connected() when the connection has been
    established and connectionVerificationDone() when the SSL handshake has
    been completed.  Make sure to connect to error() and
    connectionVerificationDone() to catch errors. The readyRead() signal is
    emitted when there is new incoming data. Call read() to read the new data
    into a buffer. When disconnected() is emitted, the other peer has closed
    the connection.

    If the ceritificate check fails, it is often useful to display
    details about the failing certificate to the user.
    peerCertificate() returns information about this certificate. To
    get the local certificate, call localCertificate().

    When building an application that uses QtSslSocket, you must link
    your application against the SSL library. Include the bundled .pri
    file, or add the following line to your .pro file, modified with
    the path to your installation of OpenSSL:

    \code
    unix:LIBS += -L/usr/lib -lssl -lcrypto
    win32:INCLUDEPATH += /openssl/include
    win32:LIBS += -L/openssl/lib ssleay32.lib libeay32.lib
    \endcode

    The pri file must be edited to set the paths of the SSL files.

    \i {Note: QtSslSocket requires OpenSSL. The
    OpenSSL toolkit is licensed under an Apache-style licence, which
    basically means that you are free to get and use it for commercial
    and non-commercial purposes subject to certain license
    conditions. The official web site is:} \l http://www.openssl.org/ \i {.
    Windows binaries are available from:} \l http://www.openssl.org/related/binaries.html \i {.}

    \i {Note: Since Qt 4.1, you can pass a QtSslSocket to QHttp by calling
    QHttp::setSocket() to achieve an SSL encrypted HTTP transport. (SSL over
    HTTP uses port 443.)}

    \sa QTcpSocket, QTcpServer
*/

/*!
    \enum QtSslSocket::Mode

    Describes whether QtSslSocket is used on the client side or the server
    side. Pass a suitable value to QtSslSocket's constructor to ensure that
    the SSL handshake is performed correctly.

    \value Client QtSslSocket is used for a client side socket (i.e., you call
    connectToHost() to connect to a remote SSL server.

    \value Server QtSslSocket is used to handle a server side connection
    (i.e., someone connected to your server).
*/

/*!
    \fn void QtSslSocket::connectionVerificationDone(QtSslSocket::VerifyResult result, bool hostNameMatched, const QString &description);

    This signal is emitted by QtSslSocket to notify that the SSL handshake has
    completed, and the connecion is ready for use. The result of the handshake
    is available through \a result, and in addition, \a hostNameMatched will
    be set to true if the hostname of the peer certificate matches the
    hostname passed to connectToHost(), or false if it doesn't. (For
    server-side connections, hostNameMatched is always true.) For displaying
    to the user, \a description contains a human readable description of the
    result of the certificate verification.
*/

/*!
    \enum QtSslSocket::VerifyResult

    Describes the result of the SSL handshake.

    \value VerifyOk The certificate has been successfully verified, and the
    peer authenticity has been established. The connection is secure, and
    ready to use.

    \value SelfSigned The peer certificate is self-signed (i.e., does not use
    a certificate authority for verification). This means that the identity of
    the peer has not been established. Self-signed peer certificates provide
    no security beyond transport encryption, and should only be used on closed
    networks or for debugging.

    \value Expired The peer certificate has expired. This means that the
    associated certificate authority no longer guarantees the identity of the
    peer. An expired certificate provides no security beyond transport
    encryption, and the administrators of the remote server should be
    notified as soon as possible.

    \value NotYetValid The peer certificate is not valid yet (i.e., the
    "opposite" of Expired). It provides no security beyond transport
    encryption, and the administrators of the remote server should be notified
    as soon as possible.

    \value UnableToVerify This value signifies an error that was not covered
    by the other (non-VerifyOK) VerifyResult values. A common cause for the
    UnableToVerify value is that there is a technical fault on either side of
    the connection that prevents the handshake from completing, such as the
    server running out of diskspace, or the client running out of memory.
*/

/*!
    Constructs a QtSslSocket running with mode \a mode. \a parent is
    passed to QTcpSocket's constructor.
*/
QtSslSocket::QtSslSocket(Mode mode, QObject *parent)
    : QTcpSocket(parent),lib(0), d(new QtSslSocketPrivate(this))
{
    d->mode = mode;
    d->socket = new QTcpSocket;
    d->initializeConnection(d->socket);

    //get the path to the SSL library

    QString libPath = Qtopia::qtopiaDir() + "lib/libssl.so";

    lib = new SSLLibrary(libPath);

    lib->SSL_library_init();
    lib->SSL_load_error_strings();
    lib->OpenSSL_add_ssl_algorithms();


    if (!lib->RAND_status()) {
        struct {
            int msec;
            int sec;
            void *stack;
        } randomish;

        randomish.stack = (void *)&randomish;
        randomish.msec = QTime::currentTime().msec();
        randomish.sec = QTime::currentTime().second();
        lib->RAND_seed(&randomish, sizeof(randomish));
    }
}

/*!
    Destroys the QtSslSocket object.
*/
QtSslSocket::~QtSslSocket()
{
    delete d;
    delete lib;
}

/*!
    Initiates a connection to \a hostName on port \a port, using the open mode
    \a openMode. QtSslSocket will emit connected() when the connection has
    been established, and connectionVerificationDone() when the SSL handshake
    has completed and the socket is ready to use.

    Note: The implementation of this function invokes the
    slot connectToHostImplementation(). To alter its behavior, you should
    reimplement connectToHostImplementation() instead.
*/
void QtSslSocket::connectToHost(const QString &hostName, quint16 port, OpenMode openMode)
{
    QMetaObject::invokeMethod(this, "connectToHostImplementation",
                              Q_ARG(QString, hostName),
                              Q_ARG(quint16, port),
                              Q_ARG(OpenMode, openMode));
}

/*!
    Initiates a disconnect from the remote host.

    Note: The implementation of this function invokes the
    disconnectFromHostImplementation() slot.
*/
void QtSslSocket::disconnectFromHost()
{
    QMetaObject::invokeMethod(this, "disconnectFromHostImplementation");
}

/*!
    Sets the path to a local file containing the client's certificate to \a
    path.  This is useful for server side connections, or for clients that
    connect to servers that need to verify the peer identity (client
    verification is not yet supported by QtSslSocket).

    \sa pathToCertificate()
*/
void QtSslSocket::setPathToCertificate(const QString &path)
{
    d->cert = path;
}

/*!
    Returns the path to the client's certificate.

    \sa setPathToCertificate()
*/
QString QtSslSocket::pathToCertificate() const
{
    return d->cert;
}

/*!
    Sets the path the a local file containing the client's private key to \a
    path. This is useful for server side connections, or for clients that
    connect to servers that need to verify the peer identity (client
    verification is not yet supported by QtSslSocket).

    \sa pathToPrivateKey()
*/
void QtSslSocket::setPathToPrivateKey(const QString &path)
{
    d->key = path;
}

/*!
    Returns the path to the client's private key.

    \sa setPathToPrivateKey()
*/
QString QtSslSocket::pathToPrivateKey() const
{
    return d->key;
}

/*!
    Sets the path to a local file containing a bundle of CA certificates to \a
    path. You can either use the bundle that comes with OpenSSL, or use a
    bundle exported by your web browser. This CA bundle is necessary for
    verifying the authenticity of the remote peer. Without such a bundle,
    QtSslSocket can never verify the peer's identity, but it will still
    encrypt the transferred data.

    \sa pathToCACertFile()
*/
void QtSslSocket::setPathToCACertFile(const QString &path)
{
    d->cafile = path;
}

/*!
    Returns the path to a local file containing a bundle of CA certificates.

    \sa setPathToCACertFile()
 */
QString QtSslSocket::pathToCACertFile() const
{
    return d->cafile;
}

/*!
    Sets the path to a local directory containing files with CA certificates
    to \a path.  This directory is generated by OpenSSL using the program
    "c_rehash", and is often installed by default as part of installing
    OpenSSL. It's common to use a CA directory containing several CA files
    instead of using only a single CA file, as you can then add more CA files
    without interfering with ones provided by the operating system, your web
    browser or by OpenSSL.

    \sa pathToCACertDir(), setPathToCACertFile()
*/
void QtSslSocket::setPathToCACertDir(const QString &path)
{
    d->cadir = path;
}

/*!
    Returns the path to a locale directory containing files with CA certificates.

   \sa setPathToCACertDir()
*/
QString QtSslSocket::pathToCACertDir() const
{
    return d->cadir;
}

/*!
    Sets the list of ciphers QtSslSocket will advertise to the remote peer
    during the SSL handshake. This effectively limits the ciphers QtSslSocket
    can use, and is therefore mostly useful for debugging the SSL handshake.
    \a ciphers contains a list of ciphers listed by name, separated by a ':',
    and listed in descending order from the most secure to the least secure
    cipher.

    The default behavior for QtSslSocket is to advertise all available ciphers
    to the remote peer, thereby maximizing the chance that a secure connection
    can be established.

    \sa ciphers()
*/
void QtSslSocket::setCiphers(const QString &ciphers)
{
    d->ciph = ciphers;
}

/*!
    Returns the list of ciphers QtSslSocket will advertise to the remote peer
    during the SSL handshake.

    \sa setCiphers()
*/
QString QtSslSocket::ciphers() const
{
    return d->ciph;
}

//==============================================================================
// contributed by Jay Case of Sarvega, Inc.; http://sarvega.com/
// Based on X509_cmp_time() for intitial buffer hacking.
//==============================================================================
static time_t getTimeFromASN1(const ASN1_TIME *aTime)
{
    time_t lResult = 0;

    char lBuffer[24];
    char *pBuffer = lBuffer;

    size_t lTimeLength = aTime->length;
    char *pString = (char *) aTime->data;

    if (aTime->type == V_ASN1_UTCTIME) {
        if ((lTimeLength < 11) || (lTimeLength > 17))
            return 0;

        memcpy(pBuffer, pString, 10);
        pBuffer += 10;
        pString += 10;
    } else {
        if (lTimeLength < 13)
            return 0;

        memcpy(pBuffer, pString, 12);
        pBuffer += 12;
        pString += 12;
    }

    if ((*pString == 'Z') || (*pString == '-') || (*pString == '+')) {
        *pBuffer++ = '0';
        *pBuffer++ = '0';
    } else {
        *pBuffer++ = *pString++;
        *pBuffer++ = *pString++;
        // Skip any fractional seconds...
        if (*pString == '.') {
            pString++;
            while ((*pString >= '0') && (*pString <= '9'))
                pString++;
        }
    }

    *pBuffer++ = 'Z';
    *pBuffer++ = '\0';

    time_t lSecondsFromUCT;
    if (*pString == 'Z') {
        lSecondsFromUCT = 0;
    } else {
        if ((*pString != '+') && (pString[5] != '-'))
            return 0;

        lSecondsFromUCT = ((pString[1] - '0') * 10 + (pString[2] - '0')) * 60;
        lSecondsFromUCT += (pString[3] - '0') * 10 + (pString[4] - '0');
        if (*pString == '-')
            lSecondsFromUCT = -lSecondsFromUCT;
    }

    tm lTime;
    lTime.tm_sec = ((lBuffer[10] - '0') * 10) + (lBuffer[11] - '0');
    lTime.tm_min = ((lBuffer[8] - '0') * 10) + (lBuffer[9] - '0');
    lTime.tm_hour = ((lBuffer[6] - '0') * 10) + (lBuffer[7] - '0');
    lTime.tm_mday = ((lBuffer[4] - '0') * 10) + (lBuffer[5] - '0');
    lTime.tm_mon = (((lBuffer[2] - '0') * 10) + (lBuffer[3] - '0')) - 1;
    lTime.tm_year = ((lBuffer[0] - '0') * 10) + (lBuffer[1] - '0');
    if (lTime.tm_year < 50)
        lTime.tm_year += 100; // RFC 2459
    lTime.tm_wday = 0;
    lTime.tm_yday = 0;
    lTime.tm_isdst = 0;  // No DST adjustment requested

    lResult = mktime(&lTime);
    if ((time_t)-1 != lResult) {
        if (0 != lTime.tm_isdst)
            lResult -= 3600;  // mktime may adjust for DST  (OS dependent)
        lResult += lSecondsFromUCT;
    } else {
        lResult = 0;
    }

    return lResult;
}

/*!
    Returns the local SSL certificate as a list of keys and values, separated
    by '/'. Call this function after the connection has been established and
    connectionVerificationDone() has been emitted to show the user the details
    of the certificate. This function is usually only called in the case where
    the SSL verification failed. If you call this function on an unconnected
    socket, or before QtSslSocket emits connectionVerificationDone(), or
    without providing QtSslSocket with a local certificate (see
    setPathToCertificate()), it will return an empty string.

    \sa peerCertificate()
*/
QString QtSslSocket::localCertificate() const
{
    // Get the local certificate. Usually this is only available in
    // server mode, but clients can in some cases also have
    // certificates.
    X509 *cert = d->ssl ? lib->SSL_get_certificate(d->ssl) : 0;
    if (!cert)
        return "";

    char *inamestr = lib->X509_NAME_oneline(lib->X509_get_issuer_name(cert), 0, 0);
    QString inamelist = inamestr;
    lib->OPENSSL_free(inamestr);

    char *snamestr = lib->X509_NAME_oneline(lib->X509_get_subject_name(cert), 0, 0);
    QString snamelist = snamestr;
    lib->OPENSSL_free(snamestr);

    ASN1_TIME *nbef = X509_get_notBefore(cert);
    ASN1_TIME *naft = X509_get_notAfter(cert);
    QDateTime bef; bef.setTime_t(getTimeFromASN1(nbef));
    QDateTime aft; aft.setTime_t(getTimeFromASN1(naft));

    lib->X509_free(cert);

    return QString("%1/%2/notValidBefore=%3/notValidAfter=%4/version=%5/serial=%6")
        .arg(inamelist).arg(snamelist).arg(bef.toString(Qt::ISODate))
        .arg(aft.toString(Qt::ISODate))
            .arg(lib->X509_get_version(cert))
            .arg(lib->ASN1_INTEGER_get(lib->X509_get_serialNumber(cert)));
}

/*!
    Returns the peer SSL certificate as a list of keys and values, separated
    by '/'. Call this function after the connection has been established and
    connectionVerificationDone() has been emitted to show the user the details
    of the peer certificate. This function is usually only called in the case
    where the SSL verification failed. If you call this function on an
    unconnected socket, or before QtSslSocket emits
    connectionVerificationDone(), it will return an empty string.

    \sa localCertificate()
*/
QString QtSslSocket::peerCertificate() const
{
    // Get the local certificate. Usually this is only available in
    // server mode, but clients can in some cases also have
    // certificates.
    X509 *cert = d->ssl ? lib->SSL_get_peer_certificate(d->ssl) : 0;
    if (!cert)
        return "";

    char *inamestr = lib->X509_NAME_oneline(lib->X509_get_issuer_name(cert), 0, 0);
    QString inamelist = inamestr;
    lib->OPENSSL_free(inamestr);

    char *snamestr = lib->X509_NAME_oneline(lib->X509_get_subject_name(cert), 0, 0);
    QString snamelist = snamestr;
    lib->OPENSSL_free(snamestr);

    ASN1_TIME *nbef = X509_get_notBefore(cert);
    ASN1_TIME *naft = X509_get_notAfter(cert);
    QDateTime bef; bef.setTime_t(getTimeFromASN1(nbef));
    QDateTime aft; aft.setTime_t(getTimeFromASN1(naft));

    lib->X509_free(cert);

    return QString("%1/%2/notValidBefore=%3/notValidAfter=%4/version=%5/serial=%6")
        .arg(inamelist).arg(snamelist).arg(bef.toString(Qt::ISODate))
        .arg(aft.toString(Qt::ISODate))
            .arg(lib->X509_get_version(cert))
            .arg(lib->ASN1_INTEGER_get(lib->X509_get_serialNumber(cert)));
}

/*!
    \reimp
*/
qint64 QtSslSocket::bytesAvailable() const
{
    return d->readBuffer.size();
}

/*!
    \reimp
*/
qint64 QtSslSocket::bytesToWrite() const
{
    return d->writeBuffer.size();
}

/*!
    \reimp
*/
bool QtSslSocket::canReadLine() const
{
    return d->readBuffer.canReadLine();
}

/*!
    \reimp
*/
void QtSslSocket::close()
{
    d->socket->abort();
    QTcpSocket::close();
}

/*!
    \reimp

    Returns true; QtSslSocket is a sequential device.
*/
bool QtSslSocket::isSequential() const
{
    return true;
}

/*!
    \reimp
*/
bool QtSslSocket::waitForReadyRead(int msecs)
{
    QTime stopWatch;
    stopWatch.start();
    d->readyReadEmitted = false;
    do {
        if (!d->writeBuffer.isEmpty())
            writeToSocket();
        if (!d->socket->waitForReadyRead(qMax(0, msecs - stopWatch.elapsed())))
            return false;
    } while (!d->readyReadEmitted);
    return true;
}

/*!
    \reimp
*/
bool QtSslSocket::waitForBytesWritten(int msecs)
{
    if (!d->writeBuffer.isEmpty())
        writeToSocket();
    return d->socket->waitForBytesWritten(msecs);
}

/*!
    Returns a pointer to the socket holding the actualy SSL connection.  This
    is different from QtSslSocket, which only acts like a wrapper. Call
    this function if you need to access the actual connected socket.
*/
QTcpSocket *QtSslSocket::socket() const
{
    return d->socket;
}

/*!
    Sets or replaces QtSslSocket's internal socket with \a socket. \a socket
    must be in ConnectedState; otherwise this function does nothing. After
    calling setSocket(), you can call sslConnect() to initiate a client-side
    handshake, or sslAccept() to initiate a server-side handshake.

    QtSslSocket disconnects all \a socket's existing signal connections as
    part of the handover, but does not take ownership of the object.

    This functionality is useful if you need to enable SSL on an existing,
    unencrypted socket. For example, certain network protocols like IMAP4,
    POP3 and SMTP allow you to use a plain text connection initially, and then
    convert to an encrypted connection later by issuing a certain command
    (e.g., "STARTTLS"). Example:

    \code
        void NetworkProtocol::readyReadSlot()
        {
            // Convert to an SSL connection.
            if (socket->canReadLine() && socket->readLine() == "STARTTLS\r\n") {
                QtSslSocket *sslSocket = new QtSslSocket(QtSslSocket::Client, this);
                sslSocket->setSocket(socket);
                socket = sslSocket;
                connectSignals(socket);
            }
        }
    \endcode

    \sa socket()
*/
void QtSslSocket::setSocket(QTcpSocket *socket)
{
    if (socket->state() != ConnectedState) {
        qWarning("QtSslSocket::setSocket: QtSslSocket::setSocket() was called with a "
                 "socket that is not in ConnectedState.");
        return;
    }
    socket->flush();
    if (d->socket)
        delete d->socket;
    d->socket = socket;
    d->initializeConnection(d->socket);
    setOpenMode(socket->openMode());
    initSsl();
}

/*!
    Initiates the SSL handshake for client-side sockets (Client). This
    function is useful for when QtSslSocket is initialized using
    setSocketDescriptor(). After calling sslConnect(), QtSslSocket will
    eventually emit connectionVerificationDone().

    If you connect to a peer by calling connectToHost(), you do not need to
    call this function as QtSslSocket will do so for you.

    \sa sslAccept()
*/
bool QtSslSocket::sslConnect()
{
    return sslAcceptOrConnect(false);
}

/*!
    Initiates the SSL handshake for server-side sockets (Server). This
    function is useful for when QtSslSocket is initialized using
    setSocketDescriptor(). After calling sslAccept(), QtSslSocket will
    eventually emit connectionVerificationDone().

    \sa sslConnect()
*/
bool QtSslSocket::sslAccept()
{
    open(QAbstractSocket::ReadWrite);
    return sslAcceptOrConnect(true);
}

/*!
    Initiates a connection to the host \a hostName on port \a port, using the
    open mode \a openMode. After QtSslSocket emits connected(), the SSL
    handshake will start automatically, and eventually QtSslSocket will emit
    connectionVerificationDone() to signal that the handshake has
    completed. \i {It is essential that the connection is not used before the
    handshake has completed.}

    Note: This function contains the implementation of connectToHost(). It is
    implemented as a slot to achieve polymorphic behavior (as
    QTcpSocket::connectToHost() is not virtual). Reimplement this function
    to alter the behavior of connectToHost() polymorphically.

    \sa disconnectFromHostImplementation()
*/
void QtSslSocket::connectToHostImplementation(const QString &hostName, quint16 port, OpenMode openMode)
{
    if (state() != UnconnectedState) {
        qWarning("QtSslSocket::connectToHost() called when not in UnconnectedState");
        return;
    }
    open(openMode);
    d->socket->connectToHost(hostName, port, openMode);
#if QT_VERSION >= 0x040100
    setSocketState(ConnectingState);
#endif
}

/*!
    This function contains the implementation of
    disconnectFromHost(). It is implemented as a slot to achieve polymorphic
    behavior (as QTcpSocket::disconnectFromHost() is not virtual). Reimplement
    this function to alter the behavior of disconnectFromHost()
    polymorphically.

    \sa connectToHostImplementation()
*/
void QtSslSocket::disconnectFromHostImplementation()
{
    d->socket->disconnectFromHost();
    setSocketState(d->socket->state());
    emit stateChanged(d->socket->state());
}

/*!
    \reimp
*/
qint64 QtSslSocket::readData(char *data, qint64 maxSize)
{
    if (!d->connectionSecured)
        return qint64(-1);

    qint64 bytesToRead = qMin(qint64(d->readBuffer.size()), maxSize);
    qint64 readSoFar = 0;
    while (readSoFar < bytesToRead) {
        char *ptr = d->readBuffer.readPointer();
        int bytesToReadFromThisBlock = qMin(int(bytesToRead - readSoFar),
                                            d->readBuffer.nextDataBlockSize());
        memcpy(data + readSoFar, ptr, bytesToReadFromThisBlock);
        readSoFar += bytesToReadFromThisBlock;
        d->readBuffer.free(bytesToReadFromThisBlock);
    }

    return readSoFar;
}

/*!
    \reimp
*/
qint64 QtSslSocket::readLineData(char *data, qint64 maxSize)
{
    return qint64(d->readBuffer.readLine(data, int(maxSize)));
}

/*!
    \reimp
*/
qint64 QtSslSocket::writeData(const char *data, qint64 size)
{
    if (!d->calledWriteToSocket) {
        d->calledWriteToSocket = true;
        QMetaObject::invokeMethod(this, "writeToSocket", Qt::QueuedConnection);
    }

    char *ptr = d->writeBuffer.reserve(size);
    if (size == 1)
        *ptr = *data;
    else
        memcpy(ptr, data, size);
    return size;
}

/*!
    \internal
*/
bool QtSslSocket::initSsl()
{
    // Precondition: certificate for servers
    if (d->mode == Server) {
        if (d->cert.isEmpty() && d->certDir.isEmpty()) {
            setErrorString(tr("QtSslSocket: Certificate required in server mode"));
            emit error(UnknownSocketError);
            return false;
        } else if (d->key.isEmpty()) {
            setErrorString(tr("Private key required in server mode"));
            emit error(UnknownSocketError);
            return false;
        }
    }

    // Create and initialize SSL context. Accept SSLv2, SSLv3 and
    // TLSv1.
    d->ctx = lib->SSL_CTX_new(d->mode == Server ? lib->SSLv23_server_method() : lib->SSLv23_client_method());
    if (!d->ctx) {
        setErrorString(tr("Error creating SSL context, %1").arg(SSL_ERRORSTR()));
        emit error(UnknownSocketError);
        return false;
    }

    // Enable all bug workarounds.
    lib->SSL_CTX_set_options(d->ctx, SSL_OP_ALL);

    // Initialize ciphers
    if (!d->ciph.isEmpty()) {
        if (!lib->SSL_CTX_set_cipher_list(d->ctx, d->ciph.toLatin1().constData())) {
            setErrorString(tr("Invalid or empty cipher list, %1").arg(SSL_ERRORSTR()));
            emit error(UnknownSocketError);
            return false;
        }
    }

    if (d->mode == Server) {
        lib->SSL_CTX_set_default_verify_paths(d->ctx);

        // Load certificate
        if (!lib->SSL_CTX_use_certificate_file(d->ctx, QFile::encodeName(d->cert).constData(), SSL_FILETYPE_PEM)) {
            setErrorString(tr("Error loading certificate, %1").arg(SSL_ERRORSTR()));
            emit error(UnknownSocketError);
            return false;
        }

        // Load private key
        if (!lib->SSL_CTX_use_PrivateKey_file(d->ctx, QFile::encodeName(d->key).constData(), SSL_FILETYPE_PEM)) {
            setErrorString(tr("Error loading private key, %1").arg(SSL_ERRORSTR()));
            emit error(UnknownSocketError);
            return false;
        }

        // Check if the certificate matches the private key.
        if  (!lib->SSL_CTX_check_private_key(d->ctx)) {
            setErrorString(tr("Private key do not certificate public key, %1").arg(SSL_ERRORSTR()));
            emit error(UnknownSocketError);
            return false;
        }
    }

    if (!d->cafile.isEmpty() || !d->cadir.isEmpty()) {
        // Load trusted certificate authority (CA) certificates.
        if (!lib->SSL_CTX_load_verify_locations(d->ctx,
                                           d->cafile.isEmpty() ? 0 : QFile::encodeName(d->cafile).constData(),
                                           d->cadir.isEmpty() ? 0 : QFile::encodeName(d->cadir).constData())) {
            setErrorString(tr("Failed to load the list of trusted CAs").arg(SSL_ERRORSTR()));
            emit error(UnknownSocketError);
            return false;
        }

        // Prior to 0.9.6, OpenSSL was unable to determine whether or
        // not a certificate came from a CA or not. We need to set the
        // verify depth to 1 in those cases, so that only root level
        // CAs are accepted for signing certificates.
#if OPENSSL_VERSION_NUMBER < 0x00906000L
        lib->SSL_CTX_set_verify_depth(d->ctx, 1);
#endif
    }

    // Create and initialize SSL session
    if (!(d->ssl = lib->SSL_new(d->ctx))) {
        setErrorString(tr("Error creating SSL session, %1").arg(SSL_ERRORSTR()));
        emit error(UnknownSocketError);
        return false;
    }

    lib->SSL_clear(d->ssl);

    // Have the SSL library read from a memory location. We will store
    // encrypted data in the write buffer, and read decrypted data
    // from the read buffer.
    d->rbio = lib->BIO_new(lib->BIO_s_mem());
    d->wbio = lib->BIO_new(lib->BIO_s_mem());
    if (!d->rbio || !d->wbio) {
        setErrorString(tr("Error creating SSL session"));
        emit error(UnknownSocketError);
        return false;
    }

    lib->SSL_set_bio(d->ssl, d->rbio, d->wbio);

    if (d->mode == Server)
        lib->SSL_set_accept_state(d->ssl);
    else
        lib->SSL_set_connect_state(d->ssl);

#if QT_VERSION >= 0x040100
    setPeerAddress(d->socket->peerAddress());
    setPeerPort(d->socket->peerPort());
    setPeerName(d->socket->peerName());
    setLocalAddress(d->socket->localAddress());
    setLocalPort(d->socket->localPort());
#endif

    // It is required that there is no data in the any output buffer
    // when the SSL library takes over communication. Subclasses with
    // buffers should reimplement flush.
    d->socket->flush();

    d->initialized = true;
    return true;
}

/*!
    \internal
*/
void QtSslSocket::handleSocketError(SocketError socketError)
{
    setErrorString(d->socket->errorString());
    emit error(socketError);
}

/*!
    \internal
*/
void QtSslSocket::readFromSocket()
{
    QByteArray block = d->socket->readAll();
    if (lib->BIO_write(d->rbio, block.data(), block.size()) != block.size()) {
        setErrorString(tr("Failed to write protocol data to BIO"));
        emit error(UnknownSocketError);
        return;
    }

    if (!d->connectionSecured) {
        if (d->mode == Client && sslConnect()) {
            emit connected();
        } else if (d->mode == Server && sslAccept()) {
            emit connected();
        }
        writeToSocket();
    }

    // The connected() and accepted() signals can be connected to
    // slots that close the socket. We don't want to read more data
    // from the SSL buffers if the socket is closing.
    if (d->socket->state() == ClosingState || d->socket->state() == UnconnectedState) {
        return;
    }

    int ret = 0;
    do {
        char buffer[SslReadBlockSize];
        ret = lib->SSL_read(d->ssl, buffer, sizeof(buffer));
        if (ret <= 0) {
            if (ret < 0 && lib->SSL_get_error(d->ssl, ret) == SSL_ERROR_WANT_READ) {
                // If a renegotiation is taking place, we will not be able to
                // read data. Rather we must submit whatever is in our
                // outbound queue (placed by SSL_read) to complete the
                // handshake. Note that this condition is also met when
                // SSL_read is called and there is no pending data.
                writeToSocket();
            } else if (ret == 0) {
                // The read operation was not successful, probably because no
                // data was available or the remote host closed the
                // connection.
                if (lib->SSL_get_error(d->ssl, 0) == SSL_ERROR_ZERO_RETURN) {
                    d->socket->disconnectFromHost();
                } else {
                    setErrorString(tr("Failed to read from SSL, %1")
                                   .arg(SSL_ERRORSTR()));
                    emit error(UnknownSocketError);
                }
            } else {
                setErrorString(tr("Failed to read from SSL, %1")
                               .arg(SSL_ERRORSTR()));
                emit error(UnknownSocketError);
            }
            break;
        }

        char *ptr = d->readBuffer.reserve(ret);
        memcpy(ptr, buffer, ret);

        emit readyRead();
        d->readyReadEmitted = true;
    } while (ret > 0);
}

/*!
    \internal
*/
void QtSslSocket::writeToSocket()
{
    d->calledWriteToSocket = false;
    if (d->connectionSecured) {
        while (!d->writeBuffer.isEmpty()) {
            int ret = lib->SSL_write(d->ssl, d->writeBuffer.readPointer(), d->writeBuffer.nextDataBlockSize());
            if (ret <= 0) {
                setErrorString(tr("SSL error: %1").arg(SSL_ERRORSTR()));
                emit error(UnknownSocketError);
                return;
            }
            d->writeBuffer.free(ret);
        }
    }

    int pending = (int)lib->BIO_ctrl(d->wbio,BIO_CTRL_PENDING,0,NULL);
    if (pending > 0) {
        QByteArray buffer;
        buffer.resize(pending);

        // With BIO_s_mem(), BIO_read can not fail.
        lib->BIO_read(d->wbio, buffer.data(), buffer.size());
        d->socket->write(buffer);
    }
}

/*!
    \internal
*/
void QtSslSocket::socketBytesWritten(qint64 bytes)
{
    emit bytesWritten(bytes);
}

/*!
    \internal
*/
QString QtSslSocket::verifyErrorString(int err)
{
    // Comments from man verify(1)
    switch (err) {
        case X509_V_OK:
            // the operation was successful.
            return tr("An unknown error occurred");
        case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
            // the passed certificate is self signed and the same
            // certificate cannot be found in the list of trusted
            // certificates.
            return tr("Self signed certificate");
        case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
            // the certificate chain could be built up using the
            // untrusted certificates but the root could not be found
            // locally.
            return tr("Self signed certificate in certificate chain");
        case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
            // the certificate signature could not be decrypted. This
            // means that the actual signature value could not be
            // determined rather than it not matching the expected
            // value, this is only meaningful for RSA keys.
            return tr("Unable to decrypt certificate's signature");
        case X509_V_ERR_CERT_SIGNATURE_FAILURE:
            // the signature of the certificate is invalid.
            return tr("Certificate signature failure");
        case X509_V_ERR_CERT_NOT_YET_VALID:
            // the certificate is not yet valid: the notBefore date is
            // after the current time.
            return tr("The certificate is not yet valid");
        case X509_V_ERR_CERT_HAS_EXPIRED:
            // the certificate has expired: that is the notAfter date
            // is before the current time.
            return tr("The certificate has expired");
        case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
            // the certificate notBefore field contains an invalid
            // time.
            return tr("Format error in certificate's notBefore field");
        case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
            // the certificate notAfter field contains an invalid
            // time.
            return tr("Format error in certificate's notAfter field");
        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
            // the issuer certificate of a locally looked up
            // certificate could not be found. This normally means
            // the list of trusted certificates is not complete.
            return tr("Unable to get local issuer certificate",
                "the issuer certificate of a locally looked up"
                " certificate could not be found. This normally means"
                " the list of trusted certificates is not complete.");
        case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
            // no signatures could be verified because the chain
            // contains only one certificate and it is not self
            // signed.
            return tr("Unable to verify the first certificate",
                "no signatures could be verified because the chain"
                " contains only one certificate and it is not self"
                " signed.");
        case X509_V_ERR_INVALID_CA:
            // a CA certificate is invalid. Either it is not a CA or
            // its extensions are not consistent with the supplied
            // purpose
            return tr("Invalid CA certificate");
        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
            // the issuer certificate could not be found: this occurs
            // if the issuer certificate of an untrusted certificate
            // cannot be found.
            return tr("Unable to get issuer certificate",
                "the issuer certificate could not be found: this occurs"
                " if the issuer certificate of an untrusted certificate"
                " cannot be found.");
        case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
            // the public key in the certificate SubjectPublicKeyInfo
            // could not be read.
            return tr("Unable to decode issuer public key");
        case X509_V_ERR_PATH_LENGTH_EXCEEDED:
            // the basicConstraints pathlength parameter has been
            // exceeded.
            return tr("Path length constraint exceeded",
                "the basicConstraints pathlength parameter has been exceeded.");
        case X509_V_ERR_INVALID_PURPOSE:
            // the supplied certificate cannot be used for the
            // specified purpose.
            return tr("Unsupported certificate purpose", "the supplied certificate cannot be used for the"
                " specified purpose." );
        case X509_V_ERR_CERT_UNTRUSTED:
            // the root CA is not marked as trusted for the specified
            // purpose.
            return tr("Certificate not trusted");
        case X509_V_ERR_CERT_REJECTED:
            // the root CA is marked to reject the specified purpose.
            return tr("Certificate rejected");
        case X509_V_ERR_SUBJECT_ISSUER_MISMATCH:
            // the current candidate issuer certificate was rejected
            // because its subject name did not match the issuer name
            // of the current certificate.
            return tr("Subject issuer mismatch", "the current candidate issuer certificate was rejected"
                " because its subject name did not match the issuer name"
                " of the current certificate.");
        case X509_V_ERR_AKID_SKID_MISMATCH:
            // the current candidate issuer certificate was rejected
            // because its subject key identifier was present and did
            // not match the authority key identifier current
            // certificate.
            return tr("Authority and subject key identifier mismatch");
        case X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH:
            // the current candidate issuer certificate was rejected
            // because its issuer name and serial number was present
            // and did not match the authority key identifier of the
            // current certificate.
            return tr("Authority and issuer serial number mismatch",
                "the current candidate issuer certificate was rejected"
                " because its issuer name and serial number was present"
                " and did not match the authority key identifier of the"
                " current certificate.");
            return tr("Authority and issuer serial number mismatch");
        case X509_V_ERR_KEYUSAGE_NO_CERTSIGN:
            // the current candidate issuer certificate was rejected
            // because its keyUsage extension does not permit
            // certificate signing.
            return tr("Key usage does not include certificate signing",
                "the current candidate issuer certificate was rejected"
                " because its keyUsage extension does not permit"
                " certificate signing.");
        case X509_V_ERR_OUT_OF_MEM:
            // an error occurred trying to allocate memory. This
            // should never happen.
            return tr("Out of memory");
        default:
            return tr("Unknown error");
    }
}

/*! \internal
    If \a accept is true, this function will attempt to negotiate a
    server side handshake with the remote client, otherwise it will
    negotiate a client side handshake with the remote server.
*/
bool QtSslSocket::sslAcceptOrConnect(bool accept)
{
    if (!d->initialized && !initSsl())
        return false;

    // SSL connect (handshake). Note that during the handshake, the
    // verify callback is called and it might emit warnings.
    int result = accept ? lib->SSL_accept(d->ssl) : lib->SSL_connect(d->ssl);
    if (result <= 0) {
        if (result < 0) {
            int err = lib->SSL_get_error(d->ssl, result);
            if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
                // The handshake is not yet complete.
                writeToSocket();
                return false;
            }
        }

        setErrorString(tr("SSL error: %1").arg(SSL_ERRORSTR()));
        emit error(UnknownSocketError);
        return false;
    }

    // Parse certificate
    QStringList items = peerCertificate().split('/');
    QString certHost;
    QDateTime notValidBefore;
    QDateTime notValidAfter;
    for (int i = 0; i < (int) items.count(); ++i) {
        QString s = items.at(i);
        if (s.startsWith("CN="))
            certHost = s.mid(3);
        else if (s.startsWith("notValidBefore="))
            notValidBefore = QDateTime::fromString(s.mid(15), Qt::ISODate);
        else if (s.startsWith("notValidAfter="))
            notValidAfter = QDateTime::fromString(s.mid(14), Qt::ISODate);
    }

    // Check if the certificate was verified. Note that err is
    // X509_V_OK also if the peer had no certificate.
    int err = lib->SSL_get_verify_result(d->ssl);
    if (err != X509_V_OK) {
        VerifyResult res = VerifyOk;
        switch (err) {
        case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
        case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
            res = SelfSigned;
            break;
        default:
            res = UnableToVerify;
        }

        emit connectionVerificationDone(res, certHost.toLower() != d->socket->peerName().toLower(),
                                        verifyErrorString(err));
    } else {
        QString errorString = tr("The SSL certificate has been verified.");
        VerifyResult result = VerifyOk;
        if (notValidBefore > QDateTime::currentDateTime()) {
            result = NotYetValid;
            errorString = tr("The SSL certificate is not valid until %1.").arg(notValidBefore.toString());
        } else if (notValidAfter < QDateTime::currentDateTime()) {
            result = Expired;
            errorString = tr("The SSL certificate expired on %1.").arg(notValidAfter.toString());
        }

        if (certHost.toLower() != d->socket->peerName().toLower()) {
            errorString += QLatin1String(" ");
            if ( result == VerifyOk ) {
                errorString += tr("The host name \"%1\" doesn't match the host name in the certificate \"%2\" though.")
                               .arg(d->socket->peerName()).arg(certHost);
            } else {
                errorString += tr("The host name \"%1\" doesn't match the host name in the certificate \"%2\".")
                               .arg(d->socket->peerName()).arg(certHost);
            }
        }

        emit connectionVerificationDone(result,certHost.toLower() != d->socket->peerName().toLower(),
                                        errorString);
    }
    if (d->socket->state() != ConnectedState) {
        return false;
    }

    setSocketState(ConnectedState);
    emit stateChanged(ConnectedState);
    d->connectionSecured = true;
    readFromSocket();
    return true;
}
