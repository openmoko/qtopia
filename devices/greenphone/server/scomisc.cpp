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

#include <qtopiaipcadaptor.h>
#include <qtopialog.h>

#include "../include/omega_sysdevs.h"
#include "../include/omega_chgled.h"
#define CONFIG_ARCH_OMEGA
#include "../include/soundcard.h"

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <sys/ioctl.h>

void bt_sco_close(void *)
{
}

bool bt_sco_open(void **, const char *)
{
    return true;
}

bool bt_sco_set_fd(void *, int sco_fd)
{
    QtopiaIpcAdaptor *adaptor = new QtopiaIpcAdaptor("QPE/GreenphoneModem", 0 );

    if (sco_fd == -1) {
        adaptor->send(MESSAGE(setOutput(int)), 0);

        int mixerFd = ::open("/dev/mixer", O_RDWR);
        if (mixerFd >= 0) {
                ::ioctl(mixerFd, IOCTL_OMEGA_SOUND_BTCALL_STOP, 0);
                ::close(mixerFd);
        }

        qLog(Bluetooth) << "Setting btcalling to off";
    }
    else {
        adaptor->send(MESSAGE(setOutput(int)), 1);

        int mixerFd = ::open("/dev/mixer", O_RDWR);
        if (mixerFd >= 0) {
                ::ioctl(mixerFd, IOCTL_OMEGA_SOUND_BTCALL_START, 0);
                ::close(mixerFd);
        }

        qLog(Bluetooth) << "Setting btcalling to on";
    }

    delete adaptor;

    return true;
}

QByteArray find_btsco_device(const QByteArray &idPref = QByteArray())
{
    return QByteArray("default");
}

