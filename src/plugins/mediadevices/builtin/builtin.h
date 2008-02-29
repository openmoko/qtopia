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

#ifndef __QTIOIA_MEDIADEVICES_BULITIN_H
#define __QTIOIA_MEDIADEVICES_BULITIN_H

#include <qtopiamedia/qmediadevicemanagerfactory.h>
#include <qtopiamedia/qmediadevicemanager.h>
#include <qtopiamedia/qmediadeviceinfo.h>
#include <qtopiamedia/qmediadevicerep.h>

class BuiltinMediaDeviceManagerFactory :
    public QObject,
    public QMediaDeviceManagerFactory
{
    Q_OBJECT
    Q_INTERFACES(QMediaDeviceManagerFactory)

public:

    BuiltinMediaDeviceManagerFactory();
    ~BuiltinMediaDeviceManagerFactory();

    QMediaDeviceManager* create();
};


class BuiltinMediaDeviceManager : public QMediaDeviceManager
{
public:
    BuiltinMediaDeviceManager();
    ~BuiltinMediaDeviceManager();

    void init();

    QMediaDevice* createDeviceInstance(QMediaDeviceRep const& mediaDevice);

private:
    QMediaDeviceInfo    m_inputDevice;
    QMediaDeviceInfo    m_outputDevice;
};

#endif  // __QTIOIA_MEDIADEVICES_BULITIN_H
