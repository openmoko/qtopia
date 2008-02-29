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
#include <qsocketnotifier.h>
#include "audioinput.h"

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
#include <errno.h>

// The OSS device to record from.
#define	AUDIO_RECORD_SOURCE	SOUND_MIXER_MIC
#endif


// Sound formats, for use by AudioFormatHandler::create().
#ifndef AFMT_U8
#define AFMT_U8                 0x00000008
#define AFMT_S16_LE             0x00000010
#define AFMT_S16_BE             0x00000020
#define AFMT_S8                 0x00000040
#define AFMT_U16_LE             0x00000080
#define AFMT_U16_BE             0x00000100
#endif


// Conversion handler for an audio input format.  Format handlers
// are responsible for converting from the device's encoding to
// normalized 16-bit host byte order samples.
class AudioFormatHandler
{
public:
    virtual ~AudioFormatHandler() { }

    // Get the read size for a specific number of raw sound samples.
    virtual unsigned int readSize( unsigned int length ) = 0;

    // Convert a buffer of audio data into raw 16-bit sound samples.
    // "length" is the number of bytes read from the device.  Returns
    // the number of raw sound samples.
    virtual int convert( short *buffer, int length ) = 0;

    // Create the appropriate conversion handler for a device format.
    static AudioFormatHandler *create( int format );

};


// Convert signed 16-bit samples to normalized raw samples.  This doesn't
// need to do anything, as the incoming data is already normalized.
class S16AudioFormatHandler : public AudioFormatHandler
{
public:

    unsigned int readSize( unsigned int length ) { return length * 2; }
    int convert( short *, int length )           { return length / 2; }

};


// Convert signed 16-bit samples to normalized raw samples by byte-swapping.
class S16SwapAudioFormatHandler : public AudioFormatHandler
{
public:

    unsigned int readSize( unsigned int length ) { return length * 2; }
    int convert( short *buffer, int length );

};


int S16SwapAudioFormatHandler::convert( short *buffer, int length )
{
    int result = length / 2;
    while ( length >= 2 ) {
	*buffer = (short)((*buffer << 8) | ((*buffer >> 8) & 0xFF));
	++buffer;
	length -= 2;
    }
    return result;
}


// Convert unsigned 16-bit samples to normalized raw samples.
class U16AudioFormatHandler : public AudioFormatHandler
{
public:

    unsigned int readSize( unsigned int length ) { return length * 2; }
    int convert( short *buffer, int length );

};


int U16AudioFormatHandler::convert( short *buffer, int length )
{
    int result = length / 2;
    while ( length >= 2 ) {
	*buffer += (short)0x8000;
	++buffer;
	length -= 2;
    }
    return result;
}


// Convert unsigned 16-bit samples to normalized raw samples and byte-swap.
class U16SwapAudioFormatHandler : public AudioFormatHandler
{
public:

    unsigned int readSize( unsigned int length ) { return length * 2; }
    int convert( short *buffer, int length );

};


int U16SwapAudioFormatHandler::convert( short *buffer, int length )
{
    int result = length / 2;
    while ( length >= 2 ) {
	*buffer = (short)(((*buffer << 8) | ((*buffer >> 8) & 0xFF)) + 0x8000);
	++buffer;
	length -= 2;
    }
    return result;
}


// Convert unsigned 8-bit samples to normalized raw samples.
class U8AudioFormatHandler : public AudioFormatHandler
{
public:

    unsigned int readSize( unsigned int length ) { return length; }
    int convert( short *buffer, int length );

};


int U8AudioFormatHandler::convert( short *buffer, int length )
{
    int result = length;
    unsigned char *buf = (((unsigned char *)buffer) + length);
    buffer += length;
    while ( length > 0 ) {
	*(--buffer) = (short)((((int)(*(--buf))) - 128) << 8);
	--length;
    }
    return result;
}


// Convert signed 8-bit samples to normalized raw samples.
class S8AudioFormatHandler : public AudioFormatHandler
{
public:

    unsigned int readSize( unsigned int length ) { return length; }
    int convert( short *buffer, int length );

};


int S8AudioFormatHandler::convert( short *buffer, int length )
{
    int result = length;
    unsigned char *buf = (((unsigned char *)buffer) + length);
    buffer += length;
    while ( length > 0 ) {
	*(--buffer) = (short)(((int)(*(--buf))) << 8);
	--length;
    }
    return result;
}


