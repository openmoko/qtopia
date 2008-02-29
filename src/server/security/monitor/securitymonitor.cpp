/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#include "securitymonitor.h"
#include <QTimer>

/*!
    \class SecurityMonitorTask
    \mainclass
    \ingroup QtopiaServer::Task
    \brief The SecurityMonitorTask class manages the lifetime of the SXE monitor process.

    The SXE monitor process responds to security policy breaches.

    The SecurityMonitorTask class provides the \c {SecurityMonitor} task.
    It is part of the Qtopia server and cannot be used by other Qtopia applications.
*/    
    
/*!
    \internal
    Construct a new SecurityMonitorTask and launch the SXE monitor process
*/ 
SecurityMonitorTask::SecurityMonitorTask()
{
#ifndef QT_NO_SXE
    isShutdown = false;
    QTimer::singleShot(0, this, SLOT(init()));
    QTimer::singleShot(0, this, SLOT(startNewSxeMonitor()));
#endif
}

void SecurityMonitorTask::init()
{
#ifndef QT_NO_SXE
    //we need to create the qcop-msg files in the temp directory for 
    // all the package binaries and reload the lids configuration
    //otherwise packages won't have permissions to access these files
    QDir packageBin( Qtopia::packagePath() + "/bin" );
    QFileInfoList files = packageBin.entryInfoList(QDir::Files | QDir::System );
    foreach ( QFileInfo fi, files )
    {
        if ( fi.isSymLink() )
        {
            QFile file( Qtopia::tempDir() + "qcop-msg-" + fi.fileName());
            file.open(QIODevice::WriteOnly );
        }
    }
    if ( QFile::exists("/proc/sys/lids/locks") )
        QProcess::startDetached( Qtopia::qtopiaDir() + "/etc/sxe_qtopia/sxe_reloadconf" );
#endif
}


/*!
    \reimp
    Handle cleanup tasks in the event of system restarting.

    Returns true if all cleanup tasks were successful, otherwise return false.
*/
bool SecurityMonitorTask::systemRestart()
{
#ifndef QT_NO_SXE
    doShutdown();
    return true;
#else
    return true;
#endif
}

/*!
    \reimp
    Handle cleanup tasks in the event of system shutdown.

    Returns true if all cleanup tasks were successful, otherwise return false. 
*/
bool SecurityMonitorTask::systemShutdown()
{
#ifndef QT_NO_SXE
    doShutdown();
    return true;
#else
    return true;
#endif
}

void SecurityMonitorTask::doShutdown()
{
    isShutdown = true;
}

void SecurityMonitorTask::finished()
{
}

#ifndef QT_NO_SXE
void SecurityMonitorTask::startNewSxeMonitor()
{
    if ( isShutdown ) return;
    m_sxeMonitorProcess = new QProcess(this);
    m_sxeMonitorProcess->setProcessChannelMode(QProcess::ForwardedChannels);
    m_sxeMonitorProcess->closeWriteChannel();
    connect(m_sxeMonitorProcess, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(sxeMonitorProcessError(QProcess::ProcessError)));
    connect(m_sxeMonitorProcess, SIGNAL(finished(int)),
            this, SLOT(sxeMonitorProcessExited(int)));
    qobject_cast<QtopiaApplication *>(qApp)->processEvents();
    m_sxeMonitorProcess->start(sxemonitorExecutable());
    qobject_cast<QtopiaApplication *>(qApp)->processEvents();
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
    if ( isShutdown )
        return;

    static int retry=0;
    retry++;

    //try restart sxemonitor if it goes down however
    //on the third retry wait 30mins, so as to not lock up the phone
    //with continual sxemonitor restarts.  Sxemonitor however should
    //never go down.
    if ( retry == 3)
    {
        QTimer::singleShot(1800000, this, SLOT(startNewSxeMonitor()));
        retry=0;
        qWarning( "Restarting of sxemonitor daemon has failed several times!!" );
    }
    else
        QTimer::singleShot( 0, this, SLOT(startNewSxeMonitor()));

}

void SecurityMonitorTask::sxeMonitorProcessExited( int e )
{
    if ( isShutdown )
        return;
    qWarning() << "SxeMonitor Process has prematurely exited, exit code: " << e;

    m_sxeMonitorProcess->disconnect();
    m_sxeMonitorProcess->deleteLater();
    QTimer::singleShot( 300000, this, SLOT(startNewSxeMonitor()));
}

#endif

QTOPIA_TASK(SecurityMonitor, SecurityMonitorTask);
QTOPIA_TASK_PROVIDES(SecurityMonitor, SystemShutdownHandler);

