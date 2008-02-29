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

#include "qaudiooutput.h"
#include <qtopianamespace.h>
#include <qtopialog.h>
#include <qfile.h>
#if !defined(MEDIA_SERVER) && defined(Q_WS_QWS)
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
    \mainclass
    \brief The QAudioOutput class provides an interface for sending live audio samples to the default audio output device.

    The QAudioOutput class provides an interface for sending live audio
    samples to the default output device.  It is intended for use by
    media streaming applications that can produce a stream of 8-bit unsigned
    or 16-bit signed audio samples.  The sound output will be mixed with
    audio data from other applications.

    The usual sequence for opening the default audio output device is as follows:

    \code
    QAudioOutput *audio = new QAudioOutput();
    audio->setFrequency( 11025 );
    audio->setChannels( 1 );
    audio->setBitsPerSample( 8 );
    audio->open( QIODevice::WriteOnly );
    \endcode

    \sa QAudioInput
    \ingroup multimedia
*/

#ifdef USE_OSS

// Conversion handler for an audio output format.  Format handlers
// are responsible for converting from the supplied sample format
// to the device's supported encoding.
class AudioOutputFormatHandler
{
public:
    virtual ~AudioOutputFormatHandler() { }

    virtual unsigned int writeSize( unsigned int length ) = 0;

    virtual int convert( short *output, const short *input, int length ) = 0;
};

class MToS16AudioOutputFormatHandler : public AudioOutputFormatHandler
{
public:
    unsigned int writeSize( unsigned int length ) { return length * 2; }

    int convert( short *output, const short *input, int length );
};

int MToS16AudioOutputFormatHandler::convert( short *output, const short *input, int length )
{
    int samples = length / 2;

    for (int i = 0; i < samples; i++) {
        output[2*i + 0] = input[i];
        output[2*i + 1] = input[i];
    }
    return 2*length;
}

#endif

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
        handler = NULL;
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

        if (qtopiaLogEnabled("QAudioOutput")) {
            int                  dir;
            unsigned int         vval, vval2;
	    snd_pcm_uframes_t    size;
            snd_pcm_uframes_t    frames;

            qLog(QAudioOutput) << "PCM handle name =" << snd_pcm_name(handle);
            qLog(QAudioOutput) << "PCM state =" << snd_pcm_state_name(snd_pcm_state(handle));
            snd_pcm_hw_params_get_access(hwparams,(snd_pcm_access_t *) &vval);
            qLog(QAudioOutput) << "access type =" << snd_pcm_access_name((snd_pcm_access_t)vval);
            snd_pcm_hw_params_get_format(hwparams, (snd_pcm_format_t *) &vval);
            qLog(QAudioOutput) << QString("format = '%1' (%2)").arg(snd_pcm_format_name((snd_pcm_format_t)vval))
                                                               .arg(snd_pcm_format_description((snd_pcm_format_t)vval))
                                                               .toLatin1().constData();
            snd_pcm_hw_params_get_subformat(hwparams,(snd_pcm_subformat_t *)&vval);
            qLog(QAudioOutput) << QString("subformat = '%1' (%2)").arg(snd_pcm_subformat_name((snd_pcm_subformat_t)vval))
                                                                  .arg(snd_pcm_subformat_description((snd_pcm_subformat_t)vval))
                                                                  .toLatin1().constData();
            snd_pcm_hw_params_get_channels(hwparams, &vval);
            qLog(QAudioOutput) << "channels =" << vval;
            snd_pcm_hw_params_get_rate(hwparams, &vval, &dir);
            qLog(QAudioOutput) << "rate =" << vval << "bps";
            snd_pcm_hw_params_get_period_time(hwparams,&vval, &dir);
            qLog(QAudioOutput) << "period time =" << vval << "us";
            snd_pcm_hw_params_get_period_size(hwparams,&frames, &dir);
            qLog(QAudioOutput) << "period size =" << (int)frames << "frames";
            snd_pcm_hw_params_get_buffer_time(hwparams,&vval, &dir);
            qLog(QAudioOutput) << "buffer time =" << vval << "us";
            snd_pcm_hw_params_get_buffer_size(hwparams,(snd_pcm_uframes_t *) &vval);
            qLog(QAudioOutput) << "buffer size =" << vval << "frames";
            snd_pcm_hw_params_get_periods(hwparams, &vval, &dir);
            qLog(QAudioOutput) << "periods per buffer =" << vval << "frames";
            snd_pcm_hw_params_get_rate_numden(hwparams, &vval, &vval2);
            qLog(QAudioOutput) << QString("exact rate = %1/%2 bps").arg(vval).arg(vval2).toLatin1().constData();
            val = snd_pcm_hw_params_get_sbits(hwparams);
            qLog(QAudioOutput) << "significant bits =" << vval;
            snd_pcm_hw_params_get_tick_time(hwparams,&vval, &dir);
            qLog(QAudioOutput) << "tick time =" << vval << "us";
            vval = snd_pcm_hw_params_is_batch(hwparams);
            qLog(QAudioOutput) << "is batch =" << vval;
            vval = snd_pcm_hw_params_is_block_transfer(hwparams);
            qLog(QAudioOutput) << "is block transfer =" << vval;
            vval = snd_pcm_hw_params_is_double(hwparams);
            qLog(QAudioOutput) << "is double =" << vval;
            vval = snd_pcm_hw_params_is_half_duplex(hwparams);
            qLog(QAudioOutput) << "is half duplex =" << vval;
            vval = snd_pcm_hw_params_is_joint_duplex(hwparams);
            qLog(QAudioOutput) << "is joint duplex =" << vval;
            vval = snd_pcm_hw_params_can_overrange(hwparams);
            qLog(QAudioOutput) << "can overrange =" << vval;
            vval = snd_pcm_hw_params_can_mmap_sample_resolution(hwparams);
            qLog(QAudioOutput) << "can mmap =" << vval;
            vval = snd_pcm_hw_params_can_pause(hwparams);
            qLog(QAudioOutput) << "can pause =" << vval;
            vval = snd_pcm_hw_params_can_resume(hwparams);
            qLog(QAudioOutput) << "can resume =" << vval;
            vval = snd_pcm_hw_params_can_sync_start(hwparams);
            qLog(QAudioOutput) << "can sync start =" << vval;
        }

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
	        qWarning("error opening audio devices /dev/dsp and /dev/dsp1, sending data to /dev/null instead" );
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

        if ( channels == 1 && chans == 2 ) {
            handler = new MToS16AudioOutputFormatHandler;
        }
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
        if ( handler ) {
            delete handler;
            handler = NULL;
        }
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
            int convertedLen;
            char *convertedData;

            if ( handler ) {
                convertedLen = handler->writeSize( len );
                convertedData = new char[convertedLen];

                handler->convert( (short *)convertedData, (const short *)data, len );
            } else {
                convertedLen = len;
                convertedData = (char *)data;
            }

            while ( ::write( fd, convertedData, convertedLen ) < 0 ) {
                if ( errno == EINTR || errno == EWOULDBLOCK )
                    continue;
                perror( "write to audio device" );
                ::close( fd );
                fd = -1;
                break;
            }

            if ( handler ) {
                delete[] convertedData;
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
    AudioOutputFormatHandler *handler;
#endif
};