AudioFormatHandler *AudioFormatHandler::create( int format )
{
    // Determine if the host is little-endian or big-endian.
    union
    {
        short v1;
        char  v2[2];
    } un;
    un.v1 = 0x0102;
    bool littleEndian = ( un.v2[0] == 0x02 );

    // Construct an appropriate handler from the incoming format.
    switch ( format ) {

	case AFMT_U8:
	    return new U8AudioFormatHandler;

	case AFMT_S16_LE:
	    if ( littleEndian )
		return new S16AudioFormatHandler;
	    else
		return new S16SwapAudioFormatHandler;

	case AFMT_S16_BE:
	    if ( littleEndian )
		return new S16SwapAudioFormatHandler;
	    else
		return new S16AudioFormatHandler;

	case AFMT_S8:
	    return new S8AudioFormatHandler;

	case AFMT_U16_LE:
	    if ( littleEndian )
		return new U16AudioFormatHandler;
	    else
		return new U16SwapAudioFormatHandler;

	case AFMT_U16_BE:
	    if ( littleEndian )
		return new U16SwapAudioFormatHandler;
	    else
		return new U16AudioFormatHandler;

	default:
	    qDebug( "unknown audio input format - assuming signed 16-bit" );
	    return new S16AudioFormatHandler;
    }
}


// Perform channel doubling to convert mono samples into stereo.
class MToSAudioFormatHandler : public AudioFormatHandler
{
public:

    MToSAudioFormatHandler( AudioFormatHandler *_linked ) { linked = _linked; }
    ~MToSAudioFormatHandler() { delete linked; }

    unsigned int readSize( unsigned int length )
	{ return linked->readSize( length ); }

    int convert( short *buffer, int length );

private:

    AudioFormatHandler *linked;

};


int MToSAudioFormatHandler::convert( short *buffer, int length )
{
    // Convert the raw samples into their normalized 16-bit form.
    int samples = linked->convert( buffer, length );

    // Perform doubling on the the samples.
    int posn = samples * 2;
    while ( posn > 0 ) {
	posn -= 2;
	buffer[posn] = buffer[posn + 1] = buffer[posn / 2];
    }
    return samples * 2;
}


// Perform channel averaging to convert stereo samples into mono.
class SToMAudioFormatHandler : public AudioFormatHandler
{
public:

    SToMAudioFormatHandler( AudioFormatHandler *_linked ) { linked = _linked; }
    ~SToMAudioFormatHandler() { delete linked; }

    unsigned int readSize( unsigned int length )
	{ return linked->readSize( length ); }

    int convert( short *buffer, int length );

private:

    AudioFormatHandler *linked;

};


int SToMAudioFormatHandler::convert( short *buffer, int length )
{
    // Convert the raw samples into their normalized 16-bit form.
    int samples = linked->convert( buffer, length );

    // Perform averaging on the the samples.
    int posn = 0;
    int limit = samples / 2;
    while ( posn < limit ) {
	buffer[posn] = (short)(((int)(buffer[posn * 2])) +
			       ((int)(buffer[posn * 2 + 1])) / 2);
	++posn;
    }
    return limit;
}


// Resample an audio stream to a different frequency.
class ResampleAudioFormatHandler : public AudioFormatHandler
{
public:

    ResampleAudioFormatHandler( AudioFormatHandler *_linked,
				int _from, int _to, int _channels,
				int bufferSize );
    ~ResampleAudioFormatHandler() { delete linked; delete[] temp; }

    unsigned int readSize( unsigned int length )
	{ return linked->readSize( length ); }

    int convert( short *buffer, int length );

private:

    AudioFormatHandler *linked;
    int from, to, channels;
    long samplesDue;
    long rollingLeft;
    long rollingRight;
    int numInRolling;
    short *temp;

};


ResampleAudioFormatHandler::ResampleAudioFormatHandler
	( AudioFormatHandler *_linked, int _from, int _to,
	  int _channels, int bufferSize )
{
    linked = _linked;
    from = _from;
    to = _to;
    channels = _channels;
    samplesDue = 0;
    rollingLeft = 0;
    rollingRight = 0;
    numInRolling = 0;
    temp = new short [bufferSize];
}

