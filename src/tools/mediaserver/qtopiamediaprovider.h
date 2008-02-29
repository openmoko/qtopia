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

#ifndef __QTOPIA_MEDIASERVER_QTOPIAMEDIAPROVIDER_H
#define __QTOPIA_MEDIASERVER_QTOPIAMEDIAPROVIDER_H

#include <quuid.h>
#include <qmap.h>

#include <qtopiaabstractservice.h>
#include <qmediacodecrep.h>
#include <qmediadevicerep.h>


namespace mediaserver
{

class MediaEngine;
class MediaSession;
class MediaControlServer;

class QtopiaMediaProvider : public QtopiaAbstractService
{
    Q_OBJECT

    typedef QMap<QUuid, MediaControlServer*>   SessionMap;

public:
    QtopiaMediaProvider(MediaEngine* mediaEngine);
    ~QtopiaMediaProvider();

public slots:
    void createContentSession(QString const& responseChannel,
                                QUuid const& id,
                                QString const& domain,
                                QString const& url,
                                QString const& command);

    void createDecodeSession(QString const& responseChannel,
                                QUuid const& id,
                                QString const& domain,
                                QString const& url,
                                QMediaCodecRep const& codec,
                                QMediaDeviceRep const& device);

    void createEncodeSession(QString const& responseChannel,
                                QUuid const& id,
                                QString const& domain,
                                QString const& url,
                                QMediaCodecRep const& codecId,
                                QMediaDeviceRep const& deviceId);

    void createSession(QString const& responseChannel,
                        QUuid const& id,
                        QString const& domain,
                        QMediaCodecRep const& codec,
                        QMediaDeviceRep const& inputSink,
                        QMediaDeviceRep const& outputSink,
                        QMediaDeviceRep const& inputSource,
                        QMediaDeviceRep const& outputSource);

    void destroySession(QUuid const& id);

signals:
    void sessionCreated(QUuid id);
    void sessionError(QUuid id, QString error);

private:
    MediaEngine*    m_mediaEngine;
    SessionMap      m_sessionMap;
};

}   // ns mediaserver

#endif  // __QTOPIA_MEDIASERVER_QTOPIAMEDIAPROVIDER_H
