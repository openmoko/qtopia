/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include "qaudiooutput.h"
#include <qtopianamespace.h>
#include <qfile.h>
#ifndef MEDIA_SERVER
    #define USE_QSS
    #include <qsoundqss_qws.h>
#else
    #ifdef HAVE_ALSA
	#include <alsa/asoundlib.h>
	#define USE_ALSA
    #else
	#include <sys/ioctl.h>
	#include <sys/soundcard.h>
	#define USE_OSS
    #endif
#endif
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

/*!
    \class QAudioOutput
    \brief The QAudioOutput class provides an interface for sending live audio samples to the default output device.

    The QAudioOutput class provides an interface for sending live audio
    samples to the default output device.  It is intended for use by
    media streaming applications that can produce a stream of 8-bit unsigned
    or 16-bit signed audio samples.  The sound output will be mixed with
    audio data from other applications.

    \sa QAudioInput
*/

class QAudioOutputPrivate
{
public:
    QAudioOutputPrivate(const QByteArray &device)
    {
        frequency = 44100;
        channels = 2;
        bitsPerSample = 16;
        m_device = device;

#ifdef USE_ALSA
        handle = 0;
#endif
#ifdef USE_QSS
        static int unique = 0;
        isopen = false;
        started = false;
        id = ((int)::getpid()) + unique++ * 100000;
        mediapipe = Qtopia::tempDir() + "mediapipe-" + QString::number(id);
        pipeFd = -1;
        if (!QFile::exists( mediapipe )) {
            if ( mknod( mediapipe.toLatin1().constData(), S_IFIFO | 0666, 0 ) )
                perror("mknod");
        }
#ifdef USE_OSS
        fd = -1;
#endif
#endif
    }
    ~QAudioOutputPrivate()
    {
        close();
    }

    bool open()
    {
#ifdef USE_ALSA
        // Open the Alsa playback device.
        int err;
        if ( ( err = snd_pcm_open
               ( &handle, m_device.constData(), SND_PCM_STREAM_PLAYBACK, 0 ) ) < 0 ) {
            qWarning( "QAudioOuput: snd_pcm_open: error %d", err );
            return false;
        }
        snd_pcm_nonblock( handle, 0 );

        // Set the desired parameters.
        snd_pcm_hw_params_t *hwparams;
        snd_pcm_hw_params_alloca( &hwparams );
        snd_pcm_hw_params_any( handle, hwparams );
        snd_pcm_hw_params_set_access
            ( handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED );
        snd_pcm_hw_params_set_format
            ( handle, hwparams,
             ( bitsPerSample == 16 ? SND_PCM_FORMAT_S16
                                   : SND_PCM_FORMAT_U8 ) );
        snd_pcm_hw_params_set_channels
            ( handle, hwparams, (unsigned int)channels );
        snd_pcm_hw_params_set_rate
            ( handle, hwparams, (unsigned int)frequency, 0 );

        // Set buffer and period sizes based on a 20ms block size.
        int samplesPerBlock = frequency * channels / 50;
        snd_pcm_uframes_t val = (snd_pcm_uframes_t)( samplesPerBlock / channels );
        snd_pcm_hw_params_set_period_size_near(handle, hwparams, &val, 0);
        val = (snd_pcm_uframes_t)( samplesPerBlock / channels * 2 );
        snd_pcm_hw_params_set_buffer_size_near(handle, hwparams, &val);

        snd_pcm_hw_params( handle, hwparams );

        //snd_pcm_uframes_t chunk_size;
        //snd_pcm_hw_params_get_period_size(hwparams, &chunk_size, 0);

        // Prepare for audio output.
        snd_pcm_prepare( handle );
#endif
#ifdef USE_QSS
        isopen = true;
        client.playRaw( id, mediapipe, frequency, channels,
                        bitsPerSample, QWSSoundClient::Streaming );
        pipeFd = ::open( mediapipe.toLatin1().constData() , O_WRONLY );
        if ( pipeFd < 0 ) {
            qWarning("error opening audio device pipe %s, sending data to /dev/null instead", mediapipe.toLatin1().constData());
            pipeFd = ::open("/dev/null", O_WRONLY);
        }
#endif
#ifdef USE_OSS

        // Open the device.
        if ( ( fd = ::open( "/dev/dsp", O_WRONLY ) ) < 0 ) {
            if ( ( fd = ::open( "/dev/dsp1", O_WRONLY ) ) < 0 ) {
	        qDebug( "error opening audio devices /dev/dsp and /dev/dsp1, sending data to /dev/null instead" );
	        fd = ::open( "/dev/null", O_WRONLY );
            }
        }
        fcntl( fd, F_SETFD, 1 );

        // Set the requested audio parameters.
        int format, freq, chans;
        if ( bitsPerSample == 8 )
            format = AFMT_U8;
        else
            format = AFMT_S16_NE;
        freq = frequency;
        chans = channels;
        if ( ::ioctl( fd, SNDCTL_DSP_SETFMT, &format ) < 0 )
            perror( "SNDCTL_DSP_SETFMT" );
        if ( ::ioctl( fd, SNDCTL_DSP_SPEED, &freq ) < 0 )
            perror( "SNDCTL_DSP_SPEED" );
        if ( ::ioctl( fd, SNDCTL_DSP_CHANNELS, &chans ) < 0 )
            perror( "SNDCTL_DSP_CHANNELS" );
#endif
        return true;
    }

