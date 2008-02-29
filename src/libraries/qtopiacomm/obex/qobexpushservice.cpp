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

#include <qobexpushservice.h>
#include <qobexsocket.h>
#include <qtopiacomm/private/qobexcommon_p.h>

#include <QSocketNotifier>
#include <QString>
#include <QTimer>
#include <QFile>
#include <QBuffer>

#include <qmimetype.h>
#include <qtopialog.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>

#include <openobex/obex.h>

static void qobex_receiver_callback(obex_t *handle, obex_object_t *object, int mode, int event, int obex_cmd, int obex_rsp );

class QObexPushService_Private : public QObject
{
    Q_OBJECT
    friend void qobex_receiver_callback(obex_t *handle, obex_object_t *object,
                                        int mode, int event, int obex_cmd, int obex_rsp );

public slots:
    void processInput();

    void deleteMeLater();
    void deleteMe();

signals:
    void putRequest(const QString &filename, const QString &mimetype);
    void getRequest(const QString &filename, const QString &mimetype);
    void requestComplete(bool error);
    void done(bool error);
    void progress(qint64, qint64);
    void stateChanged(QObex::State state);
    void aboutToDelete();

public:
    QObexPushService_Private(QObexSocket *socket, QObexPushService *parent);
    ~QObexPushService_Private();

    void close();
    void updateState(QObex::State state);
    void abortFileTransfer();

    void setAutoDelete(bool enable);
    bool autoDelete() const;

    void writeOut(int len, const char *buf);
    void readData(obex_object_t *object);
    void handlePut(obex_object_t *object);
    void preparePut(obex_object_t *object);

    void feedStream(obex_object_t *object);
    void sendBusinessCard(obex_object_t *object);

    QObexPushService *m_parent;
    obex_t *m_self;
    QObexSocket *m_socket;
    QObex::Error m_error;
    QObex::State m_state;

    bool m_autodelete;

    int m_total;
    int m_bytes;
    QFile m_outfile;
    QString m_filename;
    QString m_mimetype;
    QByteArray m_vcard;
    QString m_incoming;
    bool m_first_packet;
    QBuffer m_tmpBuffer;
};

QObexPushService_Private::QObexPushService_Private(QObexSocket *socket, QObexPushService *parent)
    : QObject(parent)
{
    m_socket = socket;
    m_self = static_cast<obex_t *>(socket->handle());
    m_state = QObex::Connecting;
    m_error = QObex::NoError;

    m_autodelete = false;
    m_parent = parent;

    m_bytes = 0;
    m_total = 0;

    m_first_packet = false;

    OBEX_SetUserCallBack(m_self, qobex_receiver_callback, this);
    OBEX_SetUserData(m_self, this);

    QSocketNotifier *sn = new QSocketNotifier( OBEX_GetFD( m_self ),
                                               QSocketNotifier::Read,
                                               this );
    connect( sn, SIGNAL(activated(int)), this, SLOT(processInput()) );
}

QObexPushService_Private::~QObexPushService_Private()
{
    if (m_socket)
        delete m_socket;
}

void QObexPushService_Private::setAutoDelete(bool enable)
{
    m_autodelete = enable;
}

bool QObexPushService_Private::autoDelete() const
{
    return m_autodelete;
}

void QObexPushService_Private::deleteMeLater()
{
    if (m_parent->autoDelete()) {
        QTimer::singleShot( 0, this, SLOT(deleteMe()) );
    }
}

void QObexPushService_Private::deleteMe()
{
    emit aboutToDelete();
    delete m_parent;
}

