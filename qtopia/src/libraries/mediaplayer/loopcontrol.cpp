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
#include <qmessagebox.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qtopia/mediaplayerplugininterface.h>
#include <qtopia/config.h>
#include <sched.h>
#include <sys/resource.h>
#include "loopcontrol.h"
#include "audiocontext.h"
#include "videocontext.h"
#include "audiodevice.h"
#include "mediaplayerstate.h"
#include "mutex.h"


LoopControl::LoopControl(QObject *parent, const char *name, AudioDevice *ad)
    : LoopControlBase( parent, name ),
	sliderId(0),
	audioDevice(ad),
	audioDeviceIsReady(false),
	startWhenAudioDeviceReady(false)
{
    connect(mediaPlayerState, SIGNAL(positionChanged(long)), this, SLOT(setPosition(long)));
    connect(mediaPlayerState, SIGNAL(pausedToggled(bool)), this, SLOT(setPaused(bool)));
    connect(mediaPlayerState, SIGNAL(seekingToggled(bool)), this, SLOT(setPaused(bool)));

    connect(audioDevice, SIGNAL(deviceReady()), this, SLOT(audioDeviceReady()));
    connect(audioDevice, SIGNAL(deviceOpenError()), this, SLOT(audioDeviceError()));

    Config cfg("AudioSettings");
    cfg.setGroup("Threading"); 
    bool audioThreaded = cfg.readBoolEntry( "Threaded", true );
    bool audioRealtime = cfg.readBoolEntry( "Realtime", false );
    bool audioFifoSchd = cfg.readBoolEntry( "Fifo", true );
    int audioRTPriority = cfg.readNumEntry( "RealtimePriority", 50 );
    int audioNRTPriority = cfg.readNumEntry( "NonRealtimePriority", -10 );
    int audioBufferSize = cfg.readNumEntry( "BufferSize", 4096 );
    int audioPriority = audioRealtime ? audioRTPriority : audioNRTPriority;

    setpriority(PRIO_PROCESS, 0, getpriority(PRIO_PROCESS, 0) - 9);

    video = new VideoLoopContext(&playtime, false, false, false, 0 );
    audio = new AudioLoopContext(audioDevice, audioThreaded, audioRealtime, audioFifoSchd, audioPriority, audioBufferSize);
}


LoopControl::~LoopControl()
{
    stop();
    delete audio;
    delete video;
}


void LoopControl::timerEvent(QTimerEvent *te)
{
    // qDebug("LoopControl::timerEvent()");

    if ( te->timerId() == sliderId ) {
	if ( mediaPlayerState->decoderVersion() == Decoder_1_6 ) {
	    MediaPlayerDecoder_1_6 *decoder = (MediaPlayerDecoder_1_6 *)mediaPlayerState->decoder();
	    mediaPlayerState->updatePosition(decoder->tell());
	} else {
	    if ( audio->enabled && !video->enabled ) {
		mediaPlayerState->updatePosition(audio->current);
	    } else if ( video->enabled ) {
		mediaPlayerState->updatePosition(video->current);
	    }
	}
    }

    if ( !video->isMore() && !audio->isMore() ) { 
	audio->stats();
	video->stats();
        mediaPlayerState->setPlaying(false);
        mediaPlayerState->setNext();
    }
}


long LoopControl::totalPlaytime() const
{
    if ( mediaPlayerState->decoderVersion() == Decoder_1_6 ) {
	MediaPlayerDecoder_1_6 *decoder = (MediaPlayerDecoder_1_6 *)mediaPlayerState->decoder();
	if ( decoder->totalTimeAvailable() )
	    return decoder->totalTime();
    }
    if ( video->time() != -1 )
	return video->time();
    return audio->time();
}


void LoopControl::setPosition(long pos)
{
    qDebug("LoopControl::setPosition()");

    AutoLockUnlockMutex alock(&audio->mutex);
    AutoLockUnlockMutex vlock(&video->mutex);

//    if ( mediaPlayerState->seeking() || mediaPlayerState->paused() )
//	killTimers();

    if ( mediaPlayerState->decoderVersion() == Decoder_1_6 ) {
	MediaPlayerDecoder_1_6 *decoder = (MediaPlayerDecoder_1_6 *)mediaPlayerState->decoder();
	if ( decoder->seekAvailable() )
	    decoder->seek(pos);
	qDebug("LoopControl::setPosition() finished");
	return;
    }

    if ( mediaPlayerState->seekable() ) {
	if ( video->enabled ) {
	    playtime.restart();
	    playtime = playtime.addMSecs(long((long long)-pos * 1000 / video->rate));
	    video->current = pos;
	    mediaPlayerState->decoder()->videoSetFrame(video->current, video->streamId);
	    if ( audio->enabled ) {
		audio->current = int((long long)video->current * audio->rate / video->rate);
		mediaPlayerState->decoder()->audioSetSample(audio->current, audio->streamId);
	    }
	} else if ( audio->enabled ) {
	    playtime.restart();
	    playtime = playtime.addMSecs(long((long long)-pos * 1000 / audio->rate));
	    audio->current = pos;
	    mediaPlayerState->decoder()->audioSetSample(audio->current, audio->streamId);
	}
    }

    qDebug("LoopControl::setPosition() finished");
}


void LoopControl::killTimers()
{
    qDebug("LoopControl::killTimers()");

    if ( sliderId ) { 
	killTimer(sliderId);
	sliderId = 0;
    }

    qDebug("stoping audio");
    audio->stop();
    qDebug("stoping video");
    video->stop();

    qDebug("LoopControl::killTimers() finished");
}