    void close()
    {
#ifdef USE_ALSA
        if ( handle ) {
            snd_pcm_drain( handle );
            snd_pcm_close( handle );
            handle = 0;
        }
#endif
#ifdef USE_QSS
        if ( isopen ) {
            if ( pipeFd != -1 )
                ::close( pipeFd );
            client.stop( id );
            isopen = false;
            started = false;
            pending.clear();
            QFile::remove( mediapipe );
        }
#endif
#ifdef USE_OSS
        if ( fd != -1 ) {
            ::close( fd );
            fd = -1;
        }
#endif
    }

    void startOutput()
    {
#ifdef USE_QSS
        if ( isopen && pipeFd != -1 && pending.size() > 0 ) {
            if ( ::write( pipeFd, pending.constData(), pending.size() ) < 0 ) {
                perror( "write to audio pipe" );
                ::close( pipeFd );
                pipeFd = -1;
            }
            pending.clear();
            started = true;
        }
#endif
    }

    void write( const char *data, qint64 len )
    {
#ifdef USE_ALSA
        if ( !handle )
            return;
        while ( len > 0 ) {
            int frames = snd_pcm_bytes_to_frames( handle, (int)len );
            int err = snd_pcm_writei( handle, data, frames );
            if ( err >= 0 ) {
                int bytes = snd_pcm_frames_to_bytes( handle, err );
                data += bytes;
                len -= bytes;
            } else {
                qWarning( "snd_pcm_writei failed with %d", err );
                if ( (err != -EAGAIN) && (err != -EPIPE) && (err != -EINTR) ) {
                    // A non-recoverable error has occurred.
                    close();
                    break;
                } else {
                    if ((err = snd_pcm_prepare (handle)) < 0) {
                        close();
                        break;
                    }
                }
            }
        }
#else
#ifdef USE_QSS
        if ( !isopen )
            return;
        if ( !started ) {
            // Queue the data until the pipe has been accepted by qss.
            int size = pending.size();
            pending.resize( size + (int)len );
            memcpy( pending.data() + size, data, (int)len );
        } else if ( pipeFd != -1 && len > 0 ) {
            if ( ::write( pipeFd, data, (int)len ) < 0 ) {
                perror( "write to audio pipe" );
                ::close( pipeFd );
                pipeFd = -1;
            }
        }
#else
#ifdef USE_OSS
        if ( fd != -1 ) {
            while ( ::write( fd, data, (int)len ) < 0 ) {
                if ( errno == EINTR || errno == EWOULDBLOCK )
                    continue;
                perror( "write to audio device" );
                ::close( fd );
                fd = -1;
                break;
            }
        }
#else
        Q_UNUSED(data)
        Q_UNUSED(len)
#endif
#endif
#endif
    }