static void qobex_receiver_callback(obex_t *handle, obex_object_t *object, int mode, int event, int obex_cmd, int obex_rsp )
{
    Q_UNUSED(obex_rsp);
    Q_UNUSED(mode);

    QObexPushService_Private* receiver =
            (QObexPushService_Private*)OBEX_GetUserData( handle );

    switch (event)        {
    case OBEX_EV_PROGRESS: // Don't do anything
        break;
    case OBEX_EV_STREAMAVAIL:
        receiver->readData( object );
        break;
    case OBEX_EV_STREAMEMPTY:
        receiver->feedStream( object );
        break;
    case OBEX_EV_REQ:
        switch(obex_cmd) {
        case OBEX_CMD_ABORT:
            receiver->abortFileTransfer();
            break;
        }
        break;
    case OBEX_EV_REQCHECK:
        qLog(Obex) << "Got a REQCHECK";
        switch(obex_cmd) {
            case OBEX_CMD_PUT:
                receiver->handlePut(object);
                break;
        }
        break;
    case OBEX_EV_REQHINT:
        /* A new request is coming in */
        switch(obex_cmd) {
        case OBEX_CMD_GET:
            receiver->m_vcard = receiver->m_parent->businessCard();
            // If we have a Business Card set, then reply with success
            // Otherwise, to be standards compliant with Obex Push, this needs to
            // return not found instead of not implemented
            if (receiver->m_vcard.isEmpty()) {
                OBEX_ObjectSetRsp(object, OBEX_RSP_NOT_FOUND, OBEX_RSP_NOT_FOUND);
            } else {
                receiver->sendBusinessCard(object);
            }
            break;

        case OBEX_CMD_PUT:
            receiver->preparePut(object);
            break;
        case OBEX_CMD_CONNECT:
            OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
            receiver->updateState(QObex::Connecting);
            break;
        case OBEX_CMD_ABORT:
            OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
            break;

        case OBEX_CMD_DISCONNECT:
            OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
            receiver->updateState(QObex::Disconnecting);
            break;
        default:
            /* Reject any other commands */
            OBEX_ObjectSetRsp(object, OBEX_RSP_NOT_IMPLEMENTED,
                            OBEX_RSP_NOT_IMPLEMENTED);
            break;
        }
        break;

    case OBEX_EV_LINKERR:
        qLog(Obex) << "Got a Link Error";
        if (receiver->m_state != QObex::Closed) {
            // If we got a link error, and the received size != received length
            // treat it as an error, otherwise just assume it was finished correctly
            // Some mobile phones behave this way.
            if ( receiver->m_total && receiver->m_total != (int)receiver->m_outfile.size() )
                receiver->m_error = QObex::LinkError;
            receiver->updateState(QObex::Closed);
        }
        break;
    case OBEX_EV_REQDONE:
        switch (obex_cmd) {
        case OBEX_CMD_CONNECT:
            qLog(Obex) << "Got a CONNECT";
            receiver->updateState(QObex::Ready);
            break;
        case OBEX_CMD_DISCONNECT:
            qLog(Obex) << "Got a DISCONNECT";
            receiver->updateState(QObex::Closed);
            break;
        }
        break;

    default:
        OBEX_ObjectSetRsp(object, OBEX_RSP_NOT_IMPLEMENTED,
                          OBEX_RSP_NOT_IMPLEMENTED);
        break;
    }
}

void QObexPushService_Private::processInput()
{
    if (m_state == QObex::Closed)
        return;

    OBEX_HandleInput( m_self, 0 );
}

void QObexPushService_Private::feedStream( obex_object_t *object )
{
    Q_ASSERT(m_state == QObex::Streaming);

    qLog(Obex) <<  "QObexPushService::Feeding Stream";

    char *streambuf = &m_vcard.data()[m_bytes];

    int len = 0;

    if ( (m_total - m_bytes) > OBEX_STREAM_BUF_SIZE) {
        len = OBEX_STREAM_BUF_SIZE;
    }
    else {
        len = m_total - m_bytes;
    }

    m_bytes += len;
    obex_headerdata_t hd;

    if ( len > 0 ) {
        hd.bs = reinterpret_cast<uint8_t *>(streambuf);
        OBEX_ObjectAddHeader(m_self, object, OBEX_HDR_BODY,
                             hd, len, OBEX_FL_STREAM_DATA);
        qLog(Obex) << "QObexPushService::emitting progress report";
        emit progress( m_bytes, m_total );
    } else {
        /* EOF */
        hd.bs = 0;
        OBEX_ObjectAddHeader(m_self, object, OBEX_HDR_BODY,
                             hd, 0, OBEX_FL_STREAM_DATAEND);
        emit requestComplete(false);
        updateState(QObex::Ready);
    }

}

