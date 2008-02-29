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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../include/soundcard.h"

#include <QTimer>
#include <QDebug>

#include <qtopiaserverapplication.h>
#include <qtopiaipcenvelope.h>

#include "greenphonevolumeservice.h"


GreenphoneVolumeService::GreenphoneVolumeService():
    QtopiaIpcAdaptor("QPE/AudioVolumeManager/GreenphoneVolumeService")
{
    publishAll(Slots);

    QTimer::singleShot(0, this, SLOT(registerService()));
}

GreenphoneVolumeService::~GreenphoneVolumeService()
{
}

//public slots:
void GreenphoneVolumeService::setVolume(int volume)
{
    adjustVolume(volume, volume, Absolute);
}

void GreenphoneVolumeService::setVolume(int leftChannel, int rightChannel)
{
    adjustVolume(leftChannel, rightChannel, Absolute);
}

void GreenphoneVolumeService::increaseVolume(int increment)
{
    adjustVolume(increment, increment, Relative);
}

void GreenphoneVolumeService::decreaseVolume(int decrement)
{
    decrement *= -1;

    adjustVolume(decrement, decrement, Relative);
}

void GreenphoneVolumeService::setMute(bool)
{
}

void GreenphoneVolumeService::registerService()
{
    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "registerHandler(QString,QString)");

    e << QString("Headset") << QString("QPE/AudioVolumeManager/GreenphoneVolumeService");

    QTimer::singleShot(0, this, SLOT(setCallDomain()));
}

void GreenphoneVolumeService::setCallDomain()
{
    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "setActiveDomain(QString)");

    e << QString("Headset");
}

void GreenphoneVolumeService::adjustVolume(int leftChannel, int rightChannel, AdjustType adjust)
{
    int mixerFd = open("/dev/mixer", O_RDWR);
    if (mixerFd >= 0) {
        unsigned int leftright;
        int left;
        int right;

        if (adjust == Relative) {
            ioctl(mixerFd, SOUND_MIXER_READ_ALTPCM, &leftright);

            left = (leftright & 0xff00) >> 8;
            right = (leftright & 0x00ff);

            left = qBound(0, left + leftChannel, 100);
            right = qBound(0, right + rightChannel, 100);
        } else {
            left = leftChannel;
            right = rightChannel;
        }

        leftright = (left << 8) | right;
        ioctl(mixerFd, SOUND_MIXER_WRITE_ALTPCM, &leftright);

        // ??
        leftright = (100 << 8) | 100;
        ioctl(mixerFd, SOUND_MIXER_WRITE_SPEAKER, &leftright);

        close(mixerFd);
    }
}

QTOPIA_TASK(GreenphoneVolumeService, GreenphoneVolumeService);

