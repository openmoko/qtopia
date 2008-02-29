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

#include "alertservicetask.h"
#include <QSoundControl>
#include "qtopiaserverapplication.h"

/*!
    \service AlertService Alert
    \brief Provides the Qtopia Alert service.

    The \i Alert service enables applications to sound the audible
    system alert.  Normally an application will use Qtopia::soundAlarm()
    for this.

    The \i Alert service is typically supplied by the AlertServiceTask task in
    the Qtopia server, but the system integrator may change the provider
    of this service if the device has special hardware (e.g. a buzzer or
    vibration system) that can alert the user.

    \sa Qtopia::soundAlarm()
*/

/*!
    \internal
*/
AlertService::AlertService( QObject *parent )
        : QtopiaAbstractService( "Alert", parent )
{
    publishAll();
}

/*!
    \internal
*/
AlertService::~AlertService()
{
}

/*!
    \fn AlertService::soundAlert()

    Sounds the audible system alert. This is used for applications such
    as Calendar when it needs to inform the user of an event.

    This slot corresponds to the QCop service message \c{Alert::soundAlert()}.
*/

/*!
  \class AlertServiceTask
  \ingroup QtopiaServer::Task
  \brief The AlertServiceTask class provides a WAV file implementation of the Alert service.

  On reception of the \c {Alert::soundAlert()} service message, the
  AlertServiceTask class will play the \c {:sound/alarm} system sound.

  The AlertServiceTask class provides the \c {AlertService} task.

  \sa AlertService
 */

/*! \internal */
AlertServiceTask::AlertServiceTask()
: AlertService(0)
{
}

/*! \internal */
void AlertServiceTask::soundAlert()
{
    QSoundControl *soundcontrol =
        new QSoundControl(new QSound(":sound/alarm"));
    soundcontrol->sound()->play();

    QObject::connect(soundcontrol, SIGNAL(done()),
                        this, SLOT(playDone()));
}

void AlertServiceTask::playDone()
{
    QSoundControl* soundControl = qobject_cast<QSoundControl*>(sender());

    soundControl->sound()->deleteLater();
    soundControl->deleteLater();
}

QTOPIA_TASK(AlertService, AlertServiceTask);
