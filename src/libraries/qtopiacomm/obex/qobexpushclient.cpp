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

#include <qmimetype.h>
#include <qtopialog.h>
#include <qobexpushclient.h>
#include <qobexsocket.h>
#include <qtopiacomm/private/qobexcommand_p.h>
#include <qtopiacomm/private/qobexcommon_p.h>

#include <qtopiaipcenvelope.h>

#include <QSocketNotifier>
#include <QString>
#include <QFile>
#include <QTimer>
#include <QQueue>
#include <qglobal.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>

#include <openobex/obex.h>

/*!
    \class QObexPushClient
    \brief The QObexPushClient class encapsulates an OBEX PUSH client.

    The QObexPushClient class can be used to send files to an OBEX Push
    server.  The file sent can either be a business card (vCard), calendar (vCal),
    or any other file type.  This class can also be used to request a business card,
    or a business card exchange.

    Here is an example of sending a file over Bluetooth

    \code
        QBluetoothObexSocket *sock =
            new QBluetoothObexSocket("00:00:00:00:00:01", 1); // Fake address, rfcomm channel 1

        if (sock->connect()) {
            QObexPushService *sender = new QObexPushService(sock);
            sender->setAutoDelete(true);
            sender->connect();
            QByteArray data = getSomeData();
            sender->send(data, "MyData.txt");
            sender->disconnect();
        }
    \endcode

    \ingroup qtopiabluetooth
 */

static void qobex_sender_callback(obex_t *handle, obex_object_t *obj, int mode, int event, int obex_cmd, int obex_rsp);

class QObexPushClient_Private : public QObject
{
    Q_OBJECT

    friend void qobex_sender_callback(obex_t *handle, obex_object_t *obj,
                                      int mode, int event, int obex_cmd, int obex_rsp);
public:
    QObexPushClient_Private(QObexSocket *socket, QObexPushClient *parent);
    ~QObexPushClient_Private();

    void feedStream( obex_object_t *object );
    void readData( obex_object_t *object );
    void putFile();
    void getFile();
    void process( obex_object_t *object );
    void commandFinished();

    void resetState();
    void updateState(QObex::State state);

    void setAutoDelete(bool enable);
    bool autoDelete() const;

    int connect();
    int disconnect();
    int send(QIODevice *device, const QString &filename,
             const QString &mimetype = QString());
    int send(const QByteArray &array, const QString &filename,
             const QString &mimetype = QString());

    int sendBusinessCard(QIODevice *vcard);
    int requestBusinessCard(QIODevice *vcard);
    void exchangeBusinessCard(QIODevice *mine, QIODevice *theirs, int *putId, int *getId);
    int close();

    void hardAbort();
    void abort();

    int addCommand(QObexCommand *cmd);

public slots:
    void deleteMeLater();

    void processInput();
    void doPending();

signals:
    void done(bool error);
    void progress(qint64, qint64);
    void stateChanged(QObex::State);
    void commandFinished(int id, bool error);
    void commandStarted(int id);

public:
    QObex::State m_state;
    QObex::Error m_error;
    bool m_autodelete;

    QObexPushClient *m_parent;
    QQueue<QObexCommand *> m_q;
    QObexSocket *m_socket;
    obex_t *m_self;

private:
    void cleanup();

    char *m_buf;
    qint64 m_total;
    qint64 m_bytes;
};

QObexPushClient_Private::QObexPushClient_Private(QObexSocket *socket, QObexPushClient *parent) :
        QObject(parent), m_self(NULL)
{
    m_socket = socket;
    m_buf = 0;

    resetState();

    m_parent = parent;
    m_autodelete = false;
}

QObexPushClient_Private::~QObexPushClient_Private()
{
    cleanup();

    if (m_socket)
        delete m_socket;
}

void QObexPushClient_Private::setAutoDelete(bool enable)
{
    m_autodelete = enable;
}

bool QObexPushClient_Private::autoDelete() const
{
    return m_autodelete;
}

void QObexPushClient_Private::resetState()
{
    m_state = QObex::Ready;
    m_error = QObex::NoError;
}

void QObexPushClient_Private::deleteMeLater()
{
    qLog(Obex) << "QObexPushClient::Will be deleting myself later";
    if (m_parent->autoDelete()) {
        m_parent->deleteLater();
    }
}

