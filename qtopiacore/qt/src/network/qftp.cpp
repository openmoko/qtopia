/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//#define QFTPPI_DEBUG
//#define QFTPDTP_DEBUG

#include "qftp.h"
#include "qabstractsocket.h"

#ifndef QT_NO_FTP

#include "qcoreapplication.h"
#include "qtcpsocket.h"
#include "qurlinfo.h"
#include "qstringlist.h"
#include "qregexp.h"
#include "qtimer.h"
#include "qfileinfo.h"
#include "qhash.h"
#include "qtcpserver.h"

class QFtpPI;

/*
    The QFtpDTP (DTP = Data Transfer Process) controls all client side
    data transfer between the client and server.
*/
class QFtpDTP : public QObject
{
    Q_OBJECT

public:
    enum ConnectState {
        CsHostFound,
        CsConnected,
        CsClosed,
        CsHostNotFound,
        CsConnectionRefused
    };

    QFtpDTP(QFtpPI *p, QObject *parent = 0);

    void setData(QByteArray *);
    void setDevice(QIODevice *);
    void writeData();
    void setBytesTotal(qint64 bytes);

    bool hasError() const;
    QString errorMessage() const;
    void clearError();

    void connectToHost(const QString & host, quint16 port);
    int setupListener(const QHostAddress &address);

    QTcpSocket::SocketState state() const;
    qint64 bytesAvailable() const;
    qint64 read(char *data, qint64 maxlen);
    QByteArray readAll();

    void abortConnection();

    static bool parseDir(const QString &buffer, const QString &userName, QUrlInfo *info);

signals:
    void listInfo(const QUrlInfo&);
    void readyRead();
    void dataTransferProgress(qint64, qint64);

    void connectState(int);

private slots:
    void socketConnected();
    void socketReadyRead();
    void socketError(QAbstractSocket::SocketError);
    void socketConnectionClosed();
    void socketBytesWritten(qint64);
    void setupSocket();

private:
    void clearData();

    QTcpSocket *socket;
    QTcpServer listener;

    QFtpPI *pi;
    QString err;
    qint64 bytesDone;
    qint64 bytesTotal;
    bool callWriteData;

    // If is_ba is true, ba is used; ba is never 0.
    // Otherwise dev is used; dev can be 0 or not.
    union {
        QByteArray *ba;
        QIODevice *dev;
    } data;
    bool is_ba;

    QByteArray bytesFromSocket;
};

/**********************************************************************
 *
 * QFtpPI - Protocol Interpreter
 *
 *********************************************************************/

class QFtpPI : public QObject
{
    Q_OBJECT

public:
    QFtpPI(QObject *parent = 0);

    void connectToHost(const QString &host, quint16 port);

    bool sendCommands(const QStringList &cmds);
    bool sendCommand(const QString &cmd)
        { return sendCommands(QStringList(cmd)); }

    void clearPendingCommands();
    void abort();

    QString currentCommand() const
        { return currentCmd; }

    bool rawCommand;
    bool transferConnectionExtended;

    QFtpDTP dtp; // the PI has a DTP which is not the design of RFC 959, but it
                 // makes the design simpler this way
signals:
    void connectState(int);
    void finished(const QString&);
    void error(int, const QString&);
    void rawFtpReply(int, const QString&);

private slots:
    void hostFound();
    void connected();
    void connectionClosed();
    void delayedCloseFinished();
    void readyRead();
    void error(QAbstractSocket::SocketError);

    void dtpConnectState(int);

private:
    // the states are modelled after the generalized state diagram of RFC 959,
    // page 58
    enum State {
        Begin,
        Idle,
        Waiting,
        Success,
        Failure
    };

    enum AbortState {
        None,
        AbortStarted,
        WaitForAbortToFinish
    };

    bool processReply();
    bool startNextCmd();

    QTcpSocket commandSocket;
    QString replyText;
    char replyCode[3];
    State state;
    AbortState abortState;
    QStringList pendingCommands;
    QString currentCmd;

    bool waitForDtpToConnect;
    bool waitForDtpToClose;

    QByteArray bytesFromSocket;

    friend class QFtpDTP;
};

/**********************************************************************
 *
 * QFtpCommand implemenatation
 *
 *********************************************************************/
class QFtpCommand
{
public:
    QFtpCommand(QFtp::Command cmd, QStringList raw, const QByteArray &ba);
    QFtpCommand(QFtp::Command cmd, QStringList raw, QIODevice *dev = 0);
    ~QFtpCommand();

    int id;
    QFtp::Command command;
    QStringList rawCmds;

    // If is_ba is true, ba is used; ba is never 0.
    // Otherwise dev is used; dev can be 0 or not.
    union {
        QByteArray *ba;
        QIODevice *dev;
    } data;
    bool is_ba;

    static QBasicAtomic idCounter;
    static int nextId();
};

QBasicAtomic QFtpCommand::idCounter = Q_ATOMIC_INIT(1);
int QFtpCommand::nextId()
{
    register int id;
    for (;;) {
        id = idCounter;
        if (idCounter.testAndSet(id, id + 1))
            break;
    }
    return id;
}

QFtpCommand::QFtpCommand(QFtp::Command cmd, QStringList raw, const QByteArray &ba)
    : command(cmd), rawCmds(raw), is_ba(true)
{
    id = nextId();
    data.ba = new QByteArray(ba);
}

QFtpCommand::QFtpCommand(QFtp::Command cmd, QStringList raw, QIODevice *dev)
    : command(cmd), rawCmds(raw), is_ba(false)
{
    id = nextId();
    data.dev = dev;
}

QFtpCommand::~QFtpCommand()
{
    if (is_ba)
        delete data.ba;
}

/**********************************************************************
 *
 * QFtpDTP implemenatation
 *
 *********************************************************************/
QFtpDTP::QFtpDTP(QFtpPI *p, QObject *parent) :
    QObject(parent),
    socket(0),
    listener(this),
    pi(p),
    callWriteData(false)
{
    clearData();
    listener.setObjectName(QLatin1String("QFtpDTP active state server"));
    connect(&listener, SIGNAL(newConnection()), SLOT(setupSocket()));
}

void QFtpDTP::setData(QByteArray *ba)
{
    is_ba = true;
    data.ba = ba;
}

void QFtpDTP::setDevice(QIODevice *dev)
{
    is_ba = false;
    data.dev = dev;
}

void QFtpDTP::setBytesTotal(qint64 bytes)
{
    bytesTotal = bytes;
    bytesDone = 0;
    emit dataTransferProgress(bytesDone, bytesTotal);
}

