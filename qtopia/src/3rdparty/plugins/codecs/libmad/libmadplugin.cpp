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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <locale.h>
#include <math.h>
#include <assert.h>
#include <qapplication.h>

#include <pthread.h>


//#define HAVE_MMAP

#if defined(HAVE_MMAP)
#   include <sys/mman.h>
#endif
#include "libmadplugin.h"


extern "C" {
#include "mad.h"
}


#define MPEG_BUFFER_SIZE    65536
//#define debugMsg(a)	    qDebug(a)
#define debugMsg(a)

/*!
  \class LibMadPlugin libmadplugin.h
  \brief The LibMadPlugin class is a wrapper for the libmad library.

  \legalese
  LibMAD is an integer-based MPEG audio decoder that supports all three
  MPEG Layers, and is used by the MediaPlayer.

  The libmad library is distributed under the terms of the GNU General Public License,
  Version 2. The primary copyright holder is <a href="mailto:rob@mars.org">Robert Leslie</a>.
  More information about the Mad library can be found at <a href="http://www.mars.org/home/rob/proj/mpeg/">http://www.mars.org/home/rob/proj/mpeg/</a>
*/

// QDOC_SKIP_BEGIN

class Input {
public:
    char const *path;
    int fd;
#if defined(HAVE_MMAP)
    void *fdm;
#endif
    unsigned long fileLength;
    unsigned char *data;
    unsigned long length;
    int eof;
};


class Output {
public:
    mad_fixed_t attenuate;
    struct filter *filters;
    unsigned int channels_in;
    unsigned int channels_out;
    unsigned int speed_in;
    unsigned int speed_out;
    const char *path;
};


# if defined(HAVE_MMAP)
static void *map_file(int fd, unsigned long *length)
{
  void *fdm;

  *length += MAD_BUFFER_GUARD;

  fdm = mmap(0, *length, PROT_READ, MAP_SHARED, fd, 0);
  if (fdm == MAP_FAILED)
    return 0;

# if defined(HAVE_MADVISE)
  madvise(fdm, *length, MADV_SEQUENTIAL);
# endif

  return fdm;
}


static int unmap_file(void *fdm, unsigned long length)
{
  if (munmap(fdm, length) == -1)
    return -1;

  return 0;
}
# endif

class Lock
{
public:
    Lock( pthread_mutex_t* mutex )
        : mutex_( mutex )
    {
        pthread_mutex_lock( mutex_ );
    }
    
    ~Lock()
    {
        pthread_mutex_unlock( mutex_ );
    }

private:
    pthread_mutex_t *mutex_;
};


class LibMadPluginData {
public:
    Input input;
    Output output;
    int bad_last_frame;
    struct mad_stream stream;
    struct mad_frame frame;
    struct mad_synth synth;

    int buffered;
    mad_fixed_t buffer[2][65536 * 2];
    bool resync;

    int frequency;
    
    pthread_mutex_t mutex;
};


LibMadPlugin::LibMadPlugin() {
    d = new LibMadPluginData;
    d->input.fd = 0;
#if defined(HAVE_MMAP)
    d->input.fdm = 0;
#endif
    d->input.data = 0;
    d->buffered = 0;
    d->resync = FALSE;
    info = qApp->translate( "MediaPlayer", "No Song Open", "libmad strings for mp3 file info" );

    d->frequency = 0;
    
    pthread_mutex_init( &d->mutex, NULL );
}

/*!
    \fn LibMadPlugin::pluginComment();
    \internal
*/

/*!
    \fn LibMadPlugin::pluginVersion();
    \internal
*/

/*!
    \fn LibMadPlugin::fileInfo()
    \internal
*/

/*!
    \fn LibMadPlugin::videoStreams()
    \internal
*/

/*!
    \fn LibMadPlugin::videoWidth(int)
    \internal
*/

/*!
    \fn LibMadPlugin::videoHeight(int)
    \internal
*/