void QObexPushClient_Private::feedStream( obex_object_t *object )
{
    Q_ASSERT(m_state == QObex::Streaming);

    qLog(Obex) <<  "QObexPushClient::Feeding Stream";

    char *streambuf = NULL;
    int len = 0;
    QObexCommand *cmd = m_q.head();

    // Handle Byte Array case
    if (cmd->m_isba) {
        streambuf = &cmd->m_data.data->data()[m_bytes];
        if ( (m_total - m_bytes) > OBEX_STREAM_BUF_SIZE)
            len = OBEX_STREAM_BUF_SIZE;
        else
            len = m_total - m_bytes;
    } else {
        streambuf = m_buf;
        len = cmd->m_data.device->read(m_buf, OBEX_STREAM_BUF_SIZE);
    }

    m_bytes += len;

    obex_headerdata_t hd;
    if ( len > 0 ) {
        hd.bs = (uchar*)streambuf;
        OBEX_ObjectAddHeader(m_self, object, OBEX_HDR_BODY,
                             hd, len, OBEX_FL_STREAM_DATA);
        qLog(Obex) << "QObexPushClient::emitting progress report" << m_bytes << "/" << m_total;
        emit progress( m_bytes, m_total );
    } else {
        /* EOF */
        hd.bs = 0;
        OBEX_ObjectAddHeader(m_self, object, OBEX_HDR_BODY,
                             hd, 0, OBEX_FL_STREAM_DATAEND);
    }
}

void QObexPushClient_Private::putFile()
{
    QObexCommand *cmd = m_q.head();

    if (cmd->m_mimetype.isEmpty())
        cmd->m_mimetype = QMimeType(cmd->m_filename).id();

    obex_object_t *object = OBEX_ObjectNew(m_self, OBEX_CMD_PUT);
    if (object == NULL) {
        qLog(Obex) << "QObexPushClient::putFile error occurred, Aborting...";
        m_error = QObex::UnknownError;
        hardAbort();
        return;
    }

    m_total = 0;
    m_bytes = 0;

    if (cmd->m_isba) {
        m_total = cmd->m_data.data->size();
    } else {
        bool error = false;
        if (cmd->m_data.device &&
            (cmd->m_data.device->isOpen() ||
             cmd->m_data.device->open(QIODevice::ReadOnly)))
        {
            if (!cmd->m_data.device->isSequential())
                m_total = cmd->m_data.device->size();
            else
                error = true;
        }
        else {
            error = true;
        }

        if (error) {
            // Couldn't open the file, mark the command as finished
            // and bail
            m_error = QObex::UnknownError;
            commandFinished();
            return;
        }
    }

    QObexHeader obexHeader;
    obexHeader.setName(cmd->m_filename);
    obexHeader.setMimeType(cmd->m_mimetype);
    obexHeader.setLength(m_total);
    setHeaders(m_self, object, obexHeader);

    obex_headerdata_t hd;
    hd.bs = 0;
    OBEX_ObjectAddHeader(m_self, object, OBEX_HDR_BODY,
                            hd, 0, OBEX_FL_STREAM_START);

    // Create the buffer (only needed in QIODevice case)
    if (!cmd->m_isba && !m_buf)
        m_buf = new char[OBEX_STREAM_BUF_SIZE];

    updateState(QObex::Streaming);

    process( object );
}

void QObexPushClient_Private::readData( obex_object_t *object )
{
    const uchar* buf;
    int len = OBEX_ObjectReadStream( m_self, object, &buf );

    qLog(Obex) <<  "QObexPushClient::Reading data";

    if (m_bytes == 0) {
        QObexHeader hdr;

        qLog(Obex) << "Gettings headers";
        getHeaders(m_self, object, hdr);

        if (hdr.mimeType() != "text/x-vCard") {
            qWarning("QObexPushClient: This is not a vCard, discarding!");
            return;
        }

        m_total = hdr.length();
    }

    QObexCommand *cmd = m_q.head();
    if (cmd->m_data.device &&
        (cmd->m_data.device->isOpen() || cmd->m_data.device->open(QIODevice::WriteOnly))) {
        updateState(QObex::Streaming);
    }
    else {
        qWarning("QObexPushClient: Could not write to QIODevice!");
    }

    if ( len > 0 ) {
        cmd->m_data.device->write( (const char*)buf, len );
        m_bytes += len;
        qLog(Obex) << "QObexPushClient::emitting progress report" << m_bytes << "/" << m_total;
        emit progress( m_bytes, m_total );
    } else if ( len == 0 ) {
        if (m_state == QObex::Streaming)
            updateState(QObex::Ready);
    }
}

