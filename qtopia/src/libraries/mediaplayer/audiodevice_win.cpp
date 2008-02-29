/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include <stdlib.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include "audiodevice.h"
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>

static const int expectedBytesPerMilliSecond = 2 * 2 * 44000 / 1000;
static const int timerResolutionMilliSeconds = 30;
static const int sound_fragment_bytes = timerResolutionMilliSeconds * expectedBytesPerMilliSecond;


class AudioDevicePrivate {
public:
    int handle;
    unsigned int frequency;
    unsigned int channels;
    unsigned int bytesPerSample;
    unsigned int bufferSize;

    static int dspFd;
    static bool muted;
    static unsigned int leftVolume;
    static unsigned int rightVolume;
};


int AudioDevicePrivate::dspFd = 0;
bool AudioDevicePrivate::muted = FALSE;
unsigned int AudioDevicePrivate::leftVolume = 0;
unsigned int AudioDevicePrivate::rightVolume = 0;


void AudioDevice::volume(int &leftVolume, int &rightVolume)
{
    unsigned int volume;
    HWAVEOUT handle;
    WAVEFORMATEX formatData;
    formatData.cbSize = sizeof(WAVEFORMATEX);
    formatData.wFormatTag = WAVE_FORMAT_PCM;
    formatData.nAvgBytesPerSec = 4 * 44000;
    formatData.nBlockAlign = 4;
    formatData.nChannels = 2;
    formatData.nSamplesPerSec = 44000;
    formatData.wBitsPerSample = 16;
    waveOutOpen(&handle, WAVE_MAPPER, &formatData, 0L, 0L, CALLBACK_NULL);
    if ( waveOutGetVolume( handle, (LPDWORD)&volume ) )
	qDebug( "get volume of audio device failed" );
    waveOutClose( handle );
    leftVolume = volume & 0xFFFF;
    rightVolume = volume >> 16;
}


void AudioDevice::setVolume(int leftVolume, int rightVolume)
{
    if ( AudioDevicePrivate::muted ) {
       AudioDevicePrivate::leftVolume = leftVolume;
       AudioDevicePrivate::rightVolume = rightVolume;
       leftVolume = 0;
       rightVolume = 0;
    } else {
        leftVolume  = ( (int) leftVolume < 0 ) ? 0 : ((  leftVolume > 0xFFFF ) ? 0xFFFF :  leftVolume );
        rightVolume = ( (int)rightVolume < 0 ) ? 0 : (( rightVolume > 0xFFFF ) ? 0xFFFF : rightVolume );
    }
    HWAVEOUT handle;
    WAVEFORMATEX formatData;
    formatData.cbSize = sizeof(WAVEFORMATEX);
    formatData.wFormatTag = WAVE_FORMAT_PCM;
    formatData.nAvgBytesPerSec = 4 * 44000;
    formatData.nBlockAlign = 4;
    formatData.nChannels = 2;
    formatData.nSamplesPerSec = 44000;
    formatData.wBitsPerSample = 16;
    waveOutOpen(&handle, WAVE_MAPPER, &formatData, 0L, 0L, CALLBACK_NULL);
    unsigned int volume = (rightVolume << 16) | leftVolume;
    if ( waveOutSetVolume( handle, volume ) )
	qDebug( "set volume of audio device failed" );
    waveOutClose( handle );
}


bool AudioDevice::muted()
{
    return AudioDevicePrivate::muted;
}


void AudioDevice::setMuted(bool m)
{
    AudioDevicePrivate::muted = m;
    int l,r;
    volume(l,r);
    setVolume(l,r);
}


AudioDevice::AudioDevice(QObject *parent, const char *name) : QObject(parent, name), d(0)
{
}


AudioDevice::~AudioDevice()
{
}


