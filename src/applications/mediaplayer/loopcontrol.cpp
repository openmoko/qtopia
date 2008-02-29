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
#include <qtopia/qpeapplication.h>
#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/mediaplayerplugininterface.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include "loopcontrol.h"
#include "videowidget.h"
#include "audiodevice.h"
#include "mediaplayerstate.h"


//#define DecodeLoopDebug(x)  qDebug x
#define DecodeLoopDebug(x)


static char	    *audioBuffer = NULL;
static AudioDevice  *audioDevice = NULL;
static bool	    disabledSuspendScreenSaver = FALSE;
static bool	    previousSuspendMode = FALSE;


pthread_t	audio_tid;
pthread_attr_t  audio_attr;
bool threadOkToGo = FALSE;


class Mutex {
public:
    Mutex() {
	pthread_mutexattr_t attr;
	pthread_mutexattr_init( &attr );
	pthread_mutex_init( &mutex, &attr );
	pthread_mutexattr_destroy( &attr );
    }

    ~Mutex() {
	pthread_mutex_destroy( &mutex );
    }

    void lock() {
	pthread_mutex_lock( &mutex );
    }

    void unlock() {
	pthread_mutex_unlock( &mutex );
    }

private:
    pthread_mutex_t mutex;
};


Mutex *audioMutex;


void *startAudioThread( void *ptr )
{
    LoopControl *mpegView = (LoopControl *)ptr;
    while ( TRUE ) {
	audioMutex->lock();
	bool needSleep = FALSE;
	if ( threadOkToGo && mpegView->moreAudio )
	    mpegView->startAudio();
	else
	    needSleep = TRUE;
	audioMutex->unlock();
	if ( needSleep )
	    usleep( 10000 ); // Semi-busy-wait till we are playing again
    }
    return 0;
}


LoopControl::LoopControl( QObject *parent, const char *name )
    : QObject( parent, name ), videoId(0)
{
    isMuted = FALSE;

    connect( qApp, SIGNAL( volumeChanged(bool) ), this, SLOT( setMute(bool) ) );
    connect( mediaPlayerState, SIGNAL( positionChanged( long ) ), this, SLOT( setPosition( long ) ) );
    connect( mediaPlayerState, SIGNAL( pausedToggled( bool ) ), this, SLOT( setPaused( bool ) ) );

    audioMutex = new Mutex;

    pthread_attr_init(&audio_attr);

    if ( getuid() == 0 ) {
        qDebug("running as root, can set realtime priority");

	// Attempt to set it to real-time round robin
	if ( pthread_attr_setschedpolicy( &audio_attr, SCHED_RR ) == 0 ) {
	//if ( pthread_attr_setschedpolicy( &audio_attr, SCHED_OTHER ) == 0 ) {
	    sched_param params;
	    params.sched_priority = 50;
	    pthread_attr_setschedparam(&audio_attr,&params);
	} else {
	    qDebug( "Error setting up a realtime thread, reverting to using a normal thread." );
	    pthread_attr_destroy(&audio_attr);
	    pthread_attr_init(&audio_attr);
	}
    }
    usleep( 100 );

    pthread_create(&audio_tid, &audio_attr, (void * (*)(void *))startAudioThread, this);
}


LoopControl::~LoopControl()
{
    stop();
}


static long prev_frame = 0;
static int currentSample = 0;
unsigned int startTime;


void LoopControl::timerEvent( QTimerEvent *te )
{
    if ( te->timerId() == videoId )
	startVideo();

    if ( te->timerId() == sliderId ) {

	if ( mediaPlayerState->decoderVersion() == Decoder_1_6 ) {

	    MediaPlayerDecoder_1_6 *decoder = (MediaPlayerDecoder_1_6 *)mediaPlayerState->decoder();
	    mediaPlayerState->updatePosition( decoder->tell() );

	} else {
	    if ( hasAudioChannel && !hasVideoChannel && moreAudio ) {
		mediaPlayerState->updatePosition( audioSampleCounter );
	    } else if ( hasVideoChannel && moreVideo ) {
		mediaPlayerState->updatePosition( current_frame );
	    }
	}

    }

    if ( !moreVideo && !moreAudio ) { 
        mediaPlayerState->setPlaying( FALSE );
        mediaPlayerState->setNext();
    }
}


