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

#include <QUrl>
#include <QMimeType>
#include <QMediaCodecPlugin>
#include <QMediaSessionRequest>
#include <QMediaDecoder>

#include "contentdevice.h"
#include "cruxusengine.h"
#include "cruxussimplesession.h"
#include "cruxusurihandlers.h"
#include "cruxusoutputdevices.h"

#include "cruxusurisessionbuilder.h"

namespace cruxus
{

// {{{ UriSessionBuilderPrivate
class UriSessionBuilderPrivate
{
public:
    Engine*                             engine;
    QMediaSessionBuilder::Attributes    attributes;
    MediaCodecPluginMap                 pluginExtensionMapping;
};
// }}}

// {{{ UriSessionBuilder

/*!
    \class cruxus::UriSessionBuilder
    \internal
*/

UriSessionBuilder::UriSessionBuilder
(
 Engine*            engine,
 QStringList const& mimeTypes,
 MediaCodecPluginMap const& pluginExtensionMapping
):
    QObject(engine),
    d(new UriSessionBuilderPrivate)
{
    d->engine = engine;
    d->pluginExtensionMapping = pluginExtensionMapping;

    // Supported URI Schems
    d->attributes.insert("uriSchemes", UriHandlers::supportedUris());

    // Supported mime types
    d->attributes.insert("mimeTypes", mimeTypes);
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
    QMediaServerSession*    session = 0;
    QUrl                    url;

    sessionRequest >> url;

    if (url.isValid())
    {
        QString     path = url.path();
        QString     extension = path.mid(path.lastIndexOf('.') + 1).toLower();

        if (!extension.isEmpty())
        {
            MediaCodecPluginMap::iterator it = d->pluginExtensionMapping.find(extension);

            // Check for a plugin
            if (it != d->pluginExtensionMapping.end())
            {
                QMediaDevice*   inputDevice  = UriHandlers::createInputDevice(url.scheme(), path);
                QMediaDevice*   outputDevice = OutputDevices::createOutputDevice();

                if (inputDevice != 0 && outputDevice != 0) {
                    QMimeType           mimeType(path);
                    QMediaCodecPlugin*  plugin = it.value();
                    QMediaDecoder*      coder = plugin->decoder(mimeType.id());

                    session = new SimpleSession(QMediaHandle(sessionRequest.id()),
                                                inputDevice,
                                                coder,
                                                outputDevice);

                    d->engine->registerSession(session);
                }
                else {
                    UriHandlers::destroyInputDevice(inputDevice);
                    OutputDevices::destroyOutputDevice(outputDevice);
                }
            }
        }
    }

    return session;
}

void UriSessionBuilder::destroySession(QMediaServerSession* session)
{
    d->engine->unregisterSession(session);
    session->stop();
    delete session;
}
// }}}

} // ns cruxus
