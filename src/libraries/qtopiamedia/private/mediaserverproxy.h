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

#ifndef __QTOPIAMEDIA_MEDIASERVERPROXY_H
#define __QTOPIAMEDIA_MEDIASERVERPROXY_H

#include <qstring.h>
#include <qmap.h>
#include <quuid.h>
#include <qpair.h>

#include "qmediadeviceinfo.h"
#include "qmediadevicerep.h"
#include "qmediacodecinfo.h"
#include "qmediacodecrep.h"

class QValueSpaceItem;
class QtopiaIpcAdaptor;

namespace mlp
{

class MediaServerCallback
{
public:

    virtual ~MediaServerCallback() {}
    virtual void mediaReady(int key, QUuid const& id) = 0;
    virtual void mediaError(int key, QString const& error) = 0;
};


class MediaServerProxy : public QObject
{
    Q_OBJECT

    typedef QMap<QUuid, QPair<int, MediaServerCallback*> >    IdMap;

public:
    ~MediaServerProxy();

    // {{{ Info
    QMediaCodecInfoList availableEncoders(QString const& domain) const;
    QMediaCodecInfoList availableDecoders(QString const& domain) const;
    QMediaDeviceInfoList availableMediaDevices(QString const& domain) const;
    // }}}

    // {{{ QMediaSession
    int createDecodeSession(MediaServerCallback* callback,
                        QString const& domain,
                        QString const& url,
                        QMediaCodecRep const& codec,
                        QMediaDeviceRep const& device);

    int createEncodeSession(MediaServerCallback* callback,
                        QString const& domain,
                        QString const& url,
                        QMediaCodecRep const& codec,
                        QMediaDeviceRep const& device);

    int createSession(MediaServerCallback* callback,
                        QString const& domain,
                        QMediaCodecRep const& codec,
                        QMediaDeviceRep const& inputSink,
                        QMediaDeviceRep const& outputSink,
                        QMediaDeviceRep const& inputSource,
                        QMediaDeviceRep const& outputSource);
    // }}}

    // {{{ QContentPlayer
    int prepareContent(MediaServerCallback* callback, QString const& domain, QString const& url);
    int playContent(MediaServerCallback* callback, QString const& domain, QString const& url);
    // }}}

    void destroySession(QUuid const& id);

    static MediaServerProxy* instance();

private slots:
    void sessionCreated(QUuid const& id);
    void sessionError(QUuid const& id, QString const& error);

    void codecsChanged();
    void devicesChanged();

private:
    MediaServerProxy();

    void buildCodecList();
    void buildDeviceList();

    QString             m_channel;
    QValueSpaceItem*    m_mediaServerCodecs;
    QValueSpaceItem*    m_mediaServerDevices;
    QMediaCodecInfoList     m_encoders;
    QMediaCodecInfoList     m_decoders;
    QMediaDeviceInfoList    m_devices;
    IdMap               m_idMap;
    QtopiaIpcAdaptor*   m_adaptor;
};

}   // ns mlp

#endif  //  __QTOPIAMEDIA_MEDIASERVERPROXY_H
