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

#ifndef __QBLUETOOTHL2CAPDATAGRAMSOCKET_H__
#define __QBLUETOOTHL2CAPDATAGRAMSOCKET_H__

#include <qtopia/comm/qbluetoothabstractsocket.h>

class QBluetoothL2CapDatagramSocketPrivate;

class QTOPIACOMM_EXPORT QBluetoothL2CapDatagramSocket : public QBluetoothAbstractSocket
{
public:
    explicit QBluetoothL2CapDatagramSocket(QObject *parent = 0);
    ~QBluetoothL2CapDatagramSocket();

    bool connect(const QBluetoothAddress &local,
                 const QBluetoothAddress &remote,
                 int psm,
                 int incomingMtu = 672,
                 int outgoingMtu = 672);

    QBluetoothAddress remoteAddress() const;
    QBluetoothAddress localAddress() const;
    int remotePsm() const;
    int localPsm() const;

    bool isEncrypted() const;
    bool isAuthenticated() const;
    QBluetooth::SecurityOptions securityOptions() const;
    bool setSecurityOptions(QBluetooth::SecurityOptions options);

    int incomingMtu() const;
    int outgoingMtu() const;

    bool bind(const QBluetoothAddress &local, int psm, int mtu = 672);
    qint64 readDatagram (char * data, qint64 maxSize,
                         QBluetoothAddress *address = 0, int *psm = 0 );

private:
    bool readSocketParameters(int sockfd);
    void resetSocketParameters();
};

#endif
