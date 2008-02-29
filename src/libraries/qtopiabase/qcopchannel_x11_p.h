/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
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

#ifndef QCOPCHANNEL_X11_P_H
#define QCOPCHANNEL_X11_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qobject.h>
#include <qunixsocket_p.h>
#include <qunixsocketserver_p.h>

#if defined(Q_WS_X11)

class QEventLoop;

class QCopX11Client : public QObject
{
    Q_OBJECT
public:
    QCopX11Client();
    QCopX11Client( QIODevice *device, QUnixSocket *socket );
    ~QCopX11Client();

    void registerChannel( const QString& ch );
    void detachChannel( const QString& ch );
    void sendChannelCommand( int cmd, const QString& ch );
    void send( const QString& ch, const QString& msg, const QByteArray& data );
    void forward( const QString& ch, const QString& msg, const QByteArray& data,
                  const QString& forwardTo);
    void isRegisteredReply( const QString& ch, bool known );
    void requestRegistered( const QString& ch );
    void flush();
    bool waitForIsRegistered();

    bool isClient() const { return !server; }
    bool isServer() const { return server; }

    static const int minPacketSize = 256;

private slots:
    void readyRead();
    void disconnected();
    void connectToServer();
    void connectSignals();

private:
    bool server;
    QIODevice *device;
    QUnixSocket *socket;

    void init();

    char outBuffer[minPacketSize];
    char inBuffer[minPacketSize];
    char *inBufferPtr;
    int inBufferUsed;
    int inBufferExpected;
    bool isRegisteredResponse;
    QEventLoop *isRegisteredWaiter;
    QByteArray pendingData;
    int retryCount;
    bool connecting;

    void write( const char *buf, int len );
};

class QCopX11Server : public QUnixSocketServer
{
    Q_OBJECT
public:
    QCopX11Server();
    ~QCopX11Server();

protected:
    void incomingConnection(int socketDescriptor);
};

#endif // Q_WS_X11

#endif // QCOPCHANNEL_X11_P_H
