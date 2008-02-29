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

#ifdef QT_QWS_N800

#include "n800hardware.h"

//#include "../include/omega_sysdevs.h"
//#include "../include/omega_chgled.h"
//#define CONFIG_ARCH_OMEGA
//#include "../include/soundcard.h"

#include <QSocketNotifier>
#include <QTimer>
#include <QLabel>
#include <QDesktopWidget>
#include <QProcess>
#include <QFile>
#include <QtGlobal>

#include <qcontentset.h>
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qbootsourceaccessory.h>
#include <qtopiaipcenvelope.h>
#include <qpowersource.h>

#include <qtopiaserverapplication.h>
#include <standarddevicefeatures.h>
//#include <ui/standarddialogs.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <sys/ioctl.h>

QTOPIA_TASK(N800Hardware, N800Hardware);

N800Hardware::N800Hardware()
    : charging(false), percent(-1), chargeId(0),
      vsoPortableHandsfree("/Hardware/Accessories/PortableHandsfree"), mountProc(NULL)
{
    //StandardDialogs::disableShutdownDialog();

//     detectFd = ::open("/dev/omega_detect", O_RDONLY|O_NDELAY, 0);
//     if (detectFd >= 0) {
//         qLog(Hardware) << "Opened omega_detect as detect input";
//         m_notifyDetect = new QSocketNotifier(detectFd, QSocketNotifier::Read, this);
//         connect(m_notifyDetect, SIGNAL(activated(int)), this, SLOT(readDetectData()));
//     } else {
//         qWarning("Cannot open omega_detect for detect (%s)", strerror(errno));
//     }

    //QObject::connect(QtopiaServerApplication::instance(), SIGNAL(shutdownRequested()), this, SLOT(shutdownRequested()));

    bootSource = new QBootSourceAccessoryProvider( "N800", this );

    batterySource = new QPowerSourceProvider(QPowerSource::Battery, "N800Battery", this);
    batterySource->setAvailability(QPowerSource::Available);
    connect(batterySource, SIGNAL(chargingChanged(bool)),
            this, SLOT(chargingChanged(bool)));
    connect(batterySource, SIGNAL(chargeChanged(int)),
            this, SLOT(chargeChanged(int)));

    wallSource = new QPowerSourceProvider(QPowerSource::Wall, "N800Charger", this);

//     readDetectData(1 << CHARGER_DETECT |
//                    1 << BOOTSRC_DETECT |
//                    1 << LOWPOWER_DETECT);

    QTimer::singleShot(30*1000, this, SLOT(delayedRead()));
}

N800Hardware::~N800Hardware()
{
    if (detectFd >= 0) {
        ::close(detectFd);
        detectFd = -1;
    }
}

void N800Hardware::mountSD()
{
    QFile partitions("/proc/partitions");
    if (!partitions.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    sdCardDevice.clear();

    QList<QByteArray> lines = partitions.readAll().split('\n');
    for (int i = 2; i < lines.count(); i++) {
        QStringList fields = QString(lines.at(i)).split(' ', QString::SkipEmptyParts);

        if (fields.count() == 4) {
            if (sdCardDevice.isEmpty() && fields.at(3) == "mmcblk") {
                sdCardDevice = "/dev/" + fields.at(3);
            } else if (fields.at(3).startsWith("mmcblk")) {
                sdCardDevice = "/dev/" + fields.at(3);
                break;
            }
        }
    }

    if (sdCardDevice.isEmpty())
        return;

    if (!mountProc)
        mountProc = new QProcess(this);

    if (mountProc->state() != QProcess::NotRunning)
        return;

    connect(mountProc, SIGNAL(finished(int,QProcess::ExitStatus)),
                       SLOT(fsckFinished(int,QProcess::ExitStatus)));

    QStringList arguments;
    arguments << "-a" << sdCardDevice;

    qLog(Hardware) << "Checking filesystem on" << sdCardDevice;
    mountProc->start("fsck", arguments);
}

void N800Hardware::unmountSD()
{
    if (!mountProc)
        mountProc = new QProcess(this);

    if (mountProc->state() != QProcess::NotRunning) {
        qLog(Hardware) << "Previous (u)mount command failed to finished";
        mountProc->kill();
    }

    connect(mountProc, SIGNAL(finished(int,QProcess::ExitStatus)),
                       SLOT(mountFinished(int,QProcess::ExitStatus)));

    qLog(Hardware) << "Unmounting /mnt/sd";
    mountProc->start("umount -l /mnt/sd");
}

void N800Hardware::fsckFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode != 0)
        qLog(Hardware) << "Filesystem errors detected on" << sdCardDevice;

    disconnect(mountProc, SIGNAL(finished(int,QProcess::ExitStatus)),
               this, SLOT(fsckFinished(int,QProcess::ExitStatus)));

    connect(mountProc, SIGNAL(finished(int,QProcess::ExitStatus)),
                       SLOT(mountFinished(int,QProcess::ExitStatus)));

    QStringList arguments;
    arguments << sdCardDevice << "/mnt/sd";

    qLog(Hardware) << "Mounting" << sdCardDevice << "on /mnt/sd";
    mountProc->start("mount", arguments);
}

