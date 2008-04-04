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
#include <sys/soundcard.h>

#include <QTimer>
#include <QDebug>
#include <QValueSpaceItem>
#include <qtopiaserverapplication.h>
#include <qtopiaipcenvelope.h>

#include <QAudioStateConfiguration>
#include <QAudioStateInfo>
#include <QValueSpaceObject>

#include "ficgta01volumeservice.h"

Ficgta01VolumeService::Ficgta01VolumeService()
    : QtopiaIpcAdaptor("QPE/AudioVolumeManager/Ficgta01VolumeService")
{
    publishAll(Slots);
    qLog(AudioState) << __PRETTY_FUNCTION__;

    m_adaptor = new QtopiaIpcAdaptor("QPE/Ficgta01Modem", this);
    m_vsoVolumeObject = new QValueSpaceObject("/Hardware/Audio");

    QValueSpaceItem *ampmode = new QValueSpaceItem("/System/Tasks/Ficgta01VolumeService/ampMode");
    QObject::connect(ampmode, SIGNAL(contentsChanged()),
                     this, SLOT(toggleAmpMode()));

    QTimer::singleShot(0, this, SLOT(registerService()));
}

Ficgta01VolumeService::~Ficgta01VolumeService()
{
}

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

    QTimer::singleShot(0, this, SLOT(initVolumes()));
}

void Ficgta01VolumeService::initVolumes()
{
    qLog(AudioState) << __PRETTY_FUNCTION__;

#ifndef QT_ILLUME_LAUNCHER
    long minPVolume;
    long maxPVolume;
    long volume;
    snd_mixer_elem_t *element;

    initMixer();
    for (element = snd_mixer_first_elem(m_mixerFd); element; element = snd_mixer_elem_next(element)) {
        if (snd_mixer_elem_get_type(element) == SND_MIXER_ELEM_SIMPLE
            && snd_mixer_selem_is_active(element)) {

            QString elementName = QString(snd_mixer_selem_get_name(element));

            // Master output
            if(elementName == QLatin1String("Headphone")) {
                snd_mixer_selem_get_playback_volume_range(element, &minPVolume, &maxPVolume);
                m_adaptor->send(MESSAGE(setSpeakerVolumeRange(int,int)), (int)minPVolume, (int)maxPVolume);
                if(snd_mixer_selem_has_playback_volume(element) > 0)
                    snd_mixer_selem_get_playback_volume(element, SND_MIXER_SCHN_FRONT_LEFT, &volume);

                m_adaptor->send(MESSAGE(setOutputVolume(int)), (int)volume);
            }

            // mic??
            if (elementName == QLatin1String("ALC Capture Target")) {
                snd_mixer_selem_get_capture_volume_range(element, &minPVolume, &maxPVolume);
                m_adaptor->send(MESSAGE(setMicVolumeRange(int,int )), (int)minPVolume, (int)maxPVolume);
                if(snd_mixer_selem_has_capture_volume(element) > 0)
                    snd_mixer_selem_get_capture_volume(element, SND_MIXER_SCHN_FRONT_LEFT, &volume);
                m_adaptor->send(MESSAGE(setMicVolume(int)), (int)volume);
            }
        }
    }
    closeMixer();
#endif
}

void Ficgta01VolumeService::adjustVolume(int leftChannel, int rightChannel, AdjustType adjust)
{
    qLog(AudioState)<< __PRETTY_FUNCTION__;

    unsigned int leftright;
    int left;
    int right;

    if (adjust == Relative) {
        left = (leftright & 0xff00) >> 8;
        right = (leftright & 0x00ff);

        left = qBound(m_minOutputVolume, left + leftChannel, m_maxOutputVolume);
        right = qBound(m_minOutputVolume, right + rightChannel, m_maxOutputVolume);
    } else {
        left = leftChannel;
        right = rightChannel;
    }

    leftright = (left << 8) | right;
    leftright = (m_maxOutputVolume << 8) | m_maxOutputVolume;
    adjustSpeakerVolume(left, right);
}




void Ficgta01VolumeService::adjustSpeakerVolume(int left, int right)
{
#ifndef QT_ILLUME_LAUNCHER
    snd_mixer_elem_t *element;

    initMixer();
    for (element = snd_mixer_first_elem(m_mixerFd); element; element = snd_mixer_elem_next(element)) {
        if (snd_mixer_elem_get_type(element) == SND_MIXER_ELEM_SIMPLE
            && snd_mixer_selem_is_active(element)) {

            QString elementName = QString(snd_mixer_selem_get_name(element));

            // Master output // could use PCM
            if (elementName == QLatin1String("Headphone")) {
                if (snd_mixer_selem_has_playback_volume(element) > 0) {
                    snd_mixer_selem_set_playback_volume(element, SND_MIXER_SCHN_FRONT_LEFT, (long)&left);
                    snd_mixer_selem_set_playback_volume(element, SND_MIXER_SCHN_FRONT_RIGHT, (long)&right);
                }
            }
        }
    }

    closeMixer();
    saveState();
#else
    Q_UNUSED(left)
    Q_UNUSED(right)
#endif
}

