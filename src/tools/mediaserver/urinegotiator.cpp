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

#include <QMap>
#include <QUrl>
#include <QMimeType>
#include <QValueSpaceObject>

#include <QMediaSessionRequest>
#include <QMediaSessionBuilder>

#include <qtopialog.h>

#include "drmsession.h"

#include "urinegotiator.h"


namespace mediaserver
{

typedef QMap<QString, QStringList>              InfoMap;
typedef QMap<QString, QMediaSessionBuilderList> UriBuilderInfo;
typedef QMap<QMediaServerSession*, QMediaSessionBuilder*>  ActiveSessionMap;

// {{{ UriNegotiatorPrivate
class UriNegotiatorPrivate
{
public:
    QValueSpaceObject*          info;
    InfoMap                     mimeTypes;
    InfoMap                     uriSchemes;
    UriBuilderInfo              mimeTypeBuilders;
    UriBuilderInfo              uriSchemeBuilders;
    QMediaSessionBuilderList    builders;
    ActiveSessionMap            activeSessions;

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

void UriNegotiator::addBuilder(QString const& tag, QMediaSessionBuilder* sessionBuilder)
{
    QMediaSessionBuilder::Attributes const& attributes = sessionBuilder->attributes();

    // Mime types
    QStringList     mimeTypes = attributes["mimeTypes"].toStringList();
    foreach (QString const& mimeType, mimeTypes)
    {
        d->mimeTypeBuilders[mimeType].append(sessionBuilder);
    }
    d->mimeTypes[tag] += mimeTypes;

    // URI Schemes
    QStringList     uriSchemes = attributes["uriSchemes"].toStringList();
    foreach (QString const& uriScheme, uriSchemes)
    {
        d->uriSchemeBuilders[uriScheme].append(sessionBuilder);
    }
    d->uriSchemes[tag] += uriSchemes;

    d->builders.push_back(sessionBuilder);

    // Update valuespace
    d->info->setAttribute(tag + "/mimeTypes", d->mimeTypes[tag]);
    d->info->setAttribute(tag + "/uriSchemes", d->uriSchemes[tag]);
}

void UriNegotiator::removeBuilder(QString const& tag, QMediaSessionBuilder* sessionBuilder)
{
    Q_UNUSED(tag);
    // Remove from mimeTypes etc

    d->builders.removeAll(sessionBuilder);
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
        QMediaSessionBuilderList    ubs = d->uriSchemeBuilders[url.scheme()];

        if (ubs.size() > 0)
        {
            QMediaSessionBuilderList    candidates(ubs);
            QMimeType                   mimeType = QMimeType::fromFileName(url.path());

            // Prune out ones that can't handle the mime type
            if (!mimeType.isNull() &&
                mimeType.id() != "application/octet-stream" /* i.e. - don't know */)
            {
                QMediaSessionBuilderList    mbs = d->mimeTypeBuilders[mimeType.id()];

                foreach (QMediaSessionBuilder* builder, ubs)
                {
                    if (!mbs.contains(builder))
                        candidates.removeAll(builder);
                }
            }

            if (candidates.size() > 0)
            {
                QMediaSessionBuilder*   sessionBuilder = 0;

                if (candidates.size() == 1)
                {
                    sessionBuilder = candidates.front();
                }
                else
                {
                    // Grab MRU
                    foreach (QMediaSessionBuilder* builder, d->builders)
                    {
                        if (candidates.contains(builder))
                        {
                            sessionBuilder = builder;
                            break;
                        }
                    }
                }

                if (sessionBuilder != 0)
                {
                    // make MRU
                    d->builders.removeAll(sessionBuilder);
                    d->builders.prepend(sessionBuilder);

                    // Actually create the session :)
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
    }

    return mediaSession;
}

void UriNegotiator::destroySession(QMediaServerSession* mediaSession)
{
    ActiveSessionMap::iterator it = d->activeSessions.find(mediaSession);

    if (it != d->activeSessions.end())
    {
        it.value()->destroySession(mediaSession);

        d->activeSessions.remove(mediaSession);
    }
}
// }}}

}   // ns mediaserver
