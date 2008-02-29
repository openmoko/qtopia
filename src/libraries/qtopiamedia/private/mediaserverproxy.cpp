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

#include <sys/types.h>
#include <unistd.h>

#include <qhash.h>

#include <qtopiaipcadaptor.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>
#include <qvaluespace.h>

#include "media.h"
#include "mediaserverproxy.h"


namespace mlp
{

MediaServerProxy::~MediaServerProxy()
{
}

QMediaCodecInfoList MediaServerProxy::availableEncoders(QString const& domain) const
{
    Q_UNUSED(domain);

    return m_encoders;
}

QMediaCodecInfoList MediaServerProxy::availableDecoders(QString const& domain) const
{
    Q_UNUSED(domain);

    return m_decoders;
}

QMediaDeviceInfoList MediaServerProxy::availableMediaDevices(QString const& domain) const
{
    Q_UNUSED(domain);

    return m_devices;
}

// {{{ QMediaSession
int MediaServerProxy::createDecodeSession
(
 MediaServerCallback*   callback,
 QString const&         domain,
 QString const&         url,
 QMediaCodecRep const&     codec,
 QMediaDeviceRep const&    device
)
{
    QUuid               uuid(QUuid::createUuid());
    QtopiaIpcEnvelope   envelope(QTOPIA_MEDIASERVER_CHANNEL, "createSession()");

    int key = qHash(uuid.toString());

    m_idMap.insert(uuid, qMakePair(key, callback));

    envelope << uuid << domain << url << codec << device;

    return key;
}

int MediaServerProxy::createEncodeSession
(
 MediaServerCallback*   callback,
 QString const&         domain,
 QString const&         url,
 QMediaCodecRep const&     codec,
 QMediaDeviceRep const&    device
)
{
    QUuid               uuid(QUuid::createUuid());
    QtopiaIpcEnvelope   envelope(QTOPIA_MEDIASERVER_CHANNEL, "createSession()");

    int key = qHash(uuid.toString());

    m_idMap.insert(uuid, qMakePair(key, callback));

    envelope << uuid << domain << url << codec << device;

    return key;
}

int MediaServerProxy::createSession
(
 MediaServerCallback*   callback,
 QString const&         domain,
 QMediaCodecRep const&     codec,
 QMediaDeviceRep const&    inputSink,
 QMediaDeviceRep const&    outputSink,
 QMediaDeviceRep const&    inputSource,
 QMediaDeviceRep const&    outputSource
)
{
    QUuid               uuid(QUuid::createUuid());
    QtopiaIpcEnvelope   envelope(QTOPIA_MEDIASERVER_CHANNEL, "createSession()");

    int key = qHash(uuid.toString());

    m_idMap.insert(uuid, qMakePair(key, callback));

    envelope <<
        uuid <<
        domain <<
        codec <<
        inputSink <<
        outputSink <<
        inputSource <<
        outputSource;

    return key;
}
// }}}

// {{{ QContentPlayer
int MediaServerProxy::prepareContent
(
 MediaServerCallback*   callback,
 QString const&         domain,
 QString const&         url
)
{
    QUuid                   uuid(QUuid::createUuid());
    QtopiaServiceRequest    envelope(QTOPIA_MEDIASERVER_CHANNEL,
                                        "createContentSession(QString,QUuid,QString,QString,QString)");

    int key = qHash(uuid.toString());

    m_idMap.insert(uuid, qMakePair(key, callback));

    envelope << m_channel <<
                uuid <<
                domain <<
                url <<
                "none";

    envelope.send();

    return key;
}

int MediaServerProxy::playContent
(
 MediaServerCallback*   callback,
 QString const&         domain,
 QString const&         url
)
{
    QUuid                   uuid(QUuid::createUuid());
    QtopiaServiceRequest    envelope(QTOPIA_MEDIASERVER_CHANNEL,
                                        "createContentSession(QString,QUuid,QString,QString,QString)");

    int key = qHash(uuid.toString());

    m_idMap.insert(uuid, qMakePair(key, callback));

    envelope << m_channel <<
                uuid <<
                domain <<
                url <<
                "play";

    envelope.send();

    return key;
}
// }}}

void MediaServerProxy::destroySession(QUuid const& id)
{
    QtopiaServiceRequest    envelope(QTOPIA_MEDIASERVER_CHANNEL,
                                        "destroySession(QUuid)");

    envelope << id;
    envelope.send();
}


MediaServerProxy* MediaServerProxy::instance()
{
    static MediaServerProxy proxy;

    return &proxy;
}

// private slots:
void MediaServerProxy::sessionCreated(QUuid const& id)
{
    IdMap::iterator it = m_idMap.find(id);

    if (it != m_idMap.end())
    {
        (*it).second->mediaReady((*it).first, id);

        m_idMap.erase(it);
    }
}

void MediaServerProxy::sessionError(QUuid const& id, QString const& error)
{
    IdMap::iterator it = m_idMap.find(id);

    if (it != m_idMap.end())
    {
        (*it).second->mediaError((*it).first, error);

        m_idMap.erase(it);
    }
}

void MediaServerProxy::codecsChanged()
{
    buildCodecList();
}

void MediaServerProxy::devicesChanged()
{
    buildDeviceList();
}


MediaServerProxy::MediaServerProxy()
{
    // channel
    m_channel = QString(QTOPIA_MEDIALIBRARY_CHANNEL).arg(getpid());
    m_adaptor = new QtopiaIpcAdaptor(m_channel, this);

    QtopiaIpcAdaptor::connect(m_adaptor, SIGNAL(sessionCreated(QUuid)),
            this, SLOT(sessionCreated(QUuid)));

    QtopiaIpcAdaptor::connect(m_adaptor, SIGNAL(sessionError(QUuid, QString)),
            this, SLOT(sessionError(QUuid, QString)));

    // values
    m_mediaServerCodecs = new QValueSpaceItem("/MediaServer/Capabilities/Codecs", this);
    connect(m_mediaServerCodecs, SIGNAL(contentsChanged()),
            this, SLOT(codecsChanged()));

    m_mediaServerDevices = new QValueSpaceItem("/MediaServer/Capabilities/Devices", this);
    connect(m_mediaServerDevices, SIGNAL(contentsChanged()),
            this, SLOT(devicesChanged()));
}

void MediaServerProxy::buildCodecList()
{
    m_encoders.clear();
    m_decoders.clear();

    QVariantList    codecList = m_mediaServerCodecs->value(NULL).toList();

    if (!codecList.isEmpty())
    {
        foreach(QVariant const& var, codecList)
        {
            QMediaCodecInfo mediaCodec = var.value<QMediaCodecInfo>();

            if (mediaCodec.canEncode())
                m_encoders.push_back(mediaCodec);

            if (mediaCodec.canDecode())
                m_decoders.push_back(mediaCodec);
        }
    }
}

void MediaServerProxy::buildDeviceList()
{
    m_devices.clear();

    QVariantList    deviceList = m_mediaServerDevices->value(NULL).toList();

    if (!deviceList.isEmpty())
    {
        foreach(QVariant const& var, deviceList)
        {
            m_devices.push_back(var.value<QMediaDeviceInfo>());
        }
    }
}

}   // ns mlp




