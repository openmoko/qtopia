/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWSSOCKET_QWS_H
#define QWSSOCKET_QWS_H

#include <QtCore/qconfig.h>

#ifndef QT_NO_SXE
#define QWS_SOCK_BASE QUnixSocket
#define QWS_SOCK_SERVER_BASE QUnixSocketServer
#include "qunixsocket_p.h"
#include "qunixsocketserver_p.h"
#include <QtCore/qmutex.h>
#else
#define QWS_SOCK_BASE QTcpSocket
#define QWS_SOCK_SERVER_BASE QTcpServer
#include <QtNetwork/qtcpsocket.h>
#include <QtNetwork/qtcpserver.h>
#endif


QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_QWS_MULTIPROCESS

class QWSSocket : public QWS_SOCK_BASE
{
    Q_OBJECT
public:
    explicit QWSSocket(QObject *parent=0);
    ~QWSSocket();

    bool connectToLocalFile(const QString &file);

#ifndef QT_NO_SXE
    QString errorString();
Q_SIGNALS:
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError);
private Q_SLOTS:
    void forwardStateChange(SocketState);
#endif

private:
    Q_DISABLE_COPY(QWSSocket)
};


class QWSServerSocket : public QWS_SOCK_SERVER_BASE
{
    Q_OBJECT
public:
    QWSServerSocket(const QString& file, QObject *parent=0);
    ~QWSServerSocket();

#ifndef QT_NO_SXE
    QWSSocket *nextPendingConnection();
Q_SIGNALS:
    void newConnection();
protected:
    void incomingConnection(int socketDescriptor);
private:
    QMutex ssmx;
    QList<int> inboundConnections;
#endif

private:
    Q_DISABLE_COPY(QWSServerSocket)

    void init(const QString &file);
};

#endif // QT_NO_QWS_MULTIPROCESS

QT_END_HEADER

#endif // QWSSOCKET_QWS_H
