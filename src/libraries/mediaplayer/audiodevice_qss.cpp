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
#define QTOPIA_INTERNAL_FILEOPERATIONS

#include <stdlib.h>
#include <errno.h>
#include <sched.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qfile.h>
#include <qtimer.h>
#include <qsoundqss_qws.h>
#include "audiodevice.h"
#include "mutex.h"

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif


/*!
  \class AudioDevice audiodevice.h
  \brief The AudioDevice class abstracts accessing the audio hardware.

    This class is now a loose wrapper around QWSSoundClient which is
    the more direct and recommended way to access the audio hardware
    in Qtopia.

    This class is currently private to the music and videos applications,
    its API may change and binary compatibilty may change in later
    versions.
*/


#define AUDIO_BUFFER_SIZE     (1<<14)


class AudioDevicePrivate : public QObject {
    Q_OBJECT
public:
    AudioDevicePrivate(QObject *parent, unsigned int f, unsigned int chs, unsigned int bps, bool needSignal);
    ~AudioDevicePrivate();

    int pipeFd;
    QString mediaPipeStr;
    const char *mediapipe;
    unsigned int frequency;
    unsigned int channels;
    unsigned int bytesPerSample;
    bool needCompletedSignal;

    static bool muted;
    static int leftVolume;
    static int rightVolume;
    static Mutex mutex;
    static Mutex volMutex;
    static Mutex fdMutex;

    QWSSoundClient c;

    bool ready;
public slots:
    void error(int);
    void clientConnected();
    void disconnected();
    void pipeReady(int);
    void deviceError(int i, QWSSoundClient::DeviceErrors err);
    void sendCompletedIO();
signals:
    void completedIO();
    void deviceReady();
    void deviceOpenError();
};


void AudioDevicePrivate::sendCompletedIO()
{
    emit completedIO();
}


void AudioDevicePrivate::error(int i)
{
    qDebug("AudioDevice recieved %i error from QSS", i);
    AutoLockUnlockMutex lock(&mutex);
    ready = false;
    emit deviceOpenError();
}


void AudioDevicePrivate::disconnected()
{
    qDebug("AudioDevice disconnected from QSS");
    AutoLockUnlockMutex lock(&mutex);
    ready = false;
    // XXX need to be restart connection process again
}


void AudioDevicePrivate::clientConnected()
{
    qDebug("AudioDevice connected to QSS");
    AutoLockUnlockMutex lock(&mutex);
    AutoLockUnlockMutex fdLock(&fdMutex);

    c.playRaw((int)::getpid(), mediapipe, channels, frequency, bytesPerSample*8, QWSSoundClient::Streaming);
    pipeFd = ::open((const char *)mediapipe, O_WRONLY);
    if ( pipeFd < 0 ) {
	qDebug("error opening audio device pipe, sending data to /dev/null instead");
	pipeFd = ::open("/dev/null", O_WRONLY);
    }
}


void AudioDevicePrivate::pipeReady(int i)
{
    qDebug("Pipe file ready for writing (for pid %i)", i);
    AutoLockUnlockMutex lock(&mutex);
    if ( i == (int)::getpid() ) {
        ready = true;
	emit deviceReady();
    }
}


void AudioDevicePrivate::deviceError(int i, QWSSoundClient::DeviceErrors err)
{
    qDebug("AudioDevice recieved %i device error for pid %i from QSS", (int)err, i);
    //AutoLockUnlockMutex lock(&mutex);
    if ( err == QWSSoundClient::ErrOpeningAudioDevice ) {
        qDebug("Error opening sound device");
        ready = false;
	emit deviceOpenError();
    } else if ( err == QWSSoundClient::ErrOpeningFile ) {
        qDebug("Error opening pipe");
	if ( i == (int)::getpid() ) {
	    ready = false;
	    emit deviceOpenError();
	}
    }
}


AudioDevicePrivate *globalDevPriv = 0;
Mutex AudioDevicePrivate::mutex;
Mutex AudioDevicePrivate::volMutex;
Mutex AudioDevicePrivate::fdMutex;
bool AudioDevicePrivate::muted = false;
int AudioDevicePrivate::leftVolume = 0xFFFF;
int AudioDevicePrivate::rightVolume = 0xFFFF;


