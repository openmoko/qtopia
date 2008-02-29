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

#include <qmap.h>
#include <qstringlist.h>

#include <qpluginmanager.h>
#include <qmediacodecplugin.h>
#include <qmediaengineinformation.h>

#include "cruxusurisessionbuilder.h"

#include "cruxusengine.h"


namespace cruxus
{

// {{{ EngineInformation
class EngineInformation : public QMediaEngineInformation
{
public:
    EngineInformation(QMediaSessionBuilderList const& sessionBuilders):
        m_sessionBuilders(sessionBuilders)
    {
    }

    ~EngineInformation()
    {
    }

    QString name() const
    {
        return "Cruxus";
    }

    QString version() const
    {
        return "1.0";
    }

    int idleTime() const
    {
        return 0;
    }

    bool hasExclusiveDeviceAccess() const
    {
#ifdef HAVE_OSS
        return true;
#else
        return false;
#endif
    }

    QMediaSessionBuilderList sessionBuilders() const
    {
        return m_sessionBuilders;
    }

private:
    QMediaSessionBuilderList const& m_sessionBuilders;
};
// }}}

// {{{ CruxusEnginePrivate
class CruxusEnginePrivate
{
public:
    QPluginManager*             pluginManager;
    MediaCodecPluginMap         pluginCodecs;
    MediaCodecPluginMap         pluginExtensionMapping;
    EngineInformation*          engineInfo;
    QMediaSessionBuilderList    sessionBuilders;
};
// }}}

// {{{ Engine

/*!
    \class cruxus::Engine
    \internal
*/

Engine::Engine():
    d(new CruxusEnginePrivate)
{
}

Engine::~Engine()
{
    delete d->pluginManager;
    delete d->engineInfo;
    delete d;
}

void Engine::initialize()
{
    QStringList         mimeTypes;

    d->pluginManager = new QPluginManager("codecs", this);

    // Find plugins
    foreach (QString const& pluginName, d->pluginManager->list())
    {
        QMediaCodecPlugin*  plugin;
        QObject*            instance = d->pluginManager->instance(pluginName);

        if ((plugin = qobject_cast<QMediaCodecPlugin*>(instance)) != 0)
        {
            // Mime types
            mimeTypes += plugin->mimeTypes();

            // Extensions
            foreach (QString const& extension, plugin->fileExtensions())
            {
                d->pluginExtensionMapping.insert(extension, plugin);
            }
        }
        else
            delete instance;    // unload unwanted
    }

    // Register Builders
    d->sessionBuilders.push_back(new UriSessionBuilder(this,
                                                       mimeTypes,
                                                       d->pluginExtensionMapping));

    // Create our info object
    d->engineInfo = new EngineInformation(d->sessionBuilders);
}


void Engine::start()
{
}

void Engine::stop()
{
}

void Engine::suspend()
{
    // nothing
}

void Engine::resume()
{
    // nothing
}

QMediaEngineInformation const* Engine::engineInformation()
{
    return d->engineInfo;
}

void Engine::registerSession(QMediaServerSession* session)
{
}

void Engine::unregisterSession(QMediaServerSession* session)
{
}
// }}}

}   // ns cruxus