int ResampleAudioFormatHandler::convert( short *buffer, int length )
{
    // Convert the raw samples into their normalized 16-bit form.
    int samples = linked->convert( buffer, length );
    if ( !samples )
	return 0;

    // Resample the data.  We should probably do some kind of curve
    // fit algorithm, but that can be *very* expensive CPU-wise.
    memcpy( temp, buffer, samples * sizeof(short) );
    int inposn = 0;
    int outposn = 0;
    short left, right;
    long due = samplesDue;
    long rollLeft = rollingLeft;
    long rollRight = rollingRight;
    int num = numInRolling;
    if ( from < to ) {
	// Replicate samples to convert to a higher sample rate.
	if ( channels == 1 ) {
	    while ( inposn < samples ) {
		due += to;
		left = temp[inposn++];
		while ( due >= from ) {
		    buffer[outposn++] = left;
		    due -= from;
		}
	    }
	} else {
	    while ( inposn < samples ) {
		due += to;
		left = temp[inposn++];
		right = temp[inposn++];
		while ( due >= from ) {
		    buffer[outposn++] = left;
		    buffer[outposn++] = right;
		    due -= from;
		}
	    }
	}
    } else {
	// Average samples to convert to a lower sample rate.
	// This may lose a small number (from / to) of samples
	// off the end of the stream.
	if ( channels == 1 ) {
	    while ( inposn < samples ) {
		left = temp[inposn++];
		rollLeft += (long)left;
		due += to;
		++num;
		if ( due >= from ) {
		    buffer[outposn++] = (short)(rollLeft / num);
		    rollLeft = 0;
		    num = 0;
		    due -= from;
		}
	    }
	} else {
	    while ( inposn < samples ) {
		left = temp[inposn++];
		right = temp[inposn++];
		rollLeft += (long)left;
		rollLeft += (long)right;
		due += to;
		++num;
		if ( due >= from ) {
		    buffer[outposn++] = (short)(rollLeft / num);
		    buffer[outposn++] = (short)(rollRight / num);
		    rollLeft = 0;
		    rollRight = 0;
		    num = 0;
		    due -= from;
		}
	    }
	}
    }
    samples = outposn;
    samplesDue = due;
    rollingLeft = rollLeft;
    rollingRight = rollRight;
    numInRolling = num;

    // Done
    return samples;
}


// Private storage for the "AudioInput" class.
class AudioInputPrivate
{
public:
    AudioInputPrivate( unsigned int freq, unsigned int chan )
    {
	fd = -1;
	notifier = 0;
	devFrequency = frequency = freq;
	devChannels = channels = chan;
	format = AFMT_S16_LE;
	devBufferSize = bufferSize = 1024;
	position = 0;
	handler = 0;
    }
    ~AudioInputPrivate()
    {
	close();
	if ( handler )
	    delete handler;
    }

#ifndef Q_OS_WIN32

    void open( AudioInput *input, bool trialOpen = false );
    void close();

#endif

public:
    int fd;
    QSocketNotifier *notifier;
    unsigned int devChannels;
    unsigned int devFrequency;
    unsigned int devBufferSize;
    unsigned int channels;
    unsigned int frequency;
    unsigned int format;
    unsigned int bufferSize;
    long position;
    AudioFormatHandler *handler;

};


#ifndef Q_OS_WIN32


// The list of audio formats that we understand, in the order
// in which we probe for them.  We prefer formats that require
// very little modification by "AudioFormatHandler" instances.
static int const formatList[] = {
    AFMT_S16_NE,	// Try 16-bit native host order first.
    AFMT_S16_LE,
    AFMT_S16_BE,
    AFMT_U16_LE,
    AFMT_U16_BE,
    AFMT_U8,
    AFMT_S8,
};
const int formatListSize = sizeof( formatList ) / sizeof( int );


