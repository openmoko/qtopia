/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include <qtopialog.h>

#include <qmediaserversession.h>

#include "mediacontrolserver.h"


namespace mediaserver
{

MediaControlServer::MediaControlServer
(
 QMediaServerSession* mediaSession,
 QMediaHandle const& handle
):
    QMediaAbstractControlServer(handle, "Basic"),
    m_mediaSession(mediaSession)
{
    // connect
    connect(m_mediaSession, SIGNAL(playerStateChanged(QtopiaMedia::State)),
            this, SLOT(stateChanged(QtopiaMedia::State)));

    connect(m_mediaSession, SIGNAL(positionChanged(quint32)),
            this, SLOT(posChanged(quint32)));

    connect(m_mediaSession, SIGNAL(lengthChanged(quint32)),
            this, SLOT(lenChanged(quint32)));

    connect(m_mediaSession, SIGNAL(volumeChanged(int)),
            this, SLOT(volChanged(int)));

    connect(m_mediaSession, SIGNAL(volumeMuted(bool)),
            this, SLOT(volMuted(bool)));

    // set initial values
    setValue(QLatin1String("playerState"), QtopiaMedia::Stopped);
    setValue(QLatin1String("length"), m_mediaSession->length());
    setValue(QLatin1String("volume"), m_mediaSession->volume());
    setValue(QLatin1String("muted"), m_mediaSession->isMuted());
    setValue(QLatin1String("errorString"), QString());

    proxyAll();
}

MediaControlServer::~MediaControlServer()
{
}

QMediaServerSession* MediaControlServer::mediaSession() const
{
    return m_mediaSession;
}

// public slots:
void MediaControlServer::start()
{
    m_mediaSession->start();
}

void MediaControlServer::pause()
{
    m_mediaSession->pause();
}

void MediaControlServer::stop()
{
    m_mediaSession->stop();
}

void MediaControlServer::seek(quint32 ms)
{
    m_mediaSession->seek(ms);
}

void MediaControlServer::setVolume(int volume)
{
    m_mediaSession->setVolume(volume);
}

void MediaControlServer::setMuted(bool mute)
{
    m_mediaSession->setMuted(mute);
}

void MediaControlServer::stateChanged(QtopiaMedia::State state)
{
    setValue("playerState", state);

    if (state == QtopiaMedia::Error)
    {
        setValue("errorString", m_mediaSession->errorString());
    }

    emit playerStateChanged(state);
}

void MediaControlServer::posChanged(quint32 ms)
{
    setValue("position", ms);

    emit positionChanged(ms);
}

void MediaControlServer::lenChanged(quint32 ms)
{
    setValue("length", ms);

    emit lengthChanged(ms);
}

void MediaControlServer::volChanged(int volume)
{
    setValue("volume", volume);

    emit volumeChanged(volume);
}

void MediaControlServer::volMuted(bool muted)
{
    setValue("muted", muted);

    emit volumeMuted(muted);
}

}   // ns mediaserver


