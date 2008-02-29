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

#include <qstringlist.h>

#include <qvaluespace.h>

#include <qmediadevicemanager.h>
#include <qmediadevicemanagerfactory.h>

#include "devicemanager.h"



namespace mediaserver
{


DeviceManager::DeviceManager()
{
}

DeviceManager::~DeviceManager()
{
    foreach (QMediaDeviceManager* deviceManager, m_managers)
    {
        delete deviceManager;
    }

    delete m_pluginManager;
}

void DeviceManager::init()
{
    m_pluginManager = new QPluginManager("mediadevices");

    // Check those supported by plugin manager
    foreach (QString pluginName, m_pluginManager->list())
    {
        QMediaDeviceManagerFactory*   factory;
        QObject*    instance = m_pluginManager->instance(pluginName);

        if ((factory = qobject_cast<QMediaDeviceManagerFactory*>(instance)) != 0)
        {
            QMediaDeviceManager*     deviceManager = factory->create();

            if (deviceManager != 0)
                m_managers.push_back(deviceManager);
        }
        else
            delete instance;
    }

    for (DeviceManagers::iterator it = m_managers.begin();
         it != m_managers.end();
         ++it)
    {
        connect(*it, SIGNAL(deviceAvailable(QMediaDeviceManager*, QMediaDeviceInfo)),
                this, SLOT(deviceAvailable(QMediaDeviceManager*, QMediaDeviceInfo)));

        connect(*it, SIGNAL(deviceUnavailable(QMediaDeviceManager*, QMediaDeviceInfo)),
                this, SLOT(deviceUnavailable(QMediaDeviceManager*, QMediaDeviceInfo)));

        (*it)->init();
    }
}

QMediaDevice* DeviceManager::createMediaDevice(QMediaDeviceRep const& mediaDevice)
{
    MediaDeviceMap::iterator it = m_deviceMap.find(mediaDevice.id());

    if (it != m_deviceMap.end())
    {
        return it.value()->createDeviceInstance(mediaDevice);
    }

    return 0;
}

DeviceManager* DeviceManager::instance()
{
    static DeviceManager self;

    return &self;
}

void DeviceManager::addMediaDeviceToValueSpace(QMediaDeviceInfo const& mediaDevice)
{
    int index = m_devices.indexOf(mediaDevice);

    if (index == -1)
    {
        m_devices.append(mediaDevice);

        QValueSpaceItem devicesItem("/MediaServer/Capabilities/Devices");

        QVariantList    deviceList;

        foreach (QMediaDeviceInfo deviceInfo, m_devices)
        {
            deviceList.push_back(qVariantFromValue(deviceInfo));
        }

        devicesItem.setValue(0, deviceList);
    }
}

void DeviceManager::removeMediaDeviceFromValueSpace(QMediaDeviceInfo const& mediaDevice)
{
    int index = m_devices.indexOf(mediaDevice);

    if (index != -1)
    {
        m_devices.removeAt(index);

        QValueSpaceItem devicesItem("/MediaServer/Capabilities/Devices");

        QVariantList    deviceList;

        foreach (QMediaDeviceInfo deviceInfo, m_devices)
        {
            deviceList.push_back(qVariantFromValue(deviceInfo));
        }

        devicesItem.setValue(0, deviceList);
    }
}


void DeviceManager::deviceAvailable
(
 QMediaDeviceManager* manager,
 QMediaDeviceInfo const& mediaDevice
)
{
    addMediaDeviceToValueSpace(mediaDevice);

    m_deviceMap.insert(mediaDevice.id(), manager);
}

void DeviceManager::deviceUnavailable
(
 QMediaDeviceManager* manager,
 QMediaDeviceInfo const& mediaDevice
)
{
    Q_UNUSED(manager);

    removeMediaDeviceFromValueSpace(mediaDevice);

    m_deviceMap.remove(mediaDevice.id());
}

} // ns mediaserver


