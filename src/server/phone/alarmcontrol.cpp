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

#include "alarmcontrol.h"
#include <QSettings>

/*!
  \class AlarmControl
  \ingroup QtopiaServer
  \brief The AlarmControl class maintains information about the daily alarm.
  
  This class is part of the Qtopia server and cannot be used by other Qtopia applications.
 */

/*!
  Returns the AlarmControl instance.
  */
AlarmControl *AlarmControl::instance()
{
    static AlarmControl * control = 0;
    if(!control)
        control = new AlarmControl;
    return control;
}

AlarmControl::AlarmControl()
: alarmOn(false), alarmValueSpace("/UI/DailyAlarm"),
  alarmChannel("QPE/AlarmServer")
{
    connect(&alarmChannel, SIGNAL(received(QString,QByteArray)),
            this, SLOT(alarmMessage(QString,QByteArray)));


    QSettings clockCfg("Trolltech","Clock");
    clockCfg.beginGroup( "Daily Alarm" );
    bool alarm = clockCfg.value("Enabled", false).toBool();
    alarmOn = !alarm;
    alarmEnabled(alarm);
}

void AlarmControl::alarmEnabled(bool on)
{
    if(on != alarmOn) {
        alarmOn = on;
        alarmValueSpace.setAttribute("", alarmOn);
        emit alarmStateChanged(alarmOn);
    }
}

/*!
  Returns true if the alarm is on, otherwise false.
  */
bool AlarmControl::alarmState() const
{
    return alarmOn;
}

/*!
  \fn void AlarmControl::alarmStateChanged(bool newState)

  Emitted whenever the alarm state changes to \a newState.
  */

void AlarmControl::alarmMessage(const QString& message, const QByteArray &data)
{
    if ( message == "dailyAlarmEnabled(bool)" ) {
        QDataStream stream( data );
        bool enabled;
        stream >> enabled;
        alarmEnabled(enabled);
    }
}

