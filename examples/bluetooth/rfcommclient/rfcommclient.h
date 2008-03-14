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

#ifndef __RFCOMMCLIENT_H__
#define __RFCOMMCLIENT_H__

#include <QMainWindow>

class QWaitWidget;
class QAction;
class QBluetoothRfcommSocket;
class QTextEdit;
class QLineEdit;

class RfcommClient : public QMainWindow
{
    Q_OBJECT

public:
    RfcommClient(QWidget *parent = 0, Qt::WFlags f = 0);
    ~RfcommClient();

public slots:
    void cancelConnect();

private slots:
    void connectSocket();
    void disconnectSocket();

    void socketConnected();
    void socketDisconnected();

    void connectFailed();

    void serverReplied();
    void newUserText();

private:
    QTextEdit *logArea;
    QLineEdit *userEntry;
    QWaitWidget *waiter;
    bool canceled;
    QAction *connectAction;
    QAction *disconnectAction;
    QAction *sendAction;
    QBluetoothRfcommSocket *socket;
};

#endif