AudioDevicePrivate::AudioDevicePrivate(QObject *parent, unsigned int f,
	unsigned int chs, unsigned int bps, bool needSignal) : QObject(parent)
{
    AutoLockUnlockMutex lock(&mutex);
    ready = false;
    connect( &c, SIGNAL(deviceReady(int)), this, SLOT(pipeReady(int)) );
    connect( &c, SIGNAL(deviceError(int, QWSSoundClient::DeviceErrors)), this, SLOT(deviceError(int, QWSSoundClient::DeviceErrors)) );
    connect( &c, SIGNAL(connected()), this, SLOT(clientConnected()) );
    connect( &c, SIGNAL(connectionClosed()), this, SLOT(disconnected()) );
    connect( &c, SIGNAL(error(int)), this, SLOT(error(int)) );

    globalDevPriv = this; // so set volume works.
    frequency = f;
    channels = chs;
    bytesPerSample = bps;
    needCompletedSignal = needSignal;
    pipeFd = 0;
    mediaPipeStr = Global::tempDir() + "mediapipe-" + parent->name();
    mediapipe = mediaPipeStr.latin1();

    if (!QFile::exists(mediapipe))
	if ( mknod(mediapipe, S_IFIFO | 0666, 0) )
	    perror("mknod");
}


AudioDevicePrivate::~AudioDevicePrivate()
{
    AutoLockUnlockMutex lock(&fdMutex);
    c.stop((int)::getpid());
    if (pipeFd)
	close(pipeFd);
    if ( globalDevPriv == this )
	globalDevPriv = 0;
}


/*!
  Retrieves the volume for the \a left and \a right channels.

  \sa setVolume()
*/
void AudioDevice::volume(int &left, int &right)
{
    AutoLockUnlockMutex lock(&AudioDevicePrivate::volMutex);
    left = AudioDevicePrivate::leftVolume;
    right = AudioDevicePrivate::rightVolume; 
}


/*!
  Sets the volume for the \a left and \a right channels. Volume ranges for setting are from 0 to 65536.

  \sa volume()
*/
void AudioDevice::setVolume(int left, int right)
{
{
    AutoLockUnlockMutex lock(&AudioDevicePrivate::volMutex);
    AudioDevicePrivate::rightVolume = (right < 0 ) ? 0 : ((right > 0xFFFF) ? 0xFFFF : right);
    AudioDevicePrivate::leftVolume  = (left  < 0 ) ? 0 : ((left  > 0xFFFF) ? 0xFFFF :  left);
}

    // Volume can be from 0 to 100 which is 101 distinct values
    int rV = (right * 101) / 65535;
    int lV = (left  * 101) / 65535;

    AutoLockUnlockMutex lock(&AudioDevicePrivate::mutex);
    if (globalDevPriv) globalDevPriv->c.setVolume((int)::getpid(), rV, lV);
}


/*!
  Returns if muting is enabled.

  \sa setMuted()
*/
bool AudioDevice::muted()
{
    AutoLockUnlockMutex lock(&AudioDevicePrivate::volMutex);
    return AudioDevicePrivate::muted;
}


/*!
  Sets device to be muted if \a m  is true.

  \sa muted()
*/
void AudioDevice::setMuted(bool m)
{
    if ( muted() != m ) {
	{
	AutoLockUnlockMutex lock(&AudioDevicePrivate::volMutex);
	AudioDevicePrivate::muted = m;
	}
	AutoLockUnlockMutex lock(&AudioDevicePrivate::mutex);
	if (globalDevPriv) globalDevPriv->c.setMute((int)::getpid(), m);
    }
}


/*!
  Constructs an AudioDevice with \a parent and \a name.

  \sa write(), open()
*/
AudioDevice::AudioDevice(QObject *parent, const char *name) : QObject(parent, name), d(0)
{
    AutoLockUnlockMutex lock(&AudioDevicePrivate::mutex);
    if ( !name )
        qWarning("Audio device pipes need a unique name to function properly");
}


/*!
  Destroys the AudioDevice.
  Volume settings are saved for the mediaplayer.
*/
AudioDevice::~AudioDevice()
{
    //AutoLockUnlockMutex lock(&AudioDevicePrivate::mutex);
}

/*!
  Opens the AudioDevice using \a f as the frequency, \a chs as the channels,
  and \a bps as the number of bytes per sample of data written.
  If \a needCompletedSignal then the completedIO() signal will be emitted 
      when the AudioDevice is ready for more data.    
  Volume for the mediaplayer is read in from its configuration file.
*/
void AudioDevice::open(unsigned int f, unsigned int chs, unsigned int bps, bool needCompletedSignal)
{
    d = new AudioDevicePrivate(this, f, chs, bps, needCompletedSignal);

    AudioDevicePrivate::mutex.lock();
    connect(d, SIGNAL(deviceReady()), this, SIGNAL(deviceReady()));
    connect(d, SIGNAL(deviceOpenError()), this, SIGNAL(deviceOpenError()));
    connect(d, SIGNAL(completedIO()), this, SIGNAL(completedIO()));

    // Get the volume setting for the app
    Config cfg( "MediaPlayer" );
    cfg.setGroup( "Options" );
    AudioDevicePrivate::mutex.unlock();
    setVolume(cfg.readNumEntry("LeftVolume", AudioDevicePrivate::leftVolume),
			cfg.readNumEntry("RightVolume", AudioDevicePrivate::rightVolume ));
    AudioDevicePrivate::mutex.lock();

    // Initialise using the global mute setting
    Config sndCfg("Sound");
    sndCfg.setGroup("System");
    AudioDevicePrivate::mutex.unlock();
    setMuted(sndCfg.readNumEntry("Muted", false));
    AudioDevicePrivate::mutex.lock();
    connect(qApp, SIGNAL(volumeChanged(bool)), this, SLOT(volumeChanged(bool)));
    AudioDevicePrivate::mutex.unlock();
}


