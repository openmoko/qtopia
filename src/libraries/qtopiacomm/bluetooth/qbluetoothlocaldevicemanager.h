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

#ifndef __QBLUETOOTHLOCALDEVICEMANAGER_H__
#define __QBLUETOOTHLOCALDEVICEMANAGER_H__

#include <qobject.h>
#include <qglobal.h>
#include <QString>

#include <qtopiaglobal.h>

class QBluetoothLocalDeviceManager_Private;

class QTOPIACOMM_EXPORT QBluetoothLocalDeviceManager : public QObject
{
    Q_OBJECT
    friend class QBluetoothLocalDeviceManager_Private;

public:
    explicit QBluetoothLocalDeviceManager(QObject *parent = 0);

    ~QBluetoothLocalDeviceManager();

    QStringList devices();
    QString defaultDevice();

signals:
    void deviceAdded(const QString &device);
    void deviceRemoved(const QString &device);

private:
    QBluetoothLocalDeviceManager_Private *m_data;
};

#endif
