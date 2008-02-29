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

#ifndef __QBLUETOOTHADDRESS_H__
#define __QBLUETOOTHADDRESS_H__

#include <qglobal.h>
#include <QString>

#include <qbluetoothglobal.h>

#ifdef QTOPIA_BLUETOOTH
#include <qtopiaipcmarshal.h>
#endif

class QBluetoothRemoteDevice;
class QBluetoothLocalDevice;

class QBLUETOOTH_EXPORT QBluetoothAddress
{

public:
    QBluetoothAddress();

    QBluetoothAddress(const QBluetoothAddress &addr);
    explicit QBluetoothAddress(const QString &addr);
    ~QBluetoothAddress();

    QBluetoothAddress &operator=(const QBluetoothAddress &other);
    bool operator==(const QBluetoothAddress &other) const;
    bool operator!=(const QBluetoothAddress &other) const
    {
        return !operator==(other);
    }

    bool isValid() const;
    QString toString() const;

    static const QBluetoothAddress invalid;
    static const QBluetoothAddress any;
    static const QBluetoothAddress all;
    static const QBluetoothAddress local;

#ifdef QTOPIA_BLUETOOTH
    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);
#endif

private:
    friend uint qHash(const QBluetoothAddress &addr);
    QString m_bdaddr;
    bool m_valid;
};

uint QBLUETOOTH_EXPORT qHash(const QBluetoothAddress &addr);

#ifdef QTOPIA_BLUETOOTH
Q_DECLARE_USER_METATYPE(QBluetoothAddress)
#endif

#endif