long LoopControl::totalPlaytime()
{
    if ( mediaPlayerState->decoderVersion() == Decoder_1_6 ) {
	MediaPlayerDecoder_1_6 *decoder = (MediaPlayerDecoder_1_6 *)mediaPlayerState->decoder();
	if ( decoder->totalTimeAvailable() )
	    return decoder->totalTime();
	else
	    return -1;
    }

    if ( hasVideoChannel ) {
	if ( framerate != 0.0 )
	    return (long)(total_video_frames / framerate);
    } else {
	if ( freq )
	    return total_audio_samples / freq;
    }
    return -1;
}


void LoopControl::setPosition( long pos )
{
    audioMutex->lock();

    if ( mediaPlayerState->decoderVersion() == Decoder_1_6 ) {
	MediaPlayerDecoder_1_6 *decoder = (MediaPlayerDecoder_1_6 *)mediaPlayerState->decoder();
	if ( decoder->seekAvailable() )
	    decoder->seek( pos );
	audioMutex->unlock();
	return;
    }

    if ( mediaPlayerState->seekable() ) {
	if ( hasVideoChannel && hasAudioChannel ) {
	    playtime.restart();
	    playtime = playtime.addMSecs( long((long long)-pos * 1000 / framerate) );
	    current_frame = pos + 1;
	    mediaPlayerState->decoder()->videoSetFrame( current_frame, vstream );
	    prev_frame = current_frame - 1;
	    currentSample = (int)( (long long)current_frame * freq / framerate );
	    mediaPlayerState->decoder()->audioSetSample( currentSample, astream );
	    audioSampleCounter = currentSample - 1;
	} else if ( hasVideoChannel ) {
	    playtime.restart();
	    playtime = playtime.addMSecs( long((long long)-pos * 1000 / framerate) );
	    current_frame = pos + 1;
	    mediaPlayerState->decoder()->videoSetFrame( current_frame, vstream );
	    prev_frame = current_frame - 1;
	} else if ( hasAudioChannel ) {
	    playtime.restart();
	    playtime = playtime.addMSecs( long((long long)-pos * 1000 / freq) );
	    currentSample = pos + 1;
	    mediaPlayerState->decoder()->audioSetSample( currentSample, astream );
	    audioSampleCounter = currentSample - 1;
	}
    }

    audioMutex->unlock();
}

void LoopControl::startVideo()
{
    if ( moreVideo ) {
        if ( mediaPlayerState->decoder() ) {

	    current_frame = long( playtime.elapsed() * framerate / 1000 );

	    if ( mediaPlayerState->decoderVersion() == Decoder_1_6 ) {

		MediaPlayerDecoder_1_6 *decoder = (MediaPlayerDecoder_1_6 *)mediaPlayerState->decoder();

		if ( current_frame != prev_frame ) {
		    // Enough time has elapsed that it is time to display another frame
		    moreVideo = mediaPlayerState->videoUI()->playVideo();
		    prev_frame = current_frame;
		    // Resync the video with the audio
		    if ( decoder->syncAvailable() )
			decoder->sync();
		}

	    } else {
		if ( hasAudioChannel ) {
		    if ( prev_frame != -1 && current_frame <= prev_frame )
			return;
		} else {
		    // Don't skip
		    current_frame++;
		}

		if ( prev_frame == -1 || current_frame > prev_frame ) {
		    if ( current_frame > prev_frame + 1 ) {
			mediaPlayerState->decoder()->videoSetFrame( current_frame, vstream );
		    }
		    moreVideo = mediaPlayerState->videoUI()->playVideo();
		    prev_frame = current_frame;
		}
	    }

	} else {
	    moreVideo = FALSE;
	    killTimer( videoId );
	}
    } else {
	killTimer( videoId );
    }
}


