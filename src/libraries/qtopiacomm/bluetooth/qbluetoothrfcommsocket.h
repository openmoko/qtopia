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

#ifndef __QBLUETOOTHRFCOMMSOCKET_H__
#define __QBLUETOOTHRFCOMMSOCKET_H__

#include <qtopia/comm/qbluetoothabstractsocket.h>

class QBluetoothRfcommSocketPrivate;

class QTOPIACOMM_EXPORT QBluetoothRfcommSocket : public QBluetoothAbstractSocket
{
public:
    explicit QBluetoothRfcommSocket(QObject *parent = 0);
    ~QBluetoothRfcommSocket();

    bool connect(const QBluetoothAddress &local,
                 const QBluetoothAddress &remote,
                 int channel,
                 QBluetooth::SecurityOptions options = 0);

    QBluetoothAddress remoteAddress() const;
    int remoteChannel() const;
    QBluetoothAddress localAddress() const;

    bool isEncrypted() const;
    bool isAuthenticated() const;
    QBluetooth::SecurityOptions securityOptions() const;

private:
    bool readSocketParameters(int sockfd);
    void resetSocketParameters();
};

#endif
