/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef QBLUETOOTHSCOSOCKET_H
#define QBLUETOOTHSCOSOCKET_H

#include <qbluetoothabstractsocket.h>

class QBluetoothScoSocketPrivate;

class QBLUETOOTH_EXPORT QBluetoothScoSocket : public QBluetoothAbstractSocket
{
public:
    explicit QBluetoothScoSocket(QObject *parent = 0);
    ~QBluetoothScoSocket();

    bool connect(const QBluetoothAddress &local,
                 const QBluetoothAddress &remote);

    QBluetoothAddress remoteAddress() const;
    QBluetoothAddress localAddress() const;

    int mtu() const;

private:
    bool readSocketParameters(int sockfd);
    void resetSocketParameters();
};

#endif
