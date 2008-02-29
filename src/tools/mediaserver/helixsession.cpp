/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include <qmediacontrol.h>
#include <qmediavideocontrol.h>

#include "helixplayer.h"
#include "videocontrolserver.h"
#include "qmediahelixsettingsserver.h"

#include "helixsession.h"


using namespace mediaserver;


namespace qtopia_helix
{

class HelixSessionPrivate
{
public:
    QUuid               id;
    QString             url;
    QStringList         interfaces;
    IHXClientEngine*    engine;
    HelixPlayer*        player;
    VideoControlServer* videoControlServer;
    QWidget*            videoWidget;
    bool                ismute;
    bool                suspended;
    quint32             position;
    quint32             oldposition;
    quint32             length;
    QString             domain;
};

HelixSession::HelixSession(IHXClientEngine* engine, QUuid const& id, QString const& url):
    d(new HelixSessionPrivate)
{
    d->id = id;
    d->url = url;
    d->interfaces << QMediaControl::name();
    d->engine = engine;
    d->videoControlServer = 0;
    d->videoWidget = 0;
    d->ismute = false;
    d->suspended = false;
    d->position = 0;
    d->length = 0;

    startupPlayer();
}

HelixSession::~HelixSession()
{
    shutdownPlayer();

    delete d->videoControlServer;
    delete d->videoWidget;
}

// {{{ MediaSession
void HelixSession::start()
{
    if (d->player->playerState() == QtopiaMedia::Stopped) {
        d->player->open(d->url);
    }
    d->player->play();
}

void HelixSession::pause()
{
    d->player->pause();
}

void HelixSession::stop()
{
    d->player->stop();
}

void HelixSession::suspend()
{
    pause();
    d->oldposition = d->player->position();
    d->suspended = true;
    stop();
}

void HelixSession::resume()
{
    start();
    QTimer::singleShot(50, this, SLOT(delaySeek()));
}

void HelixSession::seek(quint32 ms)
{
    d->player->seek(ms);
}

quint32 HelixSession::length()
{
    return d->player->length();
}

void HelixSession::setVolume(int volume)
{
    d->player->setVolume(volume);
}

int HelixSession::volume() const
{
    return d->player->volume();
}

void HelixSession::setMuted(bool mute)
{
    d->player->setMuted(mute);
}

bool HelixSession::isMuted() const
{
    return d->ismute;
}

QString HelixSession::errorString()
{
    return d->player->errorString();
}

void HelixSession::setDomain(QString const& domain)
{
    d->domain = domain;
}

QString HelixSession::domain() const
{
    return d->domain;
}

QStringList HelixSession::interfaces()
{
    return d->interfaces;
}
// }}}

// {{{ Observer
void HelixSession::update(Subject* subject)
{
    static const int MS_PER_S = 1000; // 1000 milliseconds per second

    PlayerState*    playerState;
    VideoRender*    videoRender;
    ErrorReport*    errorReport;

    if ((playerState = qobject_cast<PlayerState*>(d->player)) == subject)
    {
        if (!d->suspended)
            emit playerStateChanged(playerState->playerState());
    }
    else
    if (qobject_cast<PlaybackStatus*>(d->player) == subject)
    {
        quint32 position = (d->player->position() / MS_PER_S) * MS_PER_S;
        if( d->position != position ) {
            d->position = position;
            emit positionChanged(d->position);
        }

        quint32 length = d->player->length();
        if( d->length != length ) {
            d->length = length;
            emit lengthChanged(d->length);
        }
    }
    else
    if ((qobject_cast<VolumeControl*>(d->player)) == subject)
    {
        emit volumeChanged(d->player->volume());

        bool mute = d->player->isMuted();
        if( d->ismute != mute ) {
            d->ismute = mute;

            emit volumeMuted(d->ismute);
        }
    }
    else
    if ((videoRender = qobject_cast<VideoRender*>(d->player)) == subject)
    {
        if (videoRender->hasVideo())
        {
            d->videoControlServer = new mediaserver::VideoControlServer(d->id);

            d->videoWidget = videoRender->createVideoWidget();
            d->videoWidget->setWindowFlags( Qt::Tool | Qt::FramelessWindowHint );

            d->videoControlServer->setWId(d->videoWidget->winId());

            d->interfaces.append(QMediaVideoControl::name());
            emit interfaceAvailable(QMediaVideoControl::name());
        }
    }
    else
    if ((errorReport = qobject_cast<ErrorReport*>(d->player)) == subject)
    {
        emit playerStateChanged(QtopiaMedia::Error);
    }
}
// }}}

void HelixSession::delaySeek()
{
    static int shitcount = 0;

    if (d->suspended && shitcount++ > 2)
    {
        d->suspended = false;
        seek(d->oldposition);
        d->player->play();
        shitcount = 0;
    }
    else
        QTimer::singleShot(200, this, SLOT(delaySeek()));
}

void HelixSession::startupPlayer()
{
    d->player = new HelixPlayer(d->engine);

    PlayerState *playerState = qobject_cast<PlayerState*>(d->player);
    if (playerState != 0)
        playerState->attach(this);

    ErrorReport *error = qobject_cast<ErrorReport*>(d->player);
    if (error != 0)
        error->attach(this);

    PlaybackStatus *status = qobject_cast<PlaybackStatus*>(d->player);
    if (status != 0)
        status->attach(this);

    VolumeControl *volume = qobject_cast<VolumeControl*>(d->player);
    if (volume != 0)
        volume->attach(this);

    VideoRender *videoRender = qobject_cast<VideoRender*>(d->player);
    if (videoRender != 0)
        videoRender->attach(this);
}

void HelixSession::shutdownPlayer()
{
    PlayerState *playerState = qobject_cast<PlayerState*>(d->player);
    if (playerState != 0)
        playerState->detach(this);

    ErrorReport *error = qobject_cast<ErrorReport*>(d->player);
    if (error != 0)
        error->detach(this);

    PlaybackStatus *status = qobject_cast<PlaybackStatus*>(d->player);
    if (status != 0)
        status->detach(this);

    VolumeControl *volume = qobject_cast<VolumeControl*>(d->player);
    if (volume != 0)
        volume->detach(this);

    VideoRender *videoRender = qobject_cast<VideoRender*>(d->player);
    if (videoRender != 0)
        videoRender->detach(this);

    delete d->player;
}


}   // ns qtopia_helix