/*!
    \fn LibMadPlugin::videFrameRate(int)
    \internal
*/

/*!
    \fn LibMadPlugin::videoFrames(int)
    \internal
*/

/*!
    \fn LibMadPlugin::videoSetFrame(long,int)
    \internal
*/

/*!
    \fn LibMadPlugin::videoGetFrame(int)
    \internal
*/

/*!
    \fn LibMadPlugin::videoReadFrame(unsigned char**, int, int, int, int, ColorFormat, int )
    \internal
*/

/*!
    \fn LibMadPlugin::videoReadScaledFrame( unsigned char**, int, int, int, int, int, int, ColorFormat, int )
    \internal
*/

/*!
    \fn LibMadPlugin::videoReadYUVFrame(char *, char *, char *, int, int, int, int, int )
    \internal
*/

/*!
    \fn LibMadPlugin::setSMP(int)
    \internal
*/

/*!
    \fn LibMadPlugin::setMMX(bool)
    \internal
*/

/*!
    \fn LibMadPlugin::supportsAudio()
    \internal
*/

/*!
    \fn LibMadPlugin::supportsVideo()
    \internal
*/

/*!
    \fn LibMadPlugin::supportsYUV()
    \internal
*/

/*!
    \fn LibMadPlugin::supportsMMX()
    \internal
*/

/*!
    \fn LibMadPlugin::supportsSMP()
    \internal
*/

/*!
    \fn LibMadPlugin::supportsStereo()
    \internal
*/

/*!
    \fn LibMadPlugin::supportsScaling()
    \internal
*/

/*!
    \fn LibMadPlugin::getPlayTime()
    \internal
*/

/*!
    \fn LibMadPlugin::videoFrameRate( int )
    \internal
*/

/*! \internal */

LibMadPlugin::~LibMadPlugin() {
    close();
    delete d;
}


/*! \a path \internal */

bool LibMadPlugin::isFileSupported( const QString& path ) {
    debugMsg( "LibMadPlugin::isFileSupported" );

    // Mpeg file extensions
    //	"mp2","mp3","m1v","m2v","m2s","mpg","vob","mpeg","ac3"
    // Other media extensions
    //  "wav","mid","mod","s3m","ogg","avi","mov","sid"

    char *ext = strrchr( path.latin1(), '.' );

    // Test file extension
    if ( ext ) {
	if ( strncasecmp(ext, ".mp2", 4) == 0 )
	    return TRUE;
	if ( strncasecmp(ext, ".mp3", 4) == 0 )
	    return TRUE;
    }

    return FALSE;
}


/*! \a path \internal */

bool LibMadPlugin::open( const QString& path ) {
    Lock lock( &d->mutex );

    debugMsg( "LibMadPlugin::open" );

    d->bad_last_frame = 0;
    d->buffered = 0;
    info = QString( "" );

    //qDebug( "Opening %s", path.latin1() );

    d->input.path = path.latin1();
    d->input.fd = ::open( d->input.path, O_RDONLY );
    if (d->input.fd == -1) {
        qDebug("error opening %s", d->input.path );
	return FALSE;
    }

    struct stat stat;
    if (fstat(d->input.fd, &stat) == -1) {
	qDebug("error calling fstat"); return FALSE;
    }
    if (S_ISREG(stat.st_mode) && stat.st_size > 0)
	d->input.fileLength = stat.st_size;
    else
	d->input.fileLength = 0;
#if defined(HAVE_MMAP)
    if (S_ISREG(stat.st_mode) && stat.st_size > 0) {
	d->input.length = stat.st_size;
	d->input.fdm = map_file(d->input.fd, &d->input.length);
	if (d->input.fdm == 0) {
	    qDebug("error mmapping file"); return FALSE;
	}
	d->input.data = (unsigned char *)d->input.fdm;
    }
#endif
    if (d->input.data == 0) {
	d->input.data = (unsigned char *)malloc(MPEG_BUFFER_SIZE);
	if (d->input.data == 0) {
	    qDebug("error allocating input buffer");
	    return FALSE;
	}
	d->input.length = 0;
    }

    d->input.eof = 0;
    d->resync = TRUE;
    d->buffered = 0;

    d->frequency = 0;

    mad_stream_init(&d->stream);
    mad_frame_init(&d->frame);
    mad_synth_init(&d->synth);

    return TRUE;
}


