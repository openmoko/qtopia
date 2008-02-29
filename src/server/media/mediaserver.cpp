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

#include <QProcess>
#include <QTimer>
#include <Qtopia>
#include <qtopialog.h>


#include "mediaserver.h"


class MediaServerControlTaskPrivate
{
public:
    QProcess*               soundserver;
    int                     qssTimerId;
    bool                    shutdown;
    bool                    shutdownCompleted;
    ApplicationLauncher*    appLauncher;
};


/*!
    \class MediaServerControlTask
    \ingroup QtopiaServer::Task

    \brief The MediaServerControlTask class provides a launcher for the Media Server.

    This task launches QSS or Qtopia's Media Server process and maintains this
    process throughout the life of Qtopia. If the media process exists
    prematurely it will be restarted. QSS is launched using QProcess and
    Qtopia's mediaserver is launched by sending an application channel message
    to the mediaserver.

    This class is part of the Qtopia server and cannot be used by other Qtopia
    applications.
*/


/*!
    \internal
    Construct a task to launch the configured mediaserver.
*/

#define MEDIASERVER_IMAGE "mediaserver"

MediaServerControlTask::MediaServerControlTask():
    d(new MediaServerControlTaskPrivate)
{
    d->soundserver = 0;
    d->qssTimerId = 0;
    d->shutdown = false;
    d->shutdownCompleted = false;

#ifdef MEDIA_SERVER
    d->appLauncher = qtopiaTask<ApplicationLauncher>();

    connect(d->appLauncher, SIGNAL(applicationTerminated(QString,ApplicationTypeLauncher::TerminationReason,bool)),
            this, SLOT(applicationTerminated(QString,ApplicationTypeLauncher::TerminationReason,bool)));

    d->appLauncher->launch(MEDIASERVER_IMAGE);
#else
    d->qssTimerId = startTimer(5000);
#endif
}

/*!
    \internal
*/

void MediaServerControlTask::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == d->qssTimerId)
    {
        if (!d->soundserver)
        {
            d->soundserver = new QProcess(this);
            d->soundserver->setProcessChannelMode(QProcess::ForwardedChannels);
            d->soundserver->closeWriteChannel();

            connect(d->soundserver, SIGNAL(finished(int)), this, SLOT(soundServerExited()));
        }

        d->soundserver->start(Qtopia::qtopiaDir() + "bin/mediaserver");

        killTimer(d->qssTimerId);
        d->qssTimerId = 0;
    }
}

void MediaServerControlTask::soundServerExited()
{
    if (!d->qssTimerId && !d->shutdown)
        d->qssTimerId = startTimer(5000);

    Q_ASSERT(d->soundserver);

    d->soundserver->disconnect();
    d->soundserver->deleteLater();
    d->soundserver = 0;

    if (d->shutdown)
    {
        d->shutdownCompleted = true;

        emit proceed();
    }
}


/*!
    \internal
*/

bool MediaServerControlTask::systemRestart()
{
    return doShutdown();
}

/*!
    \internal
*/

bool MediaServerControlTask::systemShutdown()
{
    return doShutdown();
}

void MediaServerControlTask::killtimeout()
{
    Q_ASSERT(d->shutdown);

    if (!d->shutdownCompleted)
    {
        qLog(Media) << "Sound server process did not terminate during shutdown.";

        d->soundserver->disconnect();
        d->soundserver->deleteLater();
        d->soundserver = 0;
        d->shutdownCompleted = true;

        emit proceed();
    }
}

void MediaServerControlTask::applicationTerminated(QString const& name, ApplicationTypeLauncher::TerminationReason, bool)
{
    if (name == MEDIASERVER_IMAGE)
    {
        if (d->shutdown)
            emit proceed();
        else
            d->appLauncher->launch(MEDIASERVER_IMAGE);
    }
}


bool MediaServerControlTask::doShutdown()
{
    Q_ASSERT(!d->shutdown);

    d->shutdown = true;

#ifdef MEDIA_SERVER
    d->appLauncher->kill(MEDIASERVER_IMAGE);
    return false;
#else
    if (d->soundserver)
    {
        d->soundserver->kill();
        QTimer::singleShot(1000, this, SLOT(killtimeout()));
        return false;
    }
    else
    {
        d->shutdownCompleted = true;
        return true;
    }
#endif
}

QTOPIA_TASK(MediaServer, MediaServerControlTask);
QTOPIA_TASK_PROVIDES(MediaServer, SystemShutdownHandler);


