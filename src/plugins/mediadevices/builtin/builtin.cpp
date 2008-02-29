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

#include <qtopiaglobal.h>
#include "qtopiaaudioinputdevice.h"
#include "qtopiaaudiooutputdevice.h"

#include "builtin.h"



BuiltinMediaDeviceManagerFactory::BuiltinMediaDeviceManagerFactory()
{
}

BuiltinMediaDeviceManagerFactory::~BuiltinMediaDeviceManagerFactory()
{
}

QMediaDeviceManager* BuiltinMediaDeviceManagerFactory::create()
{
    return new BuiltinMediaDeviceManager;
}


BuiltinMediaDeviceManager::BuiltinMediaDeviceManager()
{
}

BuiltinMediaDeviceManager::~BuiltinMediaDeviceManager()
{
}

static
QMediaDeviceInfo makeDeviceInfo(QString const& name, bool isReadable, bool isWritable)
{
    QByteArray          rawData;
    QDataStream         ds;
    QMediaDeviceInfo    rc;

    {
        QDataStream ds(&rawData, QIODevice::WriteOnly);

        ds << name << name << isReadable << isWritable;
    }
    {
        QDataStream ds(&rawData, QIODevice::ReadOnly);
        ds >> rc;
    }

    return rc;
}

void BuiltinMediaDeviceManager::init()
{
    // TODO: better way (but not open way) to create deviceInfo)
    m_inputDevice = makeDeviceInfo("Builtin Audio Input Device", true, false);
    m_outputDevice = makeDeviceInfo("Builtin Audio Output Device", false, true);

    emit deviceAvailable(this, m_inputDevice);
    emit deviceAvailable(this, m_outputDevice);
}

QMediaDevice* BuiltinMediaDeviceManager::createDeviceInstance
(
 QMediaDeviceRep const& mediaDevice
)
{
    if (mediaDevice.id() == m_inputDevice.id())
        return new QtopiaAudioInputDevice(mediaDevice.options());

    if (mediaDevice.id() == m_outputDevice.id())
        return new QtopiaAudioOutputDevice(mediaDevice.options());

    return 0;
}

QTOPIA_EXPORT_PLUGIN(BuiltinMediaDeviceManagerFactory);

