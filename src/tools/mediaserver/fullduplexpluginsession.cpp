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

#include "plugindecodesession.h"
#include "pluginencodesession.h"
#include "fullduplexpluginsession.h"


namespace qtopia_helix
{

class FullDuplexPluginSessionPrivate
{
public:
    PluginDecodeSession*    decodeSession;
    PluginEncodeSession*    encodeSession;
};


FullDuplexPluginSession::FullDuplexPluginSession
(
 PluginDecodeSession* decodeSession,
 PluginEncodeSession* encodeSession
):
    d(new FullDuplexPluginSessionPrivate)
{
    d->decodeSession = decodeSession;
    d->encodeSession = encodeSession;
}

FullDuplexPluginSession::~FullDuplexPluginSession()
{
    delete d->decodeSession;
    delete d->encodeSession;
    delete d;
}

void FullDuplexPluginSession::start()
{
    d->decodeSession->start();
    d->encodeSession->start();
}

void FullDuplexPluginSession::pause()
{
    d->decodeSession->pause();
    d->encodeSession->pause();
}

void FullDuplexPluginSession::stop()
{
    d->decodeSession->stop();
    d->encodeSession->stop();
}

void FullDuplexPluginSession::suspend()
{
    d->decodeSession->suspend();
    d->encodeSession->suspend();
}

void FullDuplexPluginSession::resume()
{
    d->decodeSession->resume();
    d->encodeSession->resume();
}

void FullDuplexPluginSession::seek(quint32)
{
}

quint32 FullDuplexPluginSession::length()
{
    return 0;
}

void FullDuplexPluginSession::setVolume(int volume)
{
    d->decodeSession->setVolume(volume);
    d->encodeSession->setVolume(volume);
}

int FullDuplexPluginSession::volume() const
{
    return d->decodeSession->volume();
}

void FullDuplexPluginSession::setMuted(bool mute)
{
    d->decodeSession->setMuted(mute);
    d->encodeSession->setMuted(mute);
}

bool FullDuplexPluginSession::isMuted() const
{
    return d->decodeSession->isMuted();
}

QtopiaMedia::State FullDuplexPluginSession::playerState() const
{
    return d->decodeSession->playerState();
}

QString FullDuplexPluginSession::errorString()
{
    return QString();
}

void FullDuplexPluginSession::setDomain(QString const& domain)
{
    d->decodeSession->setDomain(domain);
}

QString FullDuplexPluginSession::domain() const
{
    return d->decodeSession->domain();
}


QStringList FullDuplexPluginSession::interfaces()
{
    QStringList capabilities;

    capabilities << "MediaControl";

    return capabilities;
}


}   // ns qtopia_helix

