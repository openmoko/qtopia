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

#include <QTimer>
#include <QValueSpaceObject>
#include <QMediaServerSession>
#include <QMediaSessionRequest>
#include <QDebug>

#include "mediaagent.h"
#include "domainmanager.h"
#include "sessionmanagersession.h"

#include "sessionmanager.h"


namespace mediaserver
{

// {{{ SessionStatusMonitor
class SessionStatusMonitor : public QObject
{
    Q_OBJECT

public:
    SessionStatusMonitor(QValueSpaceObject* statusStore,
                   QMediaServerSession* session,
                   QObject* parent = 0);

private slots:
    void sessionStateChanged(QtopiaMedia::State state);
    void sessionDestroyed();

private:
    QValueSpaceObject*      m_statusStore;
    QString                 m_sessionId;
    QMediaServerSession*    m_session;
};

SessionStatusMonitor::SessionStatusMonitor
(
 QValueSpaceObject* statusStore,
 QMediaServerSession* session,
 QObject* parent
):
    QObject(parent),
    m_statusStore(statusStore),
    m_session(session)
{
    connect(m_session, SIGNAL(playerStateChanged(QtopiaMedia::State)),
            this, SLOT(sessionStateChanged(QtopiaMedia::State)));

    connect(m_session, SIGNAL(destroyed(QObject*)),
            this, SLOT(sessionDestroyed()));

    m_sessionId = session->id();

    m_statusStore->setAttribute(m_sessionId + "/Domain", session->domain());
    m_statusStore->setAttribute(m_sessionId + "/Data", session->reportData());
    m_statusStore->setAttribute(m_sessionId + "/State", int(session->playerState()));
}

void SessionStatusMonitor::sessionStateChanged(QtopiaMedia::State state)
{
    m_statusStore->setAttribute(m_sessionId + "/State", int(state));
}

void SessionStatusMonitor::sessionDestroyed()
{
    m_statusStore->removeAttribute(m_sessionId);
    deleteLater();
}
// }}}

// {{{ SessionInfo
class SessionInfo
{
public:
    enum SessionState { Starting, Activated, Deactivated, Stopped };

    SessionInfo():state(Stopped) {}

    SessionState    state;
};
// }}}

// {{{ SessionManagerPrivate
class SessionManagerPrivate : public QObject
{
    Q_OBJECT

