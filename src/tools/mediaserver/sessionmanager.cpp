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

#include <qdebug.h>
#include <qtimer.h>
#include <qtopiaipcenvelope.h>

#include "mediaengine.h"
#include "mediasession.h"

#include "sessionmanager.h"


namespace mediaserver
{


// {{{ SessionManagerSession
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

    QtopiaMedia::State playerState() const
    {
        return m_mediaSession->playerState();
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
        switch (state)
        {
        case QtopiaMedia::Playing:
            m_sessionManager->sessionPlaying(m_mediaSession);
            break;

        case QtopiaMedia::Stopped:
        case QtopiaMedia::Error:
            m_sessionManager->sessionStopped(m_mediaSession);
            break;

        default:
            ;
        }

        emit playerStateChanged(state);
    }
};
// }}}

// {{{ SessionManagerPrivate
class SessionManagerPrivate
{
public:
    MediaEngine*            mediaEngine;
    QList<MediaSession*>    suspendedSessions;
    QList<MediaSession*>    nonActivatedSessions;
    MediaSession*           activeSession;
};
// }}}

// {{{ SessionManager
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
        sessionStopped(mediaSession);

        d->activeSession = 0;
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

void SessionManager::setVolume(int volume)
{
    d->activeSession->setVolume(volume);
}

void SessionManager::increaseVolume(int increment)
{
    int volume = d->activeSession->volume();
    if (volume + increment > 100)
        d->activeSession->setVolume(100);
    else
        d->activeSession->setVolume(volume + increment);
}

void SessionManager::decreaseVolume(int decrement)
{
    int volume = d->activeSession->volume();
    if (volume - decrement < 0)
        d->activeSession->setVolume(0);
    else
        d->activeSession->setVolume(volume - decrement);
}

void SessionManager::setMuted(bool mute)
{
    d->activeSession->setMuted(mute);
}


void SessionManager::rescheduleSessions()
{
    bool        newActiveSession = false;

    QString domain = d->mediaEngine->activeDomain();

    if (domain.isEmpty())
        domain = "Default";

    if (d->activeSession == 0 ||
        d->activeSession->playerState() == QtopiaMedia::Stopped ||
        d->activeSession->playerState() == QtopiaMedia::Error)
    {
        if (d->nonActivatedSessions.size() > 0)
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

                    newActiveSession = true;
                    break;
                }
            }
        }

        if (!newActiveSession && d->suspendedSessions.size() > 0)
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
    else
    {
        if (domain.compare(d->activeSession->domain(), Qt::CaseInsensitive) != 0)
        {
            d->suspendedSessions.push_front(d->activeSession);
            d->activeSession->suspend();
            d->activeSession = 0;

            QTimer::singleShot(500, this, SLOT(rescheduleSessions()));
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
    else if (activeDomain.compare("RingTone", Qt::CaseInsensitive) == 0 &&
             mediaSession->domain().compare("RingTone", Qt::CaseInsensitive) != 0)
    {
        d->nonActivatedSessions.push_front(mediaSession);
        rc = false;
    }
    else if (d->activeSession == 0)
    {
        d->activeSession = mediaSession;
        rc = true;
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

void SessionManager::sessionPlaying(MediaSession* mediaSession)
{
    Q_UNUSED(mediaSession);

    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "setActiveDomain(QString)");

    e << QString("Media");
}

void SessionManager::sessionStopped(MediaSession* mediaSession)
{
    if (mediaSession == d->activeSession)
    {
        QTimer::singleShot(500, this, SLOT(rescheduleSessions()));
    }

    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "resetActiveDomain(QString)");

    e << QString("Media");
}
// }}}

}   // ns mediaserver


#include "sessionmanager.moc"