void AudioDevice::open( unsigned int f, unsigned int chs, unsigned int bps )
{
    if ( d )
	close();
    d = new AudioDevicePrivate;
    d->frequency = f;
    d->channels = chs;
    d->bytesPerSample = bps;

    Config cfg("Sound");
    cfg.setGroup("System");
    AudioDevicePrivate::muted = cfg.readNumEntry("Muted", false);

    connect( qApp, SIGNAL( volumeChanged(bool) ), this, SLOT( volumeChanged(bool) ) );

    UINT	    result;
    WAVEFORMATEX    formatData;
    formatData.cbSize = sizeof(WAVEFORMATEX);
/*
    // Other possible formats windows supports
    formatData.wFormatTag = WAVE_FORMAT_MPEG;
    formatData.wFormatTag = WAVE_FORMAT_MPEGLAYER3;
    formatData.wFormatTag = WAVE_FORMAT_ADPCM;
*/
    formatData.wFormatTag = WAVE_FORMAT_PCM;
    formatData.nAvgBytesPerSec = bps * chs * f;
    formatData.nBlockAlign = bps * chs;
    formatData.nChannels = chs;
    formatData.nSamplesPerSec = f;
    formatData.wBitsPerSample = bps * 8;
    // Open a waveform device for output
    if (result = waveOutOpen((LPHWAVEOUT)&d->handle, WAVE_MAPPER, &formatData, 0L, 0L, CALLBACK_NULL)) {
#ifdef DEBUG
	QString errorMsg = "error opening audio device.\nReason: %i - "; // No tr
	switch (result) {
	    case MMSYSERR_ALLOCATED:	errorMsg += "Specified resource is already allocated."; break; // No tr
	    case MMSYSERR_BADDEVICEID:	errorMsg += "Specified device identifier is out of range."; break; // No tr
	    case MMSYSERR_NODRIVER:	errorMsg += "No device driver is present."; break; // No tr
	    case MMSYSERR_NOMEM:	errorMsg += "Unable to allocate or lock memory."; break; // No tr
	    case WAVERR_BADFORMAT:	errorMsg += "Attempted to open with an unsupported waveform-audio format."; break; // No tr
	    case WAVERR_SYNC:		errorMsg += "The device is synchronous but waveOutOpen was called without using the WAVE_ALLOWSYNC flag."; break; // No tr
	    default:			errorMsg += "Undefined error"; break; // No tr
	}
	qDebug( errorMsg, result );
#endif
    }

    d->bufferSize = sound_fragment_bytes;
}
    

void AudioDevice::close()
{
    if ( !d )
	return;
    waveOutClose( (HWAVEOUT)d->handle );
    delete d;
    d = 0;
}


void AudioDevice::volumeChanged( bool muted )
{
    AudioDevicePrivate::muted = muted;
}


bool AudioDevice::write( char *buffer, unsigned int length )
{
    if ( !d )
	return false;
    // returns immediately and (to be implemented) emits completedIO() when finished writing
    WAVEHDR *lpWaveHdr = (WAVEHDR *)malloc( sizeof(WAVEHDR) );
    // maybe the buffer should be copied so that this fool proof, but its a performance hit
    lpWaveHdr->lpData = buffer;
    lpWaveHdr->dwBufferLength = length;
    lpWaveHdr->dwFlags = 0L;
    lpWaveHdr->dwLoops = 0L;
    waveOutPrepareHeader( (HWAVEOUT)d->handle, lpWaveHdr, sizeof(WAVEHDR) );
    // waveOutWrite returns immediately. the data is sent in the background. 
    if ( waveOutWrite( (HWAVEOUT)d->handle, lpWaveHdr, sizeof(WAVEHDR) ) )
	qDebug( "failed to write block to audio device" );
    // emit completedIO();
    return true;
}


unsigned int AudioDevice::channels() const
{
    return d ? d->channels : 0;
}


unsigned int AudioDevice::frequency() const
{
    return d ? d->frequency : 0;
}


unsigned int AudioDevice::bytesPerSample() const
{
    return d ? d->bytesPerSample : 0;
}


unsigned int AudioDevice::bufferSize() const
{
    return d ? d->bufferSize : 0;
}

unsigned int AudioDevice::canWrite() const
{
    if ( !d )
	return 0;
    return bufferSize(); // Any better?
}


int AudioDevice::bytesWritten()
{
    if ( !d )
	return 0;
    MMTIME pmmt = { TIME_BYTES, 0 };
    if ( ( waveOutGetPosition( (HWAVEOUT)d->handle, &pmmt, sizeof(MMTIME) ) != MMSYSERR_NOERROR ) || ( pmmt.wType != TIME_BYTES ) ) {
	qDebug( "failed to get audio device position" );
	return -1;
    }
    return pmmt.u.cb;
}

