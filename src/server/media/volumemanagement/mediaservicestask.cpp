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

#include "audiovolumemanager.h"
#include "mediakeyservice.h"
#include "mediaservicestask.h"


/*!
    \class MediaServicesTask
    \ingroup QtopiaServer::Task

    \brief The MediaServicesTask class provides a task that manages services related to Media in Qtopia

    This class is used to manage media related facilities in Qtopia. It watches
    media key events and forwards them to the appropriate party.

    This class is part of the Qtopia server and cannot be used by other Qtopia applications.
*/


/*!
    \internal
*/

MediaServicesTask::MediaServicesTask()
{
    m_avm = new AudioVolumeManager();

#ifndef QT_ILLUME_LAUNCHER
    m_mks = new MediaKeyService(m_avm);
    connect(m_mks, SIGNAL(volumeChanged(bool)),
            this, SIGNAL(volumeChanged(bool)));
#endif
}

/*!
    \internal
*/

MediaServicesTask::~MediaServicesTask()
{
    delete m_avm;
}

/*!
    \internal
    Connect key management with volume management
*/

void MediaServicesTask::setVolume(bool up)
{
#ifndef QT_ILLUME_LAUNCHER
    m_mks->setVolume(up);
#endif
}

/*!
    \fn MediaServicesTask::volumeChanged(bool)
    \internal
*/

QTOPIA_TASK(MediaServicesTask, MediaServicesTask);
QTOPIA_TASK_PROVIDES(MediaServicesTask, MediaServicesTask);