void QObexPushClient_Private::getFile()
{
    QObexCommand *cmd = m_q.head();

    Q_ASSERT(!cmd->m_mimetype.isEmpty());

    obex_object_t *object = OBEX_ObjectNew(m_self, OBEX_CMD_GET);
    if (object == NULL) {
        qLog(Obex) << "QObexPushClient::getFile error occurred, Aborting...";
        m_error = QObex::UnknownError;
        hardAbort();
        return;
    }

    /* Add type header */
    obex_headerdata_t hd;
    QByteArray latinArr = cmd->m_mimetype.toLatin1();
    hd.bs = (uchar*)latinArr.constData();
    OBEX_ObjectAddHeader(m_self, object,
                         OBEX_HDR_TYPE, hd, cmd->m_mimetype.length()+1, OBEX_FL_FIT_ONE_PACKET);

    // Create the buffer (only needed in QIODevice case)
    if (!cmd->m_isba && !m_buf)
        m_buf = new char[OBEX_STREAM_BUF_SIZE];
    OBEX_ObjectReadStream(m_self, object, NULL);

    updateState(QObex::Streaming);

    m_total = 0;
    m_bytes = 0;

    process( object );
}

static void qobex_sender_callback(obex_t *handle, obex_object_t *obj, int /*mode*/,
                                  int event, int obex_cmd, int obex_rsp)
{
    QObexPushClient_Private *sender =
            (QObexPushClient_Private*)OBEX_GetUserData( handle );

    switch (event) {
        case OBEX_EV_ABORT:
            qLog(Obex) << "Got an abort";
            sender->m_error = QObex::NoError;
            sender->hardAbort();
            break;

        case OBEX_EV_REQDONE:
            qLog(Obex) << "Received REQDONE Event";
            if (obex_rsp != OBEX_RSP_SUCCESS) {
                qLog(Obex) << "Server refused PUT";
                qLog(Obex) << "Obex_rsp: " << obex_rsp;
                sender->m_error = QObex::RequestFailed;
            }
            else
                sender->m_error = QObex::NoError;
            sender->commandFinished();
            break;

        case OBEX_EV_LINKERR:
            // sometime we get a link error after we believed the connection was done.
            // This will happen if the connection was aborted, in particular.
            // Ignore this as emitting an error after done does not make sense
            if ( !sender->m_self ) {
                sender->m_error = QObex::LinkError;
                qLog(Obex) << "QObexPushClient::Link Error calling hardabort...";
                sender->hardAbort();
            }
            break;

        case OBEX_EV_STREAMEMPTY:
            // when streaming: add more
            sender->feedStream( obj );
            break;

        case OBEX_EV_STREAMAVAIL:
            sender->readData( obj );
            break;

        case OBEX_EV_REQ:
            switch(obex_cmd) {
                // a client shouldn't receive requests though
                case OBEX_CMD_ABORT:
                    sender->m_error = QObex::UnknownError;
                    sender->commandFinished();
                    break;
            }
            break;

        case OBEX_EV_REQHINT:
            /* A new request is coming in */
            switch(obex_cmd) {
                case OBEX_CMD_ABORT:
                    OBEX_ObjectSetRsp(obj, OBEX_RSP_CONTINUE, OBEX_RSP_SUCCESS);
                    break;

                default:
                    /* Reject any other commands */
                    OBEX_ObjectSetRsp(obj, OBEX_RSP_NOT_IMPLEMENTED,
                                      OBEX_RSP_NOT_IMPLEMENTED);
                    break;
            }
            break;

        default:
            break;
    };
}

void QObexPushClient_Private::processInput()
{
    // Check for a bizarre case
    //Q_ASSERT(m_state != QObexPushClient::Ready);
    Q_ASSERT(m_self);

    OBEX_HandleInput( m_self, 0 );
}

void QObexPushClient_Private::updateState(QObex::State state)
{
    qLog(Obex) << "QObexPushClient::updating state";
    if (m_state != state) {
        m_state = state;
        emit stateChanged(m_state);
    }
}

