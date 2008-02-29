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

#include <sys/types.h>
#include <unistd.h>

#include <QHash>

#include <QCoreApplication>
#include <qtopialog.h>
#include <qtopiaipcadaptor.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>
#include <qvaluespace.h>
#include <qmediasessionrequest.h>

#include "media.h"
#include "mediaserverproxy_p.h"


namespace mlp
{

// {{{ MediaServerCallback
MediaServerCallback::~MediaServerCallback()
{
}
// }}}


// {{{ MediaServerProxy
MediaServerProxy::~MediaServerProxy()
{
}

// {{ Info
QStringList MediaServerProxy::simpleProviderTags() const
{
    return m_simpleInfo->subPaths();
}

QStringList MediaServerProxy::simpleMimeTypesForProvider(QString const& providerTag)
{
    return m_simpleInfo->value(providerTag + "/mimeTypes").toStringList();
}

QStringList MediaServerProxy::simpleUriSchemesForProvider(QString const& providerTag)
{
    return m_simpleInfo->value(providerTag + "/uriSchemes").toStringList();
}
// }}}

// {{{ QMediaContent
QMediaHandle MediaServerProxy::prepareContent
(
 MediaServerCallback*   callback,
 QString const&         domain,
 QUrl const&            url
)
{
    QMediaSessionRequest    request(domain, "com.trolltech.qtopia.uri");
    QtopiaServiceRequest    envelope(QTOPIA_MEDIASERVER_CHANNEL,
                                        "createSession(QString,QMediaSessionRequest)");

    // Keep record
    m_callbackMap.insert(request.id(), callback);

    // Send request
    request << url;

    envelope << m_channel << request;
    envelope.send();

    return QMediaHandle(request.id());
}
// }}}

void MediaServerProxy::destroySession(QMediaHandle const& handle)
{
    QtopiaServiceRequest    envelope(QTOPIA_MEDIASERVER_CHANNEL,
                                        "destroySession(QUuid)");

    envelope << handle.id();
    envelope.send();

    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}


MediaServerProxy* MediaServerProxy::instance()
{
    static MediaServerProxy proxy;

    return &proxy;
}

// private slots:
void MediaServerProxy::sessionCreated(QUuid const& id)
{
    CallbackMap::iterator it = m_callbackMap.find(id);

    if (it != m_callbackMap.end())
    {
        (*it)->mediaReady();

        m_callbackMap.erase(it);
    }
}

void MediaServerProxy::sessionError(QUuid const& id, QString const& error)
{
    CallbackMap::iterator it = m_callbackMap.find(id);

    if (it != m_callbackMap.end())
    {
        (*it)->mediaError(error);

        m_callbackMap.erase(it);
    }
}

void MediaServerProxy::simpleInfoChanged()
{
}

MediaServerProxy::MediaServerProxy()
{
    // channel
    m_channel = QString(QTOPIA_MEDIALIBRARY_CHANNEL).arg(getpid());
    m_adaptor = new QtopiaIpcAdaptor(m_channel, this);

    QtopiaIpcAdaptor::connect(m_adaptor, SIGNAL(sessionCreated(QUuid)),
            this, SLOT(sessionCreated(QUuid)));

    QtopiaIpcAdaptor::connect(m_adaptor, SIGNAL(sessionError(QUuid,QString)),
            this, SLOT(sessionError(QUuid,QString)));

    // values
    m_simpleInfo = new QValueSpaceItem("/Media/Library/Info/Simple", this);
    /*   Don't currently need to monitor
    connect(m_simpleInfo, SIGNAL(contentsChanged()),
            this, SLOT(simpleInfoChanged()));
    */
}
// }}}

}   // ns mlp

