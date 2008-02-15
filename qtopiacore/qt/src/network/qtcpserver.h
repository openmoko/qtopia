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

#ifndef QTCPSERVER_H
#define QTCPSERVER_H

#include <QtCore/qobject.h>
#include <QtNetwork/qabstractsocket.h>
#include <QtNetwork/qhostaddress.h>

QT_BEGIN_HEADER

QT_MODULE(Network)

class QTcpServerPrivate;
class QNetworkProxy;
class QTcpSocket;

class Q_NETWORK_EXPORT QTcpServer : public QObject
{
    Q_OBJECT
public:
    explicit QTcpServer(QObject *parent = 0);
    virtual ~QTcpServer();

    bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
    void close();

    bool isListening() const;

    void setMaxPendingConnections(int numConnections);
    int maxPendingConnections() const;

    quint16 serverPort() const;
    QHostAddress serverAddress() const;

    int socketDescriptor() const;
    bool setSocketDescriptor(int socketDescriptor);

    bool waitForNewConnection(int msec = 0, bool *timedOut = 0);
    virtual bool hasPendingConnections() const;
    virtual QTcpSocket *nextPendingConnection();

    QAbstractSocket::SocketError serverError() const;
    QString errorString() const;

#ifndef QT_NO_NETWORKPROXY
    void setProxy(const QNetworkProxy &networkProxy);
    QNetworkProxy proxy() const;
#endif

protected:
    virtual void incomingConnection(int handle);

Q_SIGNALS:
    void newConnection();

private:
    Q_DISABLE_COPY(QTcpServer)
    Q_DECLARE_PRIVATE(QTcpServer)
};

QT_END_HEADER

#endif // QTCPSERVER_H
