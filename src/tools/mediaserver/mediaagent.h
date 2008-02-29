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

#ifndef __MEDIASERVER_MEDIAAGENT_H
#define __MEDIASERVER_MEDIAAGENT_H

#include <QObject>


class QMediaSessionRequest;
class QMediaServerSession;
class QMediaEngineInformation;

namespace mediaserver
{

class MediaAgentSession;

class MediaAgentPrivate;

class MediaAgent : public QObject
{
    Q_OBJECT
    friend class MediaAgentSession;

public:
    ~MediaAgent();

    QMediaServerSession* createSession(QMediaSessionRequest const& sessionRequest);
    void destroySession(QMediaServerSession* mediaSession);

    static MediaAgent* instance();

private:
    MediaAgent();

    void initialize();

    void sessionStarting(MediaAgentSession* session);
    void sessionStopped(MediaAgentSession* session);

    MediaAgentPrivate*  d;
};


}   // ns mediaserver

#endif  // __MEDIASERVER_MEDIAAGENT_H

