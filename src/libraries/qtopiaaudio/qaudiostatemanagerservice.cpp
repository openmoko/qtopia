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

#include "qaudiostatemanagerservice_p.h"

#include <QAudioStateManager>
#include <qaudionamespace.h>

struct QAudioStateManagerServicePrivate
{
    QAudioStateManager *manager;
};

/*!
    \class QAudioStateManagerService
    \internal
*/

/*

    Constructs a new Audio State Manager Service.  The \a parent
    parameter specifies the Audio State Manager for this service.

    The service is created on QPE/AudioStateManager.
*/
QAudioStateManagerService::QAudioStateManagerService(QAudioStateManager *parent)
    : QtopiaIpcAdaptor("QPE/AudioStateManager", parent)
{
    m_data = new QAudioStateManagerServicePrivate;
    m_data->manager = parent;

    publishAll(QtopiaIpcAdaptor::SignalsAndSlots);
}

/*
    Destroys the Audio State Manager Service.
*/
QAudioStateManagerService::~QAudioStateManagerService()
{

}

/*
    Tries to set the Audio State to a state with \a profile.  If
    the state succeeds, the current state will be reported by
    QAudioDeviceConfiguration.  If the setting of the profile
    failed, then the profileFailed() signal will be emitted
*/
void QAudioStateManagerService::setProfile(const QByteArray &profile)
{
    if (!m_data->manager->setProfile(profile))
        emit profileFailed(profile);
}

void QAudioStateManagerService::setDomain(const QByteArray &domain, int capability)
{
    if (!m_data->manager->setDomain(domain, static_cast<QAudio::AudioCapability>(capability)))
        emit domainFailed(domain, capability);
}