void QObexPushClient_Private::process( obex_object_t *object )
{
    if (!object)
        return;

    qLog(Obex) << "Sending request...";
    int ret = OBEX_Request(m_self, object);

    if (ret < 0) {
        qLog(Obex) << "OBEX_Request returned: " << ret;
        qLog(Obex) << "QObexPushClient::Process calling hardabort...";
        m_error = QObex::RequestFailed;
        hardAbort();
    }
}

void QObexPushClient_Private::commandFinished()
{
    QObexCommand *cmd = m_q.dequeue();

    emit commandFinished(cmd->m_id, m_error != QObex::NoError);

    if ((cmd->m_cmd == QObexCommand::CONNECT) ||
        (cmd->m_cmd == QObexCommand::GET) ||
        (cmd->m_cmd == QObexCommand::PUT)) {
        qLog(Obex) << "Command finished...";
        updateState(QObex::Ready);
    }
    else if (cmd->m_cmd == QObexCommand::DISCONNECT) {
        qLog(Obex) << "CMD_DISCONNECT finished...";
        m_socket->close();
        m_self = NULL;
        updateState(QObex::Closed);
    }

    qLog(Obex) << "Scheduling next command...";
    QTimer::singleShot(0, this, SLOT(doPending()));
}

void QObexPushClient_Private::doPending()
{
    qLog(Obex) << "QObexPushClient::doPending";

    if (m_q.size() == 0) {
        qLog(Obex) << "Emitting done with error:" << m_error;
        emit done(m_error != QObex::NoError);
        deleteMeLater();
        return;
    }

    if (!m_self && (m_q.head()->m_cmd != QObexCommand::CONNECT)) {
        QObexCommand *cmd = m_q.dequeue();
        emit commandStarted(cmd->m_id);
        m_error = QObex::RequestFailed;
        emit commandFinished(cmd->m_id, m_error != QObex::NoError);
        QTimer::singleShot(0, this, SLOT(doPending()));
        return;
    }

    switch (m_q.head()->m_cmd) {
        case QObexCommand::CONNECT:
        {
            QObexCommand *cmd = m_q.head();
            emit commandStarted(cmd->m_id);
            updateState(QObex::Connecting);

            if (!m_socket->isConnected() && !m_socket->connect()) {
                m_error = QObex::TransportConnectionError;
                emit commandFinished(cmd->m_id, m_error != QObex::NoError);
                updateState(QObex::Ready);
                m_q.dequeue();
                QTimer::singleShot(0, this, SLOT(doPending()));
                return;
            }

            m_self = static_cast<obex_t *>(m_socket->handle());

            OBEX_SetUserCallBack(m_self, qobex_sender_callback, 0);
            OBEX_SetUserData( m_self, this );

            QSocketNotifier *sn = new QSocketNotifier( OBEX_GetFD( m_self ),
                    QSocketNotifier::Read,
                    this );

            QObject::connect(sn, SIGNAL(activated(int)), this, SLOT(processInput()) );
            obex_object_t *object;

            // Send Connect request, we're still "connecting" at this stage
            object = OBEX_ObjectNew(m_self, OBEX_CMD_CONNECT);
            process( object );
            break;
        }
        case QObexCommand::DISCONNECT:
        {
            updateState(QObex::Disconnecting);
            qLog(Obex) << "QObexPushClient::sending disconnect";
            obex_object_t *object = OBEX_ObjectNew(m_self, OBEX_CMD_DISCONNECT);
            process( object );
            break;
        }
        case QObexCommand::PUT:
        {
            qLog(Obex) << "QObexPushClient::putting File";
            putFile();
            break;
        }
        case QObexCommand::GET:
        {
            qLog(Obex) << "QObexPushClient::getting business card";
            getFile();
            break;
        }
        default:
            qLog(Obex) << "Unknown command!";
    };
}

int QObexPushClient_Private::connect()
{
    return addCommand(new QObexCommand(QObexCommand::CONNECT, NULL, QString(), QString()));
}

int QObexPushClient_Private::disconnect()
{
    return addCommand(new QObexCommand(QObexCommand::DISCONNECT, NULL, QString(), QString()));
}

int QObexPushClient_Private::send(QIODevice *device, const QString &filename,
                                  const QString &mimetype)
{
    return addCommand(new QObexCommand(QObexCommand::PUT, device, filename, mimetype));
}

