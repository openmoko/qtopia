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

#include <QMediaPipe>
#include <QMediaDevice>
#include <QMediaDecoder>

#include "cruxusoutputthread.h"

#include "cruxussimplesession.h"



namespace cruxus
{

// {{{ SimpleSessionPrivate
class SimpleSessionPrivate
{
public:
    bool                activated;
    QMediaDevice*       source;
    QMediaDevice*       sink;
    QMediaDecoder*      coder;
    OutputThread*       thread;

    QMediaHandle        handle;
    QString             domain;
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
    d->activated = false;
    d->handle = handle;
    d->source = source;
    d->sink = sink;
    d->coder = coder;

    connect(d->coder, SIGNAL(playerStateChanged(QtopiaMedia::State)),
            this, SIGNAL(playerStateChanged(QtopiaMedia::State)));

    connect(d->coder, SIGNAL(positionChanged(quint32)),
            this, SIGNAL(positionChanged(quint32)));

    connect(d->coder, SIGNAL(lengthChanged(quint32)),
            this, SIGNAL(lengthChanged(quint32)));

    connect(d->coder, SIGNAL(volumeChanged(int)),
            this, SIGNAL(volumeChanged(int)));

    connect(d->coder, SIGNAL(volumeMuted(bool)),
            this, SIGNAL(volumeMuted(bool)));
}

SimpleSession::~SimpleSession()
{
    delete d;
}

void SimpleSession::start()
{
    if (!d->activated)
    {
        QMediaPipe*     pipe;

        pipe = new QMediaPipe(d->source, d->coder, this);
        pipe = new QMediaPipe(d->coder, d->sink, this);

        d->activated = true;
    }

    // Prep
    d->source->open(QIODevice::ReadOnly);
    d->sink->open(QIODevice::WriteOnly | QIODevice::Unbuffered);
    d->coder->start();
}

void SimpleSession::pause()
{
    d->coder->pause();
}

void SimpleSession::stop()
{
    d->coder->stop();
}

void SimpleSession::suspend()
{
}

void SimpleSession::resume()
{
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
// }}}

}   // ns cruxus