/*!
    Construct a new audio output stream and attach it to \a parent.
    The default parameters are 44100 Hz Stereo, with 16-bit samples.

    The device parameter is implementation-specific, and might
    not be honored by all implementations.  It is usually an Alsa
    device name such as \c{plughw:0,0}.  The string \c{default}
    can be passed for \a device if the client application wishes
    to use the default device and is not concerned with what
    that default device may be called.
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

/*!
    Destroy this audio output stream.
*/
QAudioOutput::~QAudioOutput()
{
    delete d;
}

/*!
    Returns the current frequency of audio samples.  The default value is 44100.

    \sa setFrequency()
*/
int QAudioOutput::frequency() const
{
    return d->frequency;
}

/*!
    Sets the frequency of audio samples to \a value.  Should be called
    before open().

    \sa frequency()
*/
void QAudioOutput::setFrequency( int value )
{
    d->frequency = value;
}

/*!
    Returns the number of playback channels.  The default value is 2.

    \sa setChannels()
*/
int QAudioOutput::channels() const
{
    return d->channels;
}

/*!
    Sets the number of playback channels to \a value.  Should be called
    before open().

    \sa channels()
*/
void QAudioOutput::setChannels( int value )
{
    d->channels = value;
}

/*!
    Returns the number of bits per sample (8 or 16).  If the value is 16, the samples
    must be signed and in host byte order.  If bitsPerSample() is 8, the samples must
    be unsigned.

    \sa setBitsPerSample()
*/
int QAudioOutput::bitsPerSample() const
{
    return d->bitsPerSample;
}

/*!
    Sets the number of bits per sample to \a value (8 or 16).  Should be
    called before open().  If the value is 16, the samples must be signed
    and in host byte order.  If bitsPerSample() is 8, the samples must be unsigned.

    \sa bitsPerSample()
*/
void QAudioOutput::setBitsPerSample( int value )
{
    d->bitsPerSample = value;
}

/*!
    Opens this audio output stream in \a mode.
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
    Closes this audio output stream.
*/
void QAudioOutput::close()
{
    d->close();
    setOpenMode( NotOpen );
}

/*!
    Determines if this QIODevice is sequential.  Always returns true.
*/
bool QAudioOutput::isSequential() const
{
    return true;
}

/*!
    Reads up to \a maxlen bytes into \a data.  Not used for audio
    output devices.
*/
qint64 QAudioOutput::readData( char *, qint64 )
{
    // Cannot read from audio output devices.
    return 0;
}

/*!
    Writes \a len bytes from \a data to the audio output stream.

    If bitsPerSample() is 16, the samples must be signed and in host byte order.
    If bitsPerSample() is 8, the samples must be unsigned.
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
