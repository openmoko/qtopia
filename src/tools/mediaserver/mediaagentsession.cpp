/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include "mediaagent.h"

#include "mediaagentsession.h"

/*!
    \class mediaserver::MediaAgentSession
    \internal
*/

namespace mediaserver
{

// {{{ MediaAgentSessionPrivate
class MediaAgentSessionPrivate
{
public:
    MediaAgent*             agent;
    QMediaServerSession*    wrapped;
};
// }}}

// {{{ MediaAgentSession
MediaAgentSession::MediaAgentSession(MediaAgent* agent, QMediaServerSession* wrapped):
    d(new MediaAgentSessionPrivate)
{
    d->agent = agent;
    d->wrapped = wrapped;

    // forward signals
    connect(wrapped, SIGNAL(playerStateChanged(QtopiaMedia::State)), this, SIGNAL(playerStateChanged(QtopiaMedia::State)));
    connect(wrapped, SIGNAL(positionChanged(quint32)), this, SIGNAL(positionChanged(quint32)));
    connect(wrapped, SIGNAL(lengthChanged(quint32)), this, SIGNAL(lengthChanged(quint32)));
    connect(wrapped, SIGNAL(volumeChanged(int)), this, SIGNAL(volumeChanged(int)));
    connect(wrapped, SIGNAL(volumeMuted(bool)), this, SIGNAL(volumeMuted(bool)));
    connect(wrapped, SIGNAL(interfaceAvailable(QString)), this, SIGNAL(interfaceAvailable(QString)));
    connect(wrapped, SIGNAL(interfaceUnavailable(QString)), this, SIGNAL(interfaceUnavailable(QString)));
}

MediaAgentSession::~MediaAgentSession()
{
    delete d;
}

QMediaServerSession* MediaAgentSession::wrappedSession() const
{
    return d->wrapped;
}

void MediaAgentSession::start()
{
    d->agent->sessionStarting(this);
}

void MediaAgentSession::pause()
{
    d->wrapped->pause();
}

void MediaAgentSession::stop()
{
    d->agent->sessionStopped(this);
}

void MediaAgentSession::suspend()
{
    d->wrapped->suspend();
}

void MediaAgentSession::resume()
{
    d->wrapped->resume();
}

void MediaAgentSession::seek(quint32 ms)
{
    d->wrapped->seek(ms);
}

quint32 MediaAgentSession::length()
{
    return d->wrapped->length();
}

void MediaAgentSession::setVolume(int volume)
{
    d->wrapped->setVolume(volume);
}

int MediaAgentSession::volume() const
{
    return d->wrapped->volume();
}

void MediaAgentSession::setMuted(bool mute)
{
    d->wrapped->setMuted(mute);
}

bool MediaAgentSession::isMuted() const
{
    return d->wrapped->isMuted();
}


QtopiaMedia::State MediaAgentSession::playerState() const
{
    return d->wrapped->playerState();
}

QString MediaAgentSession::errorString()
{
    return d->wrapped->errorString();
}

void MediaAgentSession::setDomain(QString const& domain)
{
    d->wrapped->setDomain(domain);
}

QString MediaAgentSession::domain() const
{
    return d->wrapped->domain();
}


QStringList MediaAgentSession::interfaces()
{
    return d->wrapped->interfaces();
}

QString MediaAgentSession::id() const
{
    return d->wrapped->id();
}

QString MediaAgentSession::reportData() const
{
    return d->wrapped->reportData();
}
// }}}

}   // ns mediaserver
