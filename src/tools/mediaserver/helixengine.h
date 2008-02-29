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
** ** ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __QTOPIA_MEDIASERVER_HELIXENGINE_H
#define __QTOPIA_MEDIASERVER_HELIXENGINE_H

#include "mediaengine.h"

class QTimerEvent;
class IHXClientEngine;


namespace qtopia_helix
{

class HelixEngine : public mediaserver::MediaEngine
{
    Q_OBJECT

    struct HelixEnginePrivate;

public:
    HelixEngine();
    ~HelixEngine();

    void init(mediaserver::EngineInfo& engineInfo);
    void suspend();

    QMediaCodecInfoList supportedCodecs();

    mediaserver::MediaSession* createSession(QUuid const& id, QString const& url);

    mediaserver::MediaSession* createDecodeSession(QUuid const& id,
                                                    QString const& url,
                                                    QMediaCodecRep const& codec,
                                                    QMediaDeviceRep const& device);
    mediaserver::MediaSession* createEncodeSession(QUuid const& id,
                                                    QString const& url,
                                                    QMediaCodecRep const& codec,
                                                    QMediaDeviceRep const& device);
    mediaserver::MediaSession* createSession(QUuid const& id,
                                                QMediaCodecRep const& codec,
                                                QMediaDeviceRep const& inputSink,
                                                QMediaDeviceRep const& outputSink,
                                                QMediaDeviceRep const& inputSouce,
                                                QMediaDeviceRep const& outputSource);
    void destroySession(mediaserver::MediaSession* session);

    void setActiveDomain(QString const& domain);
    QString activeDomain();

private:
    bool startHelix();
    void stopHelix();
    void timerEvent(QTimerEvent* timerEvent);

    HelixEnginePrivate* d;
};

}   // ns qtopia_helix


#endif  //__QTOPIA_MEDIASERVER_HELIXENGINE_H

