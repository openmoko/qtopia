/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef __QTOPIA_MEDIAENGINE_H
#define __QTOPIA_MEDIAENGINE_H

#include <qobject.h>
#include <quuid.h>


#include <qmediacodecinfo.h>
#include <qmediacodecrep.h>
#include <qmediadevicerep.h>

namespace mediaserver
{

class MediaSession;


struct EngineInfo
{
    int     idle_time;
};


class MediaEngine : public QObject
{
    Q_OBJECT

public:
    virtual void init(EngineInfo& engineInfo) = 0;
    virtual void suspend() = 0;

    virtual QMediaCodecInfoList supportedCodecs() = 0;

    virtual MediaSession* createSession(QUuid const& id, QString const& url) = 0;

    virtual MediaSession* createDecodeSession(QUuid const& id,
                                                QString const& url,
                                                QMediaCodecRep const& codecId,
                                                QMediaDeviceRep const& deviceId) = 0;
    virtual MediaSession* createEncodeSession(QUuid const& id,
                                                QString const& url,
                                                QMediaCodecRep const& codecId,
                                                QMediaDeviceRep const& deviceId) = 0;

    virtual MediaSession* createSession(QUuid const& id,
                                        QMediaCodecRep const& codec,
                                        QMediaDeviceRep const& inputSink,
                                        QMediaDeviceRep const& outputSink,
                                        QMediaDeviceRep const& inputSouce,
                                        QMediaDeviceRep const& outputSource) = 0;

    virtual void destroySession(MediaSession* session) = 0;

    virtual void setActiveDomain(QString const& domain) = 0;
    virtual QString activeDomain() = 0;

    static MediaEngine* createMediaEngine();

signals:
    void active();
    void inactive();
};

}   // ns mediaserver


#endif  //  __QTOPIA_MEDIAENGINE_H
