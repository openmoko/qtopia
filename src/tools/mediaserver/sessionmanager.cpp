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

#include <qtimer.h>

#include "mediaengine.h"
#include "mediasession.h"

#include "sessionmanager.h"


namespace mediaserver
{


class SessionManagerSession : public MediaSession
{
    Q_OBJECT

public:
    SessionManagerSession(MediaSession* mediaSession,
                          SessionManager* sessionManager):
        m_mediaSession(mediaSession),
        m_sessionManager(sessionManager)
    {
        // XXX: Re-emit
        connect(m_mediaSession, SIGNAL(playerStateChanged(QtopiaMedia::State)),
                this, SLOT(stateChanged(QtopiaMedia::State)));
        connect(m_mediaSession, SIGNAL(positionChanged(quint32)),
                this, SIGNAL(positionChanged(quint32)));
        connect(m_mediaSession, SIGNAL(lengthChanged(quint32)),
                this, SIGNAL(lengthChanged(quint32)));
        connect(m_mediaSession, SIGNAL(volumeChanged(int)),
                this, SIGNAL(volumeChanged(int)));
        connect(m_mediaSession, SIGNAL(volumeMuted(bool)),
                this, SIGNAL(volumeMuted(bool)));
        connect(m_mediaSession, SIGNAL(interfaceAvailable(const QString&)),
                this, SIGNAL(interfaceAvailable(const QString&)));
        connect(m_mediaSession, SIGNAL(interfaceUnavailable(const QString&)),
                this, SIGNAL(interfaceUnavailable(const QString&)));
    }

    ~SessionManagerSession()
    {
        delete m_mediaSession;
    }

    void start()
    {
        if (m_sessionManager->canStart(m_mediaSession))
            m_mediaSession->start();
    }

    void pause()
    {
        m_mediaSession->pause();
    }

    void stop()
    {
        m_mediaSession->stop();
    }


    void suspend()
    {
        m_mediaSession->suspend();
    }

    void resume()
    {
        m_mediaSession->resume();
    }

    void seek(quint32 ms)
    {
        m_mediaSession->seek(ms);
    }

    quint32 length()
    {
        return m_mediaSession->length();
    }

    void setVolume(int volume)
    {
        m_mediaSession->setVolume(volume);
    }

    int volume() const
    {
        return m_mediaSession->volume();
    }

    void setMuted(bool mute)
    {
        m_mediaSession->setMuted(mute);
    }

    bool isMuted() const
    {
        return m_mediaSession->isMuted();
    }

    QString errorString()
    {
        return m_mediaSession->errorString();
    }

    void setDomain(QString const& domain)
    {
        m_mediaSession->setDomain(domain);
    }

    QString domain() const
    {
        return m_mediaSession->domain();
    }

    QStringList interfaces()
    {
        return m_mediaSession->interfaces();
    }

    MediaSession*   m_mediaSession;
    SessionManager* m_sessionManager;

private slots:
    void stateChanged(QtopiaMedia::State state)
    {
        if (state == QtopiaMedia::Stopped ||
            state == QtopiaMedia::Error)
        {
            m_sessionManager->stopped(m_mediaSession);
        }

        emit playerStateChanged(state);
    }
};


class SessionManagerPrivate
{
public:
    MediaEngine*            mediaEngine;
    QList<MediaSession*>    suspendedSessions;
    QList<MediaSession*>    nonActivatedSessions;
    MediaSession*           activeSession;
};


SessionManager::SessionManager(MediaEngine* mediaEngine):
    d(new SessionManagerPrivate)
{
    d->mediaEngine = mediaEngine;
    d->activeSession = 0;
}

SessionManager::~SessionManager()
{
    delete d;
}


MediaSession* SessionManager::manageSession(MediaSession* mediaSession)
{
    return new SessionManagerSession(mediaSession, this);
}

void SessionManager::releaseSession(MediaSession* mediaSession)
{
    SessionManagerSession*  sms = qobject_cast<SessionManagerSession*>(mediaSession);

    if (sms != 0)
        mediaSession = sms->m_mediaSession;

    if (d->activeSession == mediaSession)
    {
        d->activeSession = 0;
        QTimer::singleShot(500, this, SLOT(rescheduleSessions()));
    }
    else
    {
        int index;
        if ((index = d->nonActivatedSessions.indexOf(mediaSession)) != -1)
            d->nonActivatedSessions.removeAt(index);
        else
        if ((index = d->suspendedSessions.indexOf(mediaSession)) != -1)
            d->suspendedSessions.removeAt(index);
    }
}

void SessionManager::activeDomainChanged()
{
    QTimer::singleShot(500, this, SLOT(rescheduleSessions()));
}

void SessionManager::rescheduleSessions()
{
    QString domain = d->mediaEngine->activeDomain();

    if (d->activeSession == 0)
    {
        // Check non-activated sessions in same domain
        QList<MediaSession*>::iterator  it = d->nonActivatedSessions.begin();

        for (;it != d->nonActivatedSessions.end(); ++it)
        {
            if (domain.compare((*it)->domain(), Qt::CaseInsensitive) == 0)
            {   // OK, activate it
                d->activeSession = *it;
                d->nonActivatedSessions.erase(it);

                d->activeSession->start();
                break;
            }
        }
    }

    if (d->activeSession == 0)
    {
        // Check suspended
        QList<MediaSession*>::iterator it = d->suspendedSessions.begin();

        for (;it != d->suspendedSessions.end(); ++it)
        {
            if (domain.compare((*it)->domain(), Qt::CaseInsensitive) == 0)
            {
                d->activeSession = *it;
                d->suspendedSessions.erase(it);

                d->activeSession->resume();
                break;
            }
        }
    }
}

bool SessionManager::canStart(MediaSession* mediaSession)
{
    bool    rc;
    QString activeDomain = d->mediaEngine->activeDomain();

    if (mediaSession == d->activeSession)
    {
        rc = true;
    }
    else if (d->activeSession == 0)
    {
        d->activeSession = mediaSession;
        rc = true;
    }
    else if (activeDomain.compare("RingTone", Qt::CaseInsensitive) == 0 &&
             mediaSession->domain().compare("RingTone", Qt::CaseInsensitive) != 0)
    {
        d->nonActivatedSessions.push_front(mediaSession);
        rc = false;
    }
    else
    {
        d->suspendedSessions.push_front(d->activeSession);
        d->activeSession->suspend();
        d->activeSession = mediaSession;

        rc = true;
    }

    return rc;
}

void SessionManager::stopped(MediaSession* mediaSession)
{
    if (mediaSession == d->activeSession)
        d->activeSession = 0;

    QTimer::singleShot(500, this, SLOT(rescheduleSessions()));
}


}   // ns mediaserver


#include "sessionmanager.moc"

