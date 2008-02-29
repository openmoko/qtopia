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

class QObexPushServicePrivate : public QObject
{
    Q_OBJECT
    friend void qobex_receiver_callback(obex_t *handle, obex_object_t *object,
                                        int mode, int event, int obex_cmd, int obex_rsp );

public slots:
    void processInput();

signals:
    void putRequest(const QString &filename, const QString &mimetype);
    void getRequest(const QString &filename, const QString &mimetype);
    void requestComplete(bool error);
    void done(bool error);
    void progress(qint64, qint64);
    void stateChanged(QObexPushService::State state);

public:
    QObexPushServicePrivate(QObexSocket *socket, QObexPushService *parent);
    ~QObexPushServicePrivate();

    void close();
    void updateState(QObexPushService::State state);
    void abortFileTransfer();

    void writeOut(int len, const char *buf);
    void readData(obex_object_t *object);
    void handlePut(obex_object_t *object);
    void preparePut(obex_object_t *object);

    void feedStream(obex_object_t *object);
    void sendBusinessCard(obex_object_t *object);

    QObexPushService *m_parent;
    obex_t *m_self;
    QObexSocket *m_socket;
    QObexPushService::Error m_error;
    QObexPushService::State m_state;

    int m_total;
    int m_bytes;
    QFile m_outfile;
    QString m_filename;
    QString m_mimetype;
    QByteArray m_vcard;
    QString m_incoming;
    bool m_first_packet;
    QBuffer m_tmpBuffer;

    QSocketNotifier *m_notifier;
};

QObexPushServicePrivate::QObexPushServicePrivate(QObexSocket *socket, QObexPushService *parent)
    : QObject(parent)
{
    m_socket = socket;
    m_self = static_cast<obex_t *>(socket->handle());
    m_state = QObexPushService::Ready;
    m_error = QObexPushService::NoError;

    m_parent = parent;

    m_bytes = 0;
    m_total = 0;

    m_first_packet = false;

    OBEX_SetUserCallBack(m_self, qobex_receiver_callback, this);
    OBEX_SetUserData(m_self, this);

    m_notifier = new QSocketNotifier( OBEX_GetFD( m_self ),
                                      QSocketNotifier::Read,
                                      this );
    connect( m_notifier, SIGNAL(activated(int)), this, SLOT(processInput()) );
}

QObexPushServicePrivate::~QObexPushServicePrivate()
{
}

static void qobex_receiver_callback(obex_t *handle, obex_object_t *object, int mode, int event, int obex_cmd, int obex_rsp )
{
    Q_UNUSED(obex_rsp);
    Q_UNUSED(mode);

    QObexPushServicePrivate* receiver =
            (QObexPushServicePrivate*)OBEX_GetUserData( handle );

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
            receiver->updateState(QObexPushService::Connecting);
            break;
        case OBEX_CMD_ABORT:
            OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
            break;

        case OBEX_CMD_DISCONNECT:
            OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
            receiver->updateState(QObexPushService::Disconnecting);
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
        if (receiver->m_state != QObexPushService::Closed) {
            // If we got a link error, and the received size != received length
            // treat it as an error, otherwise just assume it was finished correctly
            // Some mobile phones behave this way.
            if ( receiver->m_total && receiver->m_total != (int)receiver->m_outfile.size() )
                receiver->m_error = QObexPushService::LinkError;
            receiver->updateState(QObexPushService::Closed);
        }
        break;
    case OBEX_EV_REQDONE:
        switch (obex_cmd) {
        case OBEX_CMD_CONNECT:
            qLog(Obex) << "Got a CONNECT";
            receiver->updateState(QObexPushService::Ready);
            break;
        case OBEX_CMD_DISCONNECT:
            qLog(Obex) << "Got a DISCONNECT";
            receiver->updateState(QObexPushService::Closed);
            break;
        }
        break;

    default:
        OBEX_ObjectSetRsp(object, OBEX_RSP_NOT_IMPLEMENTED,
                          OBEX_RSP_NOT_IMPLEMENTED);
        break;
    }
}

void QObexPushServicePrivate::processInput()
{
    if (m_state == QObexPushService::Closed)
        return;

    OBEX_HandleInput( m_self, 0 );
}

void QObexPushServicePrivate::feedStream( obex_object_t *object )
{
    Q_ASSERT(m_state == QObexPushService::Streaming);

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

        updateState(QObexPushService::Ready);
        emit requestComplete(false);
    }

}

void QObexPushServicePrivate::sendBusinessCard(obex_object_t *object)
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
    updateState(QObexPushService::Streaming);
}

void QObexPushServicePrivate::writeOut(int len, const char *buf)
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

        if (m_state == QObexPushService::Streaming) {
            updateState(QObexPushService::Ready);
            emit requestComplete(false);
        }
    } else {
        qLog(Obex) << "Unknown error during read";
        m_error = QObexPushService::UnknownError;
        updateState(QObexPushService::Closed);
        emit requestComplete(true);
    }
}

