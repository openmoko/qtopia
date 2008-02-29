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

#include <qcontent.h>
#include <qmediaencoder.h>
#include <qmediadevice.h>
#include <qmediapipe.h>

#include "pluginencodesession.h"


using namespace mediaserver;

namespace qtopia_helix
{

struct PluginEncodeSession::PluginEncodeSessionPrivate
{
    QContent            file;
    QMediaEncoder*      mediaEncoder;
    QMediaDevice*       inputDevice;
    QMediaDevice*       outputDevice;
    QtopiaMedia::State  state;
};


PluginEncodeSession::PluginEncodeSession
(
 QMediaDevice*  inputDevice,
 QMediaEncoder* mediaEncoder,
 QMediaDevice*  outputDevice
):
    d(new PluginEncodeSessionPrivate)
{
    d->mediaEncoder = mediaEncoder;
    d->inputDevice = inputDevice;
    d->outputDevice = outputDevice;
}

PluginEncodeSession::~PluginEncodeSession()
{
    delete d->mediaEncoder;
    delete d->inputDevice;
    delete d->outputDevice;
    delete d;
}

// XXX: refactor
static
void connect_pipe(QMediaDevice* input, QMediaEncoder* output, QObject* p)
{
    QMediaPipe* pipe = new QMediaPipe(input, output, p);

    input->setOutputPipe(pipe);
    output->setInputPipe(pipe);
}

static
void connect_pipe(QMediaEncoder* input, QMediaDevice* output, QObject* p)
{
    QMediaPipe* pipe = new QMediaPipe(input, output, p);

    input->setOutputPipe(pipe);
    output->setInputPipe(pipe);
}

void PluginEncodeSession::start()
{
    connect_pipe(d->inputDevice, d->mediaEncoder, this);
    connect_pipe(d->mediaEncoder, d->outputDevice, this);

    d->mediaEncoder->start();
}

void PluginEncodeSession::pause()
{
    d->mediaEncoder->pause();
}

void PluginEncodeSession::stop()
{
    d->mediaEncoder->stop();
}

void PluginEncodeSession::suspend()
{
}

void PluginEncodeSession::resume()
{
}

void PluginEncodeSession::seek(quint32 ms)
{
    d->mediaEncoder->seek(ms);
}

quint32 PluginEncodeSession::length()
{
    return d->mediaEncoder->length();
}

void PluginEncodeSession::setVolume(int)
{
}

int PluginEncodeSession::volume() const
{
    return 0;
}

void PluginEncodeSession::setMuted(bool)
{
}

bool PluginEncodeSession::isMuted() const
{
    return false;
}

QtopiaMedia::State PluginEncodeSession::playerState() const
{
    return d->state;
}

QString PluginEncodeSession::errorString()
{
    return QString();
}

void PluginEncodeSession::setDomain(QString const& domain)
{
    Q_UNUSED(domain);
}

QString PluginEncodeSession::domain() const
{
    return QString();
}

QStringList PluginEncodeSession::interfaces()
{
    QStringList capabilities;

    capabilities << "MediaControl"; // TODO: general (QVariant) settings interface

    return capabilities;
}

}   // ns qtopiahelix






