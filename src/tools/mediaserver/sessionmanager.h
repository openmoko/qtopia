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

#ifndef __QTOPIA_MEDIASERVER_SESSIONMANAGER_H
#define __QTOPIA_MEDIASERVER_SESSIONMANAGER_H

#include <QObject>

#include <qtopiamedia/media.h>


// pre
class QMediaSessionRequest;
class QMediaServerSession;


namespace mediaserver
{

class SessionManagerSession;

class SessionManagerPrivate;

class SessionManager : public QObject
{
    Q_OBJECT
    friend class SessionManagerSession;
    friend class SessionManagerPrivate;

public:
    ~SessionManager();

    QMediaServerSession* createSession(QMediaSessionRequest const& sessionRequest);
    void destroySession(QMediaServerSession* mediaSession);

    static SessionManager* instance();

signals:
    void activeSessionCountChanged(int);

private:
    SessionManager();

    bool sessionCanStart(SessionManagerSession* session);
    void sessionStopped(SessionManagerSession* session);

    SessionManagerPrivate*  d;
};


}   // ns mediaserver

#endif  // __QTOPIA_MEDIASERVER_SESSIONMANAGER_H

