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

#include <qcontent.h>
#include <qcopchannel_qws.h>
#include <qsoundcontrol.h>

#include <qtopialog.h>

#include "mediaserver.h"
#include "mediaengine.h"
#include "mediasession.h"

#include "qsoundprovider.h"


namespace mediaserver
{


// QSoundPlayer
QSoundPlayer::QSoundPlayer(MediaEngine* engine, QUuid const& id):
    m_id(id),
    m_mediaSession(NULL),
    m_mediaEngine(engine)
{
}

QSoundPlayer::~QSoundPlayer()
{
    m_mediaEngine->destroySession(m_mediaSession);
}

void QSoundPlayer::open(QString const& url)
{
    QString     tmpUrl = url;
    QContent    content(url, false);

    if (content.drmState() == QContent::Protected)
        tmpUrl.prepend("qtopia://");
    else
        tmpUrl.prepend("file://");

    m_mediaSession = m_mediaEngine->createSession(m_id, tmpUrl);

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
        m_mediaSession->setDomain("Default");
        break;

    case QSoundControl::RingTone:
        m_mediaSession->setDomain("RingTone");
        break;
    }
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


// QSoundProvider
QSoundProvider::QSoundProvider(MediaEngine* engine):
    QtopiaIpcAdaptor(QT_MEDIASERVER_CHANNEL),
    m_mediaEngine(engine)
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
        m_mediaEngine->setActiveDomain("Default");
        break;

    case QSoundControl::RingTone:
        m_mediaEngine->setActiveDomain("RingTone");
        break;
    }
}

void QSoundProvider::subscribe(const QUuid& id)
{
     m_playerMap.insert(id, new QSoundPlayer(m_mediaEngine, id));
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

}   // ns mediaserver