void LoopControl::startAudio()
{
    if ( moreAudio ) {

	if ( mediaPlayerState->decoder() ) {

	    currentSample = audioSampleCounter + 1;
	    long samplesRead = 0;
	    bool readOkay = mediaPlayerState->decoder()->audioReadSamples( (short*)audioBuffer, channels, 1024, samplesRead, astream );
	    long sampleWeShouldBeAt = long( playtime.elapsed() ) * freq / 1000;
	    long sampleWaitTime = currentSample - sampleWeShouldBeAt;

	    if ( hasVideoChannel ) {
		if ( ( sampleWaitTime > 2000 ) && ( sampleWaitTime < 20000 ) ) {
		    usleep( (long)((double)sampleWaitTime * 1000000 / freq) );
		} else if ( sampleWaitTime <= -5000 ) {
		    qDebug("need to catch up by: %li (%i,%li)", -sampleWaitTime, currentSample, sampleWeShouldBeAt );
		    //mediaPlayerState->decoder()->audioSetSample( sampleWeShouldBeAt, astream );
		    currentSample = sampleWeShouldBeAt;
		}
	    } else {
		if ( ( sampleWaitTime > 2000 ) && ( sampleWaitTime < 20000 ) ) 
		    usleep( (long)((long long)sampleWaitTime * 100000 / freq) );
	    }

	    // ### expand samples here before writing
	    if ( readOkay )
		audioDevice->write( audioBuffer, samplesRead * 2 * channels );
	    audioSampleCounter = currentSample + samplesRead - 1;

	    // If we open a file and use a good decoder, we will know
	    // how many samples are in the file and to play
	    if ( total_audio_samples > 1000 )
		moreAudio = audioSampleCounter <= total_audio_samples;
	    // However if it is streamed data or the decoder can not
	    // tell how long the stream is, just keep playing until
	    // audioReadSamples() returns FALSE
	    else
		moreAudio = readOkay;

	} else {

	    moreAudio = FALSE;

	}

    }
}


void LoopControl::killTimers()
{
    audioMutex->lock();

    if ( hasVideoChannel ) 
	killTimer( videoId );
    killTimer( sliderId );
    threadOkToGo = FALSE;

    audioMutex->unlock();
}


void LoopControl::startTimers()
{
    audioMutex->lock();

    moreVideo = FALSE;
    moreAudio = FALSE;

    if ( hasVideoChannel ) {
	moreVideo = TRUE;
	int mSecsBetweenFrames = (int)(100 / framerate); // 10% of the real value
//	mSecsBetweenFrames = 0;
	videoId = startTimer( mSecsBetweenFrames );
    }

    if ( hasAudioChannel ) {
	moreAudio = TRUE;
	threadOkToGo = TRUE;
    }

    if ( mediaPlayerState->decoderVersion() == Decoder_1_6 ) {
	MediaPlayerDecoder_1_6 *decoder = (MediaPlayerDecoder_1_6 *)mediaPlayerState->decoder();
	mediaPlayerState->updatePosition( decoder->tell() );
	if ( decoder->tellAvailable() ) {
	    sliderId = startTimer( 300 ); // update slider every 1/3 second
	} else {
	    //disableSlider();
	}
    } else {
	sliderId = startTimer( 300 ); // update slider every 1/3 second
    }

    audioMutex->unlock();
}


void LoopControl::setPaused( bool pause )
{
    if ( !mediaPlayerState->decoder() || !mediaPlayerState->decoder()->isOpen() )
	return;

    if ( pause ) {
	killTimers();
    } else {
	mediaPlayerState->startTemporaryMute();
	// Force an update of the position
	mediaPlayerState->setPosition( mediaPlayerState->position() + 1 );
	mediaPlayerState->setPosition( mediaPlayerState->position() - 1 );
	// Just like we never stopped
	startTimers();
	mediaPlayerState->stopTemporaryMute( 1000 );
    }
}


void LoopControl::stop( bool willPlayAgainShortly )
{
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    if ( !willPlayAgainShortly && disabledSuspendScreenSaver ) {
	disabledSuspendScreenSaver = FALSE; 
	// Re-enable the suspend mode
	QPEApplication::setTempScreenSaverMode(QPEApplication::Enable);
    }
#endif

    if ( mediaPlayerState->decoder() && mediaPlayerState->decoder()->isOpen() ) {
	killTimers();

	audioMutex->lock();

	mediaPlayerState->decoder()->close();

	if ( audioDevice ) {
	    delete audioDevice;
	    delete []audioBuffer;
	    audioDevice = 0;
	    audioBuffer = 0;
	}

	audioMutex->unlock();
    }
}


