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

#ifndef __QBLUETOOTHOBEXSERVER_H__
#define __QBLUETOOTHOBEXSERVER_H__

#include <QObject>
#include <QString>
#include <qglobal.h>

#include <qbluetoothaddress.h>
#include <qobexserver.h>
#include <qbluetoothnamespace.h>

class QBluetoothObexServer_Private;

class QTOPIACOMM_EXPORT QBluetoothObexServer : public QObexServer
{
    Q_OBJECT
public:

    explicit QBluetoothObexServer(quint8 channel,
                                  const QBluetoothAddress &local = QBluetoothAddress::any,
                                  QObject *parent = 0);
    ~QBluetoothObexServer();

    qint16 serverChannel() const;
    const QBluetoothAddress & serverAddress() const;

    bool encrypted() const;
    bool authenticated() const;
    QBluetooth::SecurityOptions securityOptions() const;
    bool setSecurityOptions(QBluetooth::SecurityOptions options);

    virtual QObexSocket *nextPendingConnection();

protected:
    virtual void *registerServer();

private:
    QBluetoothObexServer_Private *m_data;
    Q_DISABLE_COPY(QBluetoothObexServer)
};

#endif
