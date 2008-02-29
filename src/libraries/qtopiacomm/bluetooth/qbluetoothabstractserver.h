/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include <qbluetoothglobal.h>
#include <qbluetoothnamespace.h>
#include <qbluetoothabstractsocket.h>

class QBluetoothAbstractServerPrivate;
struct sockaddr;

class QBLUETOOTH_EXPORT QBluetoothAbstractServer : public QObject
{
    Q_OBJECT
    friend class QBluetoothAbstractServerPrivate;

public:
    ~QBluetoothAbstractServer();

    virtual void close();

    bool isListening() const;

    QBluetoothAbstractSocket::SocketError serverError() const;
    QString errorString() const;

    int maxPendingConnections() const;
    void setMaxPendingConnections(int max);

    bool waitForNewConnection(int msec = 0, bool *timedOut = 0);
    bool hasPendingConnections() const;
    QBluetoothAbstractSocket *nextPendingConnection();

    int socketDescriptor() const;

signals:
    void newConnection();

protected:
    explicit QBluetoothAbstractServer(QBluetoothAbstractServerPrivate *data,
                                        QObject *parent = 0);
    void setListening();
    virtual QBluetoothAbstractSocket * createSocket() = 0;

    QBluetoothAbstractServerPrivate *m_data;

private:
    Q_DISABLE_COPY(QBluetoothAbstractServer)
};

#endif
