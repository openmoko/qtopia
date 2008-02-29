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

#ifndef __QBLUETOOTHOBEXSOCKET_H__
#define __QBLUETOOTHOBEXSOCKET_H__

#include <qbluetoothaddress.h>
#include <qobexsocket.h>
#include <qbluetoothnamespace.h>
#include <QtCore/qglobal.h>

class QBluetoothObexSocketPrivate;

class QTOPIACOMM_EXPORT QBluetoothObexSocket : public QObexSocket
{
    Q_OBJECT

public:
    QBluetoothObexSocket(QObject *parent = 0);
    QBluetoothObexSocket(const QBluetoothAddress &remote,
                         quint8 rfcommChannel,
                         const QBluetoothAddress &local = QBluetoothAddress::any,
                         QObject *parent = 0);
    virtual ~QBluetoothObexSocket();

    bool isEncrypted() const;
    bool isAuthenticated() const;
    QBluetooth::SecurityOptions securityOptions() const;

    qint16 channel() const;
    void setChannel(quint8 rfcomm_channel);

    const QBluetoothAddress &remoteAddress() const;
    void setRemoteAddress(const QBluetoothAddress &remote);

    const QBluetoothAddress &localAddress() const;
    void setLocalAddress(const QBluetoothAddress &local);

    virtual bool connect();

private:
    Q_DISABLE_COPY(QBluetoothObexSocket)
    QBluetoothObexSocketPrivate *m_data;
};

#endif