void Ficgta01VolumeService::adjustMicrophoneVolume( int volume )
{
#ifndef QT_ILLUME_LAUNCHER
    snd_mixer_elem_t *element;

    initMixer();
    for (element = snd_mixer_first_elem(m_mixerFd); element; element = snd_mixer_elem_next(element)) {
        if (snd_mixer_elem_get_type(element) == SND_MIXER_ELEM_SIMPLE
            && snd_mixer_selem_is_active(element)) {
            QString elementName = QString(snd_mixer_selem_get_name(element));

            // Mic Input
            if (elementName == QLatin1String("VLC Capture Target")) {
                if (snd_mixer_selem_has_playback_volume(element) > 0) {
                    snd_mixer_selem_set_capture_volume(element, SND_MIXER_SCHN_FRONT_LEFT, (long)&volume);
                }
            }
        }
    }

    closeMixer();
    saveState();
#else
    Q_UNUSED(volume)
#endif
}

int Ficgta01VolumeService::initMixer()
{
    int result;

    if ((result = snd_mixer_open(&m_mixerFd, 0)) < 0) {
        qWarning() << "snd_mixer_open error" << result;
        m_mixerFd = NULL;
        return result;
    }

    /*  hw:0 hw:0,0*/
    if ((result = snd_mixer_attach(m_mixerFd, "default")) < 0) {
        qWarning() << "snd_mixer_attach error" << result;
        snd_mixer_close(m_mixerFd);
        m_mixerFd = NULL;
        return result;
    }

    if ((result = snd_mixer_selem_register(m_mixerFd, NULL, NULL)) < 0) {
        qWarning() << "snd_mixer_selem_register error" << result;
        snd_mixer_close(m_mixerFd);
        m_mixerFd = NULL;
        return result;
    }

    if ((result = snd_mixer_load(m_mixerFd)) < 0) {
        qWarning() << "snd_mixer_load error" << result;
        snd_mixer_close(m_mixerFd);
        m_mixerFd = NULL;
        return result;
    }

    return result;
}

int Ficgta01VolumeService::closeMixer()
{
    int result = snd_mixer_detach(m_mixerFd, "default");
    result = snd_mixer_close(m_mixerFd);
    // the below causes segfault
    //snd_mixer_free(m_mixerFd);
    return 0;

}


int Ficgta01VolumeService::saveState()
{
    QAudioStateConfiguration *audioState;
    audioState = new QAudioStateConfiguration(this);

    QString currentProfile(audioState->currentState().profile());
    QString m_mode;

    if(currentProfile == "MediaSpeaker"
       || currentProfile == "MediaHeadphones"
       || currentProfile == "RingToneSpeaker") {
        m_mode = "stereoout";
    }

    if(currentProfile == "PhoneSpeaker" ) {
        m_mode = "gsmhandset";
    } else if(currentProfile == "PhoneHeadphones") {
        m_mode = "gsmheadset";
    } else if(currentProfile == "PhoneBluetoothHeadset" /*|| "MediaBluetoothHeadset"*/) {
        m_mode = "gsmbluetooth";
    }

    QString confDir;
    if(QDir("/etc/alsa").exists())
        confDir="/etc/alsa/";
    else
        confDir="/etc/";

    QString cmd = "/usr/sbin/alsactl -f "+ confDir+m_mode + ".state store";
    qLog(AudioState) << cmd;
    system(cmd.toLocal8Bit());
    return 0;
}


/**
 * @param mode false turns off amp
 */