    int frequency;
    int channels;
    int bitsPerSample;
    QByteArray m_device;
#ifdef USE_ALSA
    snd_pcm_t *handle;
#endif
#ifdef USE_QSS
    bool isopen;
    bool started;
    int id;
    QString mediapipe;
    int pipeFd;
    QWSSoundClient client;
    QByteArray pending;
#endif
#ifdef USE_OSS
    int fd;
#endif
};

/*!
    Construct a new audio output stream and attach it to \a parent.
    The default parameters are 44100 Hz Stereo, with 16-bit samples.
    The device to be opened is specified by \a device.  The device
    parameter is implementation specific, and might not be honored
    by all implementations.
 */
QAudioOutput::QAudioOutput( const QByteArray &device, QObject *parent )
    : QIODevice( parent )
{
    d = new QAudioOutputPrivate(device);
#ifdef USE_QSS
    connect( &(d->client), SIGNAL(deviceReady(int)),
               this, SLOT(deviceReady(int)) );
    connect( &(d->client), SIGNAL(deviceError(int,QWSSoundClient::DeviceErrors)),
               this, SLOT(deviceError(int)) );
#endif
}

/*!
    Construct a new audio output stream and attach it to \a parent.
    The default parameters are 44100 Hz Stereo, with 16-bit samples.
*/
QAudioOutput::QAudioOutput( QObject *parent )
    : QIODevice( parent )
{
    d = new QAudioOutputPrivate("default");
#ifdef USE_QSS
    connect( &(d->client), SIGNAL(deviceReady(int)),
             this, SLOT(deviceReady(int)) );
    connect( &(d->client), SIGNAL(deviceError(int,QWSSoundClient::DeviceErrors)),
             this, SLOT(deviceError(int)) );
#endif
}

QAudioOutput::~QAudioOutput()
{
    delete d;
}

/*!
    Get the current frequency of audio samples.  The default value is 44100.
*/
int QAudioOutput::frequency() const
{
    return d->frequency;
}

/*!
    Set the frequency of audio samples to \a value.  Should be called
    before open().
*/
void QAudioOutput::setFrequency( int value )
{
    d->frequency = value;
}

/*!
    Get the number of playback channels.  The default value is 2.
*/
int QAudioOutput::channels() const
{
    return d->channels;
}

/*!
    Set the number of playback channels to \a value.  Should be called
    before open().
*/
void QAudioOutput::setChannels( int value )
{
    d->channels = value;
}

/*!
    Get the number of bits per sample (8 or 16).  The default value is 16.
*/
int QAudioOutput::bitsPerSample() const
{
    return d->bitsPerSample;
}

/*!
    Set the number of bits per sample to \a value (8 or 16).  Should be
    called before open().
*/
void QAudioOutput::setBitsPerSample( int value )
{
    d->bitsPerSample = value;
}

/*!
    Open this audio output stream in \a mode.
*/
bool QAudioOutput::open( QIODevice::OpenMode mode )
{
    if ( isOpen() )
        return false;
    if ( !d->open() )
        return false;
    setOpenMode( mode | QIODevice::Unbuffered );
    return true;
}

/*!
    Close this audio output stream.
*/
void QAudioOutput::close()
{
    d->close();
    setOpenMode( NotOpen );
}

/*!
    Determine if this QIODevice is sequential.  Always returns true.
*/
bool QAudioOutput::isSequential() const
{
    return true;
}

/*!
    Read up to \a maxlen bytes into \a data.  Not used for audio
    output devices.
*/
qint64 QAudioOutput::readData( char *, qint64 )
{
    // Cannot read from audio output devices.
    return 0;
}

/*!
    Write \a len bytes from \a data to the audio output stream.
*/
qint64 QAudioOutput::writeData( const char *data, qint64 len )
{
    if ( !isOpen() )
        return len;
    d->write( data, len );
    return len;
}

void QAudioOutput::deviceReady( int id )
{
#ifdef USE_QSS
    if ( id == d->id )
        d->startOutput();
#else
    Q_UNUSED(id)
#endif
}

void QAudioOutput::deviceError( int id )
{
#ifdef USE_QSS
    if ( id == d->id )
        d->close();
#else
    Q_UNUSED(id)
#endif
}