    typedef QMap<SessionManagerSession*, SessionInfo>   ManagedSessions;

public:
    SessionManagerPrivate(SessionManager* manager);
    ~SessionManagerPrivate();

public slots:
    void sessionStateChange(QtopiaMedia::State state);
    void domainStatusChange(QStringList const& newDomains, QStringList const& oldDomains);

public:
    int                 activePlayingSessions;
    MediaAgent*         mediaAgent;
    DomainManager*      domainManager;
    SessionManager*     sessionManager;
    ManagedSessions     managedSessions;
    QValueSpaceObject*  status;
    QList<QMediaServerSession*> sessions;
};

SessionManagerPrivate::SessionManagerPrivate(SessionManager* manager):
    activePlayingSessions(0),
    sessionManager(manager)
{
    mediaAgent = MediaAgent::instance();
    domainManager = DomainManager::instance();

    status = new QValueSpaceObject("/Media/Sessions");

    connect(domainManager, SIGNAL(domainStatusChange(QStringList,QStringList)),
            this, SLOT(domainStatusChange(QStringList,QStringList)));
}

SessionManagerPrivate::~SessionManagerPrivate()
{
    delete status;
}

void SessionManagerPrivate::domainStatusChange
(
 QStringList const& activeDomains,
 QStringList const& inactiveDomains
)
{
    ManagedSessions::iterator it;

    // Deactivate any
    for (it = managedSessions.begin(); it != managedSessions.end(); ++it)
    {
        if (!domainManager->isActiveDomain(it.key()->domain()))
        {
            switch (it.value().state)
            {
            case SessionInfo::Activated:
                it.key()->deactivate();
                it.value().state = SessionInfo::Deactivated;
                break;

            case SessionInfo::Starting:
            case SessionInfo::Deactivated:
            case SessionInfo::Stopped:
                break;
            }
        }
    }

    // Activate any
    for (it = managedSessions.begin(); it != managedSessions.end(); ++it)
    {
        if (domainManager->isActiveDomain(it.key()->domain()))
        {
            switch (it.value().state)
            {
            case SessionInfo::Starting:
            case SessionInfo::Deactivated:
                it.key()->activate();
                it.value().state = SessionInfo::Activated;
                break;

            case SessionInfo::Activated:
            case SessionInfo::Stopped:
                break;
            }
        }
    }

    Q_UNUSED(activeDomains);
    Q_UNUSED(inactiveDomains);
}

SessionManager* SessionManager::instance()
{
    static SessionManager self;

    return &self;
}

void SessionManagerPrivate::sessionStateChange(QtopiaMedia::State state)
{
    int current = activePlayingSessions;

    // count playing state
    switch (state)
    {
    case QtopiaMedia::Playing:
        ++activePlayingSessions;
        break;

    case QtopiaMedia::Paused:
    case QtopiaMedia::Stopped:
    case QtopiaMedia::Error:
        --activePlayingSessions;
        break;

    case QtopiaMedia::Buffering:
        break;
    }

    // emit
    if (current != activePlayingSessions)
        sessionManager->activeSessionCountChanged(activePlayingSessions);
}
// }}}

// {{{ SessionManager
SessionManager::SessionManager()
{
    d = new SessionManagerPrivate(this);
}

SessionManager::~SessionManager()
{
    delete d;
}

QMediaServerSession* SessionManager::createSession(QMediaSessionRequest const& sessionRequest)
{
    QMediaServerSession*      mediaSession;

    // Create the session
    mediaSession = d->mediaAgent->createSession(sessionRequest);

    if (mediaSession != 0)
    {
        QString const& domain = sessionRequest.domain();

        // Set domain
        mediaSession->setDomain(domain);

        // Listen to aggregated status to set suspend state of device
        connect(mediaSession, SIGNAL(playerStateChanged(QtopiaMedia::State)),
                d, SLOT(sessionStateChange(QtopiaMedia::State)));

        // Monitor status of the session
        new SessionStatusMonitor(d->status, mediaSession, this);

        // Wrap
        SessionManagerSession*  session = new SessionManagerSession(this, mediaSession);

        d->managedSessions.insert(session, SessionInfo());

        mediaSession = session;

        d->sessions.append(mediaSession);
    }

    return mediaSession;
}

void SessionManager::destroySession(QMediaServerSession* mediaSession)
{
    // Unwrap
    SessionManagerSession*  wrapper = qobject_cast<SessionManagerSession*>(mediaSession);
    QMediaServerSession*    session = wrapper->wrappedSession();

    d->managedSessions.remove(wrapper);
    delete wrapper;

    // send to agent to remove
    d->mediaAgent->destroySession(session);

    d->sessions.removeAll(mediaSession);
}

bool SessionManager::sessionCanStart(SessionManagerSession* session)
{
    bool            rc = false;
    SessionInfo&    info = d->managedSessions[session];

    switch (info.state)
    {
    case SessionInfo::Starting:
    case SessionInfo::Deactivated:
        rc = false;
        break;

    case SessionInfo::Activated:
        rc = true;
        break;

    case SessionInfo::Stopped:
        rc = d->domainManager->activateDomain(session->domain());
        info.state = rc ? SessionInfo::Activated : SessionInfo::Starting;
        break;
    }

    return rc;
}

void SessionManager::sessionStopped(SessionManagerSession* session)
{
    d->domainManager->deactivateDomain(session->domain());
    d->managedSessions[session].state = SessionInfo::Stopped;
}

QList<QMediaServerSession*> const& SessionManager::sessions() const
{
    return d->sessions;
}
// }}} SessionManager

}   // ns mediaserver


#include "sessionmanager.moc"




