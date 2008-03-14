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

#ifndef OBEXCLIENTWINDOW_H
#define OBEXCLIENTWINDOW_H

#include <QMainWindow>

class QObexClientSession;
class QTcpSocket;
class QTextEdit;
class QHostAddress;

class ObexClientWindow : public QMainWindow
{
    Q_OBJECT
public:
    ObexClientWindow(const QHostAddress &serverAddress,
                     quint16 serverPort,
                     QWidget *parent = 0,
                     Qt::WFlags f = 0);

private slots:
    void socketConnected();
    void buttonClicked();
    void socketReadyRead();

private:
    QTcpSocket *m_socket;
    QObexClientSession *m_obexClient;
    QTextEdit *m_textEdit;

    void setupUi();
};

#endif
