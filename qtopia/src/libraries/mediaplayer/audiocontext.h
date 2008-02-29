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
#ifndef AUDIO_LOOP_CONTEXT_H
#define AUDIO_LOOP_CONTEXT_H


#include "audiodevice.h"
#include "loopcontextbase.h"
#include "mediaplayerstate.h"
#include <qtopia/mediaplayerplugininterface.h>
#include <pthread.h>
#include "mutex.h"


class AudioLoopContext : public LoopContextBase {
    Q_OBJECT
public:
    AudioLoopContext(AudioDevice *ad, bool t=true, bool r=false, bool f=true, int p=50, int b=4096) :
	    LoopContextBase(t,r,f,p) {
	AutoLockUnlockMutex lock(&mutex);
	audioDevice = ad;
	audioBuffer = new char[b];
	audioBufferSize = b;
    }
    ~AudioLoopContext() {
	AutoLockUnlockMutex lock(&mutex);
	delete []audioBuffer;
    }
    long timerInterval() {
	return (int)(audioBufferSize * 1000.0 / (rate * channels * 2));
    }
    void stats() {
    }
    void init() {
	qDebug("AudioContext::init()");
	qDebug("  audioBufferSize: %i", audioBufferSize);
	current = 0;
	streamId = 0;
	channels = mediaPlayerState->decoder()->audioChannels(streamId);
	qDebug("  Stream audio channels: %d", channels); // No tr
	size = mediaPlayerState->decoder()->audioSamples(streamId);
	if ( size )
	    // give it one extra iteration through the
	    // audio decoding loop after the expected EOF
	    size += 1000;
	rate = mediaPlayerState->decoder()->audioFrequency(streamId);
	qDebug("  Stream audio frequency: %d", (int)rate); // No tr
	static const int bytes_per_sample = 2; //16 bit
	audioDevice->open((long)rate, channels, bytes_per_sample, false);
	channels = audioDevice->channels();
	qDebug("  Audio device channels: %d", channels); // No tr
	qDebug("AudioContext::init() finished");
    }
    bool playMore() {
	//qDebug("AudioContext::playMore()");
	MediaPlayerDecoder *decoder = 0;
	long samplesRead = 0;
	decoder = mediaPlayerState->decoder();
	if (!decoder)
	    return false;
	bool readOkay = decoder->audioReadSamples((short*)audioBuffer,
		channels, audioBufferSize/(channels * 2), samplesRead, streamId);
	if ( readOkay )
	    audioDevice->write(audioBuffer, samplesRead * 2 * channels);
	current += samplesRead;
	// If we open a file and use a good decoder, we will know
	// how many samples are in the file and to play,
	// however if it is streamed data or the decoder can not
	// tell how long the stream is, just keep playing until
	// audioReadSamples() returns false
	//qDebug("AudioContext::playMore() finished");
	return (size > 1000) ? (current <= size) : readOkay;
    }
private:
    AudioDevice *audioDevice;
    char *audioBuffer;
    int audioBufferSize;
    int channels;
};


#endif // AUDIO_LOOP_CONTEXT_H