void QFtpDTP::connectToHost(const QString & host, quint16 port)
{
    bytesFromSocket.clear();

    if (socket)
        delete socket;
    socket = new QTcpSocket(this);
    socket->setObjectName(QLatin1String("QFtpDTP Passive state socket"));
    connect(socket, SIGNAL(connected()), SLOT(socketConnected()));
    connect(socket, SIGNAL(readyRead()), SLOT(socketReadyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(socketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(disconnected()), SLOT(socketConnectionClosed()));
    connect(socket, SIGNAL(bytesWritten(qint64)), SLOT(socketBytesWritten(qint64)));

    socket->connectToHost(host, port);
}

int QFtpDTP::setupListener(const QHostAddress &address)
{
    if (!listener.isListening() && !listener.listen(address, 0))
        return -1;

    return listener.serverPort();
}

QTcpSocket::SocketState QFtpDTP::state() const
{
    return socket ? socket->state() : QTcpSocket::UnconnectedState;
}

qint64 QFtpDTP::bytesAvailable() const
{
    if (!socket || socket->state() != QTcpSocket::ConnectedState)
        return (qint64) bytesFromSocket.size();
    return socket->bytesAvailable();
}

qint64 QFtpDTP::read(char *data, qint64 maxlen)
{
    qint64 read;
    if (socket && socket->state() == QTcpSocket::ConnectedState) {
        read = socket->read(data, maxlen);
    } else {
        read = bytesFromSocket.size();
        memcpy(data, bytesFromSocket.data(), read);
        bytesFromSocket.clear();
    }

    bytesDone += read;
    return read;
}

QByteArray QFtpDTP::readAll()
{
    QByteArray tmp;
    if (socket && socket->state() == QTcpSocket::ConnectedState) {
        tmp = socket->readAll();
        bytesDone += tmp.size();
    } else {
        tmp = bytesFromSocket;
        bytesFromSocket.clear();
    }
    return tmp;
}

void QFtpDTP::writeData()
{
    if (!socket)
        return;

    if (is_ba) {
#if defined(QFTPDTP_DEBUG)
        qDebug("QFtpDTP::writeData: write %d bytes", data.ba->size());
#endif
        if (data.ba->size() == 0)
            emit dataTransferProgress(0, bytesTotal);
        else
            socket->write(data.ba->data(), data.ba->size());

        socket->close();

        clearData();
    } else if (data.dev) {
        callWriteData = false;
        const qint64 blockSize = 16*1024;
        char buf[16*1024];
        while (!data.dev->atEnd() && socket->bytesToWrite() == 0) {
            qint64 read = data.dev->read(buf, blockSize);
#if defined(QFTPDTP_DEBUG)
            qDebug("QFtpDTP::writeData: write() of size %lli bytes", read);
#endif
            socket->write(buf, read);
            if (!data.dev)
                return; // this can happen when a command is aborted
        }
        if (data.dev->atEnd()) {
            if (bytesDone == 0 && socket->bytesToWrite() == 0)
                emit dataTransferProgress(0, bytesTotal);
            socket->close();
            clearData();
        } else {
            callWriteData = true;
        }
    }
}

inline bool QFtpDTP::hasError() const
{
    return !err.isNull();
}

inline QString QFtpDTP::errorMessage() const
{
    return err;
}

inline void QFtpDTP::clearError()
{
    err.clear();
}

void QFtpDTP::abortConnection()
{
#if defined(QFTPDTP_DEBUG)
    qDebug("QFtpDTP::abortConnection, bytesAvailable == %lli",
           socket ? socket->bytesAvailable() : (qint64) 0);
#endif
    callWriteData = false;
    clearData();

    if (socket)
        socket->abort();
}

bool QFtpDTP::parseDir(const QString &buffer, const QString &userName, QUrlInfo *info)
{
    QStringList lst = buffer.simplified().split(QLatin1String(" "));

    if (lst.count() < 9)
        return false;

    QString tmp;

    // permissions
    tmp = lst[0];

    if (tmp[0] == QLatin1Char('d')) {
        info->setDir(true);
        info->setFile(false);
        info->setSymLink(false);
    } else if (tmp[0] == QLatin1Char('-')) {
        info->setDir(false);
        info->setFile(true);
        info->setSymLink(false);
    } else if (tmp[0] == QLatin1Char('l')) {
        info->setDir(true);
        info->setFile(false);
        info->setSymLink(true);
    } else {
        return false;
    }

    static int user = 0;
    static int group = 1;
    static int other = 2;
    static int readable = 0;
    static int writable = 1;
    static int executable = 2;

    bool perms[3][3];
    perms[0][0] = (tmp[1] == QLatin1Char('r'));
    perms[0][1] = (tmp[2] == QLatin1Char('w'));
    perms[0][2] = (tmp[3] == QLatin1Char('x'));
    perms[1][0] = (tmp[4] == QLatin1Char('r'));
    perms[1][1] = (tmp[5] == QLatin1Char('w'));
    perms[1][2] = (tmp[6] == QLatin1Char('x'));
    perms[2][0] = (tmp[7] == QLatin1Char('r'));
    perms[2][1] = (tmp[8] == QLatin1Char('w'));
    perms[2][2] = (tmp[9] == QLatin1Char('x'));

    // owner
    tmp = lst[2];
    info->setOwner(tmp);

    // group
    tmp = lst[3];
    info->setGroup(tmp);

    // detect permissions
    info->setWritable((userName == info->owner() && perms[user][writable]) ||
        perms[other][writable]);
    info->setReadable((userName == info->owner() && perms[user][readable]) ||
        perms[other][readable]);

    int p = 0;
    if (perms[user][readable])
        p |= QUrlInfo::ReadOwner;
    if (perms[user][writable])
        p |= QUrlInfo::WriteOwner;
    if (perms[user][executable])
        p |= QUrlInfo::ExeOwner;
    if (perms[group][readable])
        p |= QUrlInfo::ReadGroup;
    if (perms[group][writable])
        p |= QUrlInfo::WriteGroup;
    if (perms[group][executable])
        p |= QUrlInfo::ExeGroup;
    if (perms[other][readable])
        p |= QUrlInfo::ReadOther;
    if (perms[other][writable])
        p |= QUrlInfo::WriteOther;
    if (perms[other][executable])
        p |= QUrlInfo::ExeOther;
    info->setPermissions(p);

    // size
    tmp = lst[4];
    info->setSize(tmp.toLongLong());

    // date and time
    QTime time;
    QString dateStr;
    dateStr += QLatin1String("Sun ");
    lst[5][0] = lst[5][0].toUpper();
    dateStr += lst[5];
    dateStr += QLatin1Char(' ');
    dateStr += lst[6];
    dateStr += QLatin1Char(' ');

    if (lst[7].contains(QLatin1Char(':'))) {
        time = QTime(lst[7].left(2).toInt(), lst[7].right(2).toInt());
        dateStr += QString::number(QDate::currentDate().year());
    } else {
        dateStr += lst[7];
    }

#ifndef QT_NO_DATESTRING
    QDate date = QDate::fromString(dateStr);
    info->setLastModified(QDateTime(date, time));
#endif

    if (lst[7].contains(QLatin1Char(':'))) {
        // if the year-field is missing, check the modification date/time of
        // the file and compare to "now". If the file was changed in the
        // "future", also considering a possible 24 hour time zone gap, then
        // we assume it was changed a year ago.
        const int futureTolerance = 86400;
        if(info->lastModified().secsTo(QDateTime::currentDateTime()) < -futureTolerance) {
            QDateTime dt = info->lastModified();
            QDate d = dt.date();
            d.setYMD(d.year()-1, d.month(), d.day());
            dt.setDate(d);
            info->setLastModified(dt);
        }
    }

    // name
    if (info->isSymLink())
        info->setName(lst[8].trimmed());
    else {
        QString n;
        for (int i = 8; i < lst.count(); ++i)
            n += lst[i] + QLatin1Char(' ');
        n = n.trimmed();
        info->setName(n);
    }
    return true;
}

void QFtpDTP::socketConnected()
{
    bytesDone = 0;
#if defined(QFTPDTP_DEBUG)
    qDebug("QFtpDTP::connectState(CsConnected)");
#endif
    emit connectState(QFtpDTP::CsConnected);
}

void QFtpDTP::socketReadyRead()
{
    if (!socket)
        return;

    if (pi->currentCommand().isEmpty()) {
        socket->close();
#if defined(QFTPDTP_DEBUG)
        qDebug("QFtpDTP::connectState(CsClosed)");
#endif
        emit connectState(QFtpDTP::CsClosed);
        return;
    }

    if (pi->abortState == QFtpPI::AbortStarted) {
        // discard data
        socket->readAll();
        return;
    }

    if (pi->currentCommand().startsWith(QLatin1String("LIST"))) {
        while (socket->canReadLine()) {
            QUrlInfo i;
            QString line = QString::fromAscii(socket->readLine());
#if defined(QFTPDTP_DEBUG)
            qDebug("QFtpDTP read (list): '%s'", line.toLatin1().constData());
#endif
            if (parseDir(line, QLatin1String(""), &i)) {
                emit listInfo(i);
            } else {
                // some FTP servers don't return a 550 if the file or directory
                // does not exist, but rather write a text to the data socket
                // -- try to catch these cases
                if (line.endsWith(QLatin1String("No such file or directory\r\n")))
                    err = line;
            }
        }
    } else {
        if (!is_ba && data.dev) {
            QByteArray ba;
            ba.resize(socket->bytesAvailable());
            qint64 bytesRead = socket->read(ba.data(), ba.size());
            if (bytesRead < 0) {
                // a read following a readyRead() signal will
                // never fail.
                return;
            }
            ba.resize(bytesRead);
            bytesDone += bytesRead;
#if defined(QFTPDTP_DEBUG)
            qDebug("QFtpDTP read: %lli bytes (total %lli bytes)", bytesRead, bytesDone);
#endif
            emit dataTransferProgress(bytesDone, bytesTotal);
            if (data.dev)       // make sure it wasn't deleted in the slot
                data.dev->write(ba);
        } else {
#if defined(QFTPDTP_DEBUG)
            qDebug("QFtpDTP readyRead: %lli bytes available (total %lli bytes read)",
                   bytesAvailable(), bytesDone);
#endif
            emit dataTransferProgress(bytesDone+socket->bytesAvailable(), bytesTotal);
            emit readyRead();
        }
    }
}

void QFtpDTP::socketError(QAbstractSocket::SocketError e)
{
    if (e == QTcpSocket::HostNotFoundError) {
#if defined(QFTPDTP_DEBUG)
        qDebug("QFtpDTP::connectState(CsHostNotFound)");
#endif
        emit connectState(QFtpDTP::CsHostNotFound);
    } else if (e == QTcpSocket::ConnectionRefusedError) {
#if defined(QFTPDTP_DEBUG)
        qDebug("QFtpDTP::connectState(CsConnectionRefused)");
#endif
        emit connectState(QFtpDTP::CsConnectionRefused);
    }
}

void QFtpDTP::socketConnectionClosed()
{
    if (!is_ba && data.dev) {
        clearData();
    }

    bytesFromSocket = socket->readAll();
#if defined(QFTPDTP_DEBUG)
    qDebug("QFtpDTP::connectState(CsClosed)");
#endif
    emit connectState(QFtpDTP::CsClosed);
}

void QFtpDTP::socketBytesWritten(qint64 bytes)
{
    bytesDone += bytes;
#if defined(QFTPDTP_DEBUG)
    qDebug("QFtpDTP::bytesWritten(%lli)", bytesDone);
#endif
    emit dataTransferProgress(bytesDone, bytesTotal);
    if (callWriteData)
        writeData();
}

void QFtpDTP::setupSocket()
{
    socket = listener.nextPendingConnection();
    socket->setObjectName(QLatin1String("QFtpDTP Active state socket"));
    connect(socket, SIGNAL(connected()), SLOT(socketConnected()));
    connect(socket, SIGNAL(readyRead()), SLOT(socketReadyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(socketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(disconnected()), SLOT(socketConnectionClosed()));
    connect(socket, SIGNAL(bytesWritten(qint64)), SLOT(socketBytesWritten(qint64)));

    listener.close();
}

void QFtpDTP::clearData()
{
    is_ba = false;
    data.dev = 0;
}

/**********************************************************************
 *
 * QFtpPI implemenatation
 *
 *********************************************************************/
QFtpPI::QFtpPI(QObject *parent) :
    QObject(parent),
    rawCommand(false),
    transferConnectionExtended(true),
    dtp(this),
    commandSocket(0),
    state(Begin), abortState(None),
    currentCmd(QString()),
    waitForDtpToConnect(false),
    waitForDtpToClose(false)
{
    commandSocket.setObjectName(QLatin1String("QFtpPI_socket"));
    connect(&commandSocket, SIGNAL(hostFound()),
            SLOT(hostFound()));
    connect(&commandSocket, SIGNAL(connected()),
            SLOT(connected()));
    connect(&commandSocket, SIGNAL(disconnected()),
            SLOT(connectionClosed()));
    connect(&commandSocket, SIGNAL(readyRead()),
            SLOT(readyRead()));
    connect(&commandSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(error(QAbstractSocket::SocketError)));

    connect(&dtp, SIGNAL(connectState(int)),
             SLOT(dtpConnectState(int)));
}

void QFtpPI::connectToHost(const QString &host, quint16 port)
{
    emit connectState(QFtp::HostLookup);
    commandSocket.connectToHost(host, port);
}

/*
  Sends the sequence of commands \a cmds to the FTP server. When the commands
  are all done the finished() signal is emitted. When an error occurs, the
  error() signal is emitted.

  If there are pending commands in the queue this functions returns false and
  the \a cmds are not added to the queue; otherwise it returns true.
*/
bool QFtpPI::sendCommands(const QStringList &cmds)
{
    if (!pendingCommands.isEmpty())
        return false;

    if (commandSocket.state() != QTcpSocket::ConnectedState || state!=Idle) {
        emit error(QFtp::NotConnected, QFtp::tr("Not connected"));
        return true; // there are no pending commands
    }

    pendingCommands = cmds;
    startNextCmd();
    return true;
}

void QFtpPI::clearPendingCommands()
{
    pendingCommands.clear();
    dtp.abortConnection();
    currentCmd.clear();
    state = Idle;
}

void QFtpPI::abort()
{
    pendingCommands.clear();

    if (abortState != None)
        // ABOR already sent
        return;

    abortState = AbortStarted;
#if defined(QFTPPI_DEBUG)
    qDebug("QFtpPI send: ABOR");
#endif
    commandSocket.write("ABOR\r\n", 6);

    if (currentCmd.startsWith(QLatin1String("STOR ")))
        dtp.abortConnection();
}

void QFtpPI::hostFound()
{
    emit connectState(QFtp::Connecting);
}

void QFtpPI::connected()
{
    state = Begin;
#if defined(QFTPPI_DEBUG)
//    qDebug("QFtpPI state: %d [connected()]", state);
#endif
    emit connectState(QFtp::Connected);
}

void QFtpPI::connectionClosed()
{
    commandSocket.close();
    emit connectState(QFtp::Unconnected);
}

void QFtpPI::delayedCloseFinished()
{
    emit connectState(QFtp::Unconnected);
}

void QFtpPI::error(QAbstractSocket::SocketError e)
{
    if (e == QTcpSocket::HostNotFoundError) {
        emit connectState(QFtp::Unconnected);
        emit error(QFtp::HostNotFound,
                    QFtp::tr("Host %1 not found").arg(commandSocket.peerName()));
    } else if (e == QTcpSocket::ConnectionRefusedError) {
        emit connectState(QFtp::Unconnected);
        emit error(QFtp::ConnectionRefused,
                    QFtp::tr("Connection refused to host %1").arg(commandSocket.peerName()));
    }
}

void QFtpPI::readyRead()
{
    if (waitForDtpToClose)
        return;

    while (commandSocket.canReadLine()) {
        // read line with respect to line continuation
        QString line = QString::fromAscii(commandSocket.readLine());
        if (replyText.isEmpty()) {
            if (line.length() < 3) {
                // protocol error
                return;
            }
            const int lowerLimit[3] = {1,0,0};
            const int upperLimit[3] = {5,5,9};
            for (int i=0; i<3; i++) {
                replyCode[i] = line[i].digitValue();
                if (replyCode[i]<lowerLimit[i] || replyCode[i]>upperLimit[i]) {
                    // protocol error
                    return;
                }
            }
        }
        QString endOfMultiLine;
        endOfMultiLine[0] = '0' + replyCode[0];
        endOfMultiLine[1] = '0' + replyCode[1];
        endOfMultiLine[2] = '0' + replyCode[2];
        endOfMultiLine[3] = QLatin1Char(' ');
        QString lineCont(endOfMultiLine);
        lineCont[3] = QLatin1Char('-');
        QString lineLeft4 = line.left(4);

        while (lineLeft4 != endOfMultiLine) {
            if (lineLeft4 == lineCont)
                replyText += line.mid(4); // strip 'xyz-'
            else
                replyText += line;
            if (!commandSocket.canReadLine())
                return;
            line = QString::fromAscii(commandSocket.readLine());
            lineLeft4 = line.left(4);
        }
        replyText += line.mid(4); // strip reply code 'xyz '
        if (replyText.endsWith(QLatin1String("\r\n")))
            replyText.chop(2);

        if (processReply())
            replyText = QLatin1String("");
    }
}

/*
  Process a reply from the FTP server.

  Returns true if the reply was processed or false if the reply has to be
  processed at a later point.
*/
bool QFtpPI::processReply()
{
#if defined(QFTPPI_DEBUG)
//    qDebug("QFtpPI state: %d [processReply() begin]", state);
    if (replyText.length() < 400)
        qDebug("QFtpPI recv: %d %s", 100*replyCode[0]+10*replyCode[1]+replyCode[2], replyText.toLatin1().constData());
    else
        qDebug("QFtpPI recv: %d (text skipped)", 100*replyCode[0]+10*replyCode[1]+replyCode[2]);
#endif

    int replyCodeInt = 100*replyCode[0] + 10*replyCode[1] + replyCode[2];

    // process 226 replies ("Closing Data Connection") only when the data
    // connection is really closed to avoid short reads of the DTP
    if (replyCodeInt == 226 || (replyCodeInt == 250 && currentCmd.startsWith(QLatin1String("RETR")))) {
        if (dtp.state() != QTcpSocket::UnconnectedState) {
            waitForDtpToClose = true;
            return false;
        }
    }

    switch (abortState) {
        case AbortStarted:
            abortState = WaitForAbortToFinish;
            break;
        case WaitForAbortToFinish:
            abortState = None;
            return true;
        default:
            break;
    }

    // get new state
    static const State table[5] = {
        /* 1yz   2yz      3yz   4yz      5yz */
        Waiting, Success, Idle, Failure, Failure
    };
    switch (state) {
        case Begin:
            if (replyCode[0] == 1) {
                return true;
            } else if (replyCode[0] == 2) {
                state = Idle;
                emit finished(QFtp::tr("Connected to host %1").arg(commandSocket.peerName()));
                break;
            }
            // reply codes not starting with 1 or 2 are not handled.
            return true;
        case Waiting:
            if (static_cast<signed char>(replyCode[0]) < 0 || replyCode[0] > 5)
                state = Failure;
            else
#if defined(Q_OS_IRIX) && defined(Q_CC_GNU)
            {
                // work around a crash on 64 bit gcc IRIX
                State *t = (State *) table;
                state = t[replyCode[0] - 1];
            }
#else
            if (replyCodeInt == 202)
                state = Failure;
            else
                state = table[replyCode[0] - 1];
#endif
            break;
        default:
            // ignore unrequested message
            return true;
    }
#if defined(QFTPPI_DEBUG)
//    qDebug("QFtpPI state: %d [processReply() intermediate]", state);
#endif

    // special actions on certain replies
    emit rawFtpReply(replyCodeInt, replyText);
    if (rawCommand) {
        rawCommand = false;
    } else if (replyCodeInt == 227) {
        // 227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)
        // rfc959 does not define this response precisely, and gives
        // both examples where the parenthesis are used, and where
        // they are missing. We need to scan for the address and host
        // info.
        QRegExp addrPortPattern(QLatin1String("(\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+)"));
        if (addrPortPattern.indexIn(replyText) == -1) {
#if defined(QFTPPI_DEBUG)
            qDebug("QFtp: bad 227 response -- address and port information missing");
#endif
            // this error should be reported
        } else {
            QStringList lst = addrPortPattern.capturedTexts();
            QString host = lst[1] + QLatin1Char('.') + lst[2] + QLatin1Char('.') + lst[3] + QLatin1Char('.') + lst[4];
            quint16 port = (lst[5].toUInt() << 8) + lst[6].toUInt();
            waitForDtpToConnect = true;
            dtp.connectToHost(host, port);
        }
    } else if (replyCodeInt == 229) {
        // 229 Extended Passive mode OK (|||10982|)
        int portPos = replyText.indexOf(QLatin1Char('('));
        if (portPos == -1) {
#if defined(QFTPPI_DEBUG)
            qDebug("QFtp: bad 229 response -- port information missing");
#endif
            // this error should be reported
        } else {
            ++portPos;
            QChar delimiter = replyText.at(portPos);
            QStringList epsvParameters = replyText.mid(portPos).split(delimiter);

            waitForDtpToConnect = true;
            dtp.connectToHost(commandSocket.peerAddress().toString(),
                              epsvParameters.at(3).toInt());
        }

    } else if (replyCodeInt == 230) {
        if (currentCmd.startsWith(QLatin1String("USER ")) && pendingCommands.count()>0 &&
            pendingCommands.first().startsWith(QLatin1String("PASS "))) {
            // no need to send the PASS -- we are already logged in
            pendingCommands.pop_front();
        }
        // 230 User logged in, proceed.
        emit connectState(QFtp::LoggedIn);
    } else if (replyCodeInt == 213) {
        // 213 File status.
        if (currentCmd.startsWith(QLatin1String("SIZE ")))
            dtp.setBytesTotal(replyText.simplified().toLongLong());
    } else if (replyCode[0]==1 && currentCmd.startsWith(QLatin1String("STOR "))) {
        dtp.writeData();
    }

    // react on new state
    switch (state) {
        case Begin:
            // should never happen
            break;
        case Success:
            // success handling
            state = Idle;
            // no break!
        case Idle:
            if (dtp.hasError()) {
                emit error(QFtp::UnknownError, dtp.errorMessage());
                dtp.clearError();
            }
            startNextCmd();
            break;
        case Waiting:
            // do nothing
            break;
        case Failure:
            // If the EPSV or EPRT commands fail, replace them with
            // the old PASV and PORT instead and try again.
            if (currentCmd.startsWith(QLatin1String("EPSV"))) {
                transferConnectionExtended = false;
                pendingCommands.prepend(QLatin1String("PASV\r\n"));
            } else if (currentCmd.startsWith(QLatin1String("EPRT"))) {
                transferConnectionExtended = false;
                pendingCommands.prepend(QLatin1String("PORT\r\n"));
            } else {
                emit error(QFtp::UnknownError, replyText);
            }
            if (state != Waiting) {
                state = Idle;
                startNextCmd();
            }
            break;
    }
#if defined(QFTPPI_DEBUG)
//    qDebug("QFtpPI state: %d [processReply() end]", state);
#endif
    return true;
}

/*
  Starts next pending command. Returns false if there are no pending commands,
  otherwise it returns true.
*/
bool QFtpPI::startNextCmd()
{
    if (waitForDtpToConnect)
        // don't process any new commands until we are connected
        return true;

#if defined(QFTPPI_DEBUG)
    if (state != Idle)
        qDebug("QFtpPI startNextCmd: Internal error! QFtpPI called in non-Idle state %d", state);
#endif
    if (pendingCommands.isEmpty()) {
        currentCmd.clear();
        emit finished(replyText);
        return false;
    }
    currentCmd = pendingCommands.first();

    // PORT and PASV are edited in-place, depending on whether we
    // should try the extended transfer connection commands EPRT and
    // EPSV. The PORT command also triggers setting up a listener, and
    // the address/port arguments are edited in.
    if (currentCmd.startsWith(QLatin1String("PORT"))) {
        QHostAddress address = commandSocket.localAddress();

        if (transferConnectionExtended) {
            int port = dtp.setupListener(address);
            currentCmd = QLatin1String("EPRT |");
            currentCmd += (address.protocol() == QTcpSocket::IPv4Protocol) ? QLatin1Char('1') : QLatin1Char('2');
            currentCmd += QLatin1Char('|') + address.toString() + QLatin1Char('|') + QString::number(port);
            currentCmd += QLatin1Char('|');
        } else if (address.protocol() == QTcpSocket::IPv4Protocol) {
            int port = dtp.setupListener(address);
            QString portArg;
            quint32 ip = address.toIPv4Address();
            portArg += QString::number((ip & 0xff000000) >> 24);
            portArg += QLatin1Char(',') + QString::number((ip & 0xff0000) >> 16);
            portArg += QLatin1Char(',') + QString::number((ip & 0xff00) >> 8);
            portArg += QLatin1Char(',') + QString::number(ip & 0xff);
            portArg += QLatin1Char(',') + QString::number((port & 0xff00) >> 8);
            portArg += QLatin1Char(',') + QString::number(port & 0xff);

            currentCmd = QLatin1String("PORT ");
            currentCmd += portArg;
        } else {
            // No IPv6 connection can be set up with the PORT
            // command.
            return false;
        }

        currentCmd += QLatin1String("\r\n");
    } else if (currentCmd.startsWith(QLatin1String("PASV"))) {
        if (transferConnectionExtended)
            currentCmd = QLatin1String("EPSV\r\n");
    }

    pendingCommands.pop_front();
#if defined(QFTPPI_DEBUG)
    qDebug("QFtpPI send: %s", currentCmd.left(currentCmd.length()-2).toLatin1().constData());
#endif
    state = Waiting;
    commandSocket.write(currentCmd.toLatin1());
    return true;
}

void QFtpPI::dtpConnectState(int s)
{
    switch (s) {
        case QFtpDTP::CsClosed:
            if (waitForDtpToClose) {
                // there is an unprocessed reply
                if (processReply())
                    replyText = QLatin1String("");
                else
                    return;
            }
            waitForDtpToClose = false;
            readyRead();
            return;
        case QFtpDTP::CsConnected:
            waitForDtpToConnect = false;
            startNextCmd();
            return;
        case QFtpDTP::CsHostNotFound:
        case QFtpDTP::CsConnectionRefused:
            emit error(QFtp::ConnectionRefused,
                        QFtp::tr("Connection refused for data connection"));
            startNextCmd();
            return;
        default:
            return;
    }
}

/**********************************************************************
 *
 * QFtpPrivate
 *
 *********************************************************************/

#include <private/qobject_p.h>

class QFtpPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QFtp)
public:

    inline QFtpPrivate() : close_waitForStateChange(false), state(QFtp::Unconnected),
                           transferMode(QFtp::Passive), error(QFtp::NoError)
    { }

    ~QFtpPrivate() { while (!pending.isEmpty()) delete pending.takeFirst(); }

    // private slots
    void _q_startNextCommand();
    void _q_piFinished(const QString&);
    void _q_piError(int, const QString&);
    void _q_piConnectState(int);
    void _q_piFtpReply(int, const QString&);

    int addCommand(QFtpCommand *cmd);

    QFtpPI pi;
    QList<QFtpCommand *> pending;
    bool close_waitForStateChange;
    QFtp::State state;
    QFtp::TransferMode transferMode;
    QFtp::Error error;
    QString errorString;

    QString host;
    quint16 port;
    QString proxyHost;
    quint16 proxyPort;
};

int QFtpPrivate::addCommand(QFtpCommand *cmd)
{
    pending.append(cmd);

    if (pending.count() == 1) {
        // don't emit the commandStarted() signal before the ID is returned
        QTimer::singleShot(0, q_func(), SLOT(_q_startNextCommand()));
    }
    return cmd->id;
}

/**********************************************************************
 *
 * QFtp implementation
 *
 *********************************************************************/
/*!
    \class QFtp
    \brief The QFtp class provides an implementation of the client side of FTP protocol.

    \ingroup io
    \module network
    \mainclass

    The class works asynchronously, so there are no blocking
    functions. If an operation cannot be executed immediately, the
    function will still return straight away and the operation will be
    scheduled for later execution. The results of scheduled operations
    are reported via signals. This approach depends on the event loop
    being in operation.

    The operations that can be scheduled (they are called "commands"
    in the rest of the documentation) are the following:
    connectToHost(), login(), close(), list(), cd(), get(), put(),
    remove(), mkdir(), rmdir(), rename() and rawCommand().

    All of these commands return a unique identifier that allows you
    to keep track of the command that is currently being executed.
    When the execution of a command starts, the commandStarted()
    signal with the command's identifier is emitted. When the command
    is finished, the commandFinished() signal is emitted with the
    command's identifier and a bool that indicates whether the command
    finished with an error.

    In some cases, you might want to execute a sequence of commands,
    e.g. if you want to connect and login to a FTP server. This is
    simply achieved:

    \code
    QFtp *ftp = new QFtp(parent);
    ftp->connectToHost("ftp.trolltech.com");
    ftp->login();
    \endcode

    In this case two FTP commands have been scheduled. When the last
    scheduled command has finished, a done() signal is emitted with
    a bool argument that tells you whether the sequence finished with
    an error.

    If an error occurs during the execution of one of the commands in
    a sequence of commands, all the pending commands (i.e. scheduled,
    but not yet executed commands) are cleared and no signals are
    emitted for them.

    Some commands, e.g. list(), emit additional signals to report
    their results.

    Example: If you want to download the INSTALL file from Trolltech's
    FTP server, you would write this:

    \code
    ftp->connectToHost("ftp.trolltech.com");  // id == 1
    ftp->login();                             // id == 2
    ftp->cd("qt");                            // id == 3
    ftp->get("INSTALL");                      // id == 4
    ftp->close();                             // id == 5
    \endcode

    For this example the following sequence of signals is emitted
    (with small variations, depending on network traffic, etc.):

    \code
    start(1)
    stateChanged(HostLookup)
    stateChanged(Connecting)
    stateChanged(Connected)
    finished(1, false)

    start(2)
    stateChanged(LoggedIn)
    finished(2, false)

    start(3)
    finished(3, false)

    start(4)
    dataTransferProgress(0, 3798)
    dataTransferProgress(2896, 3798)
    readyRead()
    dataTransferProgress(3798, 3798)
    readyRead()
    finished(4, false)

    start(5)
    stateChanged(Closing)
    stateChanged(Unconnected)
    finished(5, false)

    done(false)
    \endcode

    The dataTransferProgress() signal in the above example is useful
    if you want to show a \link QProgressBar progressbar \endlink to
    inform the user about the progress of the download. The
    readyRead() signal tells you that there is data ready to be read.
    The amount of data can be queried then with the bytesAvailable()
    function and it can be read with the read() or readAll()
    function.

    If the login fails for the above example, the signals would look
    like this:

    \code
    start(1)
    stateChanged(HostLookup)
    stateChanged(Connecting)
    stateChanged(Connected)
    finished(1, false)

    start(2)
    finished(2, true)

    done(true)
    \endcode

    You can then get details about the error with the error() and
    errorString() functions.

    For file transfer, QFtp can use both active or passive mode, and
    it uses passive file transfer mode by default; see the
    documentation for setTransferMode() for more details about this.

    Call setProxy() to make QFtp connect via an FTP proxy server.

    The functions currentId() and currentCommand() provide more
    information about the currently executing command.

    The functions hasPendingCommands() and clearPendingCommands()
    allow you to query and clear the list of pending commands.

    If you are an experienced network programmer and want to have
    complete control you can use rawCommand() to execute arbitrary FTP
    commands.

    \warning The current version of QFtp doesn't fully support
    non-Unix FTP servers. We hope to fix this in a future version of
    Qt.

    \sa QHttp, {FTP Example}
*/


/*!
    Constructs a QFtp object with the given \a parent.
*/
QFtp::QFtp(QObject *parent)
    : QObject(*new QFtpPrivate, parent)
{
    Q_D(QFtp);
    d->errorString = tr("Unknown error");

    connect(&d->pi, SIGNAL(connectState(int)),
            SLOT(_q_piConnectState(int)));
    connect(&d->pi, SIGNAL(finished(QString)),
            SLOT(_q_piFinished(QString)));
    connect(&d->pi, SIGNAL(error(int,QString)),
            SLOT(_q_piError(int,QString)));
    connect(&d->pi, SIGNAL(rawFtpReply(int,QString)),
            SLOT(_q_piFtpReply(int,QString)));

    connect(&d->pi.dtp, SIGNAL(readyRead()),
            SIGNAL(readyRead()));
    connect(&d->pi.dtp, SIGNAL(dataTransferProgress(qint64,qint64)),
            SIGNAL(dataTransferProgress(qint64,qint64)));
    connect(&d->pi.dtp, SIGNAL(listInfo(QUrlInfo)),
            SIGNAL(listInfo(QUrlInfo)));
}

#ifdef QT3_SUPPORT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QFtp::QFtp(QObject *parent, const char *name)
    : QObject(*new QFtpPrivate, parent)
{
    Q_D(QFtp);
    setObjectName(QLatin1String(name));
    d->errorString = tr("Unknown error");

    connect(&d->pi, SIGNAL(connectState(int)),
            SLOT(_q_piConnectState(int)));
    connect(&d->pi, SIGNAL(finished(QString)),
            SLOT(_q_piFinished(QString)));
    connect(&d->pi, SIGNAL(error(int,QString)),
            SLOT(_q_piError(int,QString)));
    connect(&d->pi, SIGNAL(rawFtpReply(int,QString)),
            SLOT(_q_piFtpReply(int,QString)));

    connect(&d->pi.dtp, SIGNAL(readyRead()),
            SIGNAL(readyRead()));
    connect(&d->pi.dtp, SIGNAL(dataTransferProgress(qint64,qint64)),
            SIGNAL(dataTransferProgress(qint64,qint64)));
    connect(&d->pi.dtp, SIGNAL(listInfo(QUrlInfo)),
            SIGNAL(listInfo(QUrlInfo)));
}
#endif

/*!
    \enum QFtp::State

    This enum defines the connection state:

    \value Unconnected There is no connection to the host.
    \value HostLookup A host name lookup is in progress.
    \value Connecting An attempt to connect to the host is in progress.
    \value Connected Connection to the host has been achieved.
    \value LoggedIn Connection and user login have been achieved.
    \value Closing The connection is closing down, but it is not yet
    closed. (The state will be \c Unconnected when the connection is
    closed.)

    \sa stateChanged() state()
*/
/*!
    \enum QFtp::TransferMode

    FTP works with two socket connections; one for commands and
    another for transmitting data. While the command connection is
    always initiated by the client, the second connection can be
    initiated by either the client or the server.

    This enum defines whether the client (Passive mode) or the server
    (Active mode) should set up the data connection.

    \value Passive The client connects to the server to transmit its
    data.

    \value Active The server connects to the client to transmit its
    data.
*/
/*!
    \enum QFtp::TransferType

    This enum identifies the data transfer type used with get and
    put commands.

    \value Binary The data will be transferred in Binary mode.

    \value Ascii The data will be transferred in Ascii mode and new line
    characters will be converted to the local format.
*/
/*!
    \enum QFtp::Error

    This enum identifies the error that occurred.

    \value NoError No error occurred.
    \value HostNotFound The host name lookup failed.
    \value ConnectionRefused The server refused the connection.
    \value NotConnected Tried to send a command, but there is no connection to
    a server.
    \value UnknownError An error other than those specified above
    occurred.

    \sa error()
*/

/*!
    \enum QFtp::Command

    This enum is used as the return value for the currentCommand() function.
    This allows you to perform specific actions for particular
    commands, e.g. in a FTP client, you might want to clear the
    directory view when a list() command is started; in this case you
    can simply check in the slot connected to the start() signal if
    the currentCommand() is \c List.

    \value None No command is being executed.
    \value SetTransferMode set the \link TransferMode transfer\endlink mode.
    \value SetProxy switch proxying on or off.
    \value ConnectToHost connectToHost() is being executed.
    \value Login login() is being executed.
    \value Close close() is being executed.
    \value List list() is being executed.
    \value Cd cd() is being executed.
    \value Get get() is being executed.
    \value Put put() is being executed.
    \value Remove remove() is being executed.
    \value Mkdir mkdir() is being executed.
    \value Rmdir rmdir() is being executed.
    \value Rename rename() is being executed.
    \value RawCommand rawCommand() is being executed.

    \sa currentCommand()
*/

/*!
    \fn void QFtp::stateChanged(int state)

    This signal is emitted when the state of the connection changes.
    The argument \a state is the new state of the connection; it is
    one of the \l State values.

    It is usually emitted in response to a connectToHost() or close()
    command, but it can also be emitted "spontaneously", e.g. when the
    server closes the connection unexpectedly.

    \sa connectToHost() close() state() State
*/

/*!
    \fn void QFtp::listInfo(const QUrlInfo &i);

    This signal is emitted for each directory entry the list() command
    finds. The details of the entry are stored in \a i.

    \sa list()
*/

/*!
    \fn void QFtp::commandStarted(int id)

    This signal is emitted when processing the command identified by
    \a id starts.

    \sa commandFinished() done()
*/

/*!
    \fn void QFtp::commandFinished(int id, bool error)

    This signal is emitted when processing the command identified by
    \a id has finished. \a error is true if an error occurred during
    the processing; otherwise \a error is false.

    \sa commandStarted() done() error() errorString()
*/

/*!
    \fn void QFtp::done(bool error)

    This signal is emitted when the last pending command has finished;
    (it is emitted after the last command's commandFinished() signal).
    \a error is true if an error occurred during the processing;
    otherwise \a error is false.

    \sa commandFinished() error() errorString()
*/

/*!
    \fn void QFtp::readyRead()

    This signal is emitted in response to a get() command when there
    is new data to read.

    If you specify a device as the second argument in the get()
    command, this signal is \e not emitted; instead the data is
    written directly to the device.

    You can read the data with the readAll() or read() functions.

    This signal is useful if you want to process the data in chunks as
    soon as it becomes available. If you are only interested in the
    complete data, just connect to the commandFinished() signal and
    read the data then instead.

    \sa get() read() readAll() bytesAvailable()
*/

/*!
    \fn void QFtp::dataTransferProgress(qint64 done, qint64 total)

    This signal is emitted in response to a get() or put() request to
    indicate the current progress of the download or upload.

    \a done is the amount of data that has already been transferred
    and \a total is the total amount of data to be read or written. It
    is possible that the QFtp class is not able to determine the total
    amount of data that should be transferred, in which case \a total
    is 0. (If you connect this signal to a QProgressBar, the progress
    bar shows a busy indicator if the total is 0).

    \warning \a done and \a total are not necessarily the size in
    bytes, since for large files these values might need to be
    "scaled" to avoid overflow.

    \sa get(), put(), QProgressBar
*/

/*!
    \fn void QFtp::rawCommandReply(int replyCode, const QString &detail);

    This signal is emitted in response to the rawCommand() function.
    \a replyCode is the 3 digit reply code and \a detail is the text
    that follows the reply code.

    \sa rawCommand()
*/

/*!
    Connects to the FTP server \a host using port \a port.

    The stateChanged() signal is emitted when the state of the
    connecting process changes, e.g. to \c HostLookup, then \c
    Connecting, then \c Connected.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa stateChanged() commandStarted() commandFinished()
*/
int QFtp::connectToHost(const QString &host, quint16 port)
{
    d_func()->pi.transferConnectionExtended = true;
    QStringList cmds;
    cmds << host;
    cmds << QString::number((uint)port);
    return d_func()->addCommand(new QFtpCommand(ConnectToHost, cmds));
}

/*!
    Logs in to the FTP server with the username \a user and the
    password \a password.

    The stateChanged() signal is emitted when the state of the
    connecting process changes, e.g. to \c LoggedIn.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::login(const QString &user, const QString &password)
{
    QStringList cmds;
    cmds << (QLatin1String("USER ") + (user.isNull() ? QLatin1String("anonymous") : user) + QLatin1String("\r\n"));
    cmds << (QLatin1String("PASS ") + (password.isNull() ? QLatin1String("anonymous@") : password) + QLatin1String("\r\n"));
    return d_func()->addCommand(new QFtpCommand(Login, cmds));
}

/*!
    Closes the connection to the FTP server.

    The stateChanged() signal is emitted when the state of the
    connecting process changes, e.g. to \c Closing, then \c
    Unconnected.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa stateChanged() commandStarted() commandFinished()
*/
int QFtp::close()
{
    return d_func()->addCommand(new QFtpCommand(Close, QStringList(QLatin1String("QUIT\r\n"))));
}

/*!
    Sets the current FTP transfer mode to \a mode. The default is QFtp::Passive.

    \sa QFtp::TransferMode
*/
int QFtp::setTransferMode(TransferMode mode)
{
    d_func()->pi.transferConnectionExtended = true;
    d_func()->transferMode = mode;
    return d_func()->addCommand(new QFtpCommand(SetTransferMode, QStringList()));
}

/*!
    Enables use of the FTP proxy on host \a host and port \a
    port. Calling this function with \a host empty disables proxying.

    QFtp does not support FTP-over-HTTP proxy servers. Use QHttp for
    this.
*/
int QFtp::setProxy(const QString &host, quint16 port)
{
    QStringList args;
    args << host << QString::number(port);
    return d_func()->addCommand(new QFtpCommand(SetProxy, args));
}

/*!
    Lists the contents of directory \a dir on the FTP server. If \a
    dir is empty, it lists the contents of the current directory.

    The listInfo() signal is emitted for each directory entry found.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa listInfo() commandStarted() commandFinished()
*/
int QFtp::list(const QString &dir)
{
    QStringList cmds;
    cmds << QLatin1String("TYPE A\r\n");
    cmds << QLatin1String(d_func()->transferMode == Passive ? "PASV\r\n" : "PORT\r\n");
    if (dir.isEmpty())
        cmds << QLatin1String("LIST\r\n");
    else
        cmds << (QLatin1String("LIST ") + dir + QLatin1String("\r\n"));
    return d_func()->addCommand(new QFtpCommand(List, cmds));
}

/*!
    Changes the working directory of the server to \a dir.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::cd(const QString &dir)
{
    return d_func()->addCommand(new QFtpCommand(Cd, QStringList(QLatin1String("CWD ") + dir + QLatin1String("\r\n"))));
}

/*!
    Downloads the file \a file from the server.

    If \a dev is 0, then the readyRead() signal is emitted when there
    is data available to read. You can then read the data with the
    read() or readAll() functions.

    If \a dev is not 0, the data is written directly to the device \a
    dev. Make sure that the \a dev pointer is valid for the duration
    of the operation (it is safe to delete it when the
    commandFinished() signal is emitted). In this case the readyRead()
    signal is \e not emitted and you cannot read data with the
    read() or readAll() functions.

    If you don't read the data immediately it becomes available, i.e.
    when the readyRead() signal is emitted, it is still available
    until the next command is started.

    For example, if you want to present the data to the user as soon
    as there is something available, connect to the readyRead() signal
    and read the data immediately. On the other hand, if you only want
    to work with the complete data, you can connect to the
    commandFinished() signal and read the data when the get() command
    is finished.

    The data is transferred as Binary or Ascii depending on the value
    of \a type.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa readyRead() dataTransferProgress() commandStarted()
    commandFinished()
*/
int QFtp::get(const QString &file, QIODevice *dev, TransferType type)
{
    QStringList cmds;
    cmds << QLatin1String("SIZE ") + file + QLatin1String("\r\n");
    if (type == Binary)
        cmds << QLatin1String("TYPE I\r\n");
    else
        cmds << QLatin1String("TYPE A\r\n");
    cmds << QLatin1String(d_func()->transferMode == Passive ? "PASV\r\n" : "PORT\r\n");
    cmds << QLatin1String("RETR ") + file + QLatin1String("\r\n");
    return d_func()->addCommand(new QFtpCommand(Get, cmds, dev));
}

/*!
    \overload

    Writes a copy of the given \a data to the file called \a file on
    the server. The progress of the upload is reported by the
    dataTransferProgress() signal.

    The data is transferred as Binary or Ascii depending on the value
    of \a type.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    Since this function takes a copy of the \a data, you can discard
    your own copy when this function returns.

    \sa dataTransferProgress() commandStarted() commandFinished()
*/
int QFtp::put(const QByteArray &data, const QString &file, TransferType type)
{
    QStringList cmds;
    if (type == Binary)
        cmds << QLatin1String("TYPE I\r\n");
    else
        cmds << QLatin1String("TYPE A\r\n");
    cmds << QLatin1String(d_func()->transferMode == Passive ? "PASV\r\n" : "PORT\r\n");
    cmds << QLatin1String("ALLO ") + QString::number(data.size()) + QLatin1String("\r\n");
    cmds << QLatin1String("STOR ") + file + QLatin1String("\r\n");
    return d_func()->addCommand(new QFtpCommand(Put, cmds, data));
}

/*!
    Reads the data from the IO device \a dev, and writes it to the
    file called \a file on the server. The data is read in chunks from
    the IO device, so this overload allows you to transmit large
    amounts of data without the need to read all the data into memory
    at once.

    The data is transferred as Binary or Ascii depending on the value
    of \a type.

    Make sure that the \a dev pointer is valid for the duration of the
    operation (it is safe to delete it when the commandFinished() is
    emitted).
*/
int QFtp::put(QIODevice *dev, const QString &file, TransferType type)
{
    QStringList cmds;
    if (type == Binary)
        cmds << QLatin1String("TYPE I\r\n");
    else
        cmds << QLatin1String("TYPE A\r\n");
    cmds << QLatin1String(d_func()->transferMode == Passive ? "PASV\r\n" : "PORT\r\n");
    if (!dev->isSequential())
        cmds << QLatin1String("ALLO ") + QString::number(dev->size()) + QLatin1String("\r\n");
    cmds << QLatin1String("STOR ") + file + QLatin1String("\r\n");
    return d_func()->addCommand(new QFtpCommand(Put, cmds, dev));
}

/*!
    Deletes the file called \a file from the server.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::remove(const QString &file)
{
    return d_func()->addCommand(new QFtpCommand(Remove, QStringList(QLatin1String("DELE ") + file + QLatin1String("\r\n"))));
}

/*!
    Creates a directory called \a dir on the server.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::mkdir(const QString &dir)
{
    return d_func()->addCommand(new QFtpCommand(Mkdir, QStringList(QLatin1String("MKD ") + dir + QLatin1String("\r\n"))));
}

/*!
    Removes the directory called \a dir from the server.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::rmdir(const QString &dir)
{
    return d_func()->addCommand(new QFtpCommand(Rmdir, QStringList(QLatin1String("RMD ") + dir + QLatin1String("\r\n"))));
}

/*!
    Renames the file called \a oldname to \a newname on the server.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::rename(const QString &oldname, const QString &newname)
{
    QStringList cmds;
    cmds << QLatin1String("RNFR ") + oldname + QLatin1String("\r\n");
    cmds << QLatin1String("RNTO ") + newname + QLatin1String("\r\n");
    return d_func()->addCommand(new QFtpCommand(Rename, cmds));
}

/*!
    Sends the raw FTP command \a command to the FTP server. This is
    useful for low-level FTP access. If the operation you wish to
    perform has an equivalent QFtp function, we recommend using the
    function instead of raw FTP commands since the functions are
    easier and safer.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa rawCommandReply() commandStarted() commandFinished()
*/
int QFtp::rawCommand(const QString &command)
{
    QString cmd = command.trimmed() + QLatin1String("\r\n");
    return d_func()->addCommand(new QFtpCommand(RawCommand, QStringList(cmd)));
}

/*!
    Returns the number of bytes that can be read from the data socket
    at the moment.

    \sa get() readyRead() read() readAll()
*/
qint64 QFtp::bytesAvailable() const
{
    return d_func()->pi.dtp.bytesAvailable();
}

/*! \fn qint64 QFtp::readBlock(char *data, quint64 maxlen)

    Use read() instead.
*/

/*!
    Reads \a maxlen bytes from the data socket into \a data and
    returns the number of bytes read. Returns -1 if an error occurred.

    \sa get() readyRead() bytesAvailable() readAll()
*/
qint64 QFtp::read(char *data, qint64 maxlen)
{
    return d_func()->pi.dtp.read(data, maxlen);
}

/*!
    Reads all the bytes available from the data socket and returns
    them.

    \sa get() readyRead() bytesAvailable() read()
*/
QByteArray QFtp::readAll()
{
    return d_func()->pi.dtp.readAll();
}

/*!
    Aborts the current command and deletes all scheduled commands.

    If there is an unfinished command (i.e. a command for which the
    commandStarted() signal has been emitted, but for which the
    commandFinished() signal has not been emitted), this function
    sends an \c ABORT command to the server. When the server replies
    that the command is aborted, the commandFinished() signal with the
    \c error argument set to \c true is emitted for the command. Due
    to timing issues, it is possible that the command had already
    finished before the abort request reached the server, in which
    case, the commandFinished() signal is emitted with the \c error
    argument set to \c false.

    For all other commands that are affected by the abort(), no
    signals are emitted.

    If you don't start further FTP commands directly after the
    abort(), there won't be any scheduled commands and the done()
    signal is emitted.

    \warning Some FTP servers, for example the BSD FTP daemon (version
    0.3), wrongly return a positive reply even when an abort has
    occurred. For these servers the commandFinished() signal has its
    error flag set to \c false, even though the command did not
    complete successfully.

    \sa clearPendingCommands()
*/
void QFtp::abort()
{
    if (d_func()->pending.isEmpty())
        return;

    clearPendingCommands();
    d_func()->pi.abort();
}

/*!
    Returns the identifier of the FTP command that is being executed
    or 0 if there is no command being executed.

    \sa currentCommand()
*/
int QFtp::currentId() const
{
    if (d_func()->pending.isEmpty())
        return 0;
    return d_func()->pending.first()->id;
}

/*!
    Returns the command type of the FTP command being executed or \c
    None if there is no command being executed.

    \sa currentId()
*/
QFtp::Command QFtp::currentCommand() const
{
    if (d_func()->pending.isEmpty())
        return None;
    return d_func()->pending.first()->command;
}

/*!
    Returns the QIODevice pointer that is used by the FTP command to read data
    from or store data to. If there is no current FTP command being executed or
    if the command does not use an IO device, this function returns 0.

    This function can be used to delete the QIODevice in the slot connected to
    the commandFinished() signal.

    \sa get() put()
*/
QIODevice* QFtp::currentDevice() const
{
    if (d_func()->pending.isEmpty())
        return 0;
    QFtpCommand *c = d_func()->pending.first();
    if (c->is_ba)
        return 0;
    return c->data.dev;
}

/*!
    Returns true if there are any commands scheduled that have not yet
    been executed; otherwise returns false.

    The command that is being executed is \e not considered as a
    scheduled command.

    \sa clearPendingCommands() currentId() currentCommand()
*/
bool QFtp::hasPendingCommands() const
{
    return d_func()->pending.count() > 1;
}

/*!
    Deletes all pending commands from the list of scheduled commands.
    This does not affect the command that is being executed. If you
    want to stop this this as well, use abort().

    \sa hasPendingCommands() abort()
*/
void QFtp::clearPendingCommands()
{
    // delete all entires except the first one
    while (d_func()->pending.count() > 1)
        delete d_func()->pending.takeLast();
}

/*!
    Returns the current state of the object. When the state changes,
    the stateChanged() signal is emitted.

    \sa State stateChanged()
*/
QFtp::State QFtp::state() const
{
    return d_func()->state;
}

/*!
    Returns the last error that occurred. This is useful to find out
    what went wrong when receiving a commandFinished() or a done()
    signal with the \c error argument set to \c true.

    If you start a new command, the error status is reset to \c NoError.
*/
QFtp::Error QFtp::error() const
{
    return d_func()->error;
}

/*!
    Returns a human-readable description of the last error that
    occurred. This is useful for presenting a error message to the
    user when receiving a commandFinished() or a done() signal with
    the \c error argument set to \c true.

    The error string is often (but not always) the reply from the
    server, so it is not always possible to translate the string. If
    the message comes from Qt, the string has already passed through
    tr().
*/
QString QFtp::errorString() const
{
    return d_func()->errorString;
}

/*! \internal
*/
void QFtpPrivate::_q_startNextCommand()
{
    Q_Q(QFtp);
    if (pending.isEmpty())
        return;
    QFtpCommand *c = pending.first();

    error = QFtp::NoError;
    errorString = QT_TRANSLATE_NOOP(QFtp, QLatin1String("Unknown error"));

    if (q->bytesAvailable())
        q->readAll(); // clear the data
    emit q->commandStarted(c->id);

    // Proxy support, replace the Login argument in place, then fall
    // through.
    if (c->command == QFtp::Login && !proxyHost.isEmpty()) {
        QString loginString = c->rawCmds.first().trimmed();
        loginString += QLatin1Char('@') + host;
        if (port && port != 21)
            loginString += QLatin1Char(':') + QString::number(port);
        loginString += QLatin1String("\r\n");
        c->rawCmds[0] = loginString;
    }

    if (c->command == QFtp::SetTransferMode) {
        _q_piFinished(QLatin1String("Transfer mode set"));
    } else if (c->command == QFtp::SetProxy) {
        proxyHost = c->rawCmds[0];
        proxyPort = c->rawCmds[1].toUInt();
        c->rawCmds.clear();
        _q_piFinished(QLatin1String("Proxy set to ") + proxyHost + QLatin1Char(':') + QString::number(proxyPort));
    } else if (c->command == QFtp::ConnectToHost) {
        if (!proxyHost.isEmpty()) {
            host = c->rawCmds[0];
            port = c->rawCmds[1].toUInt();
            pi.connectToHost(proxyHost, proxyPort);
        } else {
            pi.connectToHost(c->rawCmds[0], c->rawCmds[1].toUInt());
        }
    } else {
        if (c->command == QFtp::Put) {
            if (c->is_ba) {
                pi.dtp.setData(c->data.ba);
                pi.dtp.setBytesTotal(c->data.ba->size());
            } else if (c->data.dev && (c->data.dev->isOpen() || c->data.dev->open(QIODevice::ReadOnly))) {
                pi.dtp.setDevice(c->data.dev);
                if (c->data.dev->isSequential())
                    pi.dtp.setBytesTotal(0);
                else
                    pi.dtp.setBytesTotal(c->data.dev->size());
            }
        } else if (c->command == QFtp::Get) {
            if (!c->is_ba && c->data.dev) {
                pi.dtp.setDevice(c->data.dev);
            }
        } else if (c->command == QFtp::Close) {
            state = QFtp::Closing;
            emit q->stateChanged(state);
        }
        pi.sendCommands(c->rawCmds);
    }
}

/*! \internal
*/
void QFtpPrivate::_q_piFinished(const QString&)
{
    if (pending.isEmpty())
        return;
    QFtpCommand *c = pending.first();

    if (c->command == QFtp::Close) {
        // The order of in which the slots are called is arbitrary, so
        // disconnect the SIGNAL-SIGNAL temporary to make sure that we
        // don't get the commandFinished() signal before the stateChanged()
        // signal.
        if (state != QFtp::Unconnected) {
            close_waitForStateChange = true;
            return;
        }
    }
    emit q_func()->commandFinished(c->id, false);
    pending.removeFirst();

    delete c;

    if (pending.isEmpty()) {
        emit q_func()->done(false);
    } else {
        _q_startNextCommand();
    }
}

/*! \internal
*/
void QFtpPrivate::_q_piError(int errorCode, const QString &text)
{
    Q_Q(QFtp);
    QFtpCommand *c = pending.first();

    // non-fatal errors
    if (c->command == QFtp::Get && pi.currentCommand().startsWith(QLatin1String("SIZE "))) {
        pi.dtp.setBytesTotal(-1);
        return;
    } else if (c->command==QFtp::Put && pi.currentCommand().startsWith(QLatin1String("ALLO "))) {
        return;
    }

    error = QFtp::Error(errorCode);
    switch (q->currentCommand()) {
        case QFtp::ConnectToHost:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Connecting to host failed:\n%1"))
                          .arg(text);
            break;
        case QFtp::Login:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Login failed:\n%1"))
                          .arg(text);
            break;
        case QFtp::List:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Listing directory failed:\n%1"))
                          .arg(text);
            break;
        case QFtp::Cd:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Changing directory failed:\n%1"))
                          .arg(text);
            break;
        case QFtp::Get:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Downloading file failed:\n%1"))
                          .arg(text);
            break;
        case QFtp::Put:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Uploading file failed:\n%1"))
                          .arg(text);
            break;
        case QFtp::Remove:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Removing file failed:\n%1"))
                          .arg(text);
            break;
        case QFtp::Mkdir:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Creating directory failed:\n%1"))
                          .arg(text);
            break;
        case QFtp::Rmdir:
            errorString = QString::fromLatin1(QT_TRANSLATE_NOOP("QFtp", "Removing directory failed:\n%1"))
                          .arg(text);
            break;
        default:
            errorString = text;
            break;
    }

    pi.clearPendingCommands();
    q->clearPendingCommands();
    emit q->commandFinished(c->id, true);

    pending.removeFirst();
    delete c;
    if (pending.isEmpty())
        emit q->done(true);
    else
        _q_startNextCommand();
}

/*! \internal
*/
void QFtpPrivate::_q_piConnectState(int connectState)
{
    state = QFtp::State(connectState);
    emit q_func()->stateChanged(state);
    if (close_waitForStateChange) {
        close_waitForStateChange = false;
        _q_piFinished(QLatin1String(QT_TRANSLATE_NOOP("QFtp", "Connection closed")));
    }
}

/*! \internal
*/
void QFtpPrivate::_q_piFtpReply(int code, const QString &text)
{
    if (q_func()->currentCommand() == QFtp::RawCommand) {
        pi.rawCommand = true;
        emit q_func()->rawCommandReply(code, text);
    }
}

/*!
    Destructor.
*/
QFtp::~QFtp()
{
    abort();
    close();
}

#include "qftp.moc"

#include "moc_qftp.cpp"

#endif // QT_NO_FTP
