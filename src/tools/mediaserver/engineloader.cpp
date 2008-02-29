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

#include <qstringlist.h>
#include <qpluginmanager.h>

#include <qmediaenginefactory.h>
#include <qmediaengine.h>

#include "engineloader.h"


namespace mediaserver
{

class EngineLoaderPrivate
{
public:
    QPluginManager*     pluginManager;
    QMediaEngineList    engines;
};


// {{{ EngineLoader
EngineLoader::EngineLoader():
    d(new EngineLoaderPrivate)
{
}

EngineLoader::~EngineLoader()
{
    delete d->pluginManager;
    delete d;
}

void EngineLoader::load()
{
    d->pluginManager = new QPluginManager("mediaengines");

    // Find
    foreach (QString const& pluginName, d->pluginManager->list())
    {
        QMediaEngineFactory*    factory;
        QObject*                instance = d->pluginManager->instance(pluginName);

        if ((factory = qobject_cast<QMediaEngineFactory*>(instance)) != 0)
        {
            QMediaEngine*   mediaEngine = factory->create();

            if (mediaEngine != 0)
            {
                d->engines.push_back(mediaEngine);
            }
        }
        else
            delete instance;
    }
}

void EngineLoader::unload()
{
    foreach (QMediaEngine* engine, d->engines)
    {
        delete engine;
    }

    d->engines.clear();
}


QMediaEngineList const& EngineLoader::engines()
{
    return d->engines;
}
// }}}

}   // ns mediaserver


