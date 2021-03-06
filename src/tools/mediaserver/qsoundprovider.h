/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef __QTOPIA_MEDIASERVER_QSOUNDPROVIDER_H
#define __QTOPIA_MEDIASERVER_QSOUNDPROVIDER_H

#include <qtopiaipcadaptor.h>
#include <quuid.h>

#include <media.h>


class QMediaServerSession;


namespace mediaserver
{

class SessionManager;


class QSoundPlayer : public QObject
{
    Q_OBJECT

public:
    QSoundPlayer(QUuid const& id, SessionManager* sessionManager);
    ~QSoundPlayer();

    void open(QString const& filePath);
    void setVolume(int volume);
    void setPriority(int priority);
    void play();
    void stop();

private slots:
    void playerStateChanged(QtopiaMedia::State state);

private:
    QUuid           m_id;
    QString         m_domain;
    SessionManager* m_sessionManager;
    QMediaServerSession*  m_mediaSession;
};


class QSoundProvider : public QtopiaIpcAdaptor
{
    Q_OBJECT

    typedef QMap<QUuid, QSoundPlayer*>  PlayerMap;

public:
    QSoundProvider(SessionManager* sessionManager);
    ~QSoundProvider();

public slots:
    // System wide messages
    void setPriority(int priority);

    // Sound specific messages
    void subscribe(const QUuid& id);

    void open(const QUuid& id, const QString& url);

    void setVolume(const QUuid& id, int volume);

    void setPriority(const QUuid& id, int priority);

    void play(const QUuid& id);

    void stop(const QUuid& id);

    void revoke(const QUuid& id);

private:
    QSoundPlayer* player(QUuid const& id);

    PlayerMap       m_playerMap;
    SessionManager* m_sessionManager;
};

}   // ns mediaserver

#endif  // __QTOPIA_MEDIASERVER_QSOUNDPROVIDER_H

