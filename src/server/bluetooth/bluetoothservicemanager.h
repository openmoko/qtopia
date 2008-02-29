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

#ifndef __BLUETOOTHSERVICEMANAGER_H__
#define __BLUETOOTHSERVICEMANAGER_H__

#include <QObject>
#include <QHash>
#include <QSet>

#include <qbluetoothnamespace.h>

class QCommServiceManager;
class ServiceListenerController;
class QValueSpaceObject;
class QTranslatableSettings;
class QSDAP;
class QSDAPSearchResult;
class QBluetoothServiceControl;


class BluetoothServiceManager : public QObject
{
    Q_OBJECT
public:
    BluetoothServiceManager(QObject *parent = 0);
    ~BluetoothServiceManager();


public slots:
    // called for messages are received from a service provider
    void registerService(const QString &name, const QString &translatableDisplayName);
    void unregisterService(const QString &name);
    void serviceStarted(const QString &name, QBluetooth::ServiceError error, const QString &errorDesc);
    void serviceStopped(const QString &name, QBluetooth::ServiceError error, const QString &errorDesc);
    void serviceError(const QString &name,
                      QBluetooth::ServiceError error,
                      const QString &errorDesc);

    // called for messages received from "listeners" e.g. BTSettings
    void startService(const QString &name);
    void stopService(const QString &name, bool systemCall);
    void setServiceSecurity(const QString &name, QBluetooth::SecurityOptions);

private slots:
    void foundLocalServices(const QSDAPSearchResult &result);
    void commServiceAdded(const QString &name);
    void commServiceRemoved(const QString &name);

    void localDeviceAdded(const QString &deviceName);
    void localDeviceRemoved(const QString &deviceName);
    void startAllServices();
    void stopAllServices();

private:
    static QHash<QString, QVariant> defaultServiceSettings();
    static QList<QString> persistentServiceSettings();

    void handleServiceTerminated(const QString &name);

    void initAvailableChannels();
    void initReservedChannels();
    int findAvailableRfcommChannel(const QString &name) const;
    void initServiceSettings(const QString &name, const QString &translatableName);

    bool isRegistered(const QString &name) const;
    void setServiceValue(const QString &name, const QString &attr, const QVariant &value);
    QVariant serviceValue(const QString &name, const QString &attribute) const;
    void loadConfig();

    static const QString SETTINGS_VALUE_SPACE_PATH;
    static const QString SETTINGS_CONFIG_NAME;
    static int RFCOMM_CHANNELS_START;
    static int RFCOMM_CHANNELS_END;
    static const QList<QString> persistentSettings;

    QCommServiceManager *m_commServiceManager;
    ServiceListenerController *m_listenerMessenger;

    QValueSpaceObject *m_servicesValueSpace;
    QTranslatableSettings *m_configSettings;

    QHash<int, QString> m_usedRfcommChannels;
    QSet<int> m_reservedRfcommChannels;
    QSet<int> m_badRfcommChannels;
    QSet<QString> m_servicesWithSdpError;

    QSDAP *m_sdap;

    QHash<QString, QBluetoothServiceControl *> *m_serviceCommObjects;

    Q_DISABLE_COPY(BluetoothServiceManager)
};


#endif