/*! \internal */

bool LibMadPlugin::close() {
    Lock lock( &d->mutex );

    debugMsg( "LibMadPlugin::close" );

    int result = TRUE;

    mad_synth_finish(&d->synth);
    mad_frame_finish(&d->frame);
    mad_stream_finish(&d->stream);

#if defined(HAVE_MMAP)
    if (d->input.fdm) {
	if (unmap_file(d->input.fdm, d->input.length) == -1) {
	    qDebug("error munmapping file");
	    result = FALSE;
	}
	d->input.fdm  = 0;
	d->input.data = 0;
    }
#endif

    if (d->input.data) {
        free(d->input.data);
	d->input.data = 0;
    }

    if (::close(d->input.fd) == -1) {
        qDebug("error closing file %s", d->input.path);
	result = FALSE;
    }

    d->input.fd = 0;

    return result;
}


/*! \internal */

bool LibMadPlugin::isOpen() {
    Lock lock( &d->mutex );

    debugMsg( "LibMadPlugin::isOpen" );
    return ( d->input.fd != 0 );
}


/*! \internal */

int LibMadPlugin::audioStreams() {
    debugMsg( "LibMadPlugin::audioStreams" );
    return 1;
}


/*! \internal */

int LibMadPlugin::audioChannels( int ) {
    debugMsg( "LibMadPlugin::audioChannels" );
/*
    long t; short t1[5]; audioReadSamples( t1, 2, 1, t, 0 );
    qDebug( "LibMadPlugin::audioChannels: %i", d->frame.header.mode > 0 ? 2 : 1 );
    return d->frame.header.mode > 0 ? 2 : 1;
*/
    return 2;
}


/*! \internal */

int LibMadPlugin::audioFrequency( int ) {
    debugMsg( "LibMadPlugin::audioFrequency" );

    // If frequency not cached, retrieve frequency from frame header
    if( !d->frequency ) {
#ifdef OLD_MEDIAPLAYER_API
        long t; short t1[5]; audioReadStereoSamples( t1, 1, t, 0 );
#else
        long t; short t1[32768]; audioReadSamples( t1, 2, 1, t, 0 );
#endif
        Lock lock( &d->mutex );
        
//      qDebug( "LibMadPlugin::audioFrequency: %i", d->frame.header.samplerate );
        d->frequency = d->frame.header.samplerate;
    }

    return d->frequency;
}

/*! \internal */

int LibMadPlugin::audioSamples( int ) {
    debugMsg( "LibMadPlugin::audioSamples" );

    long t; short t1[5]; audioReadSamples( t1, 2, 1, t, 0 );
    
    Lock lock( &d->mutex );
    
    mad_header_decode( (struct mad_header *)&d->frame.header, &d->stream );
/*
    qDebug( "LibMadPlugin::audioSamples: %i*%i", d->frame.header.duration.seconds, d->frame.header.samplerate );
    return d->frame.header.duration.seconds * d->frame.header.samplerate;
*/
    if ( d->frame.header.bitrate == 0 )
	return 0;
    int samples = (d->input.fileLength / (d->frame.header.bitrate/8)) * d->frame.header.samplerate;
    qDebug( "LibMadPlugin::audioSamples: %i * %i * 8 / %i", (int)d->input.fileLength, (int)d->frame.header.samplerate, (int)d->frame.header.bitrate );
    qDebug( "LibMadPlugin::audioSamples: %i", samples );
    return samples;
}

/*! \internal \a pos*/

