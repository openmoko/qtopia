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

#ifndef __QTOPIA_MEDIASERVER_MEDIACONTROLSERVER_H
#define __QTOPIA_MEDIASERVER_MEDIACONTROLSERVER_H

#include <QMediaAbstractControlServer>

#include <media.h>

#include <qmediahandle_p.h>


class QMediaServerSession;

namespace mediaserver
{


class MediaControlServer : public QMediaAbstractControlServer
{
    Q_OBJECT

public:
    MediaControlServer(QMediaServerSession* mediaSession,
                       QMediaHandle const& handle);

    ~MediaControlServer();

    QMediaServerSession* mediaSession() const;

public slots:
    void start();
    void pause();
    void stop();
    void seek(quint32 ms);

    void setVolume(int volume);
    void setMuted(bool mute);

signals:
    void playerStateChanged(QtopiaMedia::State state);
    void positionChanged(quint32 ms);
    void lengthChanged(quint32 ms);
    void volumeChanged(int volume);
    void volumeMuted(bool muted);

private slots:
    void stateChanged(QtopiaMedia::State state);
    void posChanged(quint32 ms);
    void lenChanged(quint32 ms);
    void volChanged(int volume);
    void volMuted(bool muted);

private:
    QMediaServerSession* m_mediaSession;
};


}   // ns mediaserver

#endif  // __QTOPIA_MEDIASERVER_MEDIACONTROLSERVER_H
