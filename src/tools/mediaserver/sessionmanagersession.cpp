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

#include <QDebug>

#include "sessionmanager.h"

#include "sessionmanagersession.h"


namespace mediaserver
{

// {{{ SessionManagerSessionPrivate
class SessionManagerSessionPrivate
{
public:
    bool                    activated;
    QMediaServerSession*    wrapped;
    SessionManager*         manager;
    QtopiaMedia::State      selectState;
};
// }}}

// {{{ SessionManagerSession
SessionManagerSession::SessionManagerSession(SessionManager* manager, QMediaServerSession* wrapped):
    d(new SessionManagerSessionPrivate)
{
    d->activated = false;
    d->manager = manager;
    d->wrapped = wrapped;
    d->selectState = QtopiaMedia::Stopped;

    // forward signals
    connect(wrapped, SIGNAL(playerStateChanged(QtopiaMedia::State)), this, SIGNAL(playerStateChanged(QtopiaMedia::State)));
    connect(wrapped, SIGNAL(positionChanged(quint32)), this, SIGNAL(positionChanged(quint32)));
    connect(wrapped, SIGNAL(lengthChanged(quint32)), this, SIGNAL(lengthChanged(quint32)));
    connect(wrapped, SIGNAL(volumeChanged(int)), this, SIGNAL(volumeChanged(int)));
    connect(wrapped, SIGNAL(volumeMuted(bool)), this, SIGNAL(volumeMuted(bool)));
    connect(wrapped, SIGNAL(interfaceAvailable(QString)), this, SIGNAL(interfaceAvailable(QString)));
    connect(wrapped, SIGNAL(interfaceUnavailable(QString)), this, SIGNAL(interfaceUnavailable(QString)));

    connect(wrapped, SIGNAL(playerStateChanged(QtopiaMedia::State)),
            this, SLOT(sessionStateChanged(QtopiaMedia::State)));
}

SessionManagerSession::~SessionManagerSession()
{
    delete d;
}

QMediaServerSession* SessionManagerSession::wrappedSession() const
{
    return d->wrapped;
}

void SessionManagerSession::activate()
{
    if (d->selectState == QtopiaMedia::Playing)
        d->wrapped->start();
}

void SessionManagerSession::deactivate()
{
    if (d->selectState == QtopiaMedia::Playing)
        d->wrapped->pause();
}

void SessionManagerSession::start()
{
    if (d->manager->sessionCanStart(this))
        d->wrapped->start();

    d->selectState = QtopiaMedia::Playing;
}

void SessionManagerSession::pause()
{
    d->wrapped->pause();

    d->selectState = QtopiaMedia::Paused;
}

void SessionManagerSession::stop()
{
    d->wrapped->stop();

    d->selectState = QtopiaMedia::Stopped;
}

void SessionManagerSession::suspend()
{
    d->wrapped->suspend();
}

void SessionManagerSession::resume()
{
    d->wrapped->resume();
}

void SessionManagerSession::seek(quint32 ms)
{
    d->wrapped->seek(ms);
}

quint32 SessionManagerSession::length()
{
    return d->wrapped->length();
}

void SessionManagerSession::setVolume(int volume)
{
    d->wrapped->setVolume(volume);
}

int SessionManagerSession::volume() const
{
    return d->wrapped->volume();
}

void SessionManagerSession::setMuted(bool mute)
{
    d->wrapped->setMuted(mute);
}

bool SessionManagerSession::isMuted() const
{
    return d->wrapped->isMuted();
}

QtopiaMedia::State SessionManagerSession::playerState() const
{
    return d->wrapped->playerState();
}

QString SessionManagerSession::errorString()
{
    return d->wrapped->errorString();
}

void SessionManagerSession::setDomain(QString const& domain)
{
    d->wrapped->setDomain(domain);
}

QString SessionManagerSession::domain() const
{
    return d->wrapped->domain();
}

QStringList SessionManagerSession::interfaces()
{
    return d->wrapped->interfaces();
}

QString SessionManagerSession::id() const
{
    return d->wrapped->id();
}

QString SessionManagerSession::reportData() const
{
    return d->wrapped->reportData();
}

void SessionManagerSession::sessionStateChanged(QtopiaMedia::State state)
{
    if (state == QtopiaMedia::Stopped)
    {
        d->selectState = QtopiaMedia::Stopped;
        d->manager->sessionStopped(this);
    }
}
// }}}

}   // ns mediaserver

