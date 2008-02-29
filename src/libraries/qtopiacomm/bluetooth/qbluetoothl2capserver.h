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

#ifndef __QBLUETOOTHL2CAPSERVER_H__
#define __QBLUETOOTHL2CAPSERVER_H__

#include <qbluetoothabstractserver.h>

class QBluetoothL2CapServerPrivate;
class QBluetoothAddress;
class QBluetoothL2CapSocket;

class QBLUETOOTH_EXPORT QBluetoothL2CapServer : public QBluetoothAbstractServer
{
public:
    explicit QBluetoothL2CapServer(QObject *parent = 0);
    ~QBluetoothL2CapServer();

    void close();

    bool listen(const QBluetoothAddress &local, int psm, int mtu = 672);

    int serverPsm() const;
    QBluetoothAddress serverAddress() const;

    bool isEncrypted() const;
    bool isAuthenticated() const;
    QBluetooth::SecurityOptions securityOptions() const;
    bool setSecurityOptions(QBluetooth::SecurityOptions options);

    int mtu() const;

private:
    QBluetoothAbstractSocket *createSocket();

    Q_DISABLE_COPY(QBluetoothL2CapServer)
};

#endif
