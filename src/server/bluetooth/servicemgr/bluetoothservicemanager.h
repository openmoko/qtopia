/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef __BLUETOOTHSERVICEMANAGER_H__
#define __BLUETOOTHSERVICEMANAGER_H__

#include <qbluetoothnamespace.h>
#include <QObject>

class QValueSpaceObject;
class QSettings;
class ServiceMessenger;
class ServiceUserMessenger;
class BluetoothServiceSettings;

class BluetoothServiceManager : public QObject
{
    Q_OBJECT

public:
    BluetoothServiceManager(QObject *parent = 0);
    ~BluetoothServiceManager();

    void registerService(const QString &name, const QString &displayName);

    void startService(const QString &name);
    void serviceStarted(const QString &name, bool error, const QString &errorDesc);
    void serviceStopped(const QString &name);

    void stopService(const QString &name);
    void setServiceSecurity(const QString &name, QBluetooth::SecurityOptions options);

    BluetoothServiceSettings *m_settings;
    ServiceMessenger *m_serviceIpc;
    ServiceUserMessenger *m_serviceUserIpc;
};

#endif
