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

#include <qdrmcontent.h>

#include "mediasession.h"

#include "drmsession.h"

namespace mediaserver
{


class DrmSessionPrivate
{
public:
    MediaSession*   mediaSession;
    QDrmContent     content;
    bool            drmContent;
    bool            haveLicense;
};


DrmSession::DrmSession(QString const& url, MediaSession* mediaSession):
    d(new DrmSessionPrivate)
{
    // init
    d->mediaSession = mediaSession;
    d->haveLicense = false;

    if (url.startsWith("qtopia://"))
    {
        QContent content(url.mid(9));   // sizeof("qtopia://")

        if (content.isValid() &&
            content.drmState() == QContent::Protected)
        {
            d->drmContent = true;
            d->content.setPermission(QDrmRights::Play);
            d->haveLicense = d->content.requestLicense( content );
        }
    }

    // XXX: Re-emit
    connect(d->mediaSession, SIGNAL(playerStateChanged(QtopiaMedia::State)),
            this, SIGNAL(playerStateChanged(QtopiaMedia::State)));
    connect(d->mediaSession, SIGNAL(positionChanged(quint32)),
            this, SIGNAL(positionChanged(quint32)));
    connect(d->mediaSession, SIGNAL(lengthChanged(quint32)),
            this, SIGNAL(lengthChanged(quint32)));
    connect(d->mediaSession, SIGNAL(volumeChanged(int)),
            this, SIGNAL(volumeChanged(int)));
    connect(d->mediaSession, SIGNAL(volumeMuted(bool)),
            this, SIGNAL(volumeMuted(bool)));
    connect(d->mediaSession, SIGNAL(interfaceAvailable(const QString&)),
            this, SIGNAL(interfaceAvailable(const QString&)));
}

DrmSession::~DrmSession()
{
    delete d->mediaSession;
    delete d;
}

void DrmSession::start()
{
    d->mediaSession->start();

    if (d->haveLicense)
        d->content.renderStarted();
}

void DrmSession::pause()
{
    d->mediaSession->pause();

    if (d->haveLicense)
        d->content.renderPaused();
}

void DrmSession::stop()
{
    d->mediaSession->stop();

    if (d->haveLicense)
        d->content.renderStopped();
}

void DrmSession::suspend()
{
    d->mediaSession->suspend();
}

void DrmSession::resume()
{
    d->mediaSession->resume();
}

void DrmSession::seek(quint32 ms)
{
    d->mediaSession->seek(ms);
}

quint32 DrmSession::length()
{
    return d->mediaSession->length();
}

void DrmSession::setVolume(int volume)
{
    d->mediaSession->setVolume(volume);
}

int DrmSession::volume() const
{
    return d->mediaSession->volume();
}

void DrmSession::setMuted(bool mute)
{
    d->mediaSession->setMuted(mute);
}

bool DrmSession::isMuted() const
{
    return d->mediaSession->isMuted();
}

QString DrmSession::errorString()
{
    return d->mediaSession->errorString();
}

void DrmSession::setDomain(QString const& domain)
{
    d->mediaSession->setDomain(domain);
}

QString DrmSession::domain() const
{
    return d->mediaSession->domain();
}

QStringList DrmSession::interfaces()
{
    return d->mediaSession->interfaces();
}

}   // ns mediaserver
