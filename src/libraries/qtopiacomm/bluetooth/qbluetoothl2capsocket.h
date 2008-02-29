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

#ifndef __QBLUETOOTHL2CAPSOCKET_H__
#define __QBLUETOOTHL2CAPSOCKET_H__

#include <qbluetoothabstractsocket.h>

class QBluetoothL2CapSocketPrivate;

class QBLUETOOTH_EXPORT QBluetoothL2CapSocket : public QBluetoothAbstractSocket
{
public:
    explicit QBluetoothL2CapSocket(QObject *parent = 0);
    ~QBluetoothL2CapSocket();

    bool connect(const QBluetoothAddress &local,
                 const QBluetoothAddress &remote,
                 int psm,
                 int incomingMtu = 672,
                 int outgoingMtu = 672,
                 QBluetooth::SecurityOptions options = 0);

    QBluetoothAddress remoteAddress() const;
    QBluetoothAddress localAddress() const;
    int remotePsm() const;

    bool isEncrypted() const;
    bool isAuthenticated() const;
    QBluetooth::SecurityOptions securityOptions() const;

    int incomingMtu() const;
    int outgoingMtu() const;

private:
    bool readSocketParameters(int sockfd);
    void resetSocketParameters();
};

#endif
