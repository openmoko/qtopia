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

#include "lowmemorytask.h"
#include <QContentSet>
#include <QStringList>
#include <QtopiaIpcEnvelope>
#include "applicationlauncher.h"

/*!
  \class LowMemoryTask
  \ingroup QtopiaServer::Task
  \brief The LowMemoryTask class reponds to low memory situations by closing applications.

  The LowMemoryTask monitors memory pressure through the default MemoryMonitor
  providing task.  The following table describes the action taken by this task
  at each memory level.

  \table
  \header \o Memory State \o Action
  \row \o MemoryMonitor::MemUnknown \o No action.
  \row \o MemoryMonitor::MemNormal \o No action.
  \row \o MemoryMonitor::MemLow \o A \c {quitIfInvisible()} message is sent to the first running, non-preloaded application.  If this application is in the lazy-application-shutdown state it will consequently terminate.
  \row \o \o A \c {RecoverMemory()} message is sent to the \c {QPE/System} channel.  Applications may use this as a trigger to recover memory or to free caches.
  \row \o MemoryMonitor::MemVeryLow \o A \c {quitIfInvisible()} message is sent to all running, non-preloaded applications.  If these applications are in the lazy-application-shutdown state they will consequently terminate.
  \row \o \o A \c {RecoverMemory()} message is sent to the \c {QPE/System} channel.  Applications may use this as a trigger to recover memory or to free caches.
  \row \o MemoryMonitor::MemCritical \o A \c {quitIfInvisible()} message is sent to all running, non-preloaded applications.  If these applications are in the lazy-application-shutdown state they will consequently terminate.
  \row \o \o A \c {RecoverMemory()} message is sent to the \c {QPE/System} channel.  Applications may use this as a trigger to recover memory or to free caches.
  \row \o \o The most recently launched application is forcibly killed.  The most recently launched application refers to that most recently executed by the system, not the most recently raised.
  \endtable
 */

/*! \internal */
LowMemoryTask::LowMemoryTask()
: m_lastStartedApp(0)
{
    MemoryMonitor *mm = qtopiaTask<MemoryMonitor>();
    ApplicationLauncher *al = qtopiaTask<ApplicationLauncher>();

    if(mm && al) {
        QObject::connect(mm,
                         SIGNAL(memoryStateChanged(MemoryMonitor::MemState)),
                         this,
                         SLOT(memoryStateChanged(MemoryMonitor::MemState)));

        QObject::connect(al,
                         SIGNAL(applicationStateChanged(const QString &, ApplicationTypeLauncher::ApplicationState)),
                         this,
                         SLOT(applicationStateChanged(const QString &, ApplicationTypeLauncher::ApplicationState)));

    } else {
        QtopiaServerApplication::taskValueSpaceSetAttribute("LowMemoryTask", "Error", "No memory monitor or no application launcher");
    }
}

/*!
  Destroys the LowMemoryTask instance.
 */
LowMemoryTask::~LowMemoryTask()
{
    if(m_lastStartedApp) {
        delete m_lastStartedApp;
        m_lastStartedApp = 0;
    }
}

/*! \internal */
void LowMemoryTask::memoryStateChanged(MemoryMonitor::MemState state)
{
    if(state == MemoryMonitor::MemNormal ||
       state == MemoryMonitor::MemUnknown)
        return; // Do nothing

    qWarning("LowMemoryTask: Memory low (%d).  Recovering.", state);

    ApplicationLauncher *al = qtopiaTask<ApplicationLauncher>();
    QStringList running = al?al->applications():QStringList();

    if(running.count() > 1) {
        QContentFilter filter(QContent::Application);
        QContentSet set(filter);

        for(QStringList::const_iterator it = running.begin();
            running.end() != it;
            ++it) {
            QContent app = set.findExecutable(*it);

            if ( !app.isValid() ) continue;
            if ( !app.isPreloaded() ) {
                if(m_lastStartedApp) {
                    delete m_lastStartedApp;
                    m_lastStartedApp = 0;
                }
                qWarning("LowMemoryTask: Shutdown %s",
                         (const char *)(*it).toLatin1());

                QtopiaIpcEnvelope e(QByteArray("QPE/Application/")+(const char *)(*it).toLocal8Bit(), "quitIfInvisible()");
                if (state == MemoryMonitor::MemLow)
                    break;
            }
        }
    }

    QtopiaIpcEnvelope e("QPE/System", "RecoverMemory()");

    //kill if new app was started recently
    if(state == MemoryMonitor::MemCritical && m_lastStartedApp) {
        qWarning("LowMemoryTask: Page-thrashing: Killing %s",
                 m_lastStartedApp->toLocal8Bit().constData());
        ApplicationLauncher *al = qtopiaTask<ApplicationLauncher>();
        al->kill(*m_lastStartedApp);
    }
}

/*! \internal */
void LowMemoryTask::applicationStateChanged(const QString &app, ApplicationTypeLauncher::ApplicationState state)
{
    if(ApplicationTypeLauncher::NotRunning == state) {
        if(m_lastStartedApp) {
            delete m_lastStartedApp;
            m_lastStartedApp = 0;
        }
    } else if(ApplicationTypeLauncher::Running == state) {
        if(m_lastStartedApp)
            delete m_lastStartedApp;
        m_lastStartedApp = new QString(app);

        // This restricts the maximum number of apps we leave running.
        // If we use a memory monitor then we could just rely on it to
        // close applications when mem is tight.
        ApplicationLauncher *al = qtopiaTask<ApplicationLauncher>();
        QStringList running = al?al->applications():QStringList();
        if (running.count() > 6) {
            QContentFilter filter(QContent::Application);
            QContentSet set(filter);

            for(QStringList::const_iterator it= running.begin();
                    running.end() != it;
                    ++it) {
                QContent app = set.findExecutable( *it );
                if ( !app.isValid() ) continue;
                if ( !app.isPreloaded() ) {
                    QtopiaIpcEnvelope e(QByteArray("QPE/Application/")+(const char *)(*it).toLocal8Bit(), "quitIfInvisible()");
                    break;
                }
            }
        }

    }
}

QTOPIA_TASK(LowMemoryTask, LowMemoryTask);
