/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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

#if ( defined Q_WS_QWS || defined(_WS_QWS_) ) && !defined(QT_NO_COP)
#include "qpe/qcopenvelope_qws.h"
#endif

#ifdef Q_WS_WIN
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#endif

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#if defined(Q_OS_WIN32)
static const int expectedBytesPerMilliSecond = 2 * 2 * 44000 / 1000;
static const int timerResolutionMilliSeconds = 30;
static const int sound_fragment_bytes = timerResolutionMilliSeconds * expectedBytesPerMilliSecond;
#else
# if defined(QT_QWS_IPAQ)
static const int sound_fragment_shift = 14;
# else
static const int sound_fragment_shift = 16;
# endif
static const int sound_fragment_bytes = (1<<sound_fragment_shift);
#endif


class AudioDevicePrivate {
public:
    int handle;
    unsigned int frequency;
    unsigned int channels;
    unsigned int bytesPerSample;
    unsigned int bufferSize;
#ifndef Q_OS_WIN32
    bool can_GETOSPACE;
    char* unwrittenBuffer;
    unsigned int unwritten;
#endif

    static int dspFd;
    static bool muted;
    static unsigned int leftVolume;
    static unsigned int rightVolume;
};


#ifdef Q_WS_QWS
// This is for keeping the device open in-between playing files when
// the device makes clicks and it starts to drive you insane! :)
// Best to have the device not open when not using it though
//#define KEEP_DEVICE_OPEN
#endif


int AudioDevicePrivate::dspFd = 0;
bool AudioDevicePrivate::muted = FALSE;
unsigned int AudioDevicePrivate::leftVolume = 0;
unsigned int AudioDevicePrivate::rightVolume = 0;


void AudioDevice::getVolume( unsigned int& leftVolume, unsigned int& rightVolume, bool &muted ) {
    muted = AudioDevicePrivate::muted;
    unsigned int volume;
#ifdef Q_OS_WIN32
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
#else
# if 0
    int mixerHandle = open( "/dev/mixer", O_RDWR );
    if ( mixerHandle >= 0 ) {
        ioctl( mixerHandle, MIXER_READ(0), &volume );
        close( mixerHandle );
    } else
	qDebug( "get volume of audio device failed" );
    leftVolume  = ((volume & 0x00FF) << 16) / 100;
    rightVolume = ((volume & 0xFF00) <<  8) / 100;
# else
    Config cfg("Sound");
    cfg.setGroup("System");
    volume = cfg.readNumEntry("Volume",0);
    leftVolume  = ((volume & 0x00FF) << 16) / 100;
    rightVolume = leftVolume;
# endif
#endif
}


void AudioDevice::setVolume( unsigned int leftVolume, unsigned int rightVolume, bool muted ) {
    AudioDevicePrivate::muted = muted;
    if ( muted ) {
	AudioDevicePrivate::leftVolume = leftVolume;
	AudioDevicePrivate::rightVolume = rightVolume;
	leftVolume = 0;
	rightVolume = 0;
    } else {
        leftVolume  = ( (int) leftVolume < 0 ) ? 0 : ((  leftVolume > 0xFFFF ) ? 0xFFFF :  leftVolume );
	rightVolume = ( (int)rightVolume < 0 ) ? 0 : (( rightVolume > 0xFFFF ) ? 0xFFFF : rightVolume );
    }
#ifdef Q_OS_WIN32
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
#else
    // Volume can be from 0 to 100 which is 101 distinct values
    unsigned int rV = (rightVolume * 101) / 65535;

# if 0
    unsigned int lV = (leftVolume  * 101) >> 16;
    unsigned int volume = ((rV << 8) & 0xFF00) | (lV & 0x00FF);
    int mixerHandle = 0;
    if ( ( mixerHandle = open( "/dev/mixer", O_RDONLY ) ) >= 0 ) {
        ioctl( mixerHandle, MIXER_WRITE(0), &volume );
        close( mixerHandle );
    } else
	qDebug( "set volume of audio device failed" );
# else
    // This is the way this has to be done now I guess, doesn't allow for
    // independant right and left channel setting, or setting for different outputs
    Config cfg("Sound");
    cfg.setGroup("System");
    cfg.writeEntry("Volume",(int)rV);
# endif

#endif
//    qDebug( "setting volume to: 0x%x", volume );
#if ( defined Q_WS_QWS || defined(_WS_QWS_) ) && !defined(QT_NO_COP)
    // Send notification that the volume has changed
    QCopEnvelope( "QPE/System", "volumeChange(bool)" ) << muted; 
#endif
}




AudioDevice::AudioDevice( unsigned int f, unsigned int chs, unsigned int bps ) {
    d = new AudioDevicePrivate;
    d->frequency = f;
    d->channels = chs;
    d->bytesPerSample = bps;

    connect( qApp, SIGNAL( volumeChanged(bool) ), this, SLOT( volumeChanged(bool) ) );

#ifdef Q_OS_WIN32
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
	QString errorMsg = "error opening audio device.\nReason: %i - ";
	switch (result) {
	    case MMSYSERR_ALLOCATED:	errorMsg += "Specified resource is already allocated."; break;
	    case MMSYSERR_BADDEVICEID:	errorMsg += "Specified device identifier is out of range."; break;
	    case MMSYSERR_NODRIVER:	errorMsg += "No device driver is present."; break;
	    case MMSYSERR_NOMEM:	errorMsg += "Unable to allocate or lock memory."; break;
	    case WAVERR_BADFORMAT:	errorMsg += "Attempted to open with an unsupported waveform-audio format."; break;
	    case WAVERR_SYNC:		errorMsg += "The device is synchronous but waveOutOpen was called without using the WAVE_ALLOWSYNC flag."; break;
	    default:			errorMsg += "Undefined error"; break;
	}
	qDebug( errorMsg, result );
    }

    d->bufferSize = sound_fragment_bytes;
