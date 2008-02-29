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
#include <sys/soundcard.h>

#include <QTimer>
#include <QDebug>

#include <qtopiaserverapplication.h>
#include <qtopiaipcenvelope.h>

#include <QAudioStateConfiguration>
#include <QAudioStateInfo>

#include "ficgta01volumeservice.h"

// OSS for now
// TODO alsa

Ficgta01VolumeService::Ficgta01VolumeService():
    QtopiaIpcAdaptor("QPE/AudioVolumeManager/Ficgta01VolumeService")
{
    publishAll(Slots);
    qLog(AudioState)<<"Ficgta01VolumeService::Ficgta01VolumeService()";

    adaptor = new QtopiaIpcAdaptor("QPE/Ficgta01Modem", this);

    QTimer::singleShot(0, this, SLOT(registerService()));
}

Ficgta01VolumeService::~Ficgta01VolumeService()
{
}

//public slots:
void Ficgta01VolumeService::setVolume(int volume)
{
    adjustVolume(volume, volume, Absolute);
}

void Ficgta01VolumeService::setVolume(int leftChannel, int rightChannel)
{
    adjustVolume(leftChannel, rightChannel, Absolute);
}

void Ficgta01VolumeService::increaseVolume(int increment)
{
    adjustVolume(increment, increment, Relative);
}

void Ficgta01VolumeService::decreaseVolume(int decrement)
{
    decrement *= -1;

    adjustVolume(decrement, decrement, Relative);
}

void Ficgta01VolumeService::setMute(bool)
{
}

void Ficgta01VolumeService::registerService()
{
    QtopiaIpcEnvelope e("QPE/AudioVolumeManager", "registerHandler(QString,QString)");
    e << QString("Headset") << QString("QPE/AudioVolumeManager/Ficgta01VolumeService");

//    QTimer::singleShot(0, this, SLOT(setCallDomain()));
    QTimer::singleShot(0, this, SLOT(initVolumes()));

}

void Ficgta01VolumeService::initVolumes()
{
    qLog(AudioState)<<" Ficgta01VolumeService::initVolumes()";

    initMixer();
//alsa
    snd_mixer_elem_t *elem;
    long minPVolume;
    long maxPVolume;
    long volume;

    for ( elem = snd_mixer_first_elem( mixerFd); elem; elem = snd_mixer_elem_next( elem) ) {
        if ( snd_mixer_elem_get_type( elem ) == SND_MIXER_ELEM_SIMPLE &&
             snd_mixer_selem_is_active( elem) ) {

            elemName = QString(snd_mixer_selem_get_name( elem));

            if(elemName == "Headphone") { // Master output
                snd_mixer_selem_get_playback_volume_range( elem, &minPVolume, &maxPVolume);

                adaptor->send(MESSAGE(setSpeakerVolumeRange(int,int )), (int)minPVolume, (int)maxPVolume);

                if(snd_mixer_selem_has_playback_volume( elem) > 0)
                    snd_mixer_selem_get_playback_volume( elem,SND_MIXER_SCHN_FRONT_LEFT, &volume);

                adaptor->send(MESSAGE(setOutputVolume(int)), (int)volume);

            }

            if(elemName == "ALC Capture Target") { // mic??
                snd_mixer_selem_get_capture_volume_range( elem, &minPVolume, &maxPVolume);

                adaptor->send(MESSAGE(setMicVolumeRange(int,int )), (int)minPVolume, (int)maxPVolume);


                if(snd_mixer_selem_has_capture_volume( elem) > 0)
                    snd_mixer_selem_get_capture_volume( elem,SND_MIXER_SCHN_FRONT_LEFT, &volume);
                adaptor->send(MESSAGE(setMicVolume(int)), (int)volume);

            }
        }
    }

    closeMixer();

}

void Ficgta01VolumeService::setCallDomain()
{
    qLog(AudioState)<<"Ficgta01VolumeService::setCallDomain()";
    QtopiaIpcEnvelope e("QPE/AudioVolumeManager", "setActiveDomain(QString)");
    e << QString("Headset");
}

void Ficgta01VolumeService::adjustVolume(int leftChannel, int rightChannel, AdjustType adjust)
{
    qLog(AudioState)<<" Ficgta01VolumeService::adjustVolume";
    unsigned int leftright;
    int left;
    int right;

    if (adjust == Relative) {

        left = (leftright & 0xff00) >> 8;
        right = (leftright & 0x00ff);

        left = qBound(minOutputVolume, left + leftChannel, maxOutputVolume);
        right = qBound(minOutputVolume, right + rightChannel, maxOutputVolume);
    } else {
        left = leftChannel;
        right = rightChannel;
    }

    leftright = (left << 8) | right;

    leftright = (maxOutputVolume << 8) | maxOutputVolume;

    adjustSpeakerVolume( left, right );
}