bool LoopControl::init( const QString& filename, const QString& mimetype, bool isURL )
{
    stop();

    audioMutex->lock();

    fileName = filename;
    current_frame = 0;
    
    if ( !mediaPlayerState->decoder() ) {
	audioMutex->unlock();
	return FALSE;
    }

    if ( isURL ) {
	if ( mediaPlayerState->decoderVersion() == Decoder_1_6 ) {
	    MediaPlayerDecoder_1_6 *decoder = (MediaPlayerDecoder_1_6 *)mediaPlayerState->decoder();
	    if ( !decoder->openURL( filename, mimetype ) ) {
		audioMutex->unlock();
		return FALSE;
	    }
	} else {
	    qDebug( "This shouldn't happen: %s decoder isn't a 1.6 plugin", mediaPlayerState->decoder()->pluginName() );
	    audioMutex->unlock();
	    return FALSE;
	}
    } else {
	if ( !mediaPlayerState->decoder()->open( filename ) ) {
	    audioMutex->unlock();
	    return FALSE;
	}
    }

    qDebug( "Using the %s decoder", mediaPlayerState->decoder()->pluginName() );

    hasAudioChannel = mediaPlayerState->decoder()->audioStreams() > 0;
    hasVideoChannel = mediaPlayerState->decoder()->videoStreams() > 0;

    if ( hasAudioChannel ) {
	astream = 0;
	channels = mediaPlayerState->decoder()->audioChannels( astream );
	DecodeLoopDebug(( "channels = %d\n", channels )); // No tr
	total_audio_samples = mediaPlayerState->decoder()->audioSamples( astream );
	if ( total_audio_samples )
	    // give it one extra iteration through the
	    // audio decoding loop after the expected EOF
	    total_audio_samples += 1000;
	freq = mediaPlayerState->decoder()->audioFrequency( astream );
	DecodeLoopDebug(( "frequency = %d\n", freq )); // No tr
	audioSampleCounter = 0;

	static const int bytes_per_sample = 2; //16 bit
	audioDevice = new AudioDevice( freq, channels, bytes_per_sample );
	audioBuffer = new char[ audioDevice->bufferSize() ];
	channels = audioDevice->channels();
/*
	//### must check which frequency is actually used.
	static const int size = 1;
	short int buf[size];
	long samplesRead = 0;
	mediaPlayerState->decoder()->audioReadSamples( buf, channels, size, samplesRead, astream );
*/
    }

    if ( hasVideoChannel ) {
	vstream = 0;
	total_video_frames = mediaPlayerState->decoder()->videoFrames( vstream );
        framerate = (float)mediaPlayerState->decoder()->videoFrameRate( vstream );
        DecodeLoopDebug(( "Frame rate %g total %ld", framerate, total_video_frames )); // No tr
	if ( framerate <= 1.0 ) {
	    DecodeLoopDebug(( "Crazy frame rate, resetting to sensible" )); // No tr
	    framerate = 25;
	}
    }

    if ( mediaPlayerState->decoderVersion() == Decoder_1_6 ) {
	MediaPlayerDecoder_1_6 *decoder = (MediaPlayerDecoder_1_6 *)mediaPlayerState->decoder();
	if ( decoder->lengthAvailable() )
	    mediaPlayerState->setLength( decoder->length() );
	else
	    mediaPlayerState->setLength( 0 );
	mediaPlayerState->setSeekable( decoder->seekAvailable() );
    } else {
	if ( hasVideoChannel )
	    mediaPlayerState->setLength( total_video_frames );
	else
	    mediaPlayerState->setLength( total_audio_samples );
	// Can we set the audio sample? If we can, then the decoder and stream are seekable
	bool audioSeekable = hasAudioChannel ? mediaPlayerState->decoder()->audioSetSample( 0, 0 ) : TRUE;
	// Can we set the video frame? If we can, then the decoder and stream are seekable
	bool videoSeekable = hasVideoChannel ? mediaPlayerState->decoder()->videoSetFrame( 0, 0 ) : TRUE;
	qDebug("audio seekable: %s, video seekable: %s", audioSeekable ? "true" : "false", videoSeekable  ? "true" : "false" );
	mediaPlayerState->setSeekable( audioSeekable && videoSeekable );
    }

    if ( !mediaPlayerState->hasLength() )
	DecodeLoopDebug(( "Decoder can not query length" )); // No tr

    current_frame = 0;
    prev_frame = -1;

    audioMutex->unlock();

    return TRUE;
}


void LoopControl::play()
{
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    if ( !disabledSuspendScreenSaver || previousSuspendMode != hasVideoChannel ) {
	disabledSuspendScreenSaver = TRUE; 
	previousSuspendMode = hasVideoChannel;
        // Stop the screen from blanking and power saving state
	QPEApplication::setTempScreenSaverMode(hasVideoChannel ? QPEApplication::Disable : QPEApplication::DisableSuspend);
    }
#endif
    if ( hasVideo() )
	mediaPlayerState->videoUI(); // create it
    else
	mediaPlayerState->audioUI(); // create it

    mediaPlayerState->setView( hasVideo() ? VideoView : AudioView );

    playtime.start();
    startTimers();
}


void LoopControl::setMute( bool on )
{
    isMuted = on;
}


