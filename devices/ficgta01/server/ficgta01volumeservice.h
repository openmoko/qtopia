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

#include <QtopiaIpcAdaptor>
#include <QValueSpaceObject>

#include <alsa/asoundlib.h>


class Ficgta01VolumeService : public QtopiaIpcAdaptor
{
    Q_OBJECT

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

    void setAmpMode(bool);
    void changeAmpModeVS();
    void setAmp(QString);
    void toggleAmpMode();

private Q_SLOTS:
    void registerService();
    void initVolumes();


private:
    int initMixer();
    int closeMixer();
    int saveState();
    void adjustSpeakerVolume(int left, int right);

private:
    enum AdjustType { Relative, Absolute };
    void adjustVolume(int leftChannel, int rightChannel, AdjustType);

    int m_leftChannelVolume;
    int m_rightChannelVolume;

    QtopiaIpcAdaptor *m_adaptor;
    QValueSpaceObject *m_vsoVolumeObject;

    snd_mixer_t *m_mixerFd;

    int m_minOutputVolume;
    int m_maxOutputVolume;

    int m_minInputVolume;
    int m_maxInputVolume;
};

#endif  // __QTOPIA_MEDIA_DEFAULTVOLUMEPROVIDER_H
