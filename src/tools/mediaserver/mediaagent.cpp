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

#include <QList>
#include <QMap>

#include <QValueSpaceObject>
#include <QMediaSessionRequest>
#include <QMediaSessionBuilder>
#include <QMediaServerSession>
#include <QMediaEngine>
#include <QMediaEngineInformation>

#include "engineloader.h"
#include "buildermanager.h"
#include "mediaagentsession.h"

#include "mediaagent.h"


namespace mediaserver
{

typedef QList<MediaAgentSession*>   ActiveSessions;

// {{{ MediaAgentPrivate
class MediaAgentPrivate
{
public:
    bool                        wrapSessions;
    QValueSpaceObject*          info;
    EngineLoader                engineLoader;
    QMediaEngineList            engines;
    BuilderManager*             foreman;
    ActiveSessions              activeSessions;
};
// }}}

// {{{ MediaAgent
MediaAgent::MediaAgent():
    d(new MediaAgentPrivate)
{
    d->wrapSessions = false;
    d->info = new QValueSpaceObject("/Media/Engines");
    d->foreman = new BuilderManager;
}

MediaAgent::~MediaAgent()
{
    d->engineLoader.unload();

    delete d->foreman;
    delete d->info;
    delete d;
}

QMediaServerSession* MediaAgent::createSession(QMediaSessionRequest const& sessionRequest)
{
    QMediaServerSession*    mediaSession;
    MediaAgentSession*      sessionWrapper = 0;

    mediaSession = d->foreman->createSession(sessionRequest);
    if (mediaSession != 0)
    {
        if (d->wrapSessions)
        {
            sessionWrapper = new MediaAgentSession(this, mediaSession);
            d->activeSessions.append(sessionWrapper);

            mediaSession = sessionWrapper;
        }
    }

    return mediaSession;
}

void MediaAgent::destroySession(QMediaServerSession* mediaSession)
{
    QMediaServerSession*    original = mediaSession;

    if (d->wrapSessions)
    {
        MediaAgentSession*  sessionWrapper = static_cast<MediaAgentSession*>(mediaSession);

        d->activeSessions.removeAll(sessionWrapper);

        original = sessionWrapper->wrappedSession();

        delete sessionWrapper;
    }

    d->foreman->destroySession(original);
}

MediaAgent* MediaAgent::instance()
{
    static bool         initialized = false;
    static MediaAgent   self;

    if (!initialized)
    {
        self.initialize();

        initialized = true;
    }

    return &self;
}

// private:
void MediaAgent::initialize()
{
    bool        anExclusiveEngine = false;

    // Load all plugins
    d->engineLoader.load();

    // Sort out capabilities
    d->engines = d->engineLoader.engines();

    d->info->setAttribute("Registered", d->engines.size());

    if (d->engines.size() > 0)
    {
        // Initialize engines
        foreach (QMediaEngine* mediaEngine, d->engines)
        {
            // init
            mediaEngine->initialize();

            // Store {
            QMediaEngineInformation const*  engineInfo = mediaEngine->engineInformation();
            QString     engineName = engineInfo->name();

            // Add builders
            d->foreman->addBuilders(engineName, engineInfo->sessionBuilders());

            // Info in valuespace
            d->info->setAttribute(engineName + "/Version", engineInfo->version());
            d->info->setAttribute(engineName + "/IdleTime", engineInfo->idleTime());
            // }

            // Exclusive
            anExclusiveEngine = anExclusiveEngine || engineInfo->hasExclusiveDeviceAccess();
        }
    }

    // Should we manage engines?
    d->wrapSessions = d->engines.size() > 1 && anExclusiveEngine;
}

void MediaAgent::sessionStarting(MediaAgentSession* session)
{
    Q_UNUSED(session);
    // stop others
}

void MediaAgent::sessionStopped(MediaAgentSession* session)
{
    Q_UNUSED(session);
    // restart others
}

// }}}

}   // ns mediaserver