int QObexPushClient_Private::send(const QByteArray &array, const QString &filename,
             const QString &mimetype)
{
    return addCommand(new QObexCommand(QObexCommand::PUT, array, filename, mimetype));
}

int QObexPushClient_Private::sendBusinessCard(QIODevice *vcard)
{
    return addCommand(new QObexCommand(QObexCommand::PUT, vcard,
                      "MyBusinessCard.vcf", "text/x-vCard"));
}

int QObexPushClient_Private::requestBusinessCard(QIODevice *vcard)
{
    return addCommand(new QObexCommand(QObexCommand::GET, vcard, QString(), "text/x-vCard"));
}

void QObexPushClient_Private::exchangeBusinessCard(QIODevice *mine, QIODevice *theirs,
                                                  int *putId, int *getId )
{
    int ret;

    ret = sendBusinessCard(mine);
    if (putId)
        *putId = ret;

    ret = requestBusinessCard(theirs);
    if (getId)
        *getId = ret;
}

int QObexPushClient_Private::close()
{
    return addCommand(new QObexCommand(QObexCommand::DISCONNECT, NULL, QString(), QString()));
}

void QObexPushClient_Private::abort()
{
    // Check if we are connected.
    if (!m_self)
        return;

    qLog(Obex) << "QObexPushClient::aborting...";
    // Don't be nice (CancelRequest arg #2)
    // This sends an immediate abort event to our callback
    OBEX_CancelRequest(m_self, 0);
}

void QObexPushClient_Private::cleanup()
{
    // Delete the temp buffer
    if (m_buf) {
        delete[] m_buf;
        m_buf = 0;
    }

    // Clear the command queue
    foreach (QObexCommand *cmd, m_q) {
        delete cmd;
    }
    m_q.clear();

    // Disconnect
    m_state = QObex::Closed;
    if (m_self) {
        m_socket->close();
        m_self = NULL;
    }
}

void QObexPushClient_Private::hardAbort()
{
    qLog(Obex) << "QObexPushClient::Hard Aborting...";

    cleanup();
    emit stateChanged(QObex::Closed);

    qLog(Obex) << "m_error: " << m_error;
    emit done(m_error != QObex::NoError);

    deleteMeLater();
}

int QObexPushClient_Private::addCommand(QObexCommand *cmd)
{
    m_q.enqueue(cmd);

    if (m_q.size() == 1)
        QTimer::singleShot(0, this, SLOT(doPending()));

    return cmd->m_id;
}

/*!
    Constructs a new Bluetooth OBEX Client object.  The OBEX socket to use
    is given by \a socket.  The socket should generally be connected, however
    if it is not, the QObexSocket::connect method of the socket will be executed first.
    The \a parent parameter specifies the \c QObject parent.
*/
QObexPushClient::QObexPushClient(QObexSocket *socket, QObject *parent) : QObject(parent)
{
    m_data = new QObexPushClient_Private(socket, this);

    QObject::connect(m_data, SIGNAL(stateChanged(QObex::State)),
                     this, SIGNAL(stateChanged(QObex::State)));
    QObject::connect(m_data, SIGNAL(done(bool)),
                     this, SIGNAL(done(bool)));
    QObject::connect(m_data, SIGNAL(progress(qint64, qint64)),
                     this, SIGNAL(progress(qint64, qint64)));
    QObject::connect(m_data, SIGNAL(commandFinished(int, bool)),
                     this, SIGNAL(commandFinished(int, bool)));
    QObject::connect(m_data, SIGNAL(commandStarted(int)),
                     this, SIGNAL(commandStarted(int)));
}

/*!
    Deconstructs a Bluetooth OBEX Sender.
*/
QObexPushClient::~QObexPushClient()
{
    if (m_data)
        delete m_data;
}

/*!
    Requests the client to connect to the server.  The \c QObexSocket will
    be used to establish the connection if the socket is not yet connected.
    After this, the OBEX \bold{CONNECT} command will be sent.  This method returns
    immediately after queuing the command to be performed.  One can track
    the progress of the command by using the commandFinished and
    commandStarted.

    \sa disconnect()
*/
int QObexPushClient::connect()
{
    return m_data->connect();
}

