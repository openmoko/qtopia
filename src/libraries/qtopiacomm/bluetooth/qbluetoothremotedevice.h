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

#ifndef __QBLUETOOTHREMOTEDEVICE_H__
#define __QBLUETOOTHREMOTEDEVICE_H__

#include <qbluetoothnamespace.h>

#include <QString>
#include <qglobal.h>

class QBluetoothRemoteDevicePrivate;
class QBluetoothAddress;

class QTOPIACOMM_EXPORT QBluetoothRemoteDevice
{
public:
    explicit QBluetoothRemoteDevice(const QBluetoothAddress &address);
    QBluetoothRemoteDevice(const QBluetoothAddress &address,
                           const QString &name,
                           const QString &version,
                           const QString &revision,
                           const QString &manufacturer,
                           const QString &company,
                           QBluetooth::DeviceMajor devMajor,
                           quint8 devMinor,
                           QBluetooth::ServiceClasses serviceClasses);
    QBluetoothRemoteDevice(const QBluetoothRemoteDevice &other);

    ~QBluetoothRemoteDevice();

    QBluetoothRemoteDevice &operator=(const QBluetoothRemoteDevice &other);
    bool operator==(const QBluetoothRemoteDevice &other) const;
    bool operator!=(const QBluetoothRemoteDevice &other) const
    {
        return !operator==(other);
    }

    QString name() const;
    void setName(const QString &name);

    QString version() const;
    void setVersion(const QString &version);

    QString revision() const;
    void setRevision(const QString &revision);

    QString manufacturer() const;
    void setManufacturer(const QString &manufacturer);

    QString company() const;
    void setCompany(const QString &company);

    QBluetooth::DeviceMajor deviceMajor() const;
    QString deviceMajorAsString() const;
    void setDeviceMajor(QBluetooth::DeviceMajor deviceMajor);

    quint8 deviceMinor() const;
    QString deviceMinorAsString() const;
    void setDeviceMinor(quint8 deviceMinor);

    QBluetooth::ServiceClasses serviceClasses() const;
    QStringList serviceClassesAsString() const;
    void setServiceClasses(QBluetooth::ServiceClasses serviceClasses);

    QBluetoothAddress address() const;

private:
    QBluetoothRemoteDevice() {}
    QBluetoothRemoteDevicePrivate *m_data;
};

#endif
