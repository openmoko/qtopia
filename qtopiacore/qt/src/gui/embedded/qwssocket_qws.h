/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
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
