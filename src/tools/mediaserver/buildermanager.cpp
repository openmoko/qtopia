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

#include <QValueSpaceObject>
#include <QMediaSessionRequest>

#include "urinegotiator.h"

#include "buildermanager.h"


namespace mediaserver
{

typedef QMap<QString, BuilderNegotiator*>       NegotiatorMap;
typedef QMap<QString, QMediaSessionBuilderList> BuilderMap;
typedef QMap<QMediaServerSession*, QMediaSessionBuilder*>  ActiveSessionMap;


class BuilderManagerPrivate
{
public:
    QValueSpaceObject*  info;
    NegotiatorMap       negotiators;
    BuilderMap          builders;
    ActiveSessionMap    activeSessionMap;
};


BuilderManager::BuilderManager():
    d(new BuilderManagerPrivate)
{
    d->info = new QValueSpaceObject("/Media/Engines");

    // Hard coded
    d->negotiators.insert("com.trolltech.qtopia.uri", new UriNegotiator);
}

BuilderManager::~BuilderManager()
{
    delete d->info;
    delete d;
}

void BuilderManager::addBuilders
(
 QString const& engineName,
 QMediaSessionBuilderList const& builderList
)
{
    foreach (QMediaSessionBuilder* sessionBuilder, builderList)
    {
        QString type = sessionBuilder->type();
        NegotiatorMap::iterator it = d->negotiators.find(type);

        if (it != d->negotiators.end())
        {
            // Known builder type, let negotiator deal with it
            (*it)->addBuilder(engineName, sessionBuilder);
        }
        else
        {
            // No negotiator for builder - just install it
            d->builders[type].append(sessionBuilder);
        }

        // Add builder info
        QString buildDir = engineName + "/Builders/" + sessionBuilder->type() + "/";
        QMediaSessionBuilder::Attributes const& attributes = sessionBuilder->attributes();

        for (QMediaSessionBuilder::Attributes::const_iterator it = attributes.begin();
             it != attributes.end();
             ++it)
        {
            d->info->setAttribute(buildDir + it.key(), it.value());
        }
    }
}


QMediaServerSession* BuilderManager::createSession(QMediaSessionRequest const& sessionRequest)
{
    QMediaServerSession*        mediaSession = 0;
    QMediaSessionBuilder*       sessionBuilder;

    NegotiatorMap::iterator it = d->negotiators.find(sessionRequest.type());

    if (it != d->negotiators.end())
    {
        mediaSession = (*it)->createSession(sessionRequest);
        sessionBuilder = *it;
    }
    else
    {
        QMediaSessionBuilderList&   sessionBuilders = d->builders[sessionRequest.type()];
        int                         builderCount = sessionBuilders.size();

        for (int i = 0; i < builderCount; ++i)
        {
            sessionBuilder = sessionBuilders.takeFirst();

            mediaSession = sessionBuilder->createSession(sessionRequest);
            if (mediaSession != 0)
            {
                sessionBuilders.prepend(sessionBuilder);
                break;
            }

            sessionBuilders.append(sessionBuilder);
        }
    }

    if (mediaSession != 0)
    {
        d->activeSessionMap.insert(mediaSession, sessionBuilder);
    }

    return mediaSession;
}

void BuilderManager::destroySession(QMediaServerSession* mediaSession)
{
    ActiveSessionMap::iterator it = d->activeSessionMap.find(mediaSession);

    if (it != d->activeSessionMap.end())
    {
        // Remove status info
        it.value()->destroySession(mediaSession);

        d->activeSessionMap.remove(mediaSession);
    }
}

}   // ns mediaserver
