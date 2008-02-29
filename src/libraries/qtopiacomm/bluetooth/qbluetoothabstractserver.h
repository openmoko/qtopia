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

#ifndef __QBLUETOOTHABSTRACTSERVER_H__
#define __QBLUETOOTHABSTRACTSERVER_H__

#include <QObject>

#include <qtopiaglobal.h>
#include <qbluetoothnamespace.h>

class QBluetoothAbstractServerPrivate;
struct sockaddr;
class QBluetoothAbstractSocket;

class QTOPIACOMM_EXPORT QBluetoothAbstractServer : public QObject
{
    Q_OBJECT
    friend class QBluetoothAbstractServerPrivate;

public:
    enum ServerError {
        NoError,
        UnknownError,
        ResourceError,
        ListenError,
        BindError,
    };

    explicit QBluetoothAbstractServer(QObject *parent = 0);
    ~QBluetoothAbstractServer();

    virtual void close();

    bool isListening() const;

    QBluetoothAbstractServer::ServerError lastError() const;

    int maxPendingConnections() const;
    void setMaxPendingConnections(int max);

    bool waitForNewConnection(int msec = 0, bool *timedOut = 0);
    bool hasPendingConnections() const;
    QBluetoothAbstractSocket *nextPendingConnection();

    int socketDescriptor() const;

signals:
    void newConnection();

protected:
    bool initiateListen(int socket, sockaddr *addr, int len);
    void setError(const QBluetoothAbstractServer::ServerError &error);
    virtual QBluetoothAbstractSocket * createSocket() = 0;

private:
    Q_DISABLE_COPY(QBluetoothAbstractServer)
    QBluetoothAbstractServerPrivate *m_data;
};

#endif
