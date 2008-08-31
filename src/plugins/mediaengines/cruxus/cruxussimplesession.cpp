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

#include <QMediaDevice>
#include <QMediaDecoder>
#include <QDebug>

#include "cruxusurihandlers.h"
#include "cruxusoutputdevices.h"

#include "cruxussimplesession.h"

#include <qtopialog.h>


namespace cruxus
{

// {{{ SimpleSessionPrivate
class SimpleSessionPrivate
{
public:
    bool                opened;
    bool                connected;
    QMediaDevice*       source;
    QMediaDevice*       sink;
    QMediaDecoder*      coder;
    OutputThread*       thread;

    QMediaHandle        handle;
    QString             domain;
    QString             errorString;
    QtopiaMedia::State  state;
};
// }}}

// {{{ SimpleSession

/*!
    \class cruxus::SimpleSession
    \internal
*/

SimpleSession::SimpleSession
(
 QMediaHandle const&    handle,
 QMediaDevice*          source,
 QMediaDecoder*         coder,
 QMediaDevice*          sink
):
    d(new SimpleSessionPrivate)
{
    d->opened = false;
    d->connected = false;
    d->handle = handle;
    d->source = source;
    d->sink = sink;
    d->coder = coder;
    d->state = QtopiaMedia::Stopped;

    connect(d->coder, SIGNAL(playerStateChanged(QtopiaMedia::State)), SIGNAL(playerStateChanged(QtopiaMedia::State)));
    connect(d->coder, SIGNAL(positionChanged(quint32)), SIGNAL(positionChanged(quint32)));
    connect(d->coder, SIGNAL(lengthChanged(quint32)), SIGNAL(lengthChanged(quint32)));
    connect(d->coder, SIGNAL(volumeChanged(int)),SIGNAL(volumeChanged(int)));
    connect(d->coder, SIGNAL(volumeMuted(bool)),SIGNAL(volumeMuted(bool)));

    connect(d->coder, SIGNAL(playerStateChanged(QtopiaMedia::State)), SLOT(stateChanged(QtopiaMedia::State)));
}

SimpleSession::~SimpleSession()
{
    if (d->connected) {
        d->coder->disconnectFromInput(d->source);
        d->sink->disconnectFromInput(d->coder);
    }

    if (d->opened) {
        d->source->close();
        d->sink->close();
    }

    delete d->coder;

    UriHandlers::destroyInputDevice(d->source);
    OutputDevices::destroyOutputDevice(d->sink);

    delete d;
}

void SimpleSession::start()
{
    qLog(Media) <<"SimpleSession::start() begin "<<this;

    if (d->state == QtopiaMedia::Playing || d->state == QtopiaMedia::Error)
        return;

    qLog(Media) <<"SimpleSession::start() kickoff "<<this;

    if (!d->opened) {
        if (!d->source->open(QIODevice::ReadOnly) ||
            !d->sink->open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
            d->errorString = "Cruxus; Unable to open devices for media session";
            qWarning() << d->errorString;

            emit playerStateChanged(d->state = QtopiaMedia::Error);
        }
        else
            d->opened = true;
    }

    if (d->opened && !d->connected) {
        d->coder->connectToInput(d->source);
        d->sink->connectToInput(d->coder);

        d->connected = true;
    }

    if (d->connected)
        d->coder->start();
}

void SimpleSession::pause()
{
    qLog(Media) <<"SimpleSession::pause() "<<this;

    if (d->connected)
        d->coder->pause();
}

void SimpleSession::stop()
{
    qLog(Media) <<"SimpleSession::stop() "<<this;

    if (d->connected)
        d->coder->stop();
}

void SimpleSession::suspend()
{
    qLog(Media) <<"SimpleSession::suspend() "<<this;

    pause();

    if (d->opened) {
        d->source->close();
        d->sink->close();
        d->opened = false;
    }
}

void SimpleSession::resume()
{
    qLog(Media) <<"SimpleSession::resume() "<<this;

    start();
}

void SimpleSession::seek(quint32 ms)
{
    d->coder->seek(ms);
}

quint32 SimpleSession::length()
{
    return d->coder->length();
}

void SimpleSession::setVolume(int volume)
{
    d->coder->setVolume(volume);
}

int SimpleSession::volume() const
{
    return d->coder->volume();
}

void SimpleSession::setMuted(bool mute)
{
    d->coder->setMuted(mute);
}

bool SimpleSession::isMuted() const
{
    return d->coder->isMuted();
}

QtopiaMedia::State SimpleSession::playerState() const
{
    return d->state;
}

QString SimpleSession::errorString()
{
    return QString();
}

void SimpleSession::setDomain(QString const& domain)
{
    d->domain = domain;
}

QString SimpleSession::domain() const
{
    return d->domain;
}

QStringList SimpleSession::interfaces()
{
    return QStringList("Basic");
}

QString SimpleSession::id() const
{
    return d->handle.id().toString();
}

QString SimpleSession::reportData() const
{
    return QString();
}

void SimpleSession::stateChanged(QtopiaMedia::State state)
{
    d->state = state;
}
// }}}

}   // ns cruxus