void N800Hardware::mountFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode != 0)
        qLog(Hardware) << "Failed to (u)mount" << sdCardDevice << "on /mnt/sd";

    mountProc->deleteLater();
    mountProc = NULL;

    QtopiaIpcEnvelope msg("QPE/Card", "mtabChanged()");
}

void N800Hardware::chargingChanged(bool charging)
{
    if (charging)
        setLeds(101);
    else
        setLeds(batterySource->charge());
}

void N800Hardware::chargeChanged(int charge)
{
    if (!batterySource->charging())
        setLeds(charge);
}

void N800Hardware::setLeds(int charge)
{
/*
  int ledFd = ::open("/dev/omega_chgled", O_RDWR);
    if (ledFd < 0) {
        perror("Failed to open /dev/omega_chgled");
        return;
    }

    if (charge == 101) {
        // red on
        ::ioctl(ledFd, SET_GPO0_CTRL, 0);
        ::ioctl(ledFd, SET_GPO1_CTRL, 7);
    } else if (percent > 75) {
        // red flashing slow
        ::ioctl(ledFd, ENABLE_LED1_IN_ACT, 0);
        ::ioctl(ledFd, SET_LED1_BLINK_TIME, 7);
        ::ioctl(ledFd, SET_LED1_LIGHT_TIME, 1);
        ::ioctl(ledFd, SET_GPO0_CTRL, 0);
        ::ioctl(ledFd, SET_GPO1_CTRL, 1);
    } else if (percent > 50) {
        // red flashing slow
        ::ioctl(ledFd, ENABLE_LED1_IN_ACT, 0);
        ::ioctl(ledFd, SET_LED1_BLINK_TIME, 5);
        ::ioctl(ledFd, SET_LED1_LIGHT_TIME, 1);
        ::ioctl(ledFd, SET_GPO0_CTRL, 0);
        ::ioctl(ledFd, SET_GPO1_CTRL, 1);
    } else if (percent > 25) {
        // red flashing fast
        ::ioctl(ledFd, ENABLE_LED1_IN_ACT, 0);
        ::ioctl(ledFd, SET_LED1_BLINK_TIME, 3);
        ::ioctl(ledFd, SET_LED1_LIGHT_TIME, 1);
        ::ioctl(ledFd, SET_GPO0_CTRL, 0);
        ::ioctl(ledFd, SET_GPO1_CTRL, 1);
    } else if (percent > 0) {
        // red flashing fast
        ::ioctl(ledFd, ENABLE_LED1_IN_ACT, 0);
        ::ioctl(ledFd, SET_LED1_BLINK_TIME, 1);
        ::ioctl(ledFd, SET_LED1_LIGHT_TIME, 1);
        ::ioctl(ledFd, SET_GPO0_CTRL, 0);
        ::ioctl(ledFd, SET_GPO1_CTRL, 1);
    } else if (percent == 0) {
        // red flashing very fast
        ::ioctl(ledFd, ENABLE_LED1_IN_ACT, 0);
        ::ioctl(ledFd, SET_LED1_BLINK_TIME, 0);
        ::ioctl(ledFd, SET_LED1_LIGHT_TIME, 4);
        ::ioctl(ledFd, SET_GPO0_CTRL, 0);
        ::ioctl(ledFd, SET_GPO1_CTRL, 1);
    } else {
        // all off
        ::ioctl(ledFd, SET_GPO0_CTRL, 0);
        ::ioctl(ledFd, SET_GPO1_CTRL, 0);
    }

    if (ledFd >= 0)
        ::close(ledFd);
*/
}

void N800Hardware::shutdownRequested()
{
    QtopiaServerApplication::instance()->shutdown(QtopiaServerApplication::ShutdownSystem);
}

void N800Hardware::delayedRead()
{
//     readDetectData(1 << SDCARD_DETECT |
//                    1 << AVHEADSET_DETECT);
}

void N800Hardware::readDetectData(quint32 devices)
{
}

#endif // QT_QWS_N800