bool LibMadPlugin::audioSetSample( long pos, int ) {
    debugMsg( "LibMadPlugin::audioSetSample" );

    long totalSamples = audioSamples(0);
    if ( totalSamples <= 1 )
	return FALSE;

    Lock lock( &d->mutex );    
        
    // Seek to requested position
    qDebug( "seek pos: %i", (int)((double)pos * d->input.fileLength / totalSamples) );
    ::lseek( d->input.fd, (long)((double)pos * d->input.fileLength / totalSamples), SEEK_SET );
    mad_stream_sync(&d->stream);

    mad_stream_init(&d->stream);
    mad_frame_init(&d->frame);
    mad_synth_init(&d->synth);

    d->buffered = 0;
    d->resync = TRUE;
    return TRUE;
}

/*! \internal */

long LibMadPlugin::audioGetSample( int ) {
    debugMsg( "LibMadPlugin::audioGetSample" );
    return 0;
}


#ifdef OLD_MEDIAPLAYER_API


/*!
    \internal
*/

bool LibMadPlugin::audioReadSamples( short *, int, long, int ) {
    debugMsg( "LibMadPlugin::audioReadSamples" );
    return FALSE;
}

/*! \internal */

bool LibMadPlugin::audioReReadSamples( short *, int, long, int ) {
    debugMsg( "LibMadPlugin::audioReReadSamples" );
    return FALSE;
}


#endif

/*! \internal */

bool LibMadPlugin::read() {
    debugMsg( "LibMadPlugin::read" );
    int len;

      if (d->input.eof)
	return FALSE;

#if defined(HAVE_MMAP)
      if (d->input.fdm) {
	unsigned long skip = 0;

	if (d->stream.next_frame) {
	  struct stat stat;

	  if (fstat(d->input.fd, &stat) == -1)
	    return FALSE;

	  if (stat.st_size + MAD_BUFFER_GUARD <= (signed)d->input.length)
	    return FALSE;

	  // file size changed; update memory map
	  skip = d->stream.next_frame - d->input.data;

	  if (unmap_file(d->input.fdm, d->input.length) == -1) {
	    d->input.fdm  = 0;
	    d->input.data = 0;
	    return FALSE;
	  }

	  d->input.length = stat.st_size;

	  d->input.fdm = map_file(d->input.fd, &d->input.length);
	  if (d->input.fdm == 0) {
	    d->input.data = 0;
	    return FALSE;
	  }

	  d->input.data = (unsigned char *)d->input.fdm;
	}

	mad_stream_buffer(&d->stream, d->input.data + skip, d->input.length - skip);

      } else
#endif
      {
	if (d->stream.next_frame) {
	  if ( d->stream.next_frame > &d->input.data[d->input.length] )
	    d->input.length = 0;
	  else
	    d->input.length = &d->input.data[d->input.length] - d->stream.next_frame;
	  memmove(d->input.data, d->stream.next_frame, d->input.length);
	}

	if ( d->resync ) {
	  d->input.length = 0;
	  d->resync = FALSE;
	}

	do {
	  len = ::read(d->input.fd, d->input.data + d->input.length, MPEG_BUFFER_SIZE - d->input.length);
	}
	while (len == -1 && errno == EINTR);

	if (len == -1) {
	  qDebug("error reading audio");
	  return FALSE;
	}
	else if (len == 0) {
	  d->input.eof = 1;
	}

	mad_stream_buffer(&d->stream, d->input.data, d->input.length += len);
      }

    return TRUE;
}


static inline signed int scale( mad_fixed_t sample )
{
    sample += (1L << (MAD_F_FRACBITS - 16));
    sample = (sample >= MAD_F_ONE) ? MAD_F_ONE - 1 : ( (sample < -MAD_F_ONE) ? -MAD_F_ONE : sample );
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}


