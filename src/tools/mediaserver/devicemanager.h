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

#ifndef __QTOPIA_MEDIASERVER_DEVICEMANAGER_H
#define __QTOPIA_MEDIASERVER_DEVICEMANAGER_H

#include <qobject.h>
#include <qstring.h>
#include <qlist.h>
#include <qmap.h>

#include <qmediadeviceinfo.h>
#include <qmediadevicerep.h>



class QPluginManager;
class QMediaDeviceManager;
class QMediaDevice;

namespace mediaserver
{

class DeviceManager : public QObject
{
    Q_OBJECT

    typedef QList<QMediaDeviceManager*>     DeviceManagers;
    typedef QList<QMediaDeviceInfo>         Devices;
    typedef QMap<QString, QMediaDeviceManager*> MediaDeviceMap;

public:
    DeviceManager();
    ~DeviceManager();

    void init();

    QMediaDevice* createMediaDevice(QMediaDeviceRep const& mediaDevice);

    static DeviceManager* instance();

private slots:
    void deviceAvailable(QMediaDeviceManager* manager, QMediaDeviceInfo const& mediaDevice);
    void deviceUnavailable(QMediaDeviceManager* manager, QMediaDeviceInfo const& mediaDevice);

private:
    void addMediaDeviceToValueSpace(QMediaDeviceInfo const& mediaDevice);
    void removeMediaDeviceFromValueSpace(QMediaDeviceInfo const& mediaDevice);

    DeviceManagers      m_managers;
    MediaDeviceMap      m_deviceMap;
    Devices             m_devices;
    QPluginManager*     m_pluginManager;
};

}   // ns mediaserver

#endif  // __QTOPIA_MEDIASERVER_DEVICEMANAGER_H
