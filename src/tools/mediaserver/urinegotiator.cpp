/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include <QMap>
#include <QMultiMap>
#include <QUrl>
#include <QMimeType>
#include <QValueSpaceObject>
#include <QDebug>

#include <QMediaSessionRequest>
#include <QMediaSessionBuilder>

#include <qtopialog.h>

#include "drmsession.h"

#include "urinegotiator.h"


namespace mediaserver
{

typedef QMap<QString, QStringList>              InfoMap;
typedef QMap<QMediaServerSession*, QMediaSessionBuilder*>  ActiveSessionMap;
typedef QMultiMap<int, QMediaSessionBuilder*>   OrderedBuilders;

// {{{ UriNegotiatorPrivate
class UriNegotiatorPrivate
{
public:
    QValueSpaceObject*          info;
    InfoMap                     mimeTypes;
    InfoMap                     uriSchemes;
    ActiveSessionMap            activeSessions;
    OrderedBuilders             builders;
    QMediaSessionBuilder::Attributes    attributes;
};
// }}}

// {{{ UriNegotiator
UriNegotiator::UriNegotiator():
    d(new UriNegotiatorPrivate)
{
    d->info = new QValueSpaceObject("/Media/Library/Info/Simple");
}

UriNegotiator::~UriNegotiator()
{
    delete d->info;
    delete d;
}

QString UriNegotiator::type() const
{
    return "com.trolltech.qtopia.uri";
}

QMediaSessionBuilder::Attributes const& UriNegotiator::attributes() const
{
    return d->attributes;
}

void UriNegotiator::addBuilder
(
 QString const& tag,
 int priority,
 QMediaSessionBuilder* sessionBuilder
)
{
    QMediaSessionBuilder::Attributes const& attributes = sessionBuilder->attributes();

    // Add Builder
    d->builders.insert(priority, sessionBuilder);

    // Update valuespace
    d->mimeTypes[tag] += attributes["mimeTypes"].toStringList();
    d->uriSchemes[tag] += attributes["uriSchemes"].toStringList();
    d->info->setAttribute(tag + "/mimeTypes", d->mimeTypes[tag]);
    d->info->setAttribute(tag + "/uriSchemes", d->uriSchemes[tag]);
}

void UriNegotiator::removeBuilder(QString const& tag, QMediaSessionBuilder* sessionBuilder)
{
    Q_UNUSED(tag);
    Q_UNUSED(sessionBuilder);
}

QMediaServerSession* UriNegotiator::createSession(QMediaSessionRequest sessionRequest)
{
    QMediaSessionRequest        request(sessionRequest);
    QUrl                        url;
    QMediaServerSession*        mediaSession = 0;

    request >> url;

    qLog(Media) << "UriNegotiator::createSession" << sessionRequest.id() << sessionRequest.domain() << url;

    if (url.isValid())
    {
        QMediaSessionBuilderList    candidates;

        for (OrderedBuilders::iterator it = d->builders.begin(); it != d->builders.end(); ++it)
        {
            if ((*it)->attributes()["uriSchemes"].toStringList().contains(url.scheme()))
                candidates.append(*it);
        }

        if (candidates.size() > 0)
        {
            QMimeType           mimeType = QMimeType::fromFileName(url.path());

            // If we can id the mimetype, prune those that aren't registered
            if (!mimeType.isNull())
            {
                QString     id = mimeType.id();

                if (id != "application/octet-stream" /* i.e. - don't know */)
                {
                    foreach (QMediaSessionBuilder* builder, candidates)
                    {
                        if (!builder->attributes()["mimeTypes"].toStringList().contains(id))
                            candidates.removeAll(builder);
                    }
                }
            }

            if (candidates.size() > 0)
            {
                QMediaSessionBuilder*   sessionBuilder = candidates.front();

                mediaSession = sessionBuilder->createSession(sessionRequest);

                if (mediaSession != 0)
                {
                    // NOTE: DRM is indicated by qtopia:// uri scheme so partly dealt with
                    // here. Engines have their part in DRM, they must understand the uri
                    // scheme if they wish to play DRM media. Here we just wrap the session
                    // to provide a means to inform the DRM framework of different requests
                    // - saves the engine doing it.
                    if (url.scheme() == "qtopia")
                        mediaSession = new DrmSession(url, mediaSession);

                    d->activeSessions.insert(mediaSession, sessionBuilder);
                }
            }
        }
    }

    return mediaSession;
}

void UriNegotiator::destroySession(QMediaServerSession* mediaSession)
{
    qLog(Media) << "UriNegotiator::destroySession" << mediaSession->id();

    ActiveSessionMap::iterator it = d->activeSessions.find(mediaSession);

    if (it != d->activeSessions.end())
    {
        it.value()->destroySession(mediaSession);

        d->activeSessions.remove(mediaSession);
    }
}
// }}}

}   // ns mediaserver
