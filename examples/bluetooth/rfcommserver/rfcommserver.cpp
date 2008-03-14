/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#include <QStringList>
#include <QLabel>
#include <QTextEdit>

#include <QBluetoothRfcommServer>
#include <QBluetoothAddress>
#include <QBluetoothRfcommSocket>

#include "rfcommserver.h"

RfcommServer::RfcommServer(QWidget *parent, Qt::WFlags f)
    : QMainWindow(parent, f)
{
    textArea = new QTextEdit(this);
    textArea->setReadOnly(true);
    setCentralWidget(textArea);

    server = new QBluetoothRfcommServer(this);

    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));

    if (server->listen(QBluetoothAddress::any, 14)) {
        textArea->append(QString(tr("Server listening on: %1, channel: %2"))
                .arg(server->serverAddress().toString()).arg(server->serverChannel()));
    } else {
        textArea->append(tr("Was not able to listen on channel:"));
        textArea->append(server->errorString());
    }

    setWindowTitle(tr("RFCOMM Server"));
}

RfcommServer::~RfcommServer()
{
    
}

void RfcommServer::readyRead()
{
    QObject *s = sender();
    if (!s)
        return;

    QBluetoothRfcommSocket *socket = qobject_cast<QBluetoothRfcommSocket *>(s);

    if (!socket)
        return;

    handleClient(socket);
}

void RfcommServer::handleClient(QBluetoothRfcommSocket *socket)
{
    while (socket->canReadLine()) {
        QByteArray line = socket->readLine();
        QByteArray reply("Server Echo: ");
        reply.append(line);
        socket->write(reply);

        line.chop(2);
        textArea->append(QString(tr("From client: %1, data: %2"))
                .arg(socket->remoteAddress().toString()).arg(QString(line)));
    }
}

void RfcommServer::disconnected()
{
    QObject *s = sender();
    if (!s)
        return;

    QBluetoothRfcommSocket *socket = qobject_cast<QBluetoothRfcommSocket *>(s);

    if (!socket)
        return;

    textArea->append(QString(tr("Client %1 disconnected.")).arg(socket->remoteAddress().toString()));
}

void RfcommServer::newConnection()
{
    while (server->hasPendingConnections()) {
        QBluetoothRfcommSocket *socket =
            qobject_cast<QBluetoothRfcommSocket *>(server->nextPendingConnection());
        connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
        connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));

        textArea->append(QString(tr("Client %1 connected.")).arg(socket->remoteAddress().toString()));

        handleClient(socket);
    }
}
