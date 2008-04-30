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

#include <QTimer>
#include <QStringList>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>

#include <QBluetoothAddress>
#include <QBluetoothRfcommSocket>
#include <QBluetoothRemoteDeviceDialog>

#include <QtopiaApplication>
#include <QAction>
#include <QMenu>
#include <QSoftMenuBar>

#include "rfcommclient.h"

RfcommClient::RfcommClient(QWidget *parent, Qt::WFlags f)
    : QMainWindow(parent, f)
{
    #warning "Cancel broken due removal of WaitWidget"

    connectAction = new QAction(tr("Connect..."), this);
    connect(connectAction, SIGNAL(triggered()), this, SLOT(connectSocket()));
    QSoftMenuBar::menuFor(this)->addAction(connectAction);
    connectAction->setVisible(true);

    disconnectAction = new QAction(tr("Disconnect"), this);
    connect(disconnectAction, SIGNAL(triggered()), this, SLOT(disconnectSocket()));
    disconnectAction->setVisible(false);
    QSoftMenuBar::menuFor(this)->addAction(disconnectAction);

    sendAction = new QAction(tr("Send"), this);
    connect(sendAction, SIGNAL(triggered()), this, SLOT(newUserText()));
    sendAction->setVisible(false);
    QSoftMenuBar::menuFor(this)->addAction(sendAction);

    QWidget *frame = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;

    logArea = new QTextEdit(frame);
    layout->addWidget(logArea);
    logArea->append(tr("Not connected"));
    logArea->setReadOnly(true);
    logArea->setFocusPolicy(Qt::NoFocus);

    userEntry = new QLineEdit(frame);
    userEntry->setEnabled(false);
    connect(userEntry, SIGNAL(editingFinished()), this, SLOT(newUserText()));
    layout->addWidget(userEntry);

    frame->setLayout(layout);
    setCentralWidget(frame);

    socket = new QBluetoothRfcommSocket(this);
    connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(serverReplied()));

    setWindowTitle(tr("RFCOMM Client"));
}

RfcommClient::~RfcommClient()
{
    
}

void RfcommClient::connectSocket()
{
    QBluetoothAddress addr = QBluetoothRemoteDeviceDialog::getRemoteDevice(this);

    if (!addr.isValid()) {
        return;
    }

    connectAction->setVisible(false);

    connect(socket, SIGNAL(error(QBluetoothAbstractSocket::SocketError)),
            this, SLOT(connectFailed()));
    socket->connect(QBluetoothAddress::any, addr, 14);
}

void RfcommClient::socketConnected()
{
    disconnect(socket, SIGNAL(error(QBluetoothAbstractSocket::SocketError)),
               this, SLOT(connectFailed()));
    userEntry->setEnabled(true);

    disconnectAction->setVisible(true);
    sendAction->setVisible(true);

    logArea->append(QString(tr("Connected to %1")).arg(socket->remoteAddress().toString()));
}

void RfcommClient::connectFailed()
{
    connectAction->setVisible(true);
}

void RfcommClient::disconnectSocket()
{
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    userEntry->setEnabled(false);
    socket->disconnect();
    disconnectAction->setVisible(false);
    sendAction->setVisible(false);
}

void RfcommClient::socketDisconnected()
{
    connectAction->setVisible(true);

    disconnect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    logArea->append(tr("Disconnected from server."));
}

void RfcommClient::cancelConnect()
{
    socket->disconnect();
}

void RfcommClient::newUserText()
{
    QString text = userEntry->text();
    if (text.isEmpty())
        return;
    userEntry->clear();

    logArea->append(text);

    QByteArray textToSend = text.toLatin1();
    textToSend.append("\r\n");
    socket->write(textToSend);
}

void RfcommClient::serverReplied()
{
    while (socket->canReadLine()) {
        QByteArray line = socket->readLine();
        line.chop(2);
        logArea->append(QString(tr("From server: %1"))
                .arg(QString(line)));
    }
}
