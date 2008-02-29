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

#ifndef QT_NO_SXE
#include <monitor.h>
#endif

#ifndef QT_NO_SXE
#ifdef QTOPIA_CELL
#include "qsxemangle.h"
#endif
#endif

SecurityMonitorTask::SecurityMonitorTask()
: m_monitor(0), m_finished(false)
{
#ifndef QT_NO_SXE
    m_monitor = new SecurityMonitor( this );

#ifdef QTOPIA_PHONE
#ifdef QTOPIA_CELL
    IdMangler *idmangler = new IdMangler( this );
    connect(idmangler, SIGNAL(gotIds(const QString &)),
            m_monitor, SLOT(addMangleSeed(const QString &)));
#endif
#endif

    m_monitor->start();
    startNewSxeMonitor();
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
#ifndef QT_NO_SXE
    QObject::connect(m_monitor, SIGNAL(finished()), this, SLOT(finished()));
    QTimer::singleShot(500, this, SLOT(finished()));
    m_monitor->quit();
#endif
}

void SecurityMonitorTask::finished()
{
    if(m_finished) return;

    m_finished = true;
    emit proceed();
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

