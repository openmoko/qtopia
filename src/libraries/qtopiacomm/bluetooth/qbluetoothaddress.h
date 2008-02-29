/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef __QBLUETOOTHADDRESS_H__
#define __QBLUETOOTHADDRESS_H__

#include <qglobal.h>
#include <QString>

#include <qtopiaglobal.h>
#include <qtopiaipcmarshal.h>

class QBluetoothRemoteDevice;
class QBluetoothLocalDevice;

class QTOPIACOMM_EXPORT QBluetoothAddress
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

    bool valid() const;
    QString toString() const;

    static const QBluetoothAddress invalid;
    static const QBluetoothAddress any;
    static const QBluetoothAddress all;
    static const QBluetoothAddress local;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QString m_bdaddr;
    bool m_valid;
};

Q_DECLARE_USER_METATYPE(QBluetoothAddress)

#endif
