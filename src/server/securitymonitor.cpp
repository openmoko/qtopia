/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "securitymonitor.h"
#include <QTimer>

SecurityMonitorTask::SecurityMonitorTask()
{
#ifndef QT_NO_SXE
    // No point starting in less than 10secs - the server will take at least
    // that long to startup.
    QTimer::singleShot(10000, this, SLOT(startNewSxeMonitor()));
#endif
}

bool SecurityMonitorTask::systemRestart()
{
#ifndef QT_NO_SXE
    doShutdown();
    return false;
#else
    return true;
#endif
}

bool SecurityMonitorTask::systemShutdown()
{
#ifndef QT_NO_SXE
    doShutdown();
    return false;
#else
    return true;
#endif
}

void SecurityMonitorTask::doShutdown()
{
}

void SecurityMonitorTask::finished()
{
}

#ifndef QT_NO_SXE
void SecurityMonitorTask::startNewSxeMonitor()
{
    m_sxeMonitorProcess = new QProcess(this);
    m_sxeMonitorProcess->setProcessChannelMode(QProcess::ForwardedChannels);
    m_sxeMonitorProcess->closeWriteChannel();
    connect(m_sxeMonitorProcess, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(sxeMonitorProcessError(QProcess::ProcessError)));
    connect(m_sxeMonitorProcess, SIGNAL(finished(int)),
            this, SLOT(sxeMonitorProcessExited(int)));
    m_sxeMonitorProcess->start(sxemonitorExecutable());
}

QString SecurityMonitorTask::sxemonitorExecutable()
{
    return Qtopia::qtopiaDir() + "bin/sxemonitor";
}

void SecurityMonitorTask::sxeMonitorProcessError(QProcess::ProcessError e)
{
    switch(e)
    {
        case QProcess::FailedToStart:   qWarning() << "SxeMonitor Process failed to start"; break;
        case QProcess::Crashed:         qWarning() << "SxeMonitor Process crashed"; break;
        case QProcess::WriteError:      qWarning() << "SxeMonitor Process Write Error"; break;
        case QProcess::ReadError:       qWarning() << "SxeMonitor Process Read Error"; break;
        case QProcess::UnknownError:    qWarning() << "SxeMonitor Process Unknown error"; break;
        default:                        qWarning() << "SxeMonitor Proces error not known";
    }
    m_sxeMonitorProcess->disconnect();
    m_sxeMonitorProcess->deleteLater();
    startNewSxeMonitor();

}

void SecurityMonitorTask::sxeMonitorProcessExited( int e )
{
    qWarning() << "SxeMonitor Process has prematurely exited, exit code: " << e;

    m_sxeMonitorProcess->disconnect();
    m_sxeMonitorProcess->deleteLater();
    startNewSxeMonitor();
}

#endif

QTOPIA_TASK(SecurityMonitor, SecurityMonitorTask);
QTOPIA_TASK_PROVIDES(SecurityMonitor, SystemShutdownHandler);

