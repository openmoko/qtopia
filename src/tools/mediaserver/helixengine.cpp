/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include <qdir.h>
#include <qstringlist.h>
#include <qlibrary.h>
#include <qlibraryinfo.h>
#include <qcoreevent.h>

#include <qtopialog.h>
#include <qpluginmanager.h>
#include <qmediacodecplugin.h>
#include <qmediaencoder.h>
#include <qmediadecoder.h>
#include <qmediadevice.h>
#include <config.h>

#include <stdlib.h>

#include <hxcom.h>
#include <hxcore.h>

#include "helixutil.h"
#include "helixengine.h"
#include "helixsession.h"
#include "pluginsessionthread.h"
#include "plugindecodesession.h"
#include "pluginencodesession.h"
#include "fullduplexpluginsession.h"
#include "drmsession.h"
#include "sessionmanager.h"
#include "devicemanager.h"
#include "contentdevice.h"
#include "qmediahelixsettingsserver.h"
#include "reporterror.h"

using namespace mediaserver;


namespace qtopia_helix
{

typedef QMap<QString, QMediaCodecPlugin*>  MediaCodecPluginMap;

typedef HX_RESULT (HXEXPORT_PTR FPRMSETDLLACCESSPATH) (const char*);

struct HelixLibrary
{
    FPRMCREATEENGINE CreateEngine;
    FPRMCLOSEENGINE CloseEngine;
    FPRMSETDLLACCESSPATH SetDLLAccessPath;
};

struct HelixEngine::HelixEnginePrivate
{
    int                     sessionCount;
    int                     timerId;
    QMap<QString,QMediaCodecInfo>     helixCodecs;
    QStringList             helixExtensions;
    MediaCodecPluginMap     pluginCodecs;
    MediaCodecPluginMap     pluginExtensionMapping;
    HelixLibrary            symbols;
    IHXClientEngine*        engine;
    PluginSessionThread*    pluginSessionThread;
    QList<MediaSession*>    sessions;
    SessionManager*         sessionManager;
    QString                 activeDomain;
};



// HelixEngine
HelixEngine::HelixEngine():
    d(new HelixEnginePrivate)
{
    // init
    d->sessionCount = 0;
    d->timerId = -1;
    d->engine = 0;
    d->pluginSessionThread = 0;
    d->sessionManager = new SessionManager(this);

    // Get helix lib
    QLibrary library(helix_library_path() + QLatin1String("/clntcore.so")) ;

    d->symbols.CreateEngine = (FPRMCREATEENGINE)library.resolve( "CreateEngine" );
    d->symbols.CloseEngine = (FPRMCLOSEENGINE)library.resolve( "CloseEngine" );
    d->symbols.SetDLLAccessPath = (FPRMSETDLLACCESSPATH)library.resolve( "SetDLLAccessPath" );
}

HelixEngine::~HelixEngine()
{
    delete d;
}

void HelixEngine::init(EngineInfo& engineInfo)
{
    engineInfo.idle_time = -1;   // Never idle, keep engine loaded

    if (d->symbols.CreateEngine != 0 &&
        d->symbols.CloseEngine != 0 &&
        d->symbols.SetDLLAccessPath != 0) {

        // Set library path
        QByteArray env = getenv( "HELIX_PLUGIN_PATH" );
        QByteArray path = env.isNull() ? helix_library_path().toLatin1() : env;

        QByteArray dllAccessPath = QByteArray() + "DT_Common=" + path + '\0' +
                                    "DT_Plugins=" + path + '\0' +
                                    "DT_Codecs=" + path + "\0\0";

        d->symbols.SetDLLAccessPath(dllAccessPath);

        if (d->symbols.CreateEngine(&d->engine) == HXR_OK) {
            // Initialize engine and preload plugins
            IHXClientEngineSetup *enginesetup = 0;
            if (d->engine->QueryInterface( IID_IHXClientEngineSetup, (void**)&enginesetup) == HXR_OK) {
                GenericContext context;
                enginesetup->Setup(&context);
            }

            // Create global settings control
            new QMediaHelixSettingsServer(d->engine, "HelixGlobalSettings");

            emit active();
        }
    }
    else
    {
        REPORT_ERROR( ERR_HELIX );
    }
}

void HelixEngine::suspend()
{
    d->symbols.CloseEngine(d->engine);

    d->engine = 0;
}

static
void dataStreamFromPlugin(QByteArray& rawData, QMediaCodecPlugin* plugin)
{
    QDataStream data(&rawData, QIODevice::WriteOnly);

    data << plugin->name();
    data << plugin->comment();
    data << plugin->mimeTypes();
    data << plugin->fileExtensions();
    data << plugin->version();
    data << plugin->canEncode();
    data << plugin->canDecode();
    data << plugin->supportsVideo();
    data << plugin->supportsAudio();
}

QMediaCodecInfoList HelixEngine::supportedCodecs()
{
    QPluginManager*     pluginManager = new QPluginManager("codecs", this);
    QMediaCodecInfoList codecInfoList = d->helixCodecs.values();

    // Check those supported by plugin manager
    foreach (QString pluginName, pluginManager->list())
    {
        QMediaCodecPlugin*  plugin;
        QObject*            instance = pluginManager->instance(pluginName);

        if ((plugin = qobject_cast<QMediaCodecPlugin*>(instance)) != 0)
        {
            QMediaCodecInfo codec;
            { // {{{ Create QMediaCodecInfo
                QByteArray      rawData;

                dataStreamFromPlugin(rawData, plugin);

                QDataStream data(&rawData, QIODevice::ReadOnly);
                data >> codec;    // XXX: Do I like this?
            } // }}}

            // For Clients
            codecInfoList.push_back(codec);

            // Internal
            d->pluginCodecs.insert(plugin->name(), plugin);

            foreach (QString const& extension, plugin->fileExtensions())
            {
                qLog(Media) << "Adding" << extension << pluginName;
                d->pluginExtensionMapping.insert(extension, plugin);
            }
        }
        else
            delete instance;    // unload
    }

    return codecInfoList;
}

MediaSession* HelixEngine::createSession(QUuid const& id, QString const& url)
{
    MediaSession*   rc = 0;
    QString         extension = url.mid(url.lastIndexOf('.') + 1).toLower();

    MediaCodecPluginMap::iterator it = d->pluginExtensionMapping.find(extension);

    // Check for a plugin
    if (!extension.isEmpty() && it != d->pluginExtensionMapping.end())
    {
        qLog(Media) << "Found plugin";

        QMediaCodecPlugin*  plugin = it.value();

        // grab output device
        // {{{ XXX: dont look
        QMediaDeviceInfo    mediaDeviceInfo;
        QByteArray          rawData;
        {
            QDataStream         ds(&rawData, QIODevice::ReadWrite);

            ds << QString(QLatin1String("Builtin Audio Output Device")) <<
                  QString(QLatin1String("Builtin Audio Output Device")) <<
                  false <<
                  true;
        }
        {
            QDataStream ds(&rawData, QIODevice::ReadOnly);
            ds >> mediaDeviceInfo;
        }
        // }}}
        QMediaDeviceRep device(mediaDeviceInfo);

        qLog(Media) << "Constructed media device";

        rc = new PluginDecodeSession(new ContentDevice(url),
                                     plugin->decoder(QString()),
                                     DeviceManager::instance()->createMediaDevice(device));

        qLog(Media) << "Constructed decodesession";
    }
    else
    {   // hand off to helix
        if (startHelix())
        {
            rc = new HelixSession(d->engine, id, url);
        }
    }

    // when DRM'd wrap with DrmSession to handle
    if (rc != 0 && url.startsWith("qtopia://"))
    {
        rc = new DrmSession(url, rc);
    }

    return d->sessionManager->manageSession(rc);
}

MediaSession* HelixEngine::createDecodeSession
(
 QUuid const&           id,
 QString const&         url,
 QMediaCodecRep const&  codec,
 QMediaDeviceRep const& device
)
{
    MediaSession*       rc = 0;

    if (d->helixCodecs.find(codec.id()) != d->helixCodecs.end())
    {   // try helix
        if (startHelix())
            rc = new HelixSession(d->engine, id, url);
    }
    else
    {   // try plugins
        MediaCodecPluginMap::iterator it = d->pluginCodecs.find(codec.id());

        if (it != d->pluginCodecs.end())
        {
            QMediaCodecPlugin*  plugin = *it;

            if (plugin->canDecode())
            {
                rc = new PluginDecodeSession(new ContentDevice(url),
                                             plugin->decoder(QString()),
                                             DeviceManager::instance()->createMediaDevice(device));
            }
        }
    }

    return d->sessionManager->manageSession(rc);
}

MediaSession* HelixEngine::createEncodeSession
(
 QUuid const&           id,
 QString const&         url,
 QMediaCodecRep const&  codec,
 QMediaDeviceRep const& device
)
{
    Q_UNUSED(id);

    MediaSession*           rc = 0;
    MediaCodecPluginMap::iterator it = d->pluginCodecs.find(codec.id());

    // plugins only
    if (it != d->pluginCodecs.end())
    {
        QMediaCodecPlugin*  plugin = *it;

        if (plugin->canEncode())
        {
            rc = new PluginEncodeSession(DeviceManager::instance()->createMediaDevice(device),
                                        plugin->encoder(QString()),
                                        new ContentDevice(url));
        }
    }

    return rc;
}

MediaSession* HelixEngine::createSession
(
 QUuid const&       id,
 QMediaCodecRep const& codec,
 QMediaDeviceRep const& inputSink,
 QMediaDeviceRep const& outputSink,
 QMediaDeviceRep const& inputSource,
 QMediaDeviceRep const& outputSource
)
{
    Q_UNUSED(id);

    MediaSession*           rc = 0;
    MediaCodecPluginMap::iterator it = d->pluginCodecs.find(codec.id());

    if (it != d->pluginCodecs.end())
    {
        QMediaCodecPlugin*  plugin = *it;

        if (plugin->canDecode() && plugin->canEncode())
        {
            DeviceManager*  deviceManager = DeviceManager::instance();

            PluginDecodeSession*    decode =
                    new PluginDecodeSession(deviceManager->createMediaDevice(inputSource),
                                            plugin->decoder(QString()),
                                            deviceManager->createMediaDevice(inputSink));

            PluginEncodeSession*    encode =
                    new PluginEncodeSession(deviceManager->createMediaDevice(outputSource),
                                            plugin->encoder(QString()),
                                            deviceManager->createMediaDevice(outputSink));

            rc = new FullDuplexPluginSession(decode, encode);
        }
    }

    return rc;
}

void HelixEngine::destroySession(MediaSession* session)
{
    d->sessionManager->releaseSession(session);

    delete session;

    stopHelix();
}

void HelixEngine::setActiveDomain(QString const& domain)
{
    d->activeDomain = domain;

    d->sessionManager->activeDomainChanged();
}

QString HelixEngine::activeDomain()
{
    return d->activeDomain;
}


bool HelixEngine::startHelix()
{
    if (d->engine)
    {
        if (d->timerId == -1)
            d->timerId = startTimer(300); // XXX: voodoo - Start helix event timer

        d->sessionCount++;

        return true;
    }

    return false;
}

void HelixEngine::stopHelix()
{
    if (--d->sessionCount == 0)
    {
        killTimer(d->timerId);  // Shut down timer straight away
        d->timerId = -1;

        emit inactive();
    }
}

void HelixEngine::timerEvent(QTimerEvent* timerEvent)
{
    if (timerEvent->timerId() == d->timerId )
    {
        // Pump helix event loop
        d->engine->EventOccurred((_HXxEvent*)0);
    }
}


}   // ns qtopia_helix





