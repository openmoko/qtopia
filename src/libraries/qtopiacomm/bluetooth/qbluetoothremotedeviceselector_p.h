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

#ifndef __QBLUETOOTHREMOTEDEVICESELECTOR_H__
#define __QBLUETOOTHREMOTEDEVICESELECTOR_H__

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qbluetoothaddress.h>

#include <QWidget>

class QBluetoothRemoteDevice;
class QBluetoothRemoteDeviceSelectorPrivate;

class QBLUETOOTH_EXPORT QBluetoothRemoteDeviceSelector : public QWidget
{
    Q_OBJECT
public:
    enum DisplayFlag {
        DeviceIcon = 0x1,
        Name = 0x2,
        Alias = 0x4,
        PairingStatus = 0x8,
        ConnectionStatus = 0x10
    };
    Q_DECLARE_FLAGS(DisplayFlags, DisplayFlag)

    QBluetoothRemoteDeviceSelector(DisplayFlags displayFlags,
                                   QBluetoothLocalDevice *local,
                                   QWidget *parent = 0,
                                   Qt::WFlags flags = 0);
    QBluetoothRemoteDeviceSelector(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~QBluetoothRemoteDeviceSelector();

    DisplayFlags flags() const;

    bool insert(const QBluetoothRemoteDevice &device);
    void update(const QBluetoothRemoteDevice &device);
    void update(const QBluetoothRemoteDevice &device, DisplayFlags flags);
    bool remove(const QBluetoothAddress &address);

    bool contains(const QBluetoothAddress &address) const;
    int count() const;
    QList<QBluetoothAddress> devices() const;
    QBluetoothAddress selectedDevice() const;

public slots:
    void clear();
    void selectDevice(const QBluetoothAddress &address);

signals:
    void selectionChanged();
    void activated(const QBluetoothAddress &addr);

private:
    friend class QBluetoothRemoteDeviceSelectorPrivate;
    QBluetoothRemoteDeviceSelectorPrivate *m_data;
    Q_DISABLE_COPY(QBluetoothRemoteDeviceSelector)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QBluetoothRemoteDeviceSelector::DisplayFlags)
Q_DECLARE_USER_METATYPE_ENUM(QBluetoothRemoteDeviceSelector::DisplayFlag)

#endif
