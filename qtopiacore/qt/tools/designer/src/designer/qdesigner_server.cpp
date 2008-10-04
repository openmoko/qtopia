/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include <QtCore/QFileInfo>
#include <QtCore/QStringList>

#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

#include "qdesigner.h"
#include "qdesigner_server.h"

#include <qevent.h>

QT_BEGIN_NAMESPACE

// ### review

QDesignerServer::QDesignerServer(QObject *parent)
    : QObject(parent)
{
    m_socket = 0;
    m_server = new QTcpServer(this);
    m_server->listen(QHostAddress::LocalHost, 0);
    if (m_server->isListening())
    {
        connect(m_server, SIGNAL(newConnection()),
                this, SLOT(handleNewConnection()));
    }
}

QDesignerServer::~QDesignerServer()
{
}

quint16 QDesignerServer::serverPort() const
{
    return m_server ? m_server->serverPort() : 0;
}

void QDesignerServer::sendOpenRequest(int port, const QStringList &files)
{
    QTcpSocket *sSocket = new QTcpSocket();
    sSocket->connectToHost(QHostAddress::LocalHost, port);
    if(sSocket->waitForConnected(3000))
    {
        foreach(QString file, files)
        {
            QFileInfo fi(file);
            sSocket->write(fi.absoluteFilePath().toUtf8() + '\n');
        }
        sSocket->waitForBytesWritten(3000);
        sSocket->close();
    }
    delete sSocket;
}

void QDesignerServer::readFromClient()
{
    QString file = QString();
    while (m_socket->canReadLine()) {
        file = QString::fromUtf8(m_socket->readLine());
        if (!file.isNull()) {
            file.remove(QLatin1Char('\n'));
            file.remove(QLatin1Char('\r'));
            qDesigner->postEvent(qDesigner, new QFileOpenEvent(file));
        }
    }
}

void QDesignerServer::socketClosed()
{
    m_socket = 0;
}

void QDesignerServer::handleNewConnection()
{
    // no need for more than one connection
    if (m_socket == 0) {
        m_socket = m_server->nextPendingConnection();
        connect(m_socket, SIGNAL(readyRead()),
                this, SLOT(readFromClient()));
        connect(m_socket, SIGNAL(disconnected()),
                this, SLOT(socketClosed()));
    }
}


QDesignerClient::QDesignerClient(quint16 port, QObject *parent)
: QObject(parent)
{
    m_socket = new QTcpSocket(this);
    m_socket->connectToHost(QHostAddress::LocalHost, port);
    connect(m_socket, SIGNAL(readyRead()),
                this, SLOT(readFromSocket()));
 
}

QDesignerClient::~QDesignerClient()
{
    m_socket->close();
    m_socket->flush();
}

void QDesignerClient::readFromSocket()
{
    QString file = QString();
    while (m_socket->canReadLine()) {
        QString file = QString::fromUtf8(m_socket->readLine());
        if (!file.isNull()) {
            file.remove(QLatin1Char('\n'));
            file.remove(QLatin1Char('\r'));
            if (QFile::exists(file))
                qDesigner->postEvent(qDesigner, new QFileOpenEvent(file));
        }
    }
}

QT_END_NAMESPACE
