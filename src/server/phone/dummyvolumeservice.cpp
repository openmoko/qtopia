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

#include <qtopiaserverapplication.h>
#include <qtopiaipcenvelope.h>
#include <QTimer>

#include "dummyvolumeservice.h"


DummyVolumeService::DummyVolumeService():
    QtopiaIpcAdaptor("QPE/AudioVolumeManager/DummyVolumeService")
{
    QTimer::singleShot(0, this, SLOT(registerService()));
}

DummyVolumeService::~DummyVolumeService()
{
}

void DummyVolumeService::registerService()
{
    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "registerHandler(QString,QString)");

    e << QString("Headset") << QString("QPE/AudioVolumeManager/DummyVolumeService");

    QTimer::singleShot(0, this, SLOT(setCallDomain()));
}

void DummyVolumeService::setCallDomain()
{
    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "setActiveDomain(QString)");

    e << QString("Headset");
}


QTOPIA_TASK(DummyVolumeService, DummyVolumeService);