void AudioInputPrivate::open( AudioInput *input, bool trialOpen )
{
    // Attempt to open the DSP audio device.  Try both /dev/dsp and
    // /dev/dsp1, because different machines have the audio input
    // stream on different device nodes.
    if ( ( fd = ::open( "/dev/dsp", O_RDONLY | O_NONBLOCK ) ) < 0 ) {
	if ( ( fd = ::open( "/dev/dsp1", O_RDONLY | O_NONBLOCK ) ) < 0 ) {
	    qDebug( "error opening audio device /dev/dsp for input: %s",
	            strerror( errno ) );
	    return;
	}
    }

    // Stop the file descriptor from being inherited across an exec().
#ifdef F_SETFD
    fcntl( fd, F_SETFD, 1 );
#endif

    // Search for a supported audio format.
    int formats = 0;
    if ( ioctl( fd, SNDCTL_DSP_GETFMTS, &formats ) == -1 ) {
	// If we can't get the supported formats, then we probably can't record.
	::close( fd );
	fd = -1;
	return;
    }
    int posn;
    for ( posn = 0; posn < formatListSize; ++posn ) {
	if ( (formats & formatList[posn]) != 0 ) {
	    format = formatList[posn];
	    break;
	}
    }

#ifdef QSSAUDIO
    // if we are using qss, we need to set the values
    // to the same thing qss will want to set the values
    // to.  ideally would use qss for recording as well.
    int preffmt = AFMT_S16_LE;
    int prefchan = 2;
    int preffreq = 44100;
#else
    int preffmt = format;
    int prefchan = channels;
    int preffreq = frequency;
#endif

    // Configure the device with the recommended values,
    // and read back what the device is really capable of.
    int value = (int)preffmt;
    ioctl( fd, SNDCTL_DSP_SETFMT, &value );
    format = (unsigned int)value;
    value = (int)prefchan;
    ioctl( fd, SNDCTL_DSP_CHANNELS, &value );
    devChannels = (unsigned int)value;
#ifdef QT_QWS_SL5XXX
    // The Sharp Zaurus audio input device has a bug that causes it
    // to halve the specified frequency when set to stereo, so we
    // have to double the value before setting.
    if ( channels == 2 ) {
	value = (int)(preffreq * 2);
	ioctl( fd, SNDCTL_DSP_SPEED, &value );
	devFrequency = preffreq;
    } else {
	value = (int)preffreq;
	ioctl( fd, SNDCTL_DSP_SPEED, &value );
	devFrequency = (unsigned int)value;
    }
#else
    value = (int)preffreq;
    ioctl( fd, SNDCTL_DSP_SPEED, &value );
    devFrequency = (unsigned int)value;
#endif

    // Resample the input signal if it differs by more than 50 Hz
    // from the requested frequency rate.
    int diff = (int)(devFrequency - frequency);
    bool resample = (diff < -50 || diff > 50);
    if ( !resample )
      frequency = devFrequency;

    // Create the format handler if requested.
    if ( !trialOpen ) {
	if ( handler )
	    delete handler;
	handler = AudioFormatHandler::create( format );
    }

    // Get the recommended buffer size from the device, in 16-bit units.
    devBufferSize = 16;
    ioctl( fd, SNDCTL_DSP_GETBLKSIZE, &devBufferSize );
    if ( devBufferSize < 16 )
	devBufferSize = 16;
    if ( format != AFMT_U8 && format != AFMT_S8 )
	devBufferSize /= 2;

    // Get the inferred buffer size for the requested frequency and channels.
    if ( resample || channels != devChannels ) {
	int devSamples = (devBufferSize / devChannels);
	int actualSamples = (devSamples * frequency / devFrequency) + 1;
	bufferSize = actualSamples * channels;
	if ( bufferSize < devBufferSize )
	    bufferSize = devBufferSize;
    } else {
	bufferSize = devBufferSize;
    }

    // Add extra audio format handlers to do channel conversion and resampling.
    if ( !trialOpen ) {

	// Convert the number of channels to the requested value.
	if ( channels == 2 && devChannels == 1 )
	    handler = new MToSAudioFormatHandler( handler );
	else if ( channels == 1 && devChannels == 2 )
	    handler = new SToMAudioFormatHandler( handler );

	// Resample the frequency rate if necessary.
	if ( resample && frequency != devFrequency ) {
	    handler = new ResampleAudioFormatHandler
		( handler, devFrequency, frequency, channels, bufferSize );
	}
    }

    // Register a socket notifier to be appraised of data arrival.
    if ( !trialOpen ) {
	notifier = new QSocketNotifier( fd, QSocketNotifier::Read );
	QObject::connect( notifier, SIGNAL(activated(int)),
			  input, SIGNAL(dataAvailable()) );
    }
}


void AudioInputPrivate::close()
{
    if ( notifier ) {
	delete notifier;
	notifier = 0;
    }
    if ( fd != -1) {
	::close( fd );
	fd = -1;
    }
}


#endif


/*!
  \class AudioInput audioinput.h

  \brief Retrieve audio data from an input device such as a microphone.
*/

/*!
  Constructs a new audio input device instance.  The \a frequency and
  \a channels values are the recommended configuration parameters for
  the device.  If the device is not capable of supporting the recommended
  values, it will choose the closest configuration that it does support.
*/
AudioInput::AudioInput( unsigned int frequency, unsigned int channels )
{
    d = new AudioInputPrivate( frequency, channels );

#ifndef Q_OS_WIN32
    // Do a trial open on the device, which will adjust the sample
    // frequency, channel count, and format to something that the
    // audio device actually supports.
    d->open( this, true );
    d->close();
#endif
}


/*!
  Destructs the audio input device instance.
*/
AudioInput::~AudioInput()
{
    delete d;
}


