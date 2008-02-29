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

#include <qtopiaipcenvelope.h>
#include <QValueSpaceObject>
#include "audiovolumemanager.h"


AudioVolumeManager::AudioVolumeManager():
    QtopiaIpcAdaptor("QPE/AudioVolumeManager")
{
    publishAll(Slots);
    
    m_vsVolume = new QValueSpaceObject("/Volume");
}

AudioVolumeManager::~AudioVolumeManager()
{
    delete m_vsVolume;
}

bool AudioVolumeManager::canManageVolume() const
{
    return !m_domains.isEmpty() &&
            m_vsps.find(m_domains.front()) != m_vsps.end();
}


//public slots:
void AudioVolumeManager::setVolume(int volume)
{
    QString provider = findProvider();
    if (!provider.isEmpty())
    {
        QtopiaIpcEnvelope e(provider, "setVolume(int)");
        e << volume;
    }
}

void AudioVolumeManager::increaseVolume(int increment)
{
    QString provider = findProvider();
    if (!provider.isEmpty())
    {
        QtopiaIpcEnvelope e(provider, "increaseVolume(int)");
        e << increment;
    }
}

void AudioVolumeManager::decreaseVolume(int decrement)
{
    QString provider = findProvider();
    if (!provider.isEmpty())
    {
        QtopiaIpcEnvelope e(provider, "decreaseVolume(int)");
        e << decrement;
    }
}

void AudioVolumeManager::setMuted(bool mute)
{
    QString provider = findProvider();
    if (!provider.isEmpty())
    {
        QtopiaIpcEnvelope e(provider, "setMuted(bool)");
        e << mute;
    }
}

void AudioVolumeManager::currentVolume(QString const&  volume)
{  
     m_vsVolume->setAttribute("CurrentVolume",volume.toInt());
}

void AudioVolumeManager::registerHandler(QString const& domain, QString const& channel)
{
    m_vsps[domain] = channel;
}

void AudioVolumeManager::unregisterHandler(QString const& domain, QString const& channel)
{
    VolumeServiceProviders::iterator it = m_vsps.find(domain);
    if (it != m_vsps.end() && (*it).compare(channel) == 0)
        m_vsps.erase(it);
}

void AudioVolumeManager::setActiveDomain(QString const& activeDomain)
{
    m_domains.push_front(activeDomain);
}

void AudioVolumeManager::resetActiveDomain(QString const& oldDomain)
{
    m_domains.removeAll(oldDomain);
}

QString AudioVolumeManager::findProvider() const
{
    QString     domain;
    QString     provider;

    if (!m_domains.isEmpty())
    {
        domain = m_domains.front();

        VolumeServiceProviders::const_iterator it = m_vsps.find(domain);
        if (it != m_vsps.end())
            provider = *it;
    }

    return provider;
}
