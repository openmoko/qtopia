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

#include <QProcess>
#include <Qtopia>
#include <QTimer>
#include <qtopialog.h>
#include <qcopchannel_qws.h>

#include "mediaserver.h"

MediaServerControlTask::MediaServerControlTask()
: m_soundserver(0), m_qssTimerId(-1), m_shutdown(false),
  m_shutdownCompleted(false)
{
#ifdef MEDIA_SERVER
    QCopChannel::send( "QPE/Application/mediaserver", "execute()" );
#else
    // Gives system 5 seconds to start up (on PDA it needs that much time)
    m_qssTimerId = startTimer(5000);
#endif
}

void MediaServerControlTask::timerEvent(QTimerEvent *e)
{
    if ( e->timerId() == m_qssTimerId ) {
        if(!m_soundserver) {
            m_soundserver = new QProcess(this);
            connect(m_soundserver, SIGNAL(finished(int)),
                    this, SLOT(soundServerExited()));
            connect(m_soundserver, SIGNAL(readyReadStandardOutput()),
                    this, SLOT(soundServerReadyStdout()));
            connect(m_soundserver, SIGNAL(readyReadStandardError()),
                    this, SLOT(soundServerReadyStderr()));
        }
        m_soundserver->start(Qtopia::qtopiaDir() + MEDIA_SERVER_PATH);

        killTimer(m_qssTimerId);
        m_qssTimerId = 0;
    }
    QObject::timerEvent(e);
}

void MediaServerControlTask::soundServerExited()
{
    if (!m_qssTimerId && !m_shutdown)
        m_qssTimerId = startTimer(5000);

    Q_ASSERT(m_soundserver);
    m_soundserver->disconnect();
    m_soundserver->deleteLater();
    m_soundserver = 0;

    if(m_shutdown) {
        m_shutdownCompleted = true;
        emit proceed();
    }
}

void MediaServerControlTask::soundServerReadyStdout()
{
    m_soundserver->setReadChannel(QProcess::StandardOutput);
    while (m_soundserver->canReadLine())
    {
        QByteArray  line = m_soundserver->readLine();

        line.chop(int(line.endsWith("\r\n")) + int(line.endsWith('\n')));
        //qDebug() << "SS:stdout:" << line;
    }
}

void MediaServerControlTask::soundServerReadyStderr()
{
    m_soundserver->setReadChannel(QProcess::StandardError);
    while (m_soundserver->canReadLine())
    {
        QByteArray  line = m_soundserver->readLine();

        line.chop(int(line.endsWith("\r\n")) + int(line.endsWith('\n')));
        //qDebug() << "SS:stderr:" << line;
    }
}

bool MediaServerControlTask::systemRestart()
{
    return doShutdown();
}

bool MediaServerControlTask::systemShutdown()
{
    return doShutdown();
}

void MediaServerControlTask::killtimeout()
{
    Q_ASSERT(m_shutdown);
    if(m_shutdownCompleted)
        return;
    qLog(Media) << "Sound server process did not terminate during shutdown.";
    m_soundserver->disconnect();
    m_soundserver->deleteLater();
    m_soundserver = 0;
    m_shutdownCompleted = true;
    emit proceed();
}

bool MediaServerControlTask::doShutdown()
{
    Q_ASSERT(!m_shutdown);
    m_shutdown = true;

    if(m_soundserver) {
        m_soundserver->kill();
        QTimer::singleShot(1000, this, SLOT(killtimeout()));
        return false;
    } else {
        m_shutdownCompleted = true;
        return true;
    }
}

QTOPIA_TASK(MediaServer, MediaServerControlTask);
QTOPIA_TASK_PROVIDES(MediaServer, SystemShutdownHandler);


