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

    Here is an example of sending a file over Bluetooth:

    \code
        QBluetoothObexSocket *sock =
            new QBluetoothObexSocket("00:00:00:00:00:01", 1); // Fake address, rfcomm channel 1

        if (sock->connect()) {
            QObexPushClient *sender = new QObexPushClient(sock);
            sender->connect();
            QByteArray data = getSomeData();
            sender->send(data, "MyData.txt");
            sender->disconnect();
        }
    \endcode

    The functions connect(), disconnect(), send(), sendBusinessCard(),
    requestBusinessCard() and exchangeBusinessCard() are all asynchronous.
    When called, they return immediately, returning a unique identifier
    for that particular operation. If the command cannot be performed
    immediately because another command is in progress, the command is
    scheduled for later execution.

    When the execution of a command starts, the commandStarted() signal is
    emitted with the identifier of the command. When it is finished, the
    commandFinished() signal is emitted with the identifier and also a bool to
    indicate whether the command finished with an error.

    The done() signal is emitted when all pending commands have finished. This
    can be used to automatically delete the client object when it has finished
    its operations, by connecting the client's done() signal to the client's
    QObject::deleteLater() slot. (The socket object can similarly be connected
    to automatically delete the socket object when the client has finished.)

    If an error occurs during the execution of one of the commands in a
    sequence of commands, all the pending commands (i.e. scheduled, but not
    yet executed commands) are cleared and no signals are emitted for them.
    In this case, the done() signal is emitted with the error argument set to
    \c true.

    \bold {Note:} Once a client has been disconnected, it cannot perform any
    more commands.

    \ingroup qtopiaobex
 */

/*!
    \enum QObexPushClient::State
    Defines the possible states for a push client.

    \value Ready The client is ready to send requests to an OBEX service. This is the default state.
    \value Connecting The client is connecting to the server.
    \value Disconnecting The client is disconnecting from the server.
    \value Streaming A file transfer operation is in progress.
    \value Closed The client has been closed and cannot perform any more commands.
 */

/*!
    \enum QObexPushClient::Error
    Defines the possible errors for a push client.

    \value NoError No error has occurred.
    \value LinkError A connection link has been interrupted. If this error occurs, the client state will change to QObexPushClient::Closed and the client cannot be used for any more operations.
    \value TransportConnectionError Error while connecting the underlying socket transport.
    \value RequestFailed The client's request was refused by the remote service, or an error occured while sending the request.
    \value Aborted The command was aborted by a call to abort().
    \value UnknownError An error other than those specified above occurred.
 */


static void qobex_sender_callback(obex_t *handle, obex_object_t *obj, int mode, int event, int obex_cmd, int obex_rsp);

class QObexPushClientPrivate : public QObject
{
    Q_OBJECT

    friend void qobex_sender_callback(obex_t *handle, obex_object_t *obj,
                                      int mode, int event, int obex_cmd, int obex_rsp);
public:
    QObexPushClientPrivate(QObexSocket *socket, QObexPushClient *parent);
    ~QObexPushClientPrivate();

    void feedStream( obex_object_t *object );
    void readData( obex_object_t *object );
    void putFile();
    void getFile();
    void process( obex_object_t *object );
    void commandFinished(QObexPushClient::Error error);

    void resetState();
    void updateState(QObexPushClient::State state);

    int connect();
    int disconnect();
    int send(QIODevice *device, const QString &filename,
             const QString &mimetype = QString());
    int send(const QByteArray &array, const QString &filename,
             const QString &mimetype = QString());

    int sendBusinessCard(QIODevice *vcard);
    int requestBusinessCard(QIODevice *vcard);
    void exchangeBusinessCard(QIODevice *mine, QIODevice *theirs, int *putId, int *getId);

    void clearPendingCommands();

    void hardAbort(QObexPushClient::Error error);
    void abort();
    void performAbort();

    int addCommand(QObexCommand *cmd);

public slots:
    void processInput();
    void doPending();

signals:
    void done(bool error);
    void progress(qint64, qint64);
    void stateChanged(QObexPushClient::State);
    void commandFinished(int id, bool error);
    void commandStarted(int id);