void LoopControl::audioDeviceReady()
{
    audioDeviceIsReady = true;
    if (startWhenAudioDeviceReady)
	start();
}


void LoopControl::audioDeviceError()
{
    AutoLockUnlockMutex alock(&audio->mutex);
    AutoLockUnlockMutex vlock(&video->mutex);
    audioDeviceIsReady = false;
    QMessageBox::critical(0, tr("Audio Error"), tr("<qt>Error initialising audio.</qt>"));

    // Do in a timer because this was called from AudioDevicePrivate, and stopping
    // will close the AudioDevice which can delete the AudioDevicePrivate object this is called from
    QTimer::singleShot(50, this, SLOT(audioDeviceErrorHandler()));
}


void LoopControl::audioDeviceErrorHandler()
{ 
    mediaPlayerState->setPlaying(false);
    mediaPlayerState->closeView();
}


#define SLIDER_UPDATE_INTERVAL	     300


void LoopControl::startTimers()
{
    qDebug("LoopControl::startTimers()");

    if ( mediaPlayerState->decoderVersion() == Decoder_1_6 ) {
	MediaPlayerDecoder_1_6 *decoder = (MediaPlayerDecoder_1_6 *)mediaPlayerState->decoder();
	mediaPlayerState->updatePosition(decoder->tell());
	if ( decoder->tellAvailable() ) {
	    if (!sliderId)
		sliderId = startTimer(SLIDER_UPDATE_INTERVAL); // update slider every 1/3 second
	} else {
	    //disableSlider();
	}
    } else {
        if (!sliderId)
	    sliderId = startTimer(SLIDER_UPDATE_INTERVAL); // update slider every 1/3 second
    }

    qDebug("starting audio");
    audio->start();
    qDebug("starting video");
    video->start();

    qDebug("LoopControl::startTimers() finished");
}


void LoopControl::setPaused(bool)
{
    if ( !mediaPlayerState->decoder() || !mediaPlayerState->decoder()->isOpen() )
	return;
    if ( mediaPlayerState->seeking() || mediaPlayerState->paused() )
	killTimers();
    else
	startTimers();
}


void LoopControl::start()
{
    if ( audio->enabled && !audioDeviceIsReady ) {
	startWhenAudioDeviceReady = true;
    } else {
	startWhenAudioDeviceReady = false;
	playtime.start();
	startTimers();
    }
}


void LoopControl::stop()
{
    qDebug("LoopControl::stop()");

    if ( mediaPlayerState->decoder() && mediaPlayerState->decoder()->isOpen() ) {
	killTimers();
	mediaPlayerState->decoder()->close();
	audioDevice->close();
	audioDeviceIsReady = false;
    }

    qDebug("LoopControl::stop() finished");
}


bool LoopControl::init(const QString& filename, const QString& mimetype, bool isURL)
{
    qDebug("LoopControl::init()");

    stop();

    AutoLockUnlockMutex alock(&audio->mutex);
    AutoLockUnlockMutex vlock(&video->mutex);
    
    if ( !mediaPlayerState->decoder() ) 
	return false;

    if ( isURL ) {
	if ( mediaPlayerState->decoderVersion() == Decoder_1_6 ) {
	    MediaPlayerDecoder_1_6 *decoder = (MediaPlayerDecoder_1_6 *)mediaPlayerState->decoder();
	    if ( !decoder->openURL(filename, mimetype) ) 
		return false;
	} else {
	    qDebug("This shouldn't happen: %s decoder isn't a 1.6 plugin", mediaPlayerState->decoder()->pluginName());
	    return false;
	}
    } else {
	if ( !mediaPlayerState->decoder()->open(filename) ) 
	    return false;
    }

    qDebug("Using the %s decoder", mediaPlayerState->decoder()->pluginName());

    audio->enabled = mediaPlayerState->decoder()->audioStreams() > 0;
    video->enabled = mediaPlayerState->decoder()->videoStreams() > 0;

    if ( audio->enabled ) {
	startWhenAudioDeviceReady = false;
	audioDeviceIsReady = false;
	audio->init();
    }

    if ( video->enabled )
	video->init();

    mediaPlayerState->updatePosition(0);

    if ( mediaPlayerState->decoderVersion() == Decoder_1_6 ) {
	MediaPlayerDecoder_1_6 *decoder = (MediaPlayerDecoder_1_6 *)mediaPlayerState->decoder();
	if ( decoder->lengthAvailable() )
	    mediaPlayerState->setLength(decoder->length());
	else
	    mediaPlayerState->setLength(0);
	mediaPlayerState->setSeekable( decoder->seekAvailable() );
    } else {
	if ( video->enabled )
	    mediaPlayerState->setLength(video->size);
	else
	    mediaPlayerState->setLength(audio->size);
	// Can we set the audio sample? If we can, then the decoder and stream are seekable
	bool audioSeekable = audio->enabled ? mediaPlayerState->decoder()->audioSetSample(0, 0) : true;
	// Can we set the video frame? If we can, then the decoder and stream are seekable
	bool videoSeekable = video->enabled ? mediaPlayerState->decoder()->videoSetFrame(0, 0) : true;
	qDebug("audio seekable: %s, video seekable: %s", audioSeekable ? "true" : "false", videoSeekable  ? "true" : "false");
	mediaPlayerState->setSeekable( audioSeekable && videoSeekable );
    }

    if ( !mediaPlayerState->hasLength() )
	qDebug("Decoder can not query length"); // No tr

    qDebug("LoopControl::init() finished");
    return true;
}

