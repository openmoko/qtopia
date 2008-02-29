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

#include <qtopiaipcenvelope.h>
#include <qtopialog.h>
#include <media.h>

#include "mediaserver.h"
#include "mediaengine.h"
#include "mediasession.h"
#include "mediacontrolserver.h"

#include "qtopiamediaprovider.h"


namespace mediaserver
{

QtopiaMediaProvider::QtopiaMediaProvider(MediaEngine* mediaEngine):
    QtopiaAbstractService(QTOPIA_MEDIASERVER_CHANNEL),
    m_mediaEngine(mediaEngine)
{
    qLog(Media) << "QtopiaMediaProvider starting";

    publishAll();
}

QtopiaMediaProvider::~QtopiaMediaProvider()
{
}

void QtopiaMediaProvider::createContentSession
(
 QString const& responseChannel,
 QUuid const&   id,
 QString const& domain,
 QString const& url,
 QString const& command
)
{
    qLog(Media) << "QtopiaMediaProvider::createContentSession()" <<
                responseChannel << id << domain << url << command;

    MediaSession* mediaSession = m_mediaEngine->createSession(id, url);

    if (mediaSession != 0)
    {
        mediaSession->setDomain(domain);

        // Wrap in control object
        MediaControlServer* mediaControlServer = new MediaControlServer(mediaSession,
                                                                        id.toString());
        m_sessionMap.insert(id, mediaControlServer);

        if (command == "play")
        {
            mediaControlServer->start();
        }

        QtopiaIpcEnvelope   e(responseChannel, "sessionCreated(QUuid)");
        e << id;
    }
    else
    {
        QtopiaIpcEnvelope   e(responseChannel, "sessionError(QUuid,QString)");
        e << id << "none";
    }
}


void QtopiaMediaProvider::createDecodeSession
(
 QString const&         responseChannel,
 QUuid const&           id,
 QString const&         domain,
 QString const&         url,
 QMediaCodecRep const&  codec,
 QMediaDeviceRep const& device
)
{
    qLog(Media) << "QtopiaMediaProvider::createDecodeSession(HalfDuplex)" <<
                responseChannel << id << domain << url;

    MediaSession* mediaSession = m_mediaEngine->createDecodeSession(id,
                                                                    url,
                                                                    codec,
                                                                    device);

    if (mediaSession != 0)
    {
        mediaSession->setDomain(domain);

        // Wrap in control object
        MediaControlServer* mediaControlServer = new MediaControlServer(mediaSession,
                                                                        id.toString());
        m_sessionMap.insert(id, mediaControlServer);

        QtopiaIpcEnvelope   e(responseChannel, "sessionCreated(QUuid)");
        e << id;
    }
    else
    {
        QtopiaIpcEnvelope   e(responseChannel, "sessionError(QUuid,QString)");
        e << id << "none";
    }
}

void QtopiaMediaProvider::createEncodeSession
(
 QString const&         responseChannel,
 QUuid const&           id,
 QString const&         domain,
 QString const&         url,
 QMediaCodecRep const&  codec,
 QMediaDeviceRep const& device
)
{
    Q_UNUSED(domain);

    qLog(Media) << "QtopiaMediaProvider::createSession(HalfDuplex)" <<
                responseChannel << id << domain << url;

    MediaSession* mediaSession = m_mediaEngine->createEncodeSession(id, url,
                                                                     codec,
                                                                     device);

    if (mediaSession != 0)
    {
        // Wrap in control object
        MediaControlServer* mediaControlServer = new MediaControlServer(mediaSession,
                                                                        id.toString());
        m_sessionMap.insert(id, mediaControlServer);

        QtopiaIpcEnvelope   e(responseChannel, "sessionCreated(QUuid)");
        e << id;
    }
    else
    {
        QtopiaIpcEnvelope   e(responseChannel, "sessionError(QUuid,QString)");
        e << id << "none";
    }
}

void QtopiaMediaProvider::createSession
(
 QString const& responseChannel,
 QUuid const& id,
 QString const& domain,
 QMediaCodecRep const& codec,
 QMediaDeviceRep const& inputSink,
 QMediaDeviceRep const& outputSink,
 QMediaDeviceRep const& inputSource,
 QMediaDeviceRep const& outputSource
)
{
    Q_UNUSED(domain);

    qLog(Media) << "QtopiaMediaProvider::createSession(FullDuplex)" <<
                responseChannel << id << domain;

    MediaSession* mediaSession = m_mediaEngine->createSession(id, codec,
                                                                inputSink,
                                                                outputSink,
                                                                inputSource,
                                                                outputSource);

    if (mediaSession != 0)
    {
        // Wrap in control object
        MediaControlServer* mediaControlServer = new MediaControlServer(mediaSession,
                                                                        id.toString());
        m_sessionMap.insert(id, mediaControlServer);

        QtopiaIpcEnvelope   e(responseChannel, "sessionCreated(QUuid)");
        e << id;
    }
    else
    {
        QtopiaIpcEnvelope   e(responseChannel, "sessionError(QUuid,QString)");
        e << id << "none";
    }
}

void QtopiaMediaProvider::destroySession(QUuid const& id)
{
    qLog(Media) << "QtopiaMediaProvider::destroySession()" << id;

    SessionMap::iterator it = m_sessionMap.find(id);

    if (it != m_sessionMap.end())
    {
        MediaControlServer* mediaControlServer = *it;

        m_mediaEngine->destroySession(mediaControlServer->mediaSession());
        delete mediaControlServer;

        m_sessionMap.erase(it);
    }
}

}   // ns mediaserver