void Ficgta01VolumeService::setAmpMode(bool mode)
{
    QString currentMode;
    QValueSpaceItem ampVS("/Hardware/Audio/CurrentAmpMode");
    QString ampMode = ampVS.value().toString();

    if(mode)
        m_vsoVolumeObject->setAttribute("Amp", "On");
    else
        m_vsoVolumeObject->setAttribute("Amp","Off");


    if (ampMode.isEmpty() || !mode)
        ampMode = "Off";


#ifndef QT_ILLUME_LAUNCHER
    snd_mixer_elem_t *element;
    char itemname[40];
    unsigned int item = 0;

    initMixer();

    for (element = snd_mixer_first_elem(m_mixerFd); element; element = snd_mixer_elem_next(element) ) {
        if (snd_mixer_elem_get_type(element) == SND_MIXER_ELEM_SIMPLE
            && snd_mixer_selem_is_enumerated(element)
            && snd_mixer_selem_is_active(element)) {

            QString elementName = QString(snd_mixer_selem_get_name(element));
            if (elementName == "Amp Mode") {
                //current selection
                snd_mixer_selem_get_enum_item(element, (snd_mixer_selem_channel_id_t)0, &item);
                snd_mixer_selem_get_enum_item_name(element, item, sizeof(itemname) - 1, itemname);

                currentMode = itemname;

                if (mode && currentMode != ampMode && currentMode != "Off")
                    ampMode = currentMode;


                int enumItems = snd_mixer_selem_get_enum_items(element);
                for (item = 0; item < static_cast<unsigned>(enumItems); ++item) {
                    snd_mixer_selem_get_enum_item_name(element, item, sizeof(itemname) - 1, itemname);

                    if (!mode) {
                        if (QString(itemname) == QString("Off")) {
                            //      snd_mixer_selem_get_enum_item(elem, channelId, &item);
                            snd_mixer_selem_set_enum_item(element, (snd_mixer_selem_channel_id_t)0, item);

                            break;
                        }
                    } else {
                        if (QString(itemname) == ampMode) {
                            //      snd_mixer_selem_get_enum_item(elem, channelId, &item);
                            snd_mixer_selem_set_enum_item(element, (snd_mixer_selem_channel_id_t)0, item);

                            break;
                        }
                    }
                }
            }
        }

    }

    closeMixer();
#endif

    if (currentMode != "Off")
        ampMode = currentMode;
    m_vsoVolumeObject->setAttribute("CurrentAmpMode", ampMode);
}

/*
  sets the vso to current amp mode from mixer
 */
void Ficgta01VolumeService::changeAmpModeVS()
{
#ifndef QT_ILLUME_LAUNCHER
    char itemname[40];
    unsigned int item = 0;
    snd_mixer_elem_t *element;

    initMixer();
    for (element = snd_mixer_first_elem(m_mixerFd); element; element = snd_mixer_elem_next(element)) {
        if (snd_mixer_elem_get_type(element) == SND_MIXER_ELEM_SIMPLE
            && snd_mixer_selem_is_enumerated(element)
            && snd_mixer_selem_is_active(element) ) {

            QString elementName = QString(snd_mixer_selem_get_name(element));
            if(elementName == "Amp Mode") {

                //current selection
                snd_mixer_selem_get_enum_item(element, (snd_mixer_selem_channel_id_t)0, &item);
                snd_mixer_selem_get_enum_item_name(element, item, sizeof(itemname) - 1, itemname);

                m_vsoVolumeObject->setAttribute("CurrentAmpMode", itemname);
            }
        }
      }
      closeMixer();
#endif
}

/*
sets a new AmpMode by name
*/
void Ficgta01VolumeService::setAmp(QString amode)
{
    QValueSpaceItem ampVS("/Hardware/Audio/Amp");
    QString ok = ampVS.value().toString();

#ifndef QT_ILLUME_LAUNCHER
    char itemname[40];
    unsigned int item = 0;
    snd_mixer_elem_t *element;
    initMixer();
    for (element = snd_mixer_first_elem(m_mixerFd); element; element = snd_mixer_elem_next(element)) {
        if (snd_mixer_elem_get_type(element) == SND_MIXER_ELEM_SIMPLE
            && snd_mixer_selem_is_enumerated(element)
            && snd_mixer_selem_is_active(element)) {

            QString elementName = QString(snd_mixer_selem_get_name(element));
            if(elementName == "Amp Mode") {
                int enumItems = snd_mixer_selem_get_enum_items(element);

                for (item = 0; item < static_cast<unsigned>(enumItems); ++item) {
                    snd_mixer_selem_get_enum_item_name(element, item, sizeof(itemname) - 1, itemname);

                    //somehow this should still be off
                    if (QString(itemname) == ok) {
                        snd_mixer_selem_set_enum_item(element, (snd_mixer_selem_channel_id_t)0, item);
                    }

                    //      snd_mixer_selem_get_enum_item(elem, channelId, &item);
                    if (QString(itemname) == amode) {
                        if (ok == "On") {
                            snd_mixer_selem_set_enum_item(element, (snd_mixer_selem_channel_id_t)0, item);
                        }
                        m_vsoVolumeObject->setAttribute("CurrentAmpMode",amode);

                        break;
                    }
                }
            }
        }
    }
    closeMixer();
#else
    Q_UNUSED(amode)
#endif
}

void Ficgta01VolumeService::toggleAmpMode()
{
    QValueSpaceItem ampVS("/Hardware/Audio/CurrentAmpMode");
    QString ampMode = ampVS.value().toString();

    setAmpMode(ampMode != QLatin1String("Off"));
}



QTOPIA_TASK(Ficgta01VolumeService, Ficgta01VolumeService);

