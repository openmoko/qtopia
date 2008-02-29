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

#ifndef __QBLUETOOTHPASSKEYREQUEST_H__
#define __QBLUETOOTHPASSKEYREQUEST_H__

#include <QString>
#include <qbluetoothaddress.h>

class QTOPIACOMM_EXPORT QBluetoothPasskeyRequest
{
public:
    QBluetoothPasskeyRequest(const QString &localDevice,
                                const QBluetoothAddress &remoteDevice);
    QBluetoothPasskeyRequest(const QBluetoothPasskeyRequest &req);

    ~QBluetoothPasskeyRequest();

    QBluetoothPasskeyRequest &operator=(const QBluetoothPasskeyRequest &other);
    bool operator==(const QBluetoothPasskeyRequest &other) const;
    bool operator!=(const QBluetoothPasskeyRequest &other) const;

    const QString &localDevice() const;
    const QBluetoothAddress &remoteDevice() const;

    void setRejected();
    bool isRejected() const;

    void setPasskey(const QString &passkey);
    const QString &passkey() const;

private:
    QString m_localDevice;
    QBluetoothAddress m_remoteDevice;
    QString m_passkey;
    bool m_rejected;
};

#endif