void QObexPushServicePrivate::readData( obex_object_t *object )
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
            if (m_state == QObexPushService::Streaming) {
                updateState(QObexPushService::Ready);
                emit requestComplete(false);
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

void QObexPushServicePrivate::handlePut(obex_object_t *object)
{
    qLog(Obex) << "in handleput";
    qLog(Obex) << "QObexPushService::getting headers";

    QObexHeader hdr;
    getHeaders( m_self, object, hdr);

    m_notifier->setEnabled(false);
    bool isAccepted = m_parent->acceptFile(hdr.name(), hdr.mimeType(), hdr.length());
    m_notifier->setEnabled(true);

    if (!isAccepted) {
        qLog(Obex) << "Rejecting file";
        OBEX_ObjectSetRsp(object, OBEX_RSP_FORBIDDEN, OBEX_RSP_FORBIDDEN);
        return;
    }

    m_filename = hdr.name();
    m_mimetype = hdr.mimeType();
    m_total = hdr.length();

    emit putRequest(m_filename, m_mimetype);

    m_outfile.setFileName( m_incoming + m_filename );
    if (!m_outfile.open( QIODevice::WriteOnly )) {
        qWarning("Unable to open file %s for OPUSH receive",
                 m_filename.toLatin1().constData());
        OBEX_ObjectSetRsp(object, OBEX_RSP_INTERNAL_SERVER_ERROR,
                          OBEX_RSP_INTERNAL_SERVER_ERROR);

        m_error = QObexPushService::UnknownError;
        updateState(QObexPushService::Ready);
        emit requestComplete(true);
        return;
    }
    else {
        updateState(QObexPushService::Streaming);
        OBEX_ObjectSetRsp(object, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
    }

    // This situation should be checked if we get a 1st packet with only headers
    // and no body.  The body comes in subsequent packets.  Do this so as not to
    // trigger an EOF condition in writeOut
    if (m_bytes) {
        writeOut(m_bytes, m_tmpBuffer.data().data());
    }

    m_first_packet = false;
}

void QObexPushServicePrivate::preparePut(obex_object_t *object)
{
    qLog(Obex) << "QObexPushService::preparePut()";
    m_first_packet = true;
    m_bytes = 0;
    OBEX_ObjectReadStream( m_self, object, NULL );
}

void QObexPushServicePrivate::updateState(QObexPushService::State state)
{
    qLog(Obex) << "Updating state to:" << state;

    if (m_state == state)
        return;

    if ((m_state == QObexPushService::Streaming) &&
         ((state == QObexPushService::Closed) ||
         (state == QObexPushService::Disconnecting))) {

        // ensure an error is set
        if (m_error == QObexPushService::NoError)
            m_error = QObexPushService::UnknownError;
        emit requestComplete(true);
    }

    m_state = state;
    emit stateChanged(m_state);

    if (m_state == QObexPushService::Closed) {

        m_filename = QString();
        m_mimetype = QString();
        if (m_outfile.isOpen())
            m_outfile.close();

        emit done(m_error);
    }
}

// changed abort() -> close() since it's a hard abort that just closes the
// connection
void QObexPushServicePrivate::close()
{
    if (m_state == QObexPushService::Closed)
        return;

    if (m_state == QObexPushService::Ready)
        m_error = QObexPushService::NoError;
    else
        m_error = QObexPushService::LinkError;

    updateState(QObexPushService::Closed);
}

void QObexPushServicePrivate::abortFileTransfer()
{
    qLog(Obex) << "QObexPushServicePrivate::abortFileTransfer()";
    m_outfile.close();
    ::unlink( m_filename.toLocal8Bit().data() ); // delete if exists

    m_filename = QString();
    m_mimetype = QString();

    if (m_state == QObexPushService::Streaming) {
        m_error = QObexPushService::Aborted;
        emit requestComplete(true);
        updateState(QObexPushService::Ready);
    }
}

/*!
    \class QObexPushService
    \brief The QObexPushService class encapsulates an OBEX PUSH service.

    The QObexPushService class can be used to provide OBEX Push services
    over Bluetooth, Infrared or any other OBEX capable transport.
    This class implements the Bluetooth Object Push Profile, and can also
    be used to implement the Infrared IrXfer service.

    \ingroup qtopiaobex
    \sa QObexServer
*/

/*!
    \enum QObexPushService::State
    Defines the possible states for a push service.

    \value Ready The service is ready to receive requests from an OBEX client. This is the default state.
    \value Connecting A client is connecting.
    \value Disconnecting A client is disconnecting.
    \value Streaming A file transfer operation is in progress.
    \value Closed The service has been closed and cannot process any more requests.
 */

/*!
    \enum QObexPushService::Error
    Defines the possible errors for a push service.

    \value NoError No error has occurred.
    \value LinkError The connection link has been interrupted.
    \value Aborted The request was aborted by the client.
    \value UnknownError An error other than those specified above occurred.
 */

/*!
    Constructor for QObexPushService.  The \a socket parameter specifies the
    OBEX socket to use.  The socket is assumed to have been obtained from a
    \c QObexServer::nextPendingConnection() call.  The \a parent specifies
    the parent object.
*/
QObexPushService::QObexPushService(QObexSocket *socket, QObject *parent)
    : QObject(parent)
{
    m_data = new QObexPushServicePrivate(socket, this);

    connect(m_data, SIGNAL(putRequest(const QString &, const QString &)),
            SIGNAL(putRequest(const QString &, const QString &)));
    connect(m_data, SIGNAL(getRequest(const QString &, const QString &)),
            SIGNAL(getRequest(const QString &, const QString &)));
    connect(m_data, SIGNAL(requestComplete(bool)), SIGNAL(requestComplete(bool)));
    connect(m_data, SIGNAL(done(bool)), SIGNAL(done(bool)));

    connect(m_data, SIGNAL(stateChanged(QObexPushService::State)),
            SIGNAL(stateChanged(QObexPushService::State)));
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
    Closes the service and sets the state to Closed. If an operation is in
    progress, the commandFinished() and done() signals will be emitted with
    their \c error arguments set to \c true.
*/
void QObexPushService::close()
{
    m_data->close();
}

/*!
    Returns the last error that has occurred.
*/
QObexPushService::Error QObexPushService::error() const
{
    return m_data->m_error;
}

/*!
    Returns the current connection state of the object.
*/
QObexPushService::State QObexPushService::state() const
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
    \fn void QObexPushService::stateChanged(QObexPushService::State state)

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
