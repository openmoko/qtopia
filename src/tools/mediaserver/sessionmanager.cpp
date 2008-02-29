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

    void addSession(SessionManagerSession);

    SessionState    state;
};
// }}}

// {{{ SessionManagerPrivate
class SessionManagerPrivate :
    public QObject,
    public DomainManagerCallback
{
    Q_OBJECT

    typedef QMap<SessionManagerSession*, SessionInfo>   ManagedSessions;

public:
    SessionManagerPrivate(SessionManager* manager);
    ~SessionManagerPrivate();

    void domainChange(QStringList const& newDomains, QStringList const& oldDomains);

public slots:
    void sessionStateChange(QtopiaMedia::State state);

public:
    bool                wrapSessions;
    int                 activePlayingSessions;
    MediaAgent*         mediaAgent;
    DomainManager*      domainManager;
    SessionManager*     sessionManager;
    ManagedSessions     managedSessions;
    QValueSpaceObject*  status;
};


SessionManagerPrivate::SessionManagerPrivate(SessionManager* manager):
    wrapSessions(false),
    activePlayingSessions(0),
    sessionManager(manager)
{
    mediaAgent = MediaAgent::instance();
    domainManager = DomainManager::instance();

    status = new QValueSpaceObject("/Media/Sessions");

    // Should Wrap?
    QStringList const& audioDomains = domainManager->availableDomains();

    if (audioDomains.size() > 1)
    {
        QStringList::const_iterator it = audioDomains.begin();
        int priority = domainManager->priorityForDomain(*it);

        for (++it; it != audioDomains.end(); ++it)
        {
            if (priority != domainManager->priorityForDomain(*it))
            {
                wrapSessions = true;
                domainManager->addCallback(this);
                break;
            }
        }
    }
}

SessionManagerPrivate::~SessionManagerPrivate()
{
    delete status;
}

void SessionManagerPrivate::domainChange
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
}

SessionManager* SessionManager::instance()
{
    static SessionManager self;

    return &self;
}

// {{{ state change
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
        if (d->wrapSessions)
        {
            SessionManagerSession*  session = new SessionManagerSession(this, mediaSession);

            d->managedSessions.insert(session, SessionInfo());

            mediaSession = session;
        }
    }

    return mediaSession;
}

void SessionManager::destroySession(QMediaServerSession* mediaSession)
{
    QMediaServerSession*    session = mediaSession;

    if (d->wrapSessions)
    {
        SessionManagerSession*  wrapper = qobject_cast<SessionManagerSession*>(mediaSession);

        d->managedSessions.remove(wrapper);

        session = wrapper->wrappedSession();

        delete wrapper;
    }

    // send to agent to remove
    d->mediaAgent->destroySession(session);
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
// }}} SessionManager

}   // ns mediaserver


#include "sessionmanager.moc"