    void enableSocketNotifier(bool enable);

public:
    QObexPushClient::State m_state;
    QObexPushClient::Error m_error;
    bool m_aborting;

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

QObexPushClientPrivate::QObexPushClientPrivate(QObexSocket *socket, QObexPushClient *parent) :
        QObject(parent), m_self(NULL)
{
    m_socket = socket;
    m_buf = 0;

    resetState();

    m_parent = parent;
    m_aborting = false;
}

QObexPushClientPrivate::~QObexPushClientPrivate()
{
    emit enableSocketNotifier(false);
    cleanup();
}

void QObexPushClientPrivate::resetState()
{
    m_state = QObexPushClient::Ready;
    m_error = QObexPushClient::NoError;
}

void QObexPushClientPrivate::feedStream( obex_object_t *object )
{
    Q_ASSERT(m_state == QObexPushClient::Streaming);

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

void QObexPushClientPrivate::putFile()
{
    if (m_aborting) {
        performAbort();
        return;
    }

    QObexCommand *cmd = m_q.head();

    if (cmd->m_mimetype.isEmpty())
        cmd->m_mimetype = QMimeType(cmd->m_filename).id();

    obex_object_t *object = OBEX_ObjectNew(m_self, OBEX_CMD_PUT);
    if (object == NULL) {
        qLog(Obex) << "QObexPushClient::putFile error occurred, Aborting...";
        commandFinished(QObexPushClient::UnknownError);
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
            commandFinished(QObexPushClient::UnknownError);
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

    updateState(QObexPushClient::Streaming);

    process( object );
}

void QObexPushClientPrivate::readData( obex_object_t *object )
{
    if (m_aborting) {
        performAbort();
        return;
    }

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
        updateState(QObexPushClient::Streaming);
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
        if (m_state == QObexPushClient::Streaming)
            updateState(QObexPushClient::Ready);
    }
}

void QObexPushClientPrivate::getFile()
{
    QObexCommand *cmd = m_q.head();

    Q_ASSERT(!cmd->m_mimetype.isEmpty());

    obex_object_t *object = OBEX_ObjectNew(m_self, OBEX_CMD_GET);
    if (object == NULL) {
        qLog(Obex) << "QObexPushClient::getFile error occurred, Aborting...";
        commandFinished(QObexPushClient::UnknownError);
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

    updateState(QObexPushClient::Streaming);

    m_total = 0;
    m_bytes = 0;

    process( object );
}

static void qobex_sender_callback(obex_t *handle, obex_object_t *obj, int /*mode*/,
                                  int event, int obex_cmd, int obex_rsp)
{
    QObexPushClientPrivate *sender =
            (QObexPushClientPrivate*)OBEX_GetUserData( handle );

    switch (event) {
        case OBEX_EV_ABORT:
            // the abort request got a successful response
            qLog(Obex) << "Got an abort";
            sender->commandFinished(QObexPushClient::Aborted);
            break;

        case OBEX_EV_REQDONE:
            qLog(Obex) << "Received REQDONE Event" << obex_cmd;
            if (obex_rsp != OBEX_RSP_SUCCESS) {
                qLog(Obex) << "Server refused PUT";
                qLog(Obex) << "Obex_rsp: " << obex_rsp;
                sender->commandFinished(QObexPushClient::RequestFailed);
            } else {
                sender->commandFinished(QObexPushClient::NoError);
            }
            break;

        case OBEX_EV_LINKERR:
            // Got an error sending/receiving at transport level
            if (sender->m_aborting) {
                // we get linkerr if got non-success response from abort request
                sender->hardAbort(QObexPushClient::LinkError);
            } else {
                qLog(Obex) << "QObexPushClient::LinkError calling hardabort...";
                sender->hardAbort(QObexPushClient::LinkError);
            }
            break;

        case OBEX_EV_STREAMEMPTY:
            // when streaming: add more
            sender->feedStream( obj );
            break;

        case OBEX_EV_STREAMAVAIL:
            sender->readData( obj );
            break;

        default:
            break;
    };
}

void QObexPushClientPrivate::processInput()
{
    // Check for a bizarre case
    //Q_ASSERT(m_state != QObexPushClient::Ready);
    Q_ASSERT(m_self);

    OBEX_HandleInput( m_self, 0 );
}

void QObexPushClientPrivate::updateState(QObexPushClient::State state)
{
    qLog(Obex) << "QObexPushClient::updating state" << state;
    if (m_state != state) {
        m_state = state;
        emit stateChanged(m_state);
    }
}

void QObexPushClientPrivate::process( obex_object_t *object )
{
    if (!object)
        return;

    qLog(Obex) << "Sending request...";
    int ret = OBEX_Request(m_self, object);

    if (ret < 0) {
        qLog(Obex) << "OBEX_Request returned: " << ret;
        commandFinished(QObexPushClient::RequestFailed);
    }
}

void QObexPushClientPrivate::commandFinished(QObexPushClient::Error error)
{
    m_error = error;
    m_aborting = false;

    QObexCommand *cmd = m_q.dequeue();

    if ((cmd->m_cmd == QObexCommand::CONNECT) ||
        (cmd->m_cmd == QObexCommand::GET) ||
        (cmd->m_cmd == QObexCommand::PUT)) {
        qLog(Obex) << "Command finished... error:" << m_error;
        updateState(QObexPushClient::Ready);
    }
    else if (cmd->m_cmd == QObexCommand::DISCONNECT) {
        qLog(Obex) << "CMD_DISCONNECT finished...";
        updateState(QObexPushClient::Closed);
        cleanup();
    }

    emit commandFinished(cmd->m_id, m_error != QObexPushClient::NoError);

    if (m_error == QObexPushClient::NoError) {
        qLog(Obex) << "Scheduling next command...";
        QTimer::singleShot(0, this, SLOT(doPending()));

    } else {
        // the command failed, so clear all commands and emit done()
        for (int i=0; i<m_q.size(); i++)
            delete m_q[i];
        m_q.clear();

        qLog(Obex) << "emitting done" << m_error;
        emit done(m_error != QObexPushClient::NoError);
    }
}

void QObexPushClientPrivate::doPending()
{
    qLog(Obex) << "QObexPushClient::doPending";

    if (m_q.size() == 0) {
        qLog(Obex) << "Queue empty. Emitting done with error:" << m_error;
        emit done(m_error != QObexPushClient::NoError);
        return;
    }

    m_error = QObexPushClient::NoError;

    // notify the command has started
    QObexCommand *cmd = m_q.head();
    emit commandStarted(cmd->m_id);

    if (m_aborting) {
        // "abort" the current command by not sending it at all
        qLog(Obex) << "Aborting request, not sending request" << cmd->m_cmd;
        commandFinished(QObexPushClient::Aborted);
        return;
    }

    if (!m_self) {
        // connect transport socket
        if (!m_socket->isConnected() && !m_socket->connect()) {
            commandFinished(QObexPushClient::TransportConnectionError);
            return;
        }

        m_self = static_cast<obex_t *>(m_socket->handle());
        OBEX_SetUserCallBack(m_self, qobex_sender_callback, 0);
        OBEX_SetUserData( m_self, this );

        QSocketNotifier *sn = new QSocketNotifier( OBEX_GetFD( m_self ),
                QSocketNotifier::Read,
                this );
        QObject::connect(sn, SIGNAL(activated(int)), this, SLOT(processInput()));
        QObject::connect(this, SIGNAL(enableSocketNotifier(bool)),
                         sn, SLOT(setEnabled(bool)));
    }


    switch (cmd->m_cmd) {
        case QObexCommand::CONNECT:
        {
            updateState(QObexPushClient::Connecting);
            qLog(Obex) << "QObexPushClient::sending connect";
            obex_object_t *object = OBEX_ObjectNew(m_self, OBEX_CMD_CONNECT);
            process( object );
            break;
        }
        case QObexCommand::DISCONNECT:
        {
            updateState(QObexPushClient::Disconnecting);
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

int QObexPushClientPrivate::connect()
{
    return addCommand(new QObexCommand(QObexCommand::CONNECT, NULL, QString(), QString()));
}

int QObexPushClientPrivate::disconnect()
{
    return addCommand(new QObexCommand(QObexCommand::DISCONNECT, NULL, QString(), QString()));
}

int QObexPushClientPrivate::send(QIODevice *device, const QString &filename,
                                  const QString &mimetype)
{
    return addCommand(new QObexCommand(QObexCommand::PUT, device, filename, mimetype));
}

int QObexPushClientPrivate::send(const QByteArray &array, const QString &filename,
             const QString &mimetype)
{
    return addCommand(new QObexCommand(QObexCommand::PUT, array, filename, mimetype));
}

int QObexPushClientPrivate::sendBusinessCard(QIODevice *vcard)
{
    return addCommand(new QObexCommand(QObexCommand::PUT, vcard,
                      "MyBusinessCard.vcf", "text/x-vCard"));
}

int QObexPushClientPrivate::requestBusinessCard(QIODevice *vcard)
{
    return addCommand(new QObexCommand(QObexCommand::GET, vcard, QString(), "text/x-vCard"));
}

void QObexPushClientPrivate::exchangeBusinessCard(QIODevice *mine, QIODevice *theirs,
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

void QObexPushClientPrivate::clearPendingCommands()
{
    // leave the first item alone - do not delete the current operation!
    while (m_q.size() > 1)
        delete m_q.takeLast();
}

void QObexPushClientPrivate::abort()
{
    if (m_q.empty() || m_aborting) {
        qLog(Obex) << "QObexPushClient ignoring abort request";
        return;
    }

    // clear pending operations
    clearPendingCommands();

    // abort when we get our chance
    m_aborting = true;
}

void QObexPushClientPrivate::performAbort()
{
    // send Abort request
    if (OBEX_CancelRequest(m_self, 1) < 0) {
        qLog(Obex) << "OBEX_CancelRequest() failed!";
        hardAbort(QObexPushClient::UnknownError);
    }
}

void QObexPushClientPrivate::cleanup()
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

    // stateChanged() signal not emitted here - do it before calling this if
    // necessary
    m_state = QObexPushClient::Closed;
}

void QObexPushClientPrivate::hardAbort(QObexPushClient::Error error)
{
    qLog(Obex) << "QObexPushClient::Hard Aborting with error..." << error;
    m_error = error;

    if (!m_q.isEmpty()) {
        emit commandFinished(m_q.head()->m_id, m_error != QObexPushClient::NoError);
    }

    updateState(QObexPushClient::Closed);
    cleanup();

    qLog(Obex) << "emitting done" << m_error;
    emit done(m_error != QObexPushClient::NoError);
}

int QObexPushClientPrivate::addCommand(QObexCommand *cmd)
{
    m_q.enqueue(cmd);

    if (m_q.size() == 1)
        QTimer::singleShot(0, this, SLOT(doPending()));

    return cmd->m_id;
}

/*!
    Constructs a new OBEX Push Client.  The OBEX socket to use
    is given by \a socket.  The socket should generally be connected, however
    if it is not, the QObexSocket::connect() method of the socket will be executed first.
    The \a parent parameter specifies the \c QObject parent.
*/
QObexPushClient::QObexPushClient(QObexSocket *socket, QObject *parent) : QObject(parent)
{
    m_data = new QObexPushClientPrivate(socket, this);

    QObject::connect(m_data, SIGNAL(stateChanged(QObexPushClient::State)),
                     this, SIGNAL(stateChanged(QObexPushClient::State)));
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
    Deconstructs an OBEX Push Client.
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
    Aborts the current file transfer operation and deletes all scheduled
    commands.

    If there is a file transfer operation in progress, an \c ABORT command
    will be sent to the server. When the server replies that the command is
    aborted, the commandFinished() signal will be emitted with the \c error
    argument set to \c true, and the error() function will return
    QObexPushClient::Aborted.

    If there is no file transfer operation in progress, or the operation
    finishes before it can be aborted, the operation will be completed
    normally and the \c error argument for the commandFinished() signal will
    be set to \c false.

    If no other commands are started after the call to abort(), there will be
    no scheduled commands and the done() signal will be emitted.
*/
void QObexPushClient::abort()
{
    m_data->abort();
}

/*!
    Returns the last error that occurred. This is useful for finding out
    what happened when receiving a commandFinished() or done()
    signal that has the \c error argument set to \c true.

    If you start a new command, the error status is reset to \c NoError.
*/
QObexPushClient::Error QObexPushClient::error() const
{
    return m_data->m_error;
}

/*!
    Returns the current state of the object. When the state changes,
    the stateChanged() signal is emitted.

    \sa stateChanged()
*/
QObexPushClient::State QObexPushClient::state() const
{
    return m_data->m_state;
}

/*!
    Returns the identifier of the command that is being executed, or 0 if
    there is no command being executed.
 */
int QObexPushClient::currentId() const
{
    if (m_data->m_q.empty())
        return 0;

    return m_data->m_q.head()->m_id;
}

/*!
    Returns true if there are any commands scheduled that have not yet been
    executed; otherwise returns false.

    The command that is being executed is not considered as a scheduled command.

    \sa clearPendingCommands(), currentId()
 */
bool QObexPushClient::hasPendingCommands() const
{
    return m_data->m_q.size() > 1;
}

/*!
    Deletes all pending commands from the list of scheduled commands. This
    does not affect the command that is being executed. If you want to stop
    this command as well, use abort().

    \sa hasPendingCommands(), currentId()
 */
void QObexPushClient::clearPendingCommands()
{
    m_data->clearPendingCommands();
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
    \fn void QObexPushClient::stateChanged(QObexPushClient::State state)

    This signal is emitted whenever a sender object changes state.  The \a state
    parameter holds the current state.
 */

#include "qobexpushclient.moc"
