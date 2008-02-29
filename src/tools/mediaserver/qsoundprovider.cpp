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

#include <QUrl>

#include <qcontent.h>
#ifdef Q_WS_X11
#include <qcopchannel_x11.h>
#else
#include <qcopchannel_qws.h>
#endif
#include <qsoundcontrol.h>
#include <qtopialog.h>
#include <qmediaserversession.h>
#include <qmediasessionrequest.h>

#include "sessionmanager.h"
#include "domainmanager.h"

#include "qsoundprovider.h"


namespace mediaserver
{


// {{{ QSoundPlayer
QSoundPlayer::QSoundPlayer(QUuid const& id, SessionManager* sessionManager):
    m_id(id),
    m_domain("Default"),
    m_sessionManager(sessionManager),
    m_mediaSession(NULL)
{
}

QSoundPlayer::~QSoundPlayer()
{
    m_sessionManager->destroySession(m_mediaSession);
}

void QSoundPlayer::open(QString const& filePath)
{
    QUrl                    url;
    QContent                content(filePath, false);
    QMediaSessionRequest    sessionRequest(m_domain, "com.trolltech.qtopia.uri");

    // Make a URL
    if (content.drmState() == QContent::Protected)
        url.setScheme("qtopia");
    else
        url.setScheme("file");

    url.setPath(filePath);

    sessionRequest << url;

    m_mediaSession = m_sessionManager->createSession(sessionRequest);

    if (m_mediaSession)
    {
        connect(m_mediaSession, SIGNAL(playerStateChanged(QtopiaMedia::State)),
                this, SLOT(playerStateChanged(QtopiaMedia::State)));
    }
}

void QSoundPlayer::setVolume(int volume)
{
    if (m_mediaSession)
        m_mediaSession->setVolume(volume);
}

void QSoundPlayer::setPriority(int priority)
{
    switch (priority)
    {
    case QSoundControl::Default:
        m_domain = "Default";
        break;

    case QSoundControl::RingTone:
        m_domain = "RingTone";
        break;
    }

    m_mediaSession->setDomain(m_domain);
}

void QSoundPlayer::play()
{
    if (m_mediaSession)
        m_mediaSession->start();
}

void QSoundPlayer::stop()
{
    if (m_mediaSession)
        m_mediaSession->stop();
}

void QSoundPlayer::playerStateChanged(QtopiaMedia::State state)
{
    switch (state)
    {
    case QtopiaMedia::Stopped:
    case QtopiaMedia::Error:
        QCopChannel::send(QString("QPE/QSound/").append(m_id), "done()");
        break;

    default:
        ;
    }
}
// }}}

// {{{ QSoundProvider
QSoundProvider::QSoundProvider(SessionManager* sessionManager):
    QtopiaIpcAdaptor(QT_MEDIASERVER_CHANNEL),
    m_sessionManager(sessionManager)
{
    qLog(Media) << "QSoundProvider starting";

    publishAll(Slots);
}

QSoundProvider::~QSoundProvider()
{
}

void QSoundProvider::setPriority(int priority)
{
    switch (priority)
    {
    case QSoundControl::Default:
        DomainManager::instance()->deactivateDomain("RingTone");
        break;

    case QSoundControl::RingTone:
        DomainManager::instance()->activateDomain("RingTone");
        break;
    }
}

void QSoundProvider::subscribe(const QUuid& id)
{
     m_playerMap.insert(id, new QSoundPlayer(id, SessionManager::instance()));
}

void QSoundProvider::open(const QUuid& id, const QString& url)
{
    if (QSoundPlayer* soundPlayer = player(id))
        soundPlayer->open(url);
}

void QSoundProvider::setVolume(const QUuid& id, int volume)
{
    if (QSoundPlayer* soundPlayer = player(id))
        soundPlayer->setVolume(volume);
}

void QSoundProvider::setPriority(const QUuid& id, int priority)
{
    if (QSoundPlayer* soundPlayer = player(id))
        soundPlayer->setPriority(priority);
}

void QSoundProvider::play(const QUuid& id)
{
    if (QSoundPlayer* soundPlayer = player(id))
        soundPlayer->play();
}

void QSoundProvider::stop(const QUuid& id)
{
    if (QSoundPlayer* soundPlayer = player(id))
        soundPlayer->stop();
}

void QSoundProvider::revoke(const QUuid& id)
{
    PlayerMap::iterator it = m_playerMap.find(id);

    if (it != m_playerMap.end())
    {
        (*it)->stop();

        delete *it;

        m_playerMap.erase(it);
    }
}

QSoundPlayer* QSoundProvider::player(QUuid const& id)
{
    PlayerMap::iterator it = m_playerMap.find(id);

    return it == m_playerMap.end() ? 0 : *it;
}
// }}}

}   // ns mediaserver
