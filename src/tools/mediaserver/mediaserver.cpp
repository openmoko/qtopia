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

#include <qcoreevent.h>

#include <qtopialog.h>
#include <qvaluespace.h>

#include "devicemanager.h"
#include "mediaserver.h"


namespace mediaserver
{

MediaServer::MediaServer(int argc, char** argv):
    m_engineIdleTimer(-1)
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    m_mediaEngine = MediaEngine::createMediaEngine();

    connect(m_mediaEngine, SIGNAL(active()),
            this, SLOT(engineActive()));

    connect(m_mediaEngine, SIGNAL(inactive()),
            this, SLOT(engineInactive()));

    // init
    m_mediaEngine->init(m_engineInfo);

    // put codecs into valuespace where clients can read them
    QList<QVariant>     variantList;

    // Get codecs
    foreach (QMediaCodecInfo codec, m_mediaEngine->supportedCodecs())
    {
        variantList.push_back(qVariantFromValue(codec));
    }

    QValueSpaceItem("/MediaServer/Capabilities").setValue("Codecs", variantList);

    // Init devices
    DeviceManager::instance()->init();
}

MediaServer::~MediaServer()
{
    delete m_mediaEngine;
}


MediaEngine* MediaServer::mediaEngine() const
{
    return m_mediaEngine;
}


//private slots:

void MediaServer::engineActive()
{
    if (m_engineIdleTimer != -1)
    {
        killTimer(m_engineIdleTimer);

        m_engineIdleTimer = -1;
    }
}

void MediaServer::engineInactive()
{
    if (m_engineInfo.idle_time != -1)
        m_engineIdleTimer = startTimer(m_engineInfo.idle_time);
}

void MediaServer::timerEvent(QTimerEvent* timerEvent)
{
    if (timerEvent->timerId() == m_engineIdleTimer)
    {
        killTimer(m_engineIdleTimer);

        m_mediaEngine->suspend();
    }
}

}

