/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef __QBLUETOOTHRFCOMMSERVER_H__
#define __QBLUETOOTHRFCOMMSERVER_H__

#include <QObject>

#include <qbluetoothnamespace.h>
#include <qbluetoothabstractserver.h>

class QBluetoothRfcommServerPrivate;
class QBluetoothAddress;
class QBluetoothRfcommSocket;

class QTOPIACOMM_EXPORT QBluetoothRfcommServer : public QBluetoothAbstractServer
{
public:
    explicit QBluetoothRfcommServer(QObject *parent = 0);
    ~QBluetoothRfcommServer();

    virtual void close();

    bool listen(const QBluetoothAddress &local, int channel);

    int serverChannel() const;
    QBluetoothAddress serverAddress() const;

    bool isEncrypted() const;
    bool isAuthenticated() const;
    QBluetooth::SecurityOptions securityOptions() const;
    bool setSecurityOptions(QBluetooth::SecurityOptions options);

private:
    QBluetoothAbstractSocket * createSocket();

    Q_DISABLE_COPY(QBluetoothRfcommServer)
    QBluetoothRfcommServerPrivate *m_data;
};

#endif
