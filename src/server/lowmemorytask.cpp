// -*-C++-*-
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

#include "lowmemorytask.h"
#include <QTimer>
#include <QContentSet>
#include <QStringList>
#include <QtopiaIpcEnvelope>
#include <QValueSpaceItem>
#include "applicationlauncher.h"
#include "windowmanagement.h"

/*!
  \class LowMemoryTask
  \ingroup QtopiaServer::Task
  \ingroup QtopiaServer::Memory
  \brief The LowMemoryTask class responds to low memory situations by closing applications.


  The LowMemoryTask provides a Qtopia Server Task.  Qtopia Server Tasks are
  documented in full in the QtopiaServerApplication class documentation.

  \table
  \row \o Task Name \o LowMemoryTask
  \row \o Interfaces \o None
  \row \o Services \o None
  \endtable

  The LowMemoryTask monitors memory pressure through the default MemoryMonitor
  providing task.  The following table describes the action taken by this task
  at each memory level.

  The following table describes the action taken by this task at each
  memory level.

  \table
  \header \o Memory State \o Action
  \row
    \o MemoryMonitor::MemNormal
    \o No action.
  \row
    \o MemoryMonitor::MemLow
    \o The \c {quitIfInvisible()} message is sent to all the
       applications marked \tt expendable or \tt important that
       are in the lazy-application-shutdown state. These are
       applications that have ended but have been kept loaded
       in case they are run again. Also, the \c {RecoverMemory()}
       message is sent to the \c {QPE/System} channel, which can
       be used by running applications to trigger recovering
       memory and freeing caches.
  \row
    \o MemoryMonitor::MemVeryLow
    \o A \c {quit()} message is sent to one process. If there are
       processes running that are marked \tt expendable, one of
       these is told to quit. Otherwise, if there are processes
       running that are marked \tt important, one of these is
       told to quit. Otherwise, no process is told to quit.
  \row
    \o MemoryMonitor::MemCritical
    \o A \c {kill()} signal is sent to one process. If there are
       processes running that are marked \tt expendable, one of
       these is killed. Otherwise, if there are processes running
       that are marked \tt important, one of these is killed.
       Otherwise, no process is killed, and if the system finally
       runs out of memory, Linux will kill a process according to
       its own rules.
  \endtable

  Although the LowMemoryTask is notified of \bold all memory
  state changes, no action is taken when the memory state improves.
  
  This class is part of the Qtopia server and cannot be used by other Qtopia applications.
 */

/*!
  \internal
  The constructor sets up the class to be notified whenever the
  \l {MemoryMonitor} {memory monitor} emits the memoryStateChanged()
  signal.
 */
LowMemoryTask::LowMemoryTask()
    : m_prevState(MemoryMonitor::MemUnknown)
{
    MemoryMonitor* mm = qtopiaTask<MemoryMonitor>();
    ApplicationLauncher* al = qtopiaTask<ApplicationLauncher>();

    if (mm && al) {
        QObject::connect(mm,SIGNAL(memoryStateChanged(MemoryMonitor::MemState)),this,SLOT(avoidOutOfMemory(MemoryMonitor::MemState)));
    }
    else {
        QtopiaServerApplication::taskValueSpaceSetAttribute("LowMemoryTask",
					    "Error",
					    "Memory monitor disabled");
    }
}

/*!
  Destroys the LowMemoryTask instance.
 */
LowMemoryTask::~LowMemoryTask()
{
    // nothing to do.
}

/*!
  This slot function is notified whenever the
  \l {MemoryMonitor} {memory monitor task} detects a memory
  state change and emits the memoryStateChanged() signal.

  The slot takes action to recover memory if the new memory
  state is low, very low, or critical and the new state is
  worse than the previous one. The goal is to prevent any
  hard out-of-memory situations from occurring by taking
  some action to recover memory to get back to the normal
  memory state. If the hard out-of-memory condition occurs
  anyway, then linux will kill a process chosen according
  to its own rules.
 */
void
LowMemoryTask::avoidOutOfMemory(MemoryMonitor::MemState state)
{
    m_prevState = m_state;
    m_state = state;
    switch (m_state) {
        case MemoryMonitor::MemUnknown:
	    break;
        case MemoryMonitor::MemCritical:
	    qLog(OOM) << "Memory critically low";
	    handleCriticalMemory();
	    break;
        case MemoryMonitor::MemVeryLow:
	    qLog(OOM) << "Memory very low";
	    if (m_prevState == MemoryMonitor::MemCritical)
		break; // improving
	    handleVeryLowMemory();
	    break;
        case MemoryMonitor::MemLow:
	    qLog(OOM) << "Memory low";
	    if (m_prevState == MemoryMonitor::MemCritical)
		break; // improving
	    if (m_prevState == MemoryMonitor::MemVeryLow)
		break; // improving
	    handleLowMemory();
	    break;
        case MemoryMonitor::MemNormal:
	    qLog(OOM) << "Memory normal";
	    break;
    };
}

/*!
  This function handles the critically low memory state, the
  most dangerous of the three low memory states. In this state,
  one process is killed with a kill signal. The process that is
  killed is selected by selectProcess().
  \sa selectProcess()
 */
void
LowMemoryTask::handleCriticalMemory()
{
    kill(selectProcess());
}

/*!
  This function handles the very low memory state, which is
  more dangerous than the low memory state but not as bad as
  the critical memory state. In this state, one process is
  told to quit unconditionally but gracefully. The process
  that is told to quit is selected by selectProcess().
  \sa selectProcess()
 */
void
LowMemoryTask::handleVeryLowMemory()
{
    quit(selectProcess());
}

