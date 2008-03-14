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

#include "obexclientwindow.h"

#include <QObexClientSession>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>

ObexClientWindow::ObexClientWindow(const QHostAddress &serverAddress, quint16 serverPort, QWidget *parent, Qt::WFlags f)
    : QMainWindow(parent, f),
      m_socket(new QTcpSocket(this)),
      m_obexClient(0)
{
    setupUi();

    connect(m_socket, SIGNAL(connected()), SLOT(socketConnected()));
    m_socket->connectToHost(serverAddress, serverPort);
}

void ObexClientWindow::socketConnected()
{
    m_obexClient = new QObexClientSession(m_socket, this);
    connect(m_obexClient, SIGNAL(readyRead()), SLOT(socketReadyRead()));

    m_obexClient->connect();
}

void ObexClientWindow::buttonClicked()
{
    m_obexClient->get(QObexHeader());
}

void ObexClientWindow::socketReadyRead()
{
    m_textEdit->setText(m_obexClient->readAll());
}

void ObexClientWindow::setupUi()
{
    m_textEdit = new QTextEdit;
    QPushButton *button = new QPushButton(tr("Get a Dilbert quote!"));
    connect(button, SIGNAL(clicked()), SLOT(buttonClicked()));

    QWidget *mainWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_textEdit);
    layout->addWidget(button);
    mainWidget->setLayout(layout);
    setCentralWidget(mainWidget);
}

#include "obexclientwindow.moc"
