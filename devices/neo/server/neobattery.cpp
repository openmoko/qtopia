/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include "neobattery.h"
#include "qtopiaserverapplication.h"

#include <QPowerSourceProvider>
#include <QTimer>
#include <QFileMonitor>

#include <math.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <QValueSpaceItem>
#include <QtopiaIpcEnvelope>


// /sys/devices/platform/bq27000-battery.0/power_supply/bat

NeoBattery::NeoBattery(QObject *parent)
: QObject(parent),
  percentCharge(0),
  charging(0),
  cableEnabled(0),
  ac(0),
  battery(0),
  vsUsbCable(0)
{
    bool apm = APMEnabled();
    if(!apm) return;
    qWarning()<<"NeoBattery::NeoBattery";

    QtopiaServerApplication::taskValueSpaceSetAttribute("NeoBattery",
                                                        "APMAvailable", apm);

    ac = new QPowerSourceProvider(QPowerSource::Wall, "PrimaryAC", this);
    battery = new QPowerSourceProvider(QPowerSource::Battery, "NeoBattery", this);


    vsUsbCable = new QValueSpaceItem("/Hardware/UsbGadget/cableConnected", this);
    connect( vsUsbCable, SIGNAL(contentsChanged()), SLOT(cableChanged()));

    checkChargeState();

    startTimer(60 * 1000);

    QTimer::singleShot( 10 * 1000, this, SLOT(updateApmStatus()));
}

/*! \internal */
bool NeoBattery::APMEnabled() const
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
            qLog(PowerManagement)<<"Neo APM Enabled";
            return true;
        }
    }
    qLog(PowerManagement)<<"Neo APM Disabled";
    return false;
}

void NeoBattery::apmFileChanged(const QString &/* file*/)
{
    updateApmStatus();
}

void NeoBattery::updateApmStatus()
{
    qLog(PowerManagement) << __PRETTY_FUNCTION__;

    int acs = 0xff; // AC status
    int bs = 0xff; // Battery status
    int bf = 0xff; // Battery flag
    int pc = -1; // Remaining battery (percentage)
    int min = -1; // Remaining battery (minutes)

    charging = isCharging();

    FILE *f = fopen("/proc/apm", "r");

    if (f) {
        //I 1.13 1.2 0x02 0x00 0xff 0xff 49% 147 sec
        char u;
        fscanf(f, "%*[^ ] %*d.%*d 0x%*x 0x%x 0x%x 0x%x %d%% %i %c",
               &acs, &bs, &bf, &pc, &min, &u);
        fclose(f);
        qLog(PowerManagement) << "Neo APM:"<<acs << bs << bf << pc << min << u;

        battery->setCharge(-1);
        if(bf != 0x80) {
            switch ( u ) {
            case 'm': break;
            case 's': min /= 60; break; // ok
            default: min = -1; // unknown
            }

            if(pc != -1) {
                battery->setCharge(pc);
                qLog(PowerManagement) << "setCharge" << pc;
            }
            if (charging) {
                qLog(PowerManagement) << "charging";
                battery->setCharge(-1);
                ac->setAvailability(QPowerSource::Available);
                battery->setAvailability(QPowerSource::NotAvailable);
            } else {
                qLog(PowerManagement) << "not charging";
                ac->setAvailability(QPowerSource::NotAvailable);
                battery->setAvailability(QPowerSource::Available);
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
void NeoBattery::timerEvent(QTimerEvent *)
{
    updateApmStatus();
}

bool NeoBattery::isCharging()
{
    QString inStr;
    if ( QFileInfo("/sys/devices/platform/bq27000-battery.0/power_supply/bat/status").exists()) {
        inStr = "/sys/devices/platform/bq27000-battery.0/power_supply/bat/status";
    } else {
        return cableEnabled;
    }

    QFile battstatus( inStr);
    battstatus.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&battstatus);
    in >> inStr;
    battstatus.close();
    qLog(PowerManagement)<<"status"<< inStr;

    if (inStr.contains("Charging")) {
        return true;
    }
     return false;
}

/*! \internal */
void NeoBattery::cableChanged()
{
    bool ch = vsUsbCable->value().toBool();
    qLog(PowerManagement) << __PRETTY_FUNCTION__ << ch;
    cableEnabled = ch;
    charging = ch;

    QTimer::singleShot( 1000, this, SLOT(updateApmStatus()));
}
/*
/sys/devices/platform/bq27000-battery.0/power_supply/bat/voltage_now
/sys/devices/platform/bq27000-battery.0/power_supply/bat/time_to_empty_now

*/

// /sys/devices/platform/bq27000-battery.0/power_supply/bat/status Charging
void NeoBattery::checkChargeState()
{
    qLog(PowerManagement) << __PRETTY_FUNCTION__;
    QString chgState;
    if (QFileInfo("/sys/devices/platform/s3c2440-i2c/i2c-adapter/i2c-0/0-0073/chgstate").exists()) {
         //freerunner
        chgState = "/sys/devices/platform/s3c2440-i2c/i2c-adapter/i2c-0/0-0073/chgstate";
    } else {
        //1973
        chgState = "/sys/devices/platform/s3c2410-i2c/i2c-adapter/i2c-0/0-0008/chgstate";
    }
    QString inStr;
    QFile chgstate( chgState);
    chgstate.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&chgstate);
    in >> inStr;
    chgstate.close();
    qLog(PowerManagement) << inStr;

    if (inStr.contains("enabled")) {
        cableEnabled = true;
    } else {
        cableEnabled = false;
    }

    charging = cableEnabled;
    cableEnabled = cableEnabled;
    battery->setCharging(cableEnabled);

    QtopiaIpcEnvelope e2("QPE/Neo1973Hardware", "cableConnected(bool)");
    e2 << cableEnabled;
    updateApmStatus();
}

QTOPIA_TASK(NeoBattery, NeoBattery);
