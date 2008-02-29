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

#include "phonepowermanager.h"

#include <qtopialog.h>
#include <QList>
#include <QSettings>

#include <qtopiaipcenvelope.h>
#include <QtopiaServiceRequest>
#include <qwindowsystem_qws.h>

#include <qtopiaipcenvelope.h>
#include "qtopiaserverapplication.h"

/*!
  \class PhonePowerManager
  \ingroup QtopiaServer::Task
  \brief The PhonePowerManager class implements phone specific power management
  functionality in Qtopia.

  This manager uses three levels for power management. The first level dims
  the background light, the second level turns the light off and the last level
  returns the phone to the homescreen.

  The PhonePowerManager class provides the \c {PhonePowerManager} task.
  \sa QtopiaPowerManager
*/

/*!
  Constructs a new PhonePowerManager instance.
  */
PhonePowerManager::PhonePowerManager() : 
    QtopiaPowerManager(), showhomescreen_on(false), suspend_on(true)
{
    setDefaultIntervals();
}

/*!
  Destroys the PhonePowerManager instance
*/
PhonePowerManager::~PhonePowerManager() 
{
}

/*!
    This function activates the appropriate actions for the given
    screensaver \a level.
  */
bool PhonePowerManager::save(int level)
{
    int action = m_levelToAction.value(level);
    switch ( action) {
        case PhonePowerManager::DimLight:
            if ( m_powerConstraint > QtopiaApplication::Disable && m_dimLightEnabled ) {
                if (backlight() > 1)
                    setBacklight(1); // lowest non-off
                qLog(PowerManagement) << "Dimming light";
            }
            return true;
            break;
        case PhonePowerManager::LightOff:
            if ( m_powerConstraint > QtopiaApplication::DisableLightOff
                    && m_lightOffEnabled ) {
                setBacklight(0); // off
                qLog(PowerManagement) << "turning light off";
            }
            return true;
            break;
        case PhonePowerManager::HomeScreen:
            if (m_powerConstraint > QtopiaApplication::DisableReturnToHomeScreen
                    && showhomescreen_on) {
                qLog(PowerManagement) << "show HomeScreen";

                QSettings c("Trolltech","qpe");
                c.beginGroup("HomeScreen");
                if (c.value( "AutoKeyLock", "Disabled" ).toString() == "Enabled")
                    QtopiaIpcEnvelope showHome( "QPE/System", "showHomeScreenAndKeylock()" );
                else
                    QtopiaIpcEnvelope showHome( "QPE/System", "showHomeScreen()" );
            }
            return true;
            break;
        case PhonePowerManager::Suspend:
            if (m_powerConstraint > QtopiaApplication::DisableSuspend
                    && suspend_on) {
                QtopiaServiceRequest r("Suspend", "suspend()");
                r.send();
            }
            return true;
            break;
        default:
            ;
    }
    return false;
}


/*! 
    This function sets the internal screensaver timeouts
    to the values passed in \a ivals. \a size determines the number of entries in 
    \a ivals.


    The phone screensaver maps the timeouts to the following actions:
        \list
        \o 0 -> dim light
        \o 1 -> turn off light
        \o 3 -> show HomeScreen
        \endlist

     */
void PhonePowerManager::setIntervals(int* ivals, int size )
{
    QSettings config("Trolltech","qpe");

    QPowerStatus ps = QPowerStatusManager::readStatus();
    QString powerGroup = (ps.acStatus() == QPowerStatus::Online) ? "ExternalPower" : "BatteryPower";
    config.beginGroup( powerGroup );

    int *v = new int[size+1];
    for(int j=size; j>=0; j--)
        v[j]=0;

    m_levelToAction.clear();
    QMap<int,int> timeToAction;

    switch (size) {
        default:
        case 4:
            ivals[3] = interval(ivals[3], config, "Suspend","Interval", 60); // No tr
            v[3] = qMax( 1000*ivals[3] + 100, 100);
            timeToAction.insert(v[3], PhonePowerManager::Suspend);
        case 3:
            config.endGroup();
            config.beginGroup( "HomeScreen" );
            ivals[2] = interval(ivals[2], config, "ShowHomeScreen","Interval_HomeScreen", 300); // No tr
            v[2] = qMax( 1000*ivals[2], 100);
            if (timeToAction.contains(v[2]))
                v[2] = v[2]+100; //add few ms for next timeout
            timeToAction.insert(v[2], PhonePowerManager::HomeScreen);
            config.endGroup();
            config.beginGroup( powerGroup );
        case 2:
            ivals[1] = interval(ivals[1], config, "LightOff","Interval_LightOff", 20);
            v[1] = qMax( 1000*ivals[1], 100);
            if (timeToAction.contains(v[1]))
                v[1] = v[1]+100; //add few ms for next timeout
            timeToAction.insert(v[1], PhonePowerManager::LightOff);
        case 1:
            ivals[0] = interval(ivals[0], config, "Dim","Interval_Dim", 30); // No tr
            v[0] = qMax( 1000*ivals[0], 100);
            while ( timeToAction.contains( v[0] ) )
                v[0] = v[0]+100; //add few ms for next timeout
            timeToAction.insert(v[0], PhonePowerManager::DimLight);
        case 0:
            break;
    }

    qLog(PowerManagement) << "PhonePowerManager::setIntervals:"
                          << ivals[0] << ivals[1] << ivals[2];

    m_dimLightEnabled = ( (ivals[0] != 0) ? config.value("Dim",true).toBool() : false );
    m_lightOffEnabled = ( (ivals[1] != 0 ) ? config.value("LightOff",true).toBool() : false );
    suspend_on = ( (ivals[3] != 0) ? config.value("Suspend", true).toBool(): false );
    config.endGroup();
    config.beginGroup( "HomeScreen" );
    showhomescreen_on = ( (ivals[2] != 0) ? config.value("ShowHomeScreen", true).toBool(): false );

    if ( !ivals[0] && !ivals[1] && !ivals[2] ){
        QWSServer::setScreenSaverInterval(0);
        delete [] v;
        return;
    }

    QList<int> keys = timeToAction.keys();
    qStableSort(keys.begin(), keys.end());

    //first element
    v[0] = keys.at(0);
    m_levelToAction.insert(0, timeToAction.value(v[0]));
    int sum = v[0];

    for (int j=1; j<keys.count(); j++)
    {
        v[j]=keys.at(j)-sum;
        sum+= v[j];
        m_levelToAction.insert(j, timeToAction.value(sum));
    }

    //FIXME: add evenBlocking
    //QWSServer::setScreenSaverIntervals(v, ScreenSaver::eventBlockLevel);
    QWSServer::setScreenSaverIntervals(v);
    delete [] v;
}

QTOPIA_TASK(PhonePowerManager, PhonePowerManager);
QTOPIA_TASK_PROVIDES(PhonePowerManager,PowerManagerTask)
