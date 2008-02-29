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

#ifndef __QTOPIAMEDIA_MEDIASERVERPROXY_H
#define __QTOPIAMEDIA_MEDIASERVERPROXY_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QString>
#include <QMap>
#include <QUuid>
#include <QUrl>

#include "qmediahandle_p.h"

class QValueSpaceItem;
class QtopiaIpcAdaptor;

namespace mlp
{

class MediaServerCallback
{
public:

    virtual ~MediaServerCallback();
    virtual void mediaReady() = 0;
    virtual void mediaError(QString const& error) = 0;
};


class MediaServerProxy : public QObject
{
    Q_OBJECT

    typedef QMap<QUuid, MediaServerCallback*>   CallbackMap;

public:
    ~MediaServerProxy();

    // {{{ Info
    QStringList simpleProviderTags() const;
    QStringList simpleMimeTypesForProvider(QString const& providerTag);
    QStringList simpleUriSchemesForProvider(QString const& providerTag);
    // }}}

    // {{{ QMediaContent
    QMediaHandle prepareContent(MediaServerCallback* callback,
                                QString const& domain,
                                QUrl const& url);
    // }}}

    void destroySession(QMediaHandle const& handle);

    static MediaServerProxy* instance();

private slots:
    void sessionCreated(QUuid const& id);
    void sessionError(QUuid const& id, QString const& error);

    void simpleInfoChanged();

private:
    MediaServerProxy();

    void buildCodecList();
    void buildDeviceList();

    QString                 m_channel;
    QValueSpaceItem*        m_simpleInfo;
    QValueSpaceItem*        m_mediaServerCodecs;
    QValueSpaceItem*        m_mediaServerDevices;
    CallbackMap             m_callbackMap;
    QtopiaIpcAdaptor*       m_adaptor;
};

}   // ns mlp

#endif  //  __QTOPIAMEDIA_MEDIASERVERPROXY_H
