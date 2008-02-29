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

#include <qstring.h>
#include <qiodevice.h>

#include <qcontent.h>

#include <qmediadecoder.h>
#include <qmediadevice.h>
#include <qmediapipe.h>

#include "plugindecodesession.h"


using namespace mediaserver;


namespace qtopia_helix
{

struct PluginDecodeSession::PluginDecodeSessionPrivate
{
    QMediaDevice*       inputDevice;
    QMediaDecoder*      mediaDecoder;
    QMediaDevice*       outputDevice;
    QString             domain;
    quint32             position;
    QtopiaMedia::State  state;
};

PluginDecodeSession::PluginDecodeSession
(
 QMediaDevice*  inputDevice,
 QMediaDecoder* mediaDecoder,
 QMediaDevice*  outputDevice
):
    d(new PluginDecodeSessionPrivate)
{
    d->inputDevice = inputDevice;
    d->mediaDecoder = mediaDecoder;
    d->outputDevice = outputDevice;

    connect(d->mediaDecoder, SIGNAL(playerStateChanged(QtopiaMedia::State)),
            this, SLOT(stateChanged(QtopiaMedia::State)));

    connect(d->mediaDecoder, SIGNAL(positionChanged(quint32)),
            this, SIGNAL(positionChanged(quint32)));

    connect(d->mediaDecoder, SIGNAL(lengthChanged(quint32)),
            this, SIGNAL(lengthChanged(quint32)));

    connect(d->mediaDecoder, SIGNAL(volumeChanged(int)),
            this, SIGNAL(volumeChanged(int)));

    connect(d->mediaDecoder, SIGNAL(volumeMuted(bool)),
            this, SIGNAL(volumeMuted(bool)));
}

PluginDecodeSession::~PluginDecodeSession()
{
    delete d->inputDevice;
    delete d->mediaDecoder;
    delete d->outputDevice;
    delete d;
}

// XXX: refactor
static
void connect_pipe(QMediaDevice* input, QMediaDecoder* output, QObject* p)
{
    QMediaPipe* pipe = new QMediaPipe(input, output, p);

    input->setOutputPipe(pipe);
    output->setInputPipe(pipe);
}

static
void connect_pipe(QMediaDecoder* input, QMediaDevice* output, QObject* p)
{
    QMediaPipe* pipe = new QMediaPipe(input, output, p);

    input->setOutputPipe(pipe);
    output->setInputPipe(pipe);
}

void PluginDecodeSession::start()
{
    connect_pipe(d->inputDevice, d->mediaDecoder, this);
    connect_pipe(d->mediaDecoder, d->outputDevice, this);

    d->inputDevice->open(QIODevice::ReadOnly);
    d->outputDevice->open(QIODevice::WriteOnly);

    d->mediaDecoder->start();
}

void PluginDecodeSession::pause()
{
    d->mediaDecoder->pause();
}

void PluginDecodeSession::stop()
{
    d->mediaDecoder->stop();
}

void PluginDecodeSession::suspend()
{
    d->mediaDecoder->stop();
}

void PluginDecodeSession::resume()
{
    d->mediaDecoder->start();
}

void PluginDecodeSession::seek(quint32 ms)
{
    d->mediaDecoder->seek(ms);
}

quint32 PluginDecodeSession::length()
{
    return d->mediaDecoder->length();
}

void PluginDecodeSession::setVolume(int volume)
{
    d->mediaDecoder->setVolume(volume);
}

int PluginDecodeSession::volume() const
{
    return d->mediaDecoder->volume();
}

void PluginDecodeSession::setMuted(bool mute)
{
    d->mediaDecoder->setMuted(mute);
}

bool PluginDecodeSession::isMuted() const
{
    return d->mediaDecoder->isMuted();
}

QtopiaMedia::State PluginDecodeSession::playerState() const
{
    return d->state;
}

QString PluginDecodeSession::errorString()
{
    return QString();
}

void PluginDecodeSession::setDomain(QString const& domain)
{
    d->domain = domain;
}

QString PluginDecodeSession::domain() const
{
    return d->domain;
}

QStringList PluginDecodeSession::interfaces()
{
    QStringList capabilities;

    capabilities << "MediaControl"; // TODO: general (QVariant) settings interface

    return capabilities;
}

void PluginDecodeSession::stateChanged(QtopiaMedia::State state)
{
    d->state = state;

    if (state == QtopiaMedia::Stopped ||
        state == QtopiaMedia::Error)
    {
        // close both devices
        d->inputDevice->close();
        d->outputDevice->close();
    }

    emit playerStateChanged(state);
}


}   // ns qtopiahelix





