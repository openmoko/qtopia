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

#if ( defined Q_WS_QWS || defined(_WS_QWS_) ) && !defined(QT_NO_COP)
#include "qpe/qcopenvelope_qws.h"
#endif

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


#if defined(QT_QWS_IPAQ)
static const int sound_fragment_shift = 14;
#else
static const int sound_fragment_shift = 16;
#endif
static const int sound_fragment_bytes = (1<<sound_fragment_shift);


class AudioDevicePrivate {
public:
    int handle;
    unsigned int frequency;
    unsigned int channels;
    unsigned int bytesPerSample;
    unsigned int bufferSize;

    bool can_GETOSPACE;
    char* unwrittenBuffer;
    unsigned int unwritten;

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
#if 0
    int mixerHandle = open( "/dev/mixer", O_RDWR );
    if ( mixerHandle >= 0 ) {
        ioctl( mixerHandle, MIXER_READ(0), &volume );
        close( mixerHandle );
    } else
	qDebug( "get volume of audio device failed" );
    leftVolume  = ((volume & 0x00FF) << 16) / 100;
    rightVolume = ((volume & 0xFF00) <<  8) / 100;
#else
    Config cfg("Sound");
    cfg.setGroup("System");
    volume = cfg.readNumEntry("Volume",80);
    leftVolume  = ((volume & 0x00FF) << 16) / 100;
    rightVolume = leftVolume;
#endif
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
    // Volume can be from 0 to 100 which is 101 distinct values
    unsigned int rV = (rightVolume * 101) / 65535;

#if 0
    unsigned int lV = (leftVolume  * 101) >> 16;
    unsigned int volume = ((rV << 8) & 0xFF00) | (lV & 0x00FF);
    int mixerHandle = 0;
    if ( ( mixerHandle = open( "/dev/mixer", O_RDONLY ) ) >= 0 ) {
        ioctl( mixerHandle, MIXER_WRITE(0), &volume );
        close( mixerHandle );
    } else
	qDebug( "set volume of audio device failed" );
#else
    // This is the way this has to be done now I guess, doesn't allow for
    // independant right and left channel setting, or setting for different outputs
    Config cfg("Sound");
    cfg.setGroup("System");
    cfg.writeEntry("Volume",(int)rV);
#endif

//    qDebug( "setting volume to: 0x%x", volume );
#if ( defined Q_WS_QWS || defined(_WS_QWS_) ) && !defined(QT_NO_COP)
    // Send notification that the volume has changed
    QCopEnvelope( "QPE/System", "volumeChange(bool)" ) << AudioDevicePrivate::muted; 
#endif
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
    if ( ioctl( d->handle, SNDCTL_DSP_CHANNELS, &d->channels ) == -1 ) {
	d->channels = ( d->channels == 1 ) ? 2 : d->channels;
	ioctl( d->handle, SNDCTL_DSP_CHANNELS, &d->channels );
    }
    ioctl( d->handle, SNDCTL_DSP_SPEED, &d->frequency );

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
}
    

void AudioDevice::close()
{
    if ( !d )
	return;
#ifndef KEEP_DEVICE_OPEN 
    close( d->handle );			// Now it should be safe to shut the handle
#endif
    delete [] d->unwrittenBuffer;
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
    int t = ::write( d->handle, buffer, length );
    if ( t<0 ) t = 0;
    if ( t != (int)length) {
	qDebug("Ahhh!! memcpys 1");
	memcpy(d->unwrittenBuffer,buffer+t,length-t);
	d->unwritten = length-t;
    }
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
}


int AudioDevice::bytesWritten()
{
    if ( !d )
	return 0;
    int buffered = 0;
    if ( ioctl( d->handle, SNDCTL_DSP_GETODELAY, &buffered ) ) {
	qDebug( "failed to get audio device position" );
	return -1;
    }
    return buffered;
}

