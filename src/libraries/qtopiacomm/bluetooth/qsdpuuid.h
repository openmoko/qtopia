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

#ifndef __QSDPUUID_H__
#define __QSDPUUID_H__

#include <QVariant>
#include <QtGlobal>
#include <QMetaType>

#include <qbluetoothnamespace.h>

class QString;
struct QSDPUUID_Private;

class QTOPIACOMM_EXPORT QSDPUUID {
public:
    enum SDPUUIDType { UUID16, UUID32, UUID128 };

    QSDPUUID();
    explicit QSDPUUID(quint16 data);
    explicit QSDPUUID(quint32 data);
    explicit QSDPUUID(quint128 data);
    explicit QSDPUUID(const QString &str);
    QSDPUUID(const QSDPUUID &other);

    QSDPUUID &operator=(const QSDPUUID &other);

    ~QSDPUUID();

    bool operator==(const QSDPUUID &other) const;
    bool operator!=(const QSDPUUID &other) const
    {
        return !operator==(other);
    }

    QString toString() const;

    SDPUUIDType type() const;
    QVariant uuid();

    QSDPUUID toUUID128() const;

    static QSDPUUID PublicBrowseGroup;
    static QSDPUUID L2cap;
    static QSDPUUID create16Bit(quint16 id);
    static QSDPUUID create32Bit(quint32 id);
    static QSDPUUID create128Bit(quint128 id);

    static QSDPUUID fromProfile(QBluetooth::SDPProfile profile);

private:
    QVariant m_uuid;
    QSDPUUID::SDPUUIDType m_type;
};

Q_DECLARE_METATYPE(QSDPUUID)

#endif