void QObexPushService_Private::sendBusinessCard(obex_object_t *object)
{
    qLog(Obex) << "Client requesting my business card";

    OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
    m_mimetype = "text/x-vCard";
    m_filename = "MyBusinessCard.vcf";

    m_total = m_vcard.size();
    m_bytes = 0;

    QObexHeader header;
    header.setMimeType(m_mimetype);
    header.setName(m_filename);
    header.setLength(m_total);
    setHeaders(m_self, object, header);

    obex_headerdata_t hd;
    hd.bs = 0;
    OBEX_ObjectAddHeader(m_self, object, OBEX_HDR_BODY,
                         hd, 0, OBEX_FL_STREAM_START);

    emit getRequest(QString(), m_mimetype);
    updateState(QObex::Streaming);
}

void QObexPushService_Private::writeOut(int len, const char *buf)
{
    if ( len > 0 ) {
        if( m_outfile.isOpen() ) {// if unable to open outfile don't write to it
            m_outfile.write( buf, len );
            qLog(Obex) << "QObexPushService::emitting progress report" << m_bytes << "/" << m_total;
            emit progress( m_bytes, m_total );
        }
    } else if ( len == 0 ) {
        qLog(Obex) << "Received an EOF";
        m_outfile.close();

        if (m_state == QObex::Streaming) {
            emit requestComplete(false);
            updateState(QObex::Ready);
        }
    } else {
        qLog(Obex) << "Unknown error during read";
        m_error = QObex::UnknownError;
        emit requestComplete(true);
        updateState(QObex::Closed);
    }
}

void QObexPushService_Private::readData( obex_object_t *object )
{
    const char* buf;

    qLog(Obex) << "QObexPushService::in readData";

    int len = OBEX_ObjectReadStream( m_self, object, reinterpret_cast<const uint8_t **>(&buf) );
    m_bytes += len;

    qLog(Obex) << "Got " << len << " bytes..";

    // The streamavail notification comes before the
    // reqcheck notification.  We absolutely must read the data
    // otherwise it is lost.  Thus read the data, but do not write it to a file
    // in case the file is accepted by the handlePut function
    // the data will be written there.  Until then, buffer it up here
    if (m_first_packet) {
        // REQCHECK doesn't get called if we only receive
        // OBEX request 1 packet long (as in the case of
        // vcards
        if (len == 0) {
            handlePut(object);
            if (m_state == QObex::Streaming) {
                emit requestComplete(false);
                updateState(QObex::Ready);
            }
            return;
        }
        else {
            m_tmpBuffer.setData(buf, len);
            return;
        }
    }

    writeOut(len, buf);
}

void QObexPushService_Private::handlePut(obex_object_t *object)
{
    qLog(Obex) << "in handleput";
    qLog(Obex) << "QObexPushService::getting headers";

    QObexHeader hdr;
    getHeaders( m_self, object, hdr);

    if (!m_parent->acceptFile(hdr.name(), hdr.mimeType(), hdr.length())) {
        qLog(Obex) << "Rejecting file";
        OBEX_ObjectSetRsp(object, OBEX_RSP_FORBIDDEN, OBEX_RSP_FORBIDDEN);
        return;
    }

    OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);

    m_filename = hdr.name();
    m_mimetype = hdr.mimeType();
    m_total = hdr.length();
    emit putRequest(m_filename, m_mimetype);

    m_outfile.setFileName( m_incoming + m_filename );
    if (!m_outfile.open( QIODevice::WriteOnly )) {
        qWarning("Unable to open file %s for OPUSH receive", m_filename.toLatin1().constData());
    }
    else {
        updateState(QObex::Streaming);
    }

    // This situation should be checked if we get a 1st packet with only headers
    // and no body.  The body comes in subsequent packets.  Do this so as not to
    // trigger an EOF condition in writeOut
    if (m_bytes) {
        writeOut(m_bytes, m_tmpBuffer.data().data());
    }

    m_first_packet = false;
}

