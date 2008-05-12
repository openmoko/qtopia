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

#ifndef __QTOPIA_MEDIA_DEFAULTVOLUMEPROVIDER_H
#define __QTOPIA_MEDIA_DEFAULTVOLUMEPROVIDER_H

#include <QtopiaIpcAdaptor>


#include <alsa/asoundlib.h>
#include <QtopiaIpcAdaptor>
#include <QValueSpaceObject>


class Ficgta01VolumeService : public QtopiaIpcAdaptor
{
    Q_OBJECT
    enum AdjustType { Relative, Absolute };

public:
    Ficgta01VolumeService();
    ~Ficgta01VolumeService();

public slots:
    void setVolume(int volume);
    void setVolume(int leftChannel, int rightChannel);
    void increaseVolume(int increment);
    void decreaseVolume(int decrement);
    void setMute(bool mute);

    void adjustMicrophoneVolume(int volume);

private slots:
    void registerService();
    void initVolumes();

private:
    void adjustVolume(int leftChannel, int rightChannel, AdjustType);

    QtopiaIpcAdaptor *adaptor;
    QValueSpaceObject *vsoVolumeObject;

protected:
    snd_mixer_t *mixerFd;
    snd_mixer_elem_t *elem;
    QString elemName;

    int minOutputVolume;
    int maxOutputVolume;

    int minInputVolume;
    int maxInputVolume;

    int initMixer();
    int closeMixer();
    int saveState();
    void adjustSpeakerVolume(int left, int right);
};


#endif  // __QTOPIA_MEDIA_DEFAULTVOLUMEPROVIDER_H
