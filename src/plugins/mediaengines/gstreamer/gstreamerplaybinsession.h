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

#ifndef __GSTEAMER_PLAYBINSESSION_H
#define __GSTEAMER_PLAYBINSESSION_H

#include <QUrl>
#include <QUuid>

#include <QMediaServerSession>



namespace gstreamer
{

class Engine;
class Message;
class PlaybinSessionPrivate;

class PlaybinSession : public QMediaServerSession
{
    Q_OBJECT

public:
    PlaybinSession(Engine* engine, QUuid const& id, QUrl const& url);
    ~PlaybinSession();

    bool isValid() const;

    void start();
    void pause();
    void stop();

    void suspend();
    void resume();

    void seek(quint32 ms);
    quint32 length();

    void setVolume(int volume);
    int volume() const;

    void setMuted(bool mute);
    bool isMuted() const;

    QtopiaMedia::State playerState() const;

    QString errorString();

    void setDomain(QString const& domain);
    QString domain() const;

    QStringList interfaces();

    QString id() const;
    QString reportData() const;

private slots:
    void busMessage(Message const& msg);

private:
    void getStreamsInfo();
    void readySession();

    PlaybinSessionPrivate*  d;
};

}   // ns gstreamer


#endif  // __GSTREAMER_PLAYBINSESSION_H
