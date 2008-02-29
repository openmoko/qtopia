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

#ifndef __QTOPIA_MEDIADEVICEMANAGER_MEDIADEVICEMANAGER_H
#define __QTOPIA_MEDIADEVICEMANAGER__MEDIADEVICEMANAGER_H


#include <qobject.h>

#include <qtopia/qpluginmanager.h>

#include <qtopiamedia/qmediadeviceinfo.h>
#include <qtopiamedia/qmediadevicerep.h>


class QMediaDevice;


class QTOPIAMEDIA_EXPORT QMediaDeviceManager : public QObject
{
    Q_OBJECT

public:
    virtual void init() = 0;

    virtual QMediaDevice* createDeviceInstance(QMediaDeviceRep const& mediaDevice) = 0;

signals:
    void deviceAvailable(QMediaDeviceManager* manager, QMediaDeviceInfo const& mediaDevice);
    void deviceUnavailable(QMediaDeviceManager* manager, QMediaDeviceInfo const& mediaDevice);
};

#endif  // __QTOPIA_MEDIA_MEDIADEVICEMANAGER_H