/*!
  Read a block of raw audio samples from the input device.   The samples
  are guaranteed to be signed, 16 bits in size, and in host byte order.

  Returns the number of raw samples read, or zero if none are currently
  available.  This function will not block.

  If the device is mono, then the number of raw samples is the same
  as the number of logical samples.  If the device is stereo, then
  the number of raw samples is twice the number of logical samples.
  The \a length value is the maximum buffer size in raw samples.

  \sa frequency(), dataAvailable()
*/
int AudioInput::read( short *buffer, unsigned int length )
{
#ifdef Q_OS_WIN32
    return 0;
#else
    // Bail out if the device is not currently open.
    if ( d->fd == -1 ) {
	return 0;
    }

    // Read bytes from the device using its own encoding.
    if ( length > d->devBufferSize ) {
	length = d->devBufferSize;
    }
    unsigned int len = d->handler->readSize( length );
    int result = ::read( d->fd, (char *)buffer, len );
    if ( result <= 0 ) {
	return 0;
    }

    // Convert the device's encoding into normalized sample values.
    result = d->handler->convert( buffer, result );
    d->position += (result / d->channels);
    return result;
#endif
}


/*!
  Returns the number of audio channels on the input device (1 or 2).
*/
unsigned int AudioInput::channels() const
{
    return d->channels;
}


/*!
  Returns the sample rate frequency of the input device.  The device
  returns this many logical samples per second.

  \sa read
*/
unsigned int AudioInput::frequency() const
{
    return d->frequency;
}


/*!
  Returns the recommended buffer size, in 16-bit units.
*/
unsigned int AudioInput::bufferSize() const
{
    return d->bufferSize;
}


/*!
  Returns TRUE if the audio input device is currently open and
  actively recording.
*/
bool AudioInput::isActive() const
{
#ifdef Q_OS_WIN32
    return false;
#else
    return ( d->fd != -1 );
#endif
}


/*!
  Get the volume information for the microphone input.

  \sa setVolume()
*/
void AudioInput::getVolume( unsigned int& left, unsigned int& right )
{
#ifndef Q_OS_WIN32
    unsigned int volume;
    int mixerHandle = open( "/dev/mixer", O_RDWR );
    if ( mixerHandle >= 0 ) {
        ioctl( mixerHandle, MIXER_READ(AUDIO_RECORD_SOURCE), &volume );
        close( mixerHandle );
    } else
	qDebug( "get volume of audio input device failed" );
    left  = ((volume & 0x00FF) << 16) / 101;
    right = ((volume & 0xFF00) <<  8) / 101;
#endif
}


/*!
  Set the volume information for the microphone input.

  The \a left and \a right values must be between 0 and 0xFFFF, inclusive.

  \sa getVolume()
*/
void AudioInput::setVolume( unsigned int left, unsigned int right )
{
#ifndef Q_OS_WIN32
    left  = ( (int) left < 0 ) ? 0 : ((  left > 0xFFFF ) ? 0xFFFF :  left );
    right = ( (int)right < 0 ) ? 0 : (( right > 0xFFFF ) ? 0xFFFF : right );
    unsigned int rV = (right * 101) >> 16;
    unsigned int lV = (left  * 101) >> 16;
    unsigned int volume = ((rV << 8) & 0xFF00) | (lV & 0x00FF);
    int mixerHandle = 0;
    if ( ( mixerHandle = open( "/dev/mixer", O_RDWR ) ) >= 0 ) {
        ioctl( mixerHandle, MIXER_WRITE(AUDIO_RECORD_SOURCE), &volume );
        close( mixerHandle );
    } else
	qDebug( "set volume of audio input device failed" );
#endif
}


/*!
  Return the current position in logical samples since audio input
  was \link start() started \endlink.  If the device is not
  currently recording, this will return the number of logical
  samples that were read during the last recording session.

  \sa read(), start()
*/
long AudioInput::position() const
{
    return d->position;
}


/*!
  Start recording audio input from the primary input device
  (normally a microphone).  The call will be ignored if recording
  is already in progress.

  \sa stop(), position(), read()
*/
void AudioInput::start()
{
#ifndef Q_OS_WIN32
    if ( d->fd == -1 ) {
	d->position = 0;
	d->open( this );
    }
#endif
}


/*!
  Stop recording audio input from the primary input device.
  The call will be ignored if recording is not currently in progress.

  \sa start()
*/
void AudioInput::stop()
{
#ifndef Q_OS_WIN32
    d->close();
#endif
}

/*!
  \fn void AudioInput::dataAvailable()

  This signal is emitted when new data becomes available on the
  audio input device.  The application should \link read() read \endlink
  the data as soon as possible, to prevent backlogs.

  \sa read()
*/
