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

#ifndef __QBLUETOOTHSCOSOCKET_H__
#define __QBLUETOOTHSCOSOCKET_H__

#include <qtopia/comm/qbluetoothabstractsocket.h>

class QBluetoothScoSocketPrivate;

class QTOPIACOMM_EXPORT QBluetoothScoSocket : public QBluetoothAbstractSocket
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
