/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
#include <linux/soundcard.h>

#include <QTimer>
#include <QDebug>

#include <qtopiaserverapplication.h>
#include <qtopiaipcenvelope.h>

#include "zylonitevolumeservice.h"


ZyloniteVolumeService::ZyloniteVolumeService():
    QtopiaIpcAdaptor("QPE/AudioVolumeManager/ZyloniteVolumeService")
{
    publishAll(Slots);

    QTimer::singleShot(0, this, SLOT(registerService()));
}

ZyloniteVolumeService::~ZyloniteVolumeService()
{
}

//public slots:
void ZyloniteVolumeService::setVolume(int volume)
{
    adjustVolume(volume, volume, Absolute);
}

void ZyloniteVolumeService::setVolume(int leftChannel, int rightChannel)
{
    adjustVolume(leftChannel, rightChannel, Absolute);
}

void ZyloniteVolumeService::increaseVolume(int increment)
{
    adjustVolume(increment, increment, Relative);
}

void ZyloniteVolumeService::decreaseVolume(int decrement)
{
    decrement *= -1;

    adjustVolume(decrement, decrement, Relative);
}

void ZyloniteVolumeService::setMute(bool)
{
}

void ZyloniteVolumeService::registerService()
{
    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "registerHandler(QString,QString)");

    e << QString("Headset") << QString("QPE/AudioVolumeManager/ZyloniteVolumeService");

    QTimer::singleShot(0, this, SLOT(setCallDomain()));
}

void ZyloniteVolumeService::setCallDomain()
{
    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "setActiveDomain(QString)");

    e << QString("Headset");
}

void ZyloniteVolumeService::adjustVolume(int leftChannel, int rightChannel, AdjustType adjust)
{
    qWarning("ZyloniteVolumeService::adjustVolume()");
    int mixerFd = open("/dev/mixer", O_RDWR);
    if (mixerFd >= 0)
    {
        unsigned int leftright;
        unsigned int left;
        unsigned int right;

        if (adjust == Relative)
        {
            ioctl(mixerFd, SOUND_MIXER_READ_ALTPCM, &leftright);

            left = (leftright & 0xff00) >> 8;
            right = (leftright & 0x00ff);

            left += leftChannel;
            right += rightChannel;
        }
        else
        {
            left = leftChannel;
            right = rightChannel;
        }

        if (left < 0)
            left = 0;
        if (left > 100)
            left = 100;

        if (right < 0)
            right = 0;
        if (right > 100)
            right = 100;

        leftright = (left << 8) | right;
        ioctl(mixerFd, SOUND_MIXER_WRITE_ALTPCM, &leftright);

        // ??
        leftright = (100 << 8) | 100;
        ioctl(mixerFd, SOUND_MIXER_WRITE_SPEAKER, &leftright);

        close(mixerFd);
    }
}

QTOPIA_TASK(ZyloniteVolumeService, ZyloniteVolumeService);