void QObexPushService_Private::preparePut(obex_object_t *object)
{
    qLog(Obex) << "QObexPushService::preparePut()";
    m_first_packet = true;
    m_bytes = 0;
    OBEX_ObjectReadStream( m_self, object, NULL );
}

void QObexPushService_Private::updateState(QObex::State state)
{
    qLog(Obex) << "Updating state to:" << state;

    if ((m_state == QObex::Streaming) &&
         ((state == QObex::Closed) ||
         (state == QObex::Disconnecting))) {
        emit requestComplete(true);
    }

    m_state = state;
    emit stateChanged(m_state);
    if (m_state == QObex::Closed) {
        OBEX_TransportDisconnect( m_self );

        emit done(m_error);

        m_filename = QString();
        m_mimetype = QString();
        if (m_outfile.isOpen())
            m_outfile.close();

        deleteMeLater();
    }
}

// changed abort() -> close() since it's a hard abort that just closes the
// connection
void QObexPushService_Private::close()
{
    // just disconnect the connection
    m_error = QObex::NoError;
    updateState(QObex::Closed);
}

void QObexPushService_Private::abortFileTransfer()
{
    qLog(Bluetooth) << "Got Abort!";
    m_outfile.close();
    ::unlink( m_filename.toLocal8Bit().data() ); // delete if exists

    m_filename = QString();
    m_mimetype = QString();

    if (m_state == QObex::Streaming)
        updateState(QObex::Ready);
}

/*!
    \class QObexPushService
    \brief The QObexPushService class encapsulates an OBEX PUSH service.

    The QObexPushService class can be used to provide OBEX PUSH services
    over Bluetooth, Infrared or any other OBEX capable transport.
    This class implements the Bluetooth Object Push Profile, and can also
    be used to implement the Infrared IrXfer service.

    \sa QObexServer
*/

/*!
    Constructor for QObexPushService.  The \a socket parameter specifies the
    OBEX socket to use.  The socket is assumed to have been obtained from a
    \c QObexServer::nextPendingConnection call.  The \a parent specifies
    the parent object.

    Note that the QObexPushService takes ownership of the socket.  When
    QObexPushService object is deleted, \c socket is deleted as well.
*/
QObexPushService::QObexPushService(QObexSocket *socket, QObject *parent)
    : QObject(parent)
{
    m_data = new QObexPushService_Private(socket, this);

    connect(m_data, SIGNAL(putRequest(const QString &, const QString &)),
            SIGNAL(putRequest(const QString &, const QString &)));
    connect(m_data, SIGNAL(getRequest(const QString &, const QString &)),
            SIGNAL(getRequest(const QString &, const QString &)));
    connect(m_data, SIGNAL(requestComplete(bool)), SIGNAL(requestComplete(bool)));
    connect(m_data, SIGNAL(done(bool)), SIGNAL(done(bool)));

    connect(m_data, SIGNAL(aboutToDelete()), SIGNAL(aboutToDelete()));

    connect(m_data, SIGNAL(stateChanged(QObex::State)),
            SIGNAL(stateChanged(QObex::State)));
    connect(m_data, SIGNAL(progress(qint64, qint64)), SIGNAL(progress(qint64, qint64)));
}

/*!
    Destructor.
*/
QObexPushService::~QObexPushService()
{
    if (m_data)
        delete m_data;
}

/*!
    Stops the operation currently in progress, sets the state to Unconnected and
    if the autoDelete property is set to true, schedules deletion of the service.
*/
void QObexPushService::close()
{
    m_data->close();
}

