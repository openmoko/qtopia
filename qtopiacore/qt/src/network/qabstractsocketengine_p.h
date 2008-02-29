/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtNetwork module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QABSTRACTSOCKETENGINE_P_H
#define QABSTRACTSOCKETENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtNetwork/qhostaddress.h"
#include "QtNetwork/qabstractsocket.h"
#include "private/qobject_p.h"
class QAuthenticator;

class QAbstractSocketEnginePrivate;


class QAbstractSocketEngineReceiver {
public:
    virtual ~QAbstractSocketEngineReceiver(){}
    virtual void readNotification()= 0;
    virtual void writeNotification()= 0;
    virtual void exceptionNotification()= 0;
    virtual void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator)= 0;
};

class Q_AUTOTEST_EXPORT QAbstractSocketEngine : public QObject
{
    Q_OBJECT
public:

    static QAbstractSocketEngine *createSocketEngine(const QHostAddress &address, QAbstractSocket::SocketType socketType, QObject *parent);
    static QAbstractSocketEngine *createSocketEngine(int socketDescripter, QObject *parent);

    QAbstractSocketEngine(QObject *parent = 0);

    enum SocketOption {
        NonBlockingSocketOption,
        BroadcastSocketOption,
        ReceiveBufferSocketOption,
        SendBufferSocketOption,
        AddressReusable,
        BindExclusively,
        ReceiveOutOfBandData
    };

    virtual bool initialize(QAbstractSocket::SocketType type, QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::IPv4Protocol) = 0;

    virtual bool initialize(int socketDescriptor, QAbstractSocket::SocketState socketState = QAbstractSocket::ConnectedState) = 0;

    virtual int socketDescriptor() const = 0;

    virtual bool isValid() const = 0;

    virtual bool connectToHost(const QHostAddress &address, quint16 port) = 0;
    virtual bool bind(const QHostAddress &address, quint16 port) = 0;
    virtual bool listen() = 0;
    virtual int accept() = 0;
    virtual void close() = 0;

    virtual qint64 bytesAvailable() const = 0;

    virtual qint64 read(char *data, qint64 maxlen) = 0;
    virtual qint64 write(const char *data, qint64 len) = 0;

#ifndef QT_NO_UDPSOCKET
    virtual qint64 readDatagram(char *data, qint64 maxlen, QHostAddress *addr = 0,
                                quint16 *port = 0) = 0;
    virtual qint64 writeDatagram(const char *data, qint64 len, const QHostAddress &addr,
                                 quint16 port) = 0;
    virtual bool hasPendingDatagrams() const = 0;
    virtual qint64 pendingDatagramSize() const = 0;
#endif

    virtual int option(SocketOption option) const = 0;
    virtual bool setOption(SocketOption option, int value) = 0;

    virtual bool waitForRead(int msecs = 30000, bool *timedOut = 0) const = 0;
    virtual bool waitForWrite(int msecs = 30000, bool *timedOut = 0) const = 0;
    virtual bool waitForReadOrWrite(bool *readyToRead, bool *readyToWrite,
			    bool checkRead, bool checkWrite,
			    int msecs = 30000, bool *timedOut = 0) const = 0;

    QAbstractSocket::SocketError error() const;
    QString errorString() const;
    QAbstractSocket::SocketState state() const;
    QAbstractSocket::SocketType socketType() const;
    QAbstractSocket::NetworkLayerProtocol protocol() const;

    QHostAddress localAddress() const;
    quint16 localPort() const;
    QHostAddress peerAddress() const;
    quint16 peerPort() const;

    virtual bool isReadNotificationEnabled() const = 0;
    virtual void setReadNotificationEnabled(bool enable) = 0;
    virtual bool isWriteNotificationEnabled() const = 0;
    virtual void setWriteNotificationEnabled(bool enable) = 0;
    virtual bool isExceptionNotificationEnabled() const = 0;
    virtual void setExceptionNotificationEnabled(bool enable) = 0;

public Q_SLOTS:
    void readNotification();
    void writeNotification();
    void exceptionNotification();
    void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator);

public:
    void setReceiver(QAbstractSocketEngineReceiver *receiver);
protected:
    QAbstractSocketEngine(QAbstractSocketEnginePrivate &dd, QObject* parent = 0);

    void setError(QAbstractSocket::SocketError error, const QString &errorString) const;
    void setState(QAbstractSocket::SocketState state);
    void setSocketType(QAbstractSocket::SocketType socketType);
    void setProtocol(QAbstractSocket::NetworkLayerProtocol protocol);
    void setLocalAddress(const QHostAddress &address);
    void setLocalPort(quint16 port);
    void setPeerAddress(const QHostAddress &address);
    void setPeerPort(quint16 port);

private:
    Q_DECLARE_PRIVATE(QAbstractSocketEngine)
    Q_DISABLE_COPY(QAbstractSocketEngine)
};

class QAbstractSocketEnginePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAbstractSocketEngine)
public:
    QAbstractSocketEnginePrivate();

    mutable QAbstractSocket::SocketError socketError;
    mutable bool hasSetSocketError;
    mutable QString socketErrorString;
    QAbstractSocket::SocketState socketState;
    QAbstractSocket::SocketType socketType;
    QAbstractSocket::NetworkLayerProtocol socketProtocol;
    QHostAddress localAddress;
    quint16 localPort;
    QHostAddress peerAddress;
    quint16 peerPort;
    QAbstractSocketEngineReceiver *receiver;
};


class Q_AUTOTEST_EXPORT QSocketEngineHandler
{
protected:
    QSocketEngineHandler();
    virtual ~QSocketEngineHandler();
    virtual QAbstractSocketEngine *createSocketEngine(const QHostAddress &address, QAbstractSocket::SocketType socketType, QObject *parent) = 0;
    virtual QAbstractSocketEngine *createSocketEngine(int socketDescripter, QObject *parent) = 0;

private:
    friend class QAbstractSocketEngine;
};

#endif // QABSTRACTSOCKETENGINE_P_H