void Ficgta01VolumeService::adjustSpeakerVolume( int left, int right )
{
    initMixer();

    for ( elem = snd_mixer_first_elem( mixerFd); elem; elem = snd_mixer_elem_next( elem) ) {
        if ( snd_mixer_elem_get_type( elem ) == SND_MIXER_ELEM_SIMPLE &&
             snd_mixer_selem_is_active( elem) ) {

            elemName = QString(snd_mixer_selem_get_name( elem));

            if(elemName == "Headphone") { // Master output // could use PCM

                if(snd_mixer_selem_has_playback_volume( elem) > 0) {
                    snd_mixer_selem_set_playback_volume( elem,SND_MIXER_SCHN_FRONT_LEFT, (long)&left);
                    snd_mixer_selem_set_playback_volume( elem,SND_MIXER_SCHN_FRONT_RIGHT, (long)&right);
                }
            }
        }
    }

    closeMixer();
    saveState();
}

void Ficgta01VolumeService::adjustMicrophoneVolume( int volume )
{

    initMixer();
    for ( elem = snd_mixer_first_elem( mixerFd); elem; elem = snd_mixer_elem_next( elem) ) {
        if ( snd_mixer_elem_get_type( elem ) == SND_MIXER_ELEM_SIMPLE &&
             snd_mixer_selem_is_active( elem) ) {

            elemName = QString(snd_mixer_selem_get_name( elem));

            if(elemName == "VLC Capture Target") { // Mic Input

                if(snd_mixer_selem_has_playback_volume( elem) > 0) {
                    snd_mixer_selem_set_capture_volume( elem,SND_MIXER_SCHN_FRONT_LEFT, (long)&volume);
                }
            }
        }
    }

    closeMixer();
    saveState();
}

int Ficgta01VolumeService::initMixer()
{
    int result;
  if ((result = snd_mixer_open( &mixerFd, 0)) < 0) {
        qWarning()<<"snd_mixer_open error"<< result;
        mixerFd = NULL;
        return result;
    }
/*  hw:0
  hw:0,0*/
    if ((result = snd_mixer_attach( mixerFd, "default")) < 0) {
        qWarning()<<"snd_mixer_attach error"<< result;
        snd_mixer_close(mixerFd);
        mixerFd = NULL;
        return result;
    }
    if ((result = snd_mixer_selem_register( mixerFd, NULL, NULL)) < 0) {
        qWarning()<<"snd_mixer_selem_register error"<<result;
        snd_mixer_close(mixerFd);
        mixerFd = NULL;
        return result;
    }
    if ((result = snd_mixer_load( mixerFd)) < 0) {
        qWarning()<<"snd_mixer_load error"<<result;
        snd_mixer_close(mixerFd);
        mixerFd = NULL;
        return result;
    }
    return result;
}

int Ficgta01VolumeService::closeMixer()
{
     int result = snd_mixer_detach( mixerFd, "default" );
     result = snd_mixer_close( mixerFd );
//     snd_mixer_free( mixerFd ); //causes segfault
    return 0;

}


int Ficgta01VolumeService::saveState()
{

    QAudioStateConfiguration *audioState;
    audioState = new QAudioStateConfiguration(this);

    QString currentProfile( audioState->currentState().profile());
    QString m_mode;

    if(currentProfile == "MediaSpeaker"
       || currentProfile == "MediaHeadphones"
       || currentProfile == "RingToneSpeaker") {
        m_mode = "stereoout";
    }

    if(currentProfile == "PhoneSpeaker" ) {
        m_mode = "gsmhandset";
    }
    if(currentProfile == "PhoneHeadphones") {
        m_mode = "gsmheadset";
    }
    if(currentProfile == "PhoneBluetoothHeadset" /*|| "MediaBluetoothHeadset"*/) {
        m_mode = "gsmbluetooth";
    }
//Ficgta01CallVolume
    QString confDir;
    if( QDir("/etc/alsa").exists())
        confDir="/etc/alsa/";
    else
        confDir="/etc/";

    QString cmd = "/usr/sbin/alsactl -f "+confDir+m_mode+".state store";
    qLog(AudioState)<<cmd;
    int result = system(cmd.toLocal8Bit());
return 0;
}





QTOPIA_TASK(Ficgta01VolumeService, Ficgta01VolumeService);

