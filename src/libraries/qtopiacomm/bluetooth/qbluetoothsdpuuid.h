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

#ifndef __QBluetoothSdpUuid_H__
#define __QBluetoothSdpUuid_H__

#include <QVariant>
#include <QtGlobal>
#include <QMetaType>

#include <qbluetoothnamespace.h>

class QString;
struct QBluetoothSdpUuid_Private;

class QBLUETOOTH_EXPORT QBluetoothSdpUuid {
public:
    enum Type { UUID16, UUID32, UUID128 };

    QBluetoothSdpUuid();
    explicit QBluetoothSdpUuid(quint16 data);
    explicit QBluetoothSdpUuid(quint32 data);
    explicit QBluetoothSdpUuid(quint128 data);
    explicit QBluetoothSdpUuid(const QString &str);
    QBluetoothSdpUuid(const QBluetoothSdpUuid &other);

    QBluetoothSdpUuid &operator=(const QBluetoothSdpUuid &other);

    ~QBluetoothSdpUuid();

    bool operator==(const QBluetoothSdpUuid &other) const;
    bool operator!=(const QBluetoothSdpUuid &other) const
    {
        return !operator==(other);
    }

    QString toString() const;

    QBluetoothSdpUuid::Type type() const;
    QVariant uuid();
    bool isValid() const;

    QBluetoothSdpUuid toUuid128() const;

    static QBluetoothSdpUuid PublicBrowseGroup;
    static QBluetoothSdpUuid L2cap;
    static QBluetoothSdpUuid create16Bit(quint16 id);
    static QBluetoothSdpUuid create32Bit(quint32 id);
    static QBluetoothSdpUuid create128Bit(quint128 id);

    static QBluetoothSdpUuid fromProfile(QBluetooth::SDPProfile profile);

private:
    bool m_isValid;
    QVariant m_uuid;
    QBluetoothSdpUuid::Type m_type;
};

Q_DECLARE_METATYPE(QBluetoothSdpUuid)

#endif