/*!
    Queues a disconnect request.  The OBEX \bold{DISCONNECT} comand will be sent,
    after which the socket will be closed.  This method returns immediately after
    queuing the command to be performed.  One can track the progress of the command
    by using the commandFinished and commandStarted signals.

    Returns the id of the command.

    \sa connect()
*/
int QObexPushClient::disconnect()
{
    return m_data->disconnect();
}

/*!
    Initiates a new send request.  The sender will connect to the server and perform a push
    operation with the contents of \a device.  The \a filename is used for display purposes.
    If the \a mimetype is not provided (empty) then the mimetype will be guessed based on
    the file extension.

    This method returns a unique command id.
 */
int QObexPushClient::send(QIODevice *device, const QString &filename,
                          const QString &mimetype)
{
    return m_data->send(device, filename, mimetype);
}

/*!
    This is a convenience method.  It functions essentially like the above method.
    The \a array contains the contents to be pushed to the server, while \a filename
    and \a mimetype function as in the above method.
*/
int QObexPushClient::send(const QByteArray &array, const QString &filename,
                          const QString &mimetype)
{
    return m_data->send(array, filename, mimetype);
}

/*!
    Performs a push operation on a business card to the server.  The contents of the
    business card are given by \a vcard.
*/
int QObexPushClient::sendBusinessCard(QIODevice *vcard)
{
    return m_data->sendBusinessCard(vcard);
}

/*!
    Requests a business card from the server.  The received contents will be put in
    \a vcard.
 */
int QObexPushClient::requestBusinessCard(QIODevice *vcard)
{
    return m_data->requestBusinessCard(vcard);
}

/*!
    Performs a business card exchange operation, by first pushing own business card
    given by \a mine, and requesting a business card from the remote device.  The
    results will be placed in \a theirs.  The \bold PUT command id is returned in
    \a putId.  The \bold GET command id is returned in \a getId.

    Note that this method is equivalent to first calling the \c sendBusinessCard method
    and then calling the \c requestBusinessCard method.
*/
void QObexPushClient::exchangeBusinessCard(QIODevice *mine, QIODevice *theirs,
                                           int *putId, int *getId)
{
    m_data->exchangeBusinessCard(mine, theirs, putId, getId);
}

/*!
    Disconnects from the server.
*/
int QObexPushClient::close()
{
    return m_data->close();
}

/*!
    Attempts to abort the file send operation.
*/
void QObexPushClient::abort()
{
    m_data->abort();
}

/*!
    Sets the object \c autoDelete property to \a enable.  If \c autoDelete is enabled,
    then the sender object will automatically delete itself once it completes its request
    or is aborted.  If \c autoDelete is disabled, the programmer has to manually handle the
    destruction of the object.
*/
void QObexPushClient::setAutoDelete(bool enable)
{
    m_data->setAutoDelete(enable);
}

/*!
    Returns whether \c autoDelete property is enabled.
*/
bool QObexPushClient::autoDelete() const
{
    return m_data->autoDelete();
}

/*!
    Returns the error condition of the last command executed
*/
QObex::Error QObexPushClient::error() const
{
    return m_data->m_error;
}

/*!
    Returns the current state of the object
*/
QObex::State QObexPushClient::state() const
{
    return m_data->m_state;
}

/*!
    \fn void QObexPushClient::commandFinished(int id, bool error);

    This signal is emitted whenever a queued command has been performed.  The
    \a id parameter holds the id of the command finished.  The \a error parameter
    holds whether an error occurred.

    \sa commandStarted()
*/

/*!
    \fn void QObexPushClient::commandStarted(int id);

    This signal is emitted whenever a queued command has been started.  The
    \a id parameter holds the id of the command.

    \sa commandFinished()
*/

/*!
    \fn void QObexPushClient::done(bool error);

    This signal is emitted whenever all pending requests have been completed.
    The \a error parameter reports whether an error occurred during processing.
 */

/*!
    \fn void QObexPushClient::progress(qint64 completed, qint64 total);

    This signal is emitted reports the progress of the file send operation.
    The \a completed parameter reports how many bytes were sent, and \a total
    parameter reports the total number of bytes to send.
 */

/*!
    \fn void QObexPushClient::stateChanged(QObex::State state)

    This signal is emitted whenever a sender object changes state.  The \a state
    parameter holds the current state.
 */

#include "qobexpushclient.moc"
