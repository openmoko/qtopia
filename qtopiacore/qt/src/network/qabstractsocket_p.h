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
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QABSTRACTSOCKET_P_H
#define QABSTRACTSOCKET_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "QtNetwork/qabstractsocket.h"
#include "QtCore/qbytearray.h"
#include "QtCore/qlist.h"
#include "QtCore/qtimer.h"
#include "private/qringbuffer_p.h"
#include "private/qiodevice_p.h"
#include "private/qnativesocketengine_p.h"
#include "qnetworkproxy.h"

class QHostInfo;

class QAbstractSocketPrivate : public QIODevicePrivate, public QAbstractSocketEngineReceiver
{
    Q_DECLARE_PUBLIC(QAbstractSocket)
public:
    QAbstractSocketPrivate();
    virtual ~QAbstractSocketPrivate();

    // from QAbstractSocketEngineReceiver
    inline void readNotification() { canReadNotification(); }
    inline void writeNotification() { canWriteNotification(); }
    inline void exceptionNotification() {}
    inline void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator) {
        Q_Q(QAbstractSocket);
        q->proxyAuthenticationRequired(proxy, authenticator);
    }

    bool canReadNotification();
    bool canWriteNotification();

    // slots
    void _q_connectToNextAddress();
    void _q_startConnecting(const QHostInfo &hostInfo);
    void _q_testConnection();
    void _q_abortConnectionAttempt();

    bool readSocketNotifierCalled;
    bool readSocketNotifierState;
    bool readSocketNotifierStateSet;

    bool emittedReadyRead;
    bool emittedBytesWritten;

    bool closeCalled;

    QString hostName;
    quint16 port;
    QHostAddress host;
    QList<QHostAddress> addresses;

    quint16 localPort;
    quint16 peerPort;
    QHostAddress localAddress;
    QHostAddress peerAddress;
    QString peerName;

    QAbstractSocketEngine *socketEngine;
    int cachedSocketDescriptor;

    void resetSocketLayer();
    bool flush();

    bool initSocketLayer(const QHostAddress &host, QAbstractSocket::SocketType socketType);
    void fetchConnectionParameters();
    void setupSocketNotifiers();
    bool readFromSocket();

#ifdef Q_OS_LINUX
    qint64 addToBytesAvailable;
#endif
    qint64 readBufferMaxSize;
    QRingBuffer readBuffer;
    QRingBuffer writeBuffer;

    bool isBuffered;
    int blockingTimeout;

    QTimer *connectTimer;
    int connectTimeElapsed;

    int hostLookupId;

    QAbstractSocket::SocketType socketType;
    QAbstractSocket::SocketState state;

    QAbstractSocket::SocketError socketError;

#ifndef QT_NO_NETWORKPROXY
    QNetworkProxy *proxy;
#endif
};

#endif // QABSTRACTSOCKET_P_H