/*!
  This function handles the low memory state, which is the
  least dangerous of the three low memory states. It tells
  all the lazy shutdown processes that have been shutdown
  to quit.

  Note that only lazy shutdown processes marked as being
  either expendable or important are afected, which means
  that this function might not do anything at all. So if
  there are no lazy shutdown processes that can quit, the
  low memory state will persist.

  \sa handleVeryLowMemory(), handleCriticalMemory()
 */
void
LowMemoryTask::handleLowMemory()
{
    int count = quitIfInvisible(m_oomManager.expendableProcs());
    count += quitIfInvisible(m_oomManager.importantProcs());
    if (!count)
	qLog(OOM) << "No lazy shutdown procs to quit";
}

/*!
  \fn void LowMemoryTask::showWarning(const QString &title, const QString &body)

  Emitted whenever the LowMemoryTask requires the UI to display a message with
  specified \a title and \a body to the user.
 */

/*!
  Sends a \l {QCop} {message} to the application process
  identified by \a proc telling it to quit gracefully,
  regardless of whether it is invisible or not, ie quit
  unconditionally.

  If \a proc is empty, false is returned indicating we did
  not tell a process to quit. If true is returned, it means
  we told a process to quit.

  This is not the same as sending a kill signal, which
  would cause the process to die without shutting itself
  down.

  This function emits a showWarning() signal with the texts
  required for raising a message box to warn the user that a
  process has been terminated.
 */
bool
LowMemoryTask::quit(const QString& proc)
{
    if (proc.isEmpty()) {
	qLog(OOM) << "No proc to quit";
	return false;
    }
    qLog(OOM) << "Quitting" << proc;
    QtopiaIpcEnvelope e(QByteArray("QPE/Application/") +
			(const char*)proc.toLocal8Bit(),
			"quit()");
    QString title("Very Low Memory");
    QString text = QString("%1 quit to recover memory").arg(proc);
    emit showWarning(title,text);
    return true;
}

/*!
  Sends a kill signal to the application process identified
  by \a proc killing the process immediately at the system
  level with no chance for it to die gracefully.

  If \a proc is empty, false is returned indicating we did
  not kill a process. If true is returned, it means we did
  kill a process.

  This function emits a showWarning() signal with the texts
  required for raising a message box to warn the user that a
  process has been terminated.
 */
bool
LowMemoryTask::kill(const QString& proc)
{
    if (proc.isEmpty()) {
	qLog(OOM) << "No proc to kill";
	return false;
    }
    ApplicationLauncher* al = qtopiaTask<ApplicationLauncher>();
    if (al) {
	qLog(OOM) << "LowMemoryTask::kill():" << proc;
	al->kill(proc);
	QString title("Very Low Memory");
	QString text =
	    QString("%1 killed to recover memory").arg(proc);
	emit showWarning(title,text);
    }
#if 0
    QtopiaIpcEnvelope e(QByteArray("QPE/Application/") +
			(const char*)proc.toLocal8Bit(),
			"quit()");
#endif
    return true;
}

/*!
  Returns the name of a running application. The application
  is selected by the following algorithm.

  1. If the active process is expendable, return it.

  2. If there are expendable applications running, return the
  one with the highest OOM score as computed by the Linux kernel.

  3. If the active process is marked important, return it.

  4. If there are applications running that are marked important,
  return the one with the highest OOM score as computed by the
  Linux kernel.

  5. Return the empty string.

  The selected application will either be told to quit, or
  it will be killed outright.
 */
QString LowMemoryTask::selectProcess()
{
    QString app = WindowManagement::activeAppName();
    if (m_oomManager.isExpendable(app)) {
	return app;
    }
    app = m_oomManager.procWithBiggestScore(OomManager::Expendable);
    if (!app.isEmpty()) {
	return app;
    }
    if (m_oomManager.isImportant(app)) {
	return app;
    }
    app = m_oomManager.procWithBiggestScore(OomManager::Important);
    if (app.isEmpty())
	qLog(OOM) << "No non-critical procs to kill";
    return app;
}

/*!
  This function iterates through the \a map telling each
  valid, non-preloaded process to quit if it has no windows
  currently displayed. Processes that have no windows to
  display are lazy shutdown processes that are no longer
  running. They have been kept loaded on the assumption that
  they will be run again later and can be restarted more
  quickly if they are kept loaded in memory. Telling these
  lazy shutdown processes to die should free up memory.

  This function also sends the RecoverMemory() message to
  the \c {QPE/System} channel. Applications may use this as
  a trigger to recover memory or to free caches.
 */
int
LowMemoryTask::quitIfInvisible(const QMap<QString,int>& map)
{
    if (map.isEmpty())
	return 0;
    int count = 0;
    QContentFilter filter(QContent::Application);
    QContentSet set(filter);
    QMap<QString,int>::const_iterator i;
    for (i=map.constBegin(); i!=map.constEnd(); ++i) {
	QContent app = set.findExecutable(i.key());
	if (!app.isValid())
	    continue;
	if (app.isPreloaded())
	    continue;
	QString vsi_path =
	    QString("/System/Applications/%1/Tasks/UI").arg(i.key());
	QValueSpaceItem vsi(vsi_path);
	QVariant v = vsi.value();
	if (v.isValid()) {
	    if (v.toBool() == false) {
		qLog(OOM) << "Quit lazy shutdown app:"
			  << i.key() << i.value(); 
		QtopiaIpcEnvelope e(QByteArray("QPE/Application/") + i.key(),
				    "quitIfInvisible()");
		++count;
	    }
	}
    }
    QtopiaIpcEnvelope e("QPE/System", "RecoverMemory()");
    return count;
}

QTOPIA_TASK(LowMemoryTask, LowMemoryTask);
QTOPIA_TASK_PROVIDES(LowMemoryTask, LowMemoryTask);
