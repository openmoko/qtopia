/****************************************************************************
**
** Copyright (C) 2007-2007 TROLLTECH ASA. All rights reserved.
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

#include "ficgta01battery.h"
#include "qtopiaserverapplication.h"

#include <QPowerSourceProvider>
#include <QTimer>
#include <QFileMonitor>

#include <math.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


struct Ficgta01Detect {
    unsigned int   dummy1;
    unsigned int   dummy2;
    unsigned short type;
    unsigned short code;
    unsigned int   value;
};

Ficgta01Battery::Ficgta01Battery(QObject *parent)
: QObject(parent), ac(0), battery(0), backup(0)
{
    bool apm = APMEnabled();
qWarning()<<"Ficgta01Battery::Ficgta01Battery";

    QtopiaServerApplication::taskValueSpaceSetAttribute("Ficgta01Battery",
                                                        "APMAvailable", apm);

    if(!apm) return;

    ac = new QPowerSourceProvider(QPowerSource::Wall, "PrimaryAC", this);
    battery = new QPowerSourceProvider(QPowerSource::Battery, "Ficgta01Battery", this);
    backup = new QPowerSourceProvider(QPowerSource::Wall, "BackupAC", this);

    QFileMonitor *fileMon;
    fileMon = new QFileMonitor( "/proc/apm", QFileMonitor::Auto, this);
    connect(fileMon, SIGNAL( fileChanged ( const QString & )),
            this,SLOT(apmFileChanged(const QString &)));

    startTimer(60 * 1000);
    openDetect();
    QTimer::singleShot( 10 * 1000, this, SLOT(updateFicStatus()));
}

void Ficgta01Battery::apmFileChanged(const QString & file)
{
    qWarning()<<"apmFileChanged(const QString & file)"<<file;
    updateFicStatus();
}

void Ficgta01Battery::openDetect()
{
    kbdFDpower = ::open("/dev/input/event2", O_RDONLY|O_NDELAY, 0);
    if (kbdFDpower >= 0) {
        powerNotify = new QSocketNotifier( kbdFDpower, QSocketNotifier::Read, this );
        connect( powerNotify, SIGNAL(activated(int)), this, SLOT(readPowerKbdData()));
    } else {
        qWarning("Cannot open /dev/input/event2 for keypad (%s)", strerror(errno));
        //     err = 1;
    }

//    if(err !=0)
//        return;

}

void Ficgta01Battery::updateFicStatus()
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
        qLog(PowerManagement) << "Fic"<<acs << bs << bf << pc << min << u;

        // Wall sources
        if(0x00 == acs) {
            // Off line
            ac->setAvailability(QPowerSource::NotAvailable);
            backup->setAvailability(QPowerSource::NotAvailable);
            qLog(PowerManagement)<<"ac not"<<"backup not";
        } else if(0x01 == acs) {
            ac->setAvailability(QPowerSource::Available);
            backup->setAvailability(QPowerSource::NotAvailable);
            qLog(PowerManagement)<<"ac avail"<<"backup not";
        } else if(0x02 == acs) {
            ac->setAvailability(QPowerSource::NotAvailable);
            backup->setAvailability(QPowerSource::Available);
            qLog(PowerManagement)<<"ac not"<<"backup avail";
        } else {
            ac->setAvailability(QPowerSource::Failed);
            backup->setAvailability(QPowerSource::Failed);
            qLog(PowerManagement)<<"ac failed"<<"backuip failed";
        }

        // Battery source
              battery->setCharge(-1);
        if(bf == 0x80) {
            battery->setAvailability(QPowerSource::NotAvailable);
             qLog(PowerManagement)<<"battery not";
        } else {
            battery->setAvailability(QPowerSource::Available);
             qLog(PowerManagement)<<"battery avail";
            switch ( u ) {
                case 'm': break;
                case 's': min /= 60; break; // ok
                default: min = -1; // unknown
            }
            bool charging = false;
            if(pc != -1) {
                battery->setCharge(pc);
                qLog(PowerManagement)<<"setCharge"<<pc;
            } else if(bf & 0x01) {
                qLog(PowerManagement)<<"High";
                battery->setCharge(100);
            } else if(bf & 0x02) {
                qLog(PowerManagement)<<"Low";
                battery->setCharge(25);
            } else if(bf & 0x04) {
                qLog(PowerManagement)<<"Critical";
                battery->setCharge(5);
            } else if(bf & 0x08) {
                qLog(PowerManagement)<<"Charging";
                if(batteryIsFull()) {
                    charging = false;
                } else {
                    charging = true;
                }
                battery->setCharge(percentCharge);
            } else if(bf & 0xFF) {
                qLog(PowerManagement)<<"Critical/Unknown";
                battery->setCharge(5);
            } else {
                qLog(PowerManagement)<<"setCharge -1";
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
void Ficgta01Battery::timerEvent(QTimerEvent *)
{
    updateFicStatus();
}

/*! \internal */
bool Ficgta01Battery::APMEnabled() const
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
			qLog(PowerManagement)<<"FIC APM Enabled";
            return true;
        }
    }
	qLog(PowerManagement)<<"FIC APM Disabled";
    return false;
}


void Ficgta01Battery::readPowerKbdData()
{
    Ficgta01Detect event;

    int n = read(kbdFDpower, &event, sizeof(Ficgta01Detect));
    if(n != (int)sizeof(Ficgta01Detect)) {
        return;
    }
    switch(event.code) {
    case 0x164: //usb/power plug up = out.down = in
        if(event.value != 0) {
               QTimer::singleShot( 1000, this, SLOT(updateFicStatus()));
        } else {
            updateFicStatus();
        }
        break;
    }
}

int Ficgta01Battery::getBatteryLevel()
{
    int voltage = 0;
    QString inStr;
    QFile battvolt( "/sys/bus/i2c/devices/0-0008/battvolt");
    battvolt.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&battvolt);
    in >> inStr;
    voltage = inStr.toInt();
    battvolt.close();


    voltage = voltage - 2800;
    float perc = voltage  / 14;
    percentCharge = round( perc + 0.5);
    percentCharge = qBound<quint16>(0, percentCharge, 100);
    qLog(PowerManagement)<<"Battery volt"<<voltage+2800 << percentCharge<<"%";

/*
4200 = 100%
4000 = 90%
3600 = 20%
3400 = 5%
2800 = 0%
*/
    return voltage;
}

bool Ficgta01Battery::batteryIsFull()
{
    if(getBatteryLevel() + 2800 > 4200)
        return true;
    return false;
}

QTOPIA_TASK(Ficgta01Battery, Ficgta01Battery);