#else

    int fragments = 0x10000 * 8 + sound_fragment_shift;
    int format = AFMT_S16_LE;
    int capabilities = 0;

#ifdef KEEP_DEVICE_OPEN
    if ( AudioDevicePrivate::dspFd == 0 ) {
#endif
    if ( ( d->handle = ::open( "/dev/dsp", O_WRONLY ) ) < 0 ) {
	qDebug( "error opening audio device /dev/dsp, sending data to /dev/null instead" );
	d->handle = ::open( "/dev/null", O_WRONLY );
    }
#ifdef KEEP_DEVICE_OPEN 
	AudioDevicePrivate::dspFd = d->handle;
    } else {
        d->handle = AudioDevicePrivate::dspFd;
    }
#endif

    ioctl( d->handle, SNDCTL_DSP_GETCAPS, &capabilities );
    ioctl( d->handle, SNDCTL_DSP_SETFRAGMENT, &fragments );
    ioctl( d->handle, SNDCTL_DSP_SETFMT, &format );
    ioctl( d->handle, SNDCTL_DSP_SPEED, &d->frequency );
    if ( ioctl( d->handle, SNDCTL_DSP_CHANNELS, &d->channels ) == -1 ) {
	d->channels = ( d->channels == 1 ) ? 2 : d->channels;
	ioctl( d->handle, SNDCTL_DSP_CHANNELS, &d->channels );
    }

    d->bufferSize = sound_fragment_bytes;
    d->unwrittenBuffer = new char[d->bufferSize];
    d->unwritten = 0;
    d->can_GETOSPACE = TRUE; // until we find otherwise
 
    //if ( chs != d->channels )		    qDebug( "Wanted %d, got %d channels", chs, d->channels );
    //if ( f != d->frequency )		    qDebug( "wanted %dHz, got %dHz", f, d->frequency );
    //if ( capabilities & DSP_CAP_BATCH )   qDebug( "Sound card has local buffer" );
    //if ( capabilities & DSP_CAP_REALTIME )qDebug( "Sound card has realtime sync" );
    //if ( capabilities & DSP_CAP_TRIGGER ) qDebug( "Sound card has precise trigger" );
    //if ( capabilities & DSP_CAP_MMAP )    qDebug( "Sound card can mmap" );
#endif
}
    

AudioDevice::~AudioDevice() {
#ifdef Q_OS_WIN32
    waveOutClose( (HWAVEOUT)d->handle );
#else
# ifndef KEEP_DEVICE_OPEN 
    close( d->handle );			// Now it should be safe to shut the handle
# endif
    delete [] d->unwrittenBuffer;
    delete d;
#endif
}


void AudioDevice::volumeChanged( bool muted )
{
    AudioDevicePrivate::muted = muted;
}


void AudioDevice::write( char *buffer, unsigned int length )
{
#ifdef Q_OS_WIN32
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
#else
    int t = ::write( d->handle, buffer, length );
    if ( t<0 ) t = 0;
    if ( t != (int)length) {
	qDebug("Ahhh!! memcpys 1");
	memcpy(d->unwrittenBuffer,buffer+t,length-t);
	d->unwritten = length-t;
    }
#endif
}


unsigned int AudioDevice::channels() const
{
    return d->channels;
}


unsigned int AudioDevice::frequency() const
{
    return d->frequency;
}


unsigned int AudioDevice::bytesPerSample() const
{
    return d->bytesPerSample;
}


unsigned int AudioDevice::bufferSize() const
{
    return d->bufferSize;
}

unsigned int AudioDevice::canWrite() const
{
#ifdef Q_OS_WIN32
    return bufferSize(); // Any better?
#else
    audio_buf_info info;
    if ( d->can_GETOSPACE && ioctl(d->handle,SNDCTL_DSP_GETOSPACE,&info) ) {
	d->can_GETOSPACE = FALSE;
	fcntl( d->handle, F_SETFL, O_NONBLOCK );
    }
    if ( d->can_GETOSPACE ) {
	int t = info.fragments * sound_fragment_bytes;
	return QMIN(t,(int)bufferSize());
    } else {
	if ( d->unwritten ) {
	    int t = ::write( d->handle, d->unwrittenBuffer, d->unwritten );
	    if ( t<0 ) t = 0;
	    if ( (unsigned)t!=d->unwritten ) {
		memcpy(d->unwrittenBuffer,d->unwrittenBuffer+t,d->unwritten-t);
		d->unwritten -= t;
	    } else {
		d->unwritten = 0;
	    }
	}
	if ( d->unwritten )
	    return 0;
	else
	    return d->bufferSize;
    }
#endif
}


int AudioDevice::bytesWritten() {
#ifdef Q_OS_WIN32
    MMTIME pmmt = { TIME_BYTES, 0 };
    if ( ( waveOutGetPosition( (HWAVEOUT)d->handle, &pmmt, sizeof(MMTIME) ) != MMSYSERR_NOERROR ) || ( pmmt.wType != TIME_BYTES ) ) {
	qDebug( "failed to get audio device position" );
	return -1;
    }
    return pmmt.u.cb;
#else
    int buffered = 0;
    if ( ioctl( d->handle, SNDCTL_DSP_GETODELAY, &buffered ) ) {
	qDebug( "failed to get audio device position" );
	return -1;
    }
    return buffered;
#endif
}

