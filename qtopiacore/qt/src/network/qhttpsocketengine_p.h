/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
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
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QHTTPSOCKETENGINE_P_H
#define QHTTPSOCKETENGINE_P_H

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

#include "qabstractsocketengine_p.h"
#include "qabstractsocket.h"
#include "qnetworkproxy.h"
#include "qauthenticator_p.h"

#if !defined(QT_NO_NETWORKPROXY) && !defined(QT_NO_HTTP)

class QTcpSocket;
class QHttpSocketEnginePrivate;

class Q_AUTOTEST_EXPORT QHttpSocketEngine : public QAbstractSocketEngine
{
    Q_OBJECT
public:
    enum HttpState {
        None,
        ConnectSent,
        Connected,
        SendAuthentication,
        ReadResponseContent
    };
    QHttpSocketEngine(QObject *parent = 0);
    ~QHttpSocketEngine();

    bool initialize(QAbstractSocket::SocketType type, QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::IPv4Protocol);
    bool initialize(int socketDescriptor, QAbstractSocket::SocketState socketState = QAbstractSocket::ConnectedState);

    void setProxy(const QNetworkProxy &networkProxy);

    int socketDescriptor() const;

    bool isValid() const;

    bool connectToHost(const QHostAddress &address, quint16 port);
    bool bind(const QHostAddress &address, quint16 port);
    bool listen();
    int accept();
    void close();

    qint64 bytesAvailable() const;

    qint64 read(char *data, qint64 maxlen);
    qint64 write(const char *data, qint64 len);

#ifndef QT_NO_UDPSOCKET
    qint64 readDatagram(char *data, qint64 maxlen, QHostAddress *addr = 0,
        quint16 *port = 0);
    qint64 writeDatagram(const char *data, qint64 len, const QHostAddress &addr,
        quint16 port);
    bool hasPendingDatagrams() const;
    qint64 pendingDatagramSize() const;
#endif // QT_NO_UDPSOCKET

    int option(SocketOption option) const;
    bool setOption(SocketOption option, int value);

    bool waitForRead(int msecs = 30000, bool *timedOut = 0) const;
    bool waitForWrite(int msecs = 30000, bool *timedOut = 0) const;
    bool waitForReadOrWrite(bool *readyToRead, bool *readyToWrite,
                            bool checkRead, bool checkWrite,
                            int msecs = 30000, bool *timedOut = 0) const;

    bool isReadNotificationEnabled() const;
    void setReadNotificationEnabled(bool enable);
    bool isWriteNotificationEnabled() const;
    void setWriteNotificationEnabled(bool enable);
    bool isExceptionNotificationEnabled() const;
    void setExceptionNotificationEnabled(bool enable);

public slots:
    void slotSocketConnected();
    void slotSocketDisconnected();
    void slotSocketReadNotification();
    void slotSocketBytesWritten();
    void slotSocketError(QAbstractSocket::SocketError error);
    void slotSocketStateChanged(QAbstractSocket::SocketState state);

private slots:
    void emitPendingReadNotification();
    void emitPendingWriteNotification();

private:
    void emitReadNotification();
    void emitWriteNotification();

    Q_DECLARE_PRIVATE(QHttpSocketEngine)
    Q_DISABLE_COPY(QHttpSocketEngine)

};


class QHttpSocketEnginePrivate : public QAbstractSocketEnginePrivate
{
    Q_DECLARE_PUBLIC(QHttpSocketEngine)
public:
    QHttpSocketEnginePrivate();
    ~QHttpSocketEnginePrivate();

    QNetworkProxy proxy;
    QTcpSocket *socket;
    QByteArray readBuffer;
    QHttpSocketEngine::HttpState state;
    QAuthenticator authenticator;
    bool readNotificationEnabled;
    bool writeNotificationEnabled;
    bool exceptNotificationEnabled;
    bool readNotificationActivated;
    bool writeNotificationActivated;
    bool readNotificationPending;
    bool writeNotificationPending;
    uint pendingResponseData;
};

class Q_AUTOTEST_EXPORT QHttpSocketEngineHandler : public QSocketEngineHandler
{
public:
    virtual QAbstractSocketEngine *createSocketEngine(const QHostAddress &address, QAbstractSocket::SocketType socketType, QObject *parent);
    virtual QAbstractSocketEngine *createSocketEngine(int socketDescripter, QObject *parent);
};
#endif

#endif // QHTTPSOCKETENGINE_H
