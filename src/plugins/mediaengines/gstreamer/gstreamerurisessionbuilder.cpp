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

#include <QUrl>
#include <QSettings>
#include <QMediaSessionRequest>

#include "gstreamerplaybinsession.h"

#include "gstreamerurisessionbuilder.h"


namespace gstreamer
{

// {{{ UriSessionBuilderPrivate
class UriSessionBuilderPrivate
{
public:
    Engine*     engine;
    QMediaSessionBuilder::Attributes    attributes;
};
// }}}


// {{{ UriSessionBuilder

/*!
    \class gstreamer::UriSessionBuilder
    \internal
*/

UriSessionBuilder::UriSessionBuilder(Engine* engine):
    d(new UriSessionBuilderPrivate)
{
    d->engine = engine;

    // GStreamer does not support dynamically discovering the mime types and
    // uri schemes.  Grab them from QSettings
    QSettings   settings("Trolltech", "gstreamer");
    settings.beginGroup("Simple");

    // Supported URI Schemes
    d->attributes.insert("uriSchemes", settings.value("UriSchemes").toStringList());

    // Supported mime types
    d->attributes.insert("mimeTypes", settings.value("MimeTypes").toStringList());
}

UriSessionBuilder::~UriSessionBuilder()
{
    delete d;
}


QString UriSessionBuilder::type() const
{
    return "com.trolltech.qtopia.uri";
}

QMediaSessionBuilder::Attributes const& UriSessionBuilder::attributes() const
{
    return d->attributes;
}

QMediaServerSession* UriSessionBuilder::createSession(QMediaSessionRequest sessionRequest)
{
    QUrl            url;
    PlaybinSession* mediaSession = 0;

    sessionRequest >> url;

    if (url.isValid())
    {
        mediaSession = new PlaybinSession(d->engine,
                                          sessionRequest.id(),
                                          url);

        if (!mediaSession->isValid())
        {
            delete mediaSession;
            mediaSession = 0;
        }
    }

    return mediaSession;
}

void UriSessionBuilder::destroySession(QMediaServerSession* mediaSession)
{
    delete mediaSession;
}
// }}}

}   // ns gstreamer

