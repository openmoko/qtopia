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

#include "qtopiaserverapplication.h"
#include <QtopiaIpcEnvelope>
#include "applicationlauncher.h"
#include <QTimer>

class ApplicationShutdownTask : public SystemShutdownHandler
{
Q_OBJECT
public:
    ApplicationShutdownTask();

    virtual bool systemRestart();
    virtual bool systemShutdown();

public slots:
    void terminated();
    void timeout();

private:
    bool doShutdown();
    bool allAppsQuit();
    void killAll();

    enum State { NoShutdown, WaitingForShutdown, KilledApps, Shutdown };
    State m_state;
    QTimer m_timer;
    ApplicationLauncher *m_launcher;
};

ApplicationShutdownTask::ApplicationShutdownTask()
: SystemShutdownHandler(), m_state(NoShutdown), m_launcher(0)
{
    m_launcher = qtopiaTask<ApplicationLauncher>();
    if(!m_launcher) return;
    
    m_timer.setInterval(3000);
    m_timer.setSingleShot(true);
    QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    QObject::connect(m_launcher, SIGNAL(applicationTerminated(QString,ApplicationTypeLauncher::TerminationReason,bool)), this, SLOT(terminated()));
}

bool ApplicationShutdownTask::systemRestart()
{
    return doShutdown();
}

bool ApplicationShutdownTask::systemShutdown()
{
    return doShutdown();
}

bool ApplicationShutdownTask::doShutdown()
{
    if(!m_launcher || m_state != NoShutdown) return true;

    if(!allAppsQuit()) {
        QtopiaIpcEnvelope env("QPE/System", "quit()");
        m_timer.start();
        m_state = WaitingForShutdown;
        return false;
    } else {
        m_state = Shutdown;
        return true;
    }
}

void ApplicationShutdownTask::killAll()
{
    QStringList apps = m_launcher->applications();
    for(int ii = 0; ii < apps.count(); ++ii)
        m_launcher->kill(apps.at(ii));
}

bool ApplicationShutdownTask::allAppsQuit()
{
    QStringList apps = m_launcher->applications();
    return (apps.isEmpty() || (apps.count() == 1 && apps.first() == "qpe"));
}

void ApplicationShutdownTask::timeout()
{
    switch(m_state) {
        case NoShutdown:
        case Shutdown:
        default:
            Q_ASSERT(!"Unknown");
            break;
        case WaitingForShutdown:
            // Issue kill
            killAll();
            m_timer.start();
            m_state = KilledApps;
            break;
        case KilledApps:
            // Failed - proceed anyway
            m_state = Shutdown;
            qWarning("Applications failed to exit at system shutdown");
            emit proceed();
            break;
    }
}

void ApplicationShutdownTask::terminated()
{
    if(allAppsQuit()) {
        m_timer.stop();
        if(m_state != NoShutdown && m_state != Shutdown) {
            m_state = Shutdown;
            emit proceed();
        }
    }
}

QTOPIA_DEMAND_TASK(ApplicationShutdownTask, ApplicationShutdownTask);
QTOPIA_TASK_PROVIDES(ApplicationShutdownTask, SystemShutdownHandler);

#include "applicationshutdowntask.moc"
