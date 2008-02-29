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
#include <qstring.h>
#include <qtopiaipcenvelope.h>

#include "sessionmanager.h"
#include "mediavolumecontrol.h"


namespace mediaserver
{

class MediaVolumeControlPrivate
{
public:
    SessionManager* sessionManager;
};


MediaVolumeControl::MediaVolumeControl(SessionManager* sessionManager):
    QtopiaIpcAdaptor("QPE/MediaServer/MediaVolumeControl"),
    d(new MediaVolumeControlPrivate)
{
    // init
    d->sessionManager = sessionManager;

    //
    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "registerHandler(QString,QString)");

    e << QString("Media") << QString("QPE/MediaServer/MediaVolumeControl");

    //
    publishAll(Slots);
}

MediaVolumeControl::~MediaVolumeControl()
{
    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "unregisterHandler(QString,QString)");

    e << QString("Media") << QString("QPE/MediaServer/MediaVolumeControl");
}


//public slots:
void MediaVolumeControl::setVolume(int volume)
{
    d->sessionManager->setVolume(volume);
}

void MediaVolumeControl::increaseVolume(int increment)
{
    d->sessionManager->increaseVolume(increment);
}

void MediaVolumeControl::decreaseVolume(int decrement)
{
    d->sessionManager->decreaseVolume(decrement);
}

void MediaVolumeControl::setMuted(bool mute)
{
    d->sessionManager->setMuted(mute);
}

}   // ns mediaserver
