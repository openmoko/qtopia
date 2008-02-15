/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef DEVICE_CONNECTOR_H
#define DEVICE_CONNECTOR_H

#include <QObject>
#include <QStringList>

class QTcpSocket;
class MessageRecord;
class QTimer;

class DeviceConnector : public QObject
{
    Q_OBJECT
public:
    enum MessageState {
        NoMessage,
        WaitForWelcome,
        User,
        WaitForUserOk,
        Password,
        WaitForPasswordOk,
        Message,
        WaitForMessageOk,
        MessageDelivered,
        MessageFailed
    };

    DeviceConnector();
    ~DeviceConnector();
    void sendQcop( const QString &, const QString &, const QByteArray & );
signals:
    void startingConnect();
    void finishedConnect();
    void sendProgress(int);
    void deviceConnMessage( const QString & );
private slots:
    void socketError();
    void socketReadyRead();
    void socketDisconnected();
    void processMessages();
private:
    void connect();
    void teardown();
    void sendMessage( const QString & );
    QTcpSocket *mSocket;
    QList<MessageRecord*> messageQueue;
    QStringList replyQueue;
    bool loginDone;
    QString loginName;
};

#endif
