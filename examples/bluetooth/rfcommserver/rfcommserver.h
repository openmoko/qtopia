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

#ifndef __RFCOMMSERVER_H__
#define __RFCOMMSERVER_H__

#include <QMainWindow>

class QTextEdit;
class QBluetoothRfcommServer;
class QBluetoothRfcommSocket;

class RfcommServer : public QMainWindow
{
    Q_OBJECT

public:
    RfcommServer(QWidget *parent = 0, Qt::WFlags f = 0);
    ~RfcommServer();

private slots:
    void newConnection();
    void readyRead();
    void disconnected();

private:
    void handleClient(QBluetoothRfcommSocket *socket);

private:
    QTextEdit *textArea;
    QBluetoothRfcommServer *server;
};

#endif