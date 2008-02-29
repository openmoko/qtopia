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

#include "private/mediaserverproxy.h"

#include "qmediasession.h"


using namespace mlp;


class QMediaSessionPrivate : public MediaServerCallback
{
public:
    QMediaSession*      mediaSession;
    QString             domain;
    MediaServerProxy*   serverProxy;

    void mediaReady(int key, QUuid const& id)
    {
        mediaSession->mediaReady(key, new QMediaControl(QMediaHandle(id), mediaSession));
    }

    void mediaError(int key, QString const& id)
    {
        mediaSession->mediaError(key, id);
    }
};



QMediaSession::QMediaSession(QString const& domain, QObject* parent):
    QObject(parent),
    d(new QMediaSessionPrivate)
{
    d->mediaSession = this;
    d->domain = domain;
    d->serverProxy = MediaServerProxy::instance();
}

QMediaSession::~QMediaSession()
{
    delete d;
}

QMediaCodecInfoList QMediaSession::availableEncoders() const
{
    return d->serverProxy->availableEncoders(d->domain);
}

QMediaCodecInfoList QMediaSession::availableDecoders() const
{
    return d->serverProxy->availableDecoders(d->domain);
}

QMediaDeviceInfoList QMediaSession::availableMediaDevices() const
{
    return d->serverProxy->availableMediaDevices(d->domain);
}

QMediaCodecInfo QMediaSession::defaultEncoder() const
{
    /*
    foreach (QMediaCodec codec, availableEncoders())
    {
        if (codec.isDomainDefault())
            return codec;
    }
    */

    return availableEncoders()[0];  // TODO:
}

QMediaDeviceInfo QMediaSession::defaultOuptutDevice() const
{
    /*
    foreach (QMediaDevice device, availableMediaDevices())
    {
        if (device.isOutputDefault())
            return device;
    }
    */

    return availableMediaDevices()[0];   // TODO:
}

QMediaDeviceInfo QMediaSession::defaultInputDevice() const
{
    /*
    foreach (QMediaDevice device, availableMediaDevices())
    {
        if (device.isInputDefault())
            return device;
    }
    */
    return availableMediaDevices()[0];
}

int QMediaSession::preparePlay
(
 QString const&         url,
 QMediaCodecRep const&     codec,
 QMediaDeviceRep const&    device
)
{
    return d->serverProxy->createDecodeSession(d, d->domain, url, codec, device);
}

int QMediaSession::prepareRecord
(
 QString const&         url,
 QMediaCodecRep const&     codec,
 QMediaDeviceRep const&    device
)
{
    return d->serverProxy->createEncodeSession(d, d->domain, url, codec, device);
}

int QMediaSession::preparePlayAndRecord
(
 QMediaCodecRep const& codec,
 QMediaDeviceRep const& inputSink,
 QMediaDeviceRep const& outputSink,
 QMediaDeviceRep const& inputSource,
 QMediaDeviceRep const& outputSource
)
{
    return d->serverProxy->createSession(d,
                                            d->domain,
                                            codec,
                                            inputSink,
                                            outputSink,
                                            inputSource,
                                            outputSource);
}