inline void audio_pcm( unsigned short *data, unsigned int nsamples, mad_fixed_t *left, mad_fixed_t *right )
{
    if ( right ) {
	while (nsamples--) {
	    *data++ = scale( *left++ );
	    *data++ = scale( *right++ );
	}
    } else {
	while (nsamples--) {
	    signed int mono = scale( *left++ );
	    *data++ = mono;
	    *data++ = mono;
	}
    }
}

/*! \a output \a samples \a samplesMade \internal */

bool LibMadPlugin::decode( short *output, long samples, long& samplesMade ) {
    debugMsg( "LibMadPlugin::decode" );

    static const int maxBuffered = 8000; // 65536;

    samplesMade = 0;

    if ( samples > maxBuffered )
	samples = maxBuffered;

    int offset = d->buffered;

    while ( d->buffered < maxBuffered ) {

	while (mad_frame_decode(&d->frame, &d->stream) == -1) {
	    if ( d->stream.error == MAD_ERROR_BUFLEN ) {
		debugMsg( "feed me" ); // No tr
		qDebug( "feed me" );
		return FALSE; // Feed me
	    }
	    if ( d->stream.error == MAD_ERROR_BADCRC ) {
		mad_frame_mute(&d->frame);
		qDebug( "error decoding, bad crc" );
	    }
	}

	mad_synth_frame(&d->synth, &d->frame);
	int decodedSamples = d->synth.pcm.length;
	memcpy( &(d->buffer[0][offset]), d->synth.pcm.samples[0], decodedSamples * sizeof(mad_fixed_t) );
	if ( d->synth.pcm.channels == 2 )
	    memcpy( &(d->buffer[1][offset]), d->synth.pcm.samples[1], decodedSamples * sizeof(mad_fixed_t) );
	offset += decodedSamples;
	d->buffered += decodedSamples;
    }

    if ( samples >= d->buffered )
	samples = d->buffered;

    audio_pcm( (unsigned short*)output, samples, d->buffer[0], (d->synth.pcm.channels == 2) ? d->buffer[1] : 0 );
    samplesMade = samples;
    d->buffered -= samples;
    memmove( d->buffer[0], &(d->buffer[0][samples]), d->buffered * sizeof(mad_fixed_t) );
    if ( d->synth.pcm.channels == 2 )
	memmove( d->buffer[1], &(d->buffer[1][samples]), d->buffered * sizeof(mad_fixed_t) );

    return TRUE;
}


#ifdef OLD_MEDIAPLAYER_API


bool LibMadPlugin::audioReadMonoSamples( short *, long, long&, int ) {
    debugMsg( "LibMadPlugin::audioReadMonoSamples" );
    return FALSE;
}


bool LibMadPlugin::audioReadStereoSamples( short *output, long samples, long& samplesMade, int ) {

#else

/*!
    \internal
*/

bool LibMadPlugin::audioReadSamples( short *output, int /*channels*/, long samples, long& samplesMade, int ) {

#endif
    debugMsg( "LibMadPlugin::audioReadStereoSamples" );

    static bool needInput = TRUE;

    if ( !isOpen() )
	return FALSE;

    Lock lock( &d->mutex );

    if ( samples == 0 )
	return TRUE;

    if ( d->resync || needInput ) {
	if ( d->input.eof ) {
	    samplesMade = samples;
	    memset( output, 0, samples );
	    return FALSE;
	}
	qDebug("getting more input");
	if ( !read() ) {
	    qDebug("failed getting more input");
	    if ( !d->input.eof ) {
		// Perhaps a timeout happened or some error
		// return 0 samples read but keep needInput flag as TRUE
		// so it retrys reading next time
		samplesMade = 0;
		qDebug("ret a - couldn't get more input");
		return TRUE;
	    }
	}
	needInput = FALSE;
    }

    if ( !decode( output, samples, samplesMade ) ) 
	// Need more input to decode
	needInput = TRUE;
    
    return TRUE;
}

/*! \internal */

double LibMadPlugin::getTime() {
    debugMsg( "LibMadPlugin::getTime" );
    return 0.0;
}

// QDOC_SKIP_END
