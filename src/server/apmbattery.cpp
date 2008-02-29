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

#include "apmbattery.h"
#include <QPowerSourceProvider>
#include "qtopiaserverapplication.h"

/*!
  \class APMBattery
  \ingroup QtopiaServer::Task
  \brief The APMBattery class provides a QPowerSource mapping to the Linux APM
  interface.

  The APMBattery provides a Qtopia Server Task.  Qtopia Server Tasks are
  documented in full in the QtopiaServerApplication class documentation.

  \table
  \row \o Task Name \o APMBattery
  \row \o Interfaces \o None
  \row \o Services \o None
  \endtable

  The APMBattery task creates three QPowerSource providers, named \c PrimaryAC,
  \c APMBattery and \c BackupAC that map to the primary ac, battery and backup
  ac information respectively provided by the Linux APM \c {/proc/apm} file 
  system interface.

  The APMBattery task also exports the following value space items:
  \table
  \header \o Item \o Description
  \row \o \c {/ServerTasks/APMBattery/APMAvailable} \o Set to true if APM is 
  available on the target system, and false otherwise.
  \endtable

  This task is only necessary on devices that use the Linux APM interface as 
  their only source of battery and wall power information.  As the APM interface
  is not as rich as Qtopia's, some QPowerSource fields will be unavailable or
  incomplete.

  Information is polled from \c {/proc/apm} once a minute.
  
  This class is part of the Qtopia server and cannot be used by other Qtopia applications.
*/

/*! \internal */
APMBattery::APMBattery(QObject *parent)
: QObject(parent), ac(0), battery(0), backup(0)
{
    bool apm = APMEnabled();

    QtopiaServerApplication::taskValueSpaceSetAttribute("APMBattery", 
                                                        "APMAvailable", apm);

    if(!apm) return;

    ac = new QPowerSourceProvider(QPowerSource::Wall, "PrimaryAC", this);
    battery = new QPowerSourceProvider(QPowerSource::Battery, "APMBattery", this);
    backup = new QPowerSourceProvider(QPowerSource::Wall, "BackupAC", this);

    startTimer(60 * 1000);
    updateStatus();
}

/*! \internal */
void APMBattery::updateStatus()
{
    bool ok = false;

    int acs = 0xff; // AC status
    int bs = 0xff; // Battery status
    int bf = 0xff; // Battery flag 
    int pc = -1; // Remaining battery (percentage)
    int min = -1; // Remaining battery (minutes)

    FILE *f = fopen("/proc/apm", "r");

    if ( f  ) {
        //I 1.13 1.2 0x02 0x00 0xff 0xff 49% 147 sec
        char u;
        fscanf(f, "%*[^ ] %*d.%*d 0x%*x 0x%x 0x%x 0x%x %d%% %i %c",
                &acs, &bs, &bf, &pc, &min, &u);
        fclose(f);
        qLog(PowerManagement) << acs << bs << bf << pc << min << u;

        // Wall sources
        if(0x00 == acs) {
            // Off line
            ac->setAvailability(QPowerSource::NotAvailable);
            backup->setAvailability(QPowerSource::NotAvailable);
        } else if(0x01 == acs) {
            ac->setAvailability(QPowerSource::Available);
            backup->setAvailability(QPowerSource::NotAvailable);
        } else if(0x02 == acs) {
            ac->setAvailability(QPowerSource::NotAvailable);
            backup->setAvailability(QPowerSource::Available);
        } else {
            ac->setAvailability(QPowerSource::Failed);
            backup->setAvailability(QPowerSource::Failed);
        }

        // Battery source

        if(bf == 0x80) {
            battery->setAvailability(QPowerSource::NotAvailable);
        } else {
            battery->setAvailability(QPowerSource::Available);
            switch ( u ) {
                case 'm': break;
                case 's': min /= 60; break; // ok
                default: min = -1; // unknown
            }
			bool charging = false;
            if(pc != -1) {
                battery->setCharge(pc);
            } else if(bf & 0x01) {
                // High
				charging = true;
                battery->setCharge(100);
            } else if(bf & 0x02) {
                // Low
				charging = true;
                battery->setCharge(25);
            } else if(bf & 0x04) {
                // Critical
				charging = true;
                battery->setCharge(5);
            } else if(bf & 0x08) {
                // Charging
                battery->setCharge(5);
				charging = true;
            } else if(bf & 0xFF) {
                // Critical/Unknown
                battery->setCharge(5);
				charging = true;
            } else {
                battery->setCharge(-1);
            }
            battery->setCharging( charging);
            battery->setTimeRemaining(min);
        }
    } else {
        ac->setAvailability(QPowerSource::Failed);
        battery->setAvailability(QPowerSource::Failed);
        backup->setAvailability(QPowerSource::Failed);
		qLog(PowerManagement)<<"APM QPowerSource Failed";
		
    }
}

/*! \internal */
void APMBattery::timerEvent(QTimerEvent *)
{
    updateStatus();
}

/*! \internal */
bool APMBattery::APMEnabled() const
{
    int apm_install_flags;
    FILE *f = fopen("/proc/apm", "r");
    if ( f ) {
        //I 1.13 1.2 0x02 0x00 0xff 0xff 49% 147 sec
        fscanf(f, "%*[^ ] %*d.%*d 0x%x 0x%*x 0x%*x 0x%*x %*d%% %*i %*c",
                &apm_install_flags);
        fclose(f);

        if (!(apm_install_flags & 0x08)) //!APM_BIOS_DISABLED
        {
			qLog(PowerManagement)<<"APM Enabled";
            return true;
        }
    }
	qLog(PowerManagement)<<"APM Disabled";
    return false;
}

QTOPIA_TASK(APMBattery, APMBattery);