void AudioDevice::close()
{
    Config cfg( "MediaPlayer" );
    cfg.setGroup( "Options" );
    {
    AutoLockUnlockMutex lock(&AudioDevicePrivate::volMutex);
    cfg.writeEntry( "LeftVolume", AudioDevicePrivate::leftVolume );
    cfg.writeEntry( "RightVolume", AudioDevicePrivate::rightVolume );
    }

    AutoLockUnlockMutex lock(&AudioDevicePrivate::mutex);
    if ( d ) {
	delete d;
	d = 0;
    }
}


/*!
  Slot to notify AudioDevice of change in global mute setting. 
    Device is muted if \a muted is true.
*/
void AudioDevice::volumeChanged(bool muted)
{
    AudioDevice::setMuted(muted);
}


/*!
  Sends \a length bytes from \a buffer to the audio device. The data is
  to be formatted with the set frequency, channels and bytes per sample.
  The latency depends on implementation, currently QWSSoundClient has
  approximately a latency of 100ms.

  \sa AudioDevice(), channels(), frequency(), bytesPerSample()
*/
bool AudioDevice::write( char *buffer, unsigned int length )
{
    AutoLockUnlockMutex lock(&AudioDevicePrivate::mutex);

    if ( !length ) {
	qDebug("Zero length buffer");
	return false;
    }

    if ( !d || !d->ready ) {
	qDebug("QSS not ready for write");
	return false;
    }

    int pipeFd = d->pipeFd;

    AudioDevicePrivate::mutex.unlock();
    AudioDevicePrivate::fdMutex.lock();
    // qDebug("about to write to audio device, fd: %i ptr: %p len: %i", d->pipeFd, buffer, length);
    int result = ::write(pipeFd, buffer, length);
    // qDebug("wrote to the audio device, result: %i", result);
    AudioDevicePrivate::fdMutex.unlock();
    AudioDevicePrivate::mutex.lock();

    if ( result == -1 ) {
	if ( errno == EPIPE ) {
	    qDebug("Pipe broken!");
	    return false;
	}
    }

    if ( result != (int)length ) {
	qDebug("Failed to write");
	return false;
    }

    if ( d->needCompletedSignal && d->bytesPerSample && d->frequency )
        QTimer::singleShot(length/(d->bytesPerSample*d->frequency), d, SLOT(sendCompletedIO()));

    return true;
}


/*!
  Returns the number of channels data is to be formatted in, as set in the constructor.

  \sa AudioDevice(), write()
*/
unsigned int AudioDevice::channels() const
{
    AutoLockUnlockMutex lock(&AudioDevicePrivate::mutex);
    return d->channels;
}


/*!
  Returns the frequency data is to be formatted in, as set in the constructor.

  \sa AudioDevice(), write()
*/
unsigned int AudioDevice::frequency() const
{
    AutoLockUnlockMutex lock(&AudioDevicePrivate::mutex);
    return d->frequency;
}


/*!
  Returns the bytes per sample data is to be formatted in, as set in the constructor.

  \sa AudioDevice(), write()
*/
unsigned int AudioDevice::bytesPerSample() const
{
    AutoLockUnlockMutex lock(&AudioDevicePrivate::mutex);
    return d->bytesPerSample;
}


/*!
  Retruns a value which is reasonable to write at a time.

  \sa write()
*/
unsigned int AudioDevice::bufferSize() const
{
    // should return the size of the pipe.
    return AUDIO_BUFFER_SIZE;
}


/*!
  Some implementations might be able to tell when it is okay to write more.
*/
unsigned int AudioDevice::canWrite() const
{
    return 0;
}


/*!
  Some implementations might be able to tell how much of the written data has been processed.
*/
int AudioDevice::bytesWritten()
{
    return 0;
}


#include "audiodevice_qss.moc"