/*!
    Sets the autoDelete property to \a enable.  If autoDelete is enabled, then the
    object is automatically scheduled for deletion when the object next enters the
    Unconnected state.

    \sa autoDelete()
*/
void QObexPushService::setAutoDelete(bool enable)
{
    m_data->setAutoDelete(enable);
}

/*!
    Returns true if autoDelete is enabled, returns false otherwise.

    \sa setAutoDelete()
*/
bool QObexPushService::autoDelete() const
{
    return m_data->autoDelete();
}

/*!
    Returns the last error that has occurred.
*/
QObex::Error QObexPushService::error() const
{
    return m_data->m_error;
}

/*!
    Returns the current connection state of the object.
*/
QObex::State QObexPushService::state() const
{
    return m_data->m_state;
}

/*!
    Sets the incomding directory to \a dir.  This directory will be used to place all files
    which are being received by the service.
*/
void QObexPushService::setIncomingDirectory(const QString &dir)
{
    m_data->m_incoming = dir;
    if (!m_data->m_incoming.endsWith('/'))
        m_data->m_incoming.append('/');
}

/*!
    Returns the incoming directory.
*/
const QString &QObexPushService::incomingDirectory() const
{
    return m_data->m_incoming;
}

/*!
    Override this function to provide custom intelligence as to whether to accept
    an incoming file or not (e.g. popping a message box asking the user to accept, or
    using a file threshold, etc)

    The \a filename holds the name of the file, \a mimetype holds the mimetype.
    The \a size parameter holds the size of the file.

    The default implementation accepts all files unless no incoming directory is set.
*/
bool QObexPushService::acceptFile(const QString &filename, const QString &mimetype, qint64 size)
{
    Q_UNUSED(filename)
    Q_UNUSED(mimetype)
    Q_UNUSED(size)

    if (m_data->m_incoming.isEmpty())
        return false;

    return true;
}

/*!
    Clients should override this function to provide personal business card data
    in vCard format.  This is required for Business Card exchange feature of the
    Bluetooth Object Push Profile.  If an empty byte array is returned, the
    request for a business card will be rejected.
*/
QByteArray QObexPushService::businessCard() const
{
    return QByteArray();
}

/*!
    Returns the socket associated with this service.
*/
QObexSocket *QObexPushService::socket()
{
    return m_data->m_socket;
}

/*!
    \fn void QObexPushService::done(bool error);

    This signal is emitted whenever the client has disconnected from the service
    or the connection has been terminated.  The \a error parameter reports whether
    an error occurred during processing.
 */

/*!
    \fn void QObexPushService::progress(qint64 completed, qint64 total);

    This signal is emitted reports the progress of the file send operation.
    The \a completed parameter reports how many bytes were sent, and \a total
    parameter reports the total number of bytes to send.
 */

/*!
    \fn void QObexPushService::aboutToDelete()

    This signal is emitted whenever an object is about to delete
    itself.  This will only happen if the \c autoDelete property is set.

    \sa setAutoDelete(), autoDelete()
 */

/*!
    \fn void QObexPushService::stateChanged(QObex::State state)

    This signal is emitted whenever a service object changes state.  The \a state
    parameter holds the current state.
 */

/*!
    \fn void QObexPushService::putRequest(const QString &filename, const QString &mimetype)

    This signal is emitted whenever a new put request comes in from the client.
    The \a filename parameter contains the filename, and \a mimetype contains
    the mimetype of the request.
*/

/*!
    \fn void QObexPushService::getRequest(const QString &filename, const QString &mimetype)

    This signal is emitted whenever a new get request comes in from the client.
    The only requests supported by the OBEX Push Service are for the Business Card.
    As such, the \a filename will parameter will always be a null string, and the
    \a mimetype will always be "text/x-vCard"
*/

/*!
    \fn void QObexPushService::requestComplete(bool error)

    This signal is emitted whenever a request has been completed.  The \a error parameter
    holds whether an error has occurred during a request
*/


#include "qobexpushservice.moc"
