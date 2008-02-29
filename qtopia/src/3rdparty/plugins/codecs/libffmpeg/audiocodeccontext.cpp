/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
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
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include <qapplication.h>
#include "audiocodeccontext.h"


const long qssBufferSize = 4096*4; // Used to calculate the exact timing audio bytes will hit the speakers


AudioCodecContext::AudioCodecContext(MediaPacketBuffer *buffer)
{
    FFAutoLockUnlockMutex contextLock(audioMutex);
    FFAutoLockUnlockMutex timestampLock(timeStampMutex);
    FFAutoLockUnlockMutex audioStreamsLock(audioStreamsMutex);
    audioStreamsCount = 0;
    lastAudioTimeStamp = -1;
    currentAudioTimeStampStart = -1;
    currentAudioTimeStampEnd = 0;
    fakedAudioTimeStampStart = 0;
    fakedAudioTimeStampEnd = 0;
    packetBuffer = buffer;
    audioCodecContext = 0;
    multiplier = 0;
    bytesToSamplesDivisor = 0;

    // Test file named abyss21.avi when decoding audio overruns the buffer if not around this size
    // libavcodec API is a bit screwed in that you can't pass the size of the output array to it
    bufferedSamples = new uchar[AVCODEC_MAX_AUDIO_FRAME_SIZE*8];
}


AudioCodecContext::~AudioCodecContext()
{
    // qDebug("AudioCodecContext::~AudioCodecContext()");
    close();
    delete bufferedSamples;
    // qDebug("AudioCodecContext::~AudioCodecContext() done");
}


void AudioCodecContext::fileInit()
{
    FFAutoLockUnlockMutex contextLock(audioMutex);
    FFAutoLockUnlockMutex timestampLock(timeStampMutex);
    FFAutoLockUnlockMutex audioStreamsLock(audioStreamsMutex);
    audioStreamsCount = 0;
    fakedAudioTimeStampStart = 0;
    fakedAudioTimeStampEnd = 0;
    lastAudioTimeStamp = -1;
    currentAudioTimeStampStart = -1;
    currentAudioTimeStampEnd = 0;
    audioCodec = 0;
    audioCodecContext = 0;
    bufferedSamplesCount = 0;
    bufferedBytesRemaining = 0;
    totalAudioBytesRead = 0;
    totalAudioSamplesDecoded = 0;
    multiplier = 0;
    bytesToSamplesDivisor = 0;
}


long AudioCodecContext::bitRate()
{
    FFAutoLockUnlockMutex lock(audioMutex);
    if ( audioCodecContext ) {
	if ( audioCodecContext->bit_rate )
	    return audioCodecContext->bit_rate;
	if ( totalAudioBytesRead > 100 ) {
	    long long audioBitRate = (long long)totalAudioBytesRead * 8 * audioCodecContext->sample_rate;
	    return (long)(audioBitRate / totalAudioSamplesDecoded);
	}
    }
    return -1;
}


long AudioCodecContext::absoluteTimeStamp()
{
    FFAutoLockUnlockMutex lock(timeStampMutex);
    if ( !multiplier )
	return -1;
    if ( currentAudioTimeStampStart != -1 )
        return currentAudioTimeStampStart / multiplier;
    return -1;
}


long AudioCodecContext::timeStamp()
{
    FFAutoLockUnlockMutex lock(timeStampMutex);
    if ( !multiplier )
	return -1;
    if ( currentAudioTimeStampStart == -1 ) {
	long fakeTimeStamp = fakedAudioTimeStampStart / multiplier + audioTime.elapsed();
	return QMIN(fakeTimeStamp, fakedAudioTimeStampEnd / multiplier);
    }
    long estimatedTimeStamp = currentAudioTimeStampStart / multiplier + audioTime.elapsed();
    return QMIN(estimatedTimeStamp, currentAudioTimeStampEnd / multiplier);
}


QString AudioCodecContext::fileInfo()
{
    FFAutoLockUnlockMutex lock(audioMutex);
    QString audioInfo = "";
    if ( audioCodecContext ) {
	if ( audioCodecContext->codec )
	    audioInfo += qApp->translate( "LibFFMpegPlugin", "Audio: " ) +  audioCodecContext->codec->name + ",";
	audioInfo += qApp->translate( "LibFFMpegPlugin", "Rate: " ) + QString::number( audioCodecContext->bit_rate ) + ",";
	audioInfo += qApp->translate( "LibFFMpegPlugin", "Chans: " ) + QString::number( audioCodecContext->channels ) + ",";
	audioInfo += qApp->translate( "LibFFMpegPlugin", "Freq: " ) + QString::number( audioCodecContext->sample_rate ) + ",";
    }
    return audioInfo;
}


void AudioCodecContext::flush()
{
    FFAutoLockUnlockMutex lock(audioMutex);
    if ( audioCodecContext )
	avcodec_flush_buffers( audioCodecContext );
    bufferedSamplesCount = 0;
    bufferedBytesRemaining = 0;
}


void AudioCodecContext::close()
{
    FFAutoLockUnlockMutex lock(audioMutex);
    FFAutoLockUnlockMutex audioStreamsLock(audioStreamsMutex);
    audioStreamsCount = 0;
    if ( audioCodecContext )
	avcodec_flush_buffers( audioCodecContext );
    if ( audioCodecContext )
	avcodec_close( audioCodecContext );
    audioCodecContext = 0;
    bufferedSamplesCount = 0;
    bufferedBytesRemaining = 0;
}


int AudioCodecContext::audioStreams()
{
    FFAutoLockUnlockMutex audioStreamsLock(audioStreamsMutex);
    return audioStreamsCount;
}


int AudioCodecContext::audioChannels( int )
{
    FFAutoLockUnlockMutex lock(audioMutex);
    return (audioCodecContext) ? audioCodecContext->channels : 2;
}


int AudioCodecContext::audioFrequency( int )
{
    FFAutoLockUnlockMutex lock(audioMutex);
    return (audioCodecContext) ? audioCodecContext->sample_rate : 44100;
}


bool AudioCodecContext::audioReadSamples(short *_output, int channels, long samples, long& samplesRead)
{
    // qDebug("AudioCodecContext::audioReadSamples()");
    FFAutoLockUnlockMutex lock(audioMutex);

    samplesRead = 0;

    if ( !audioCodecContext || !audioCodecContext->codec || !channels || !_output ) {
	qDebug("No audio decoder for stream");
	return false;
    }

    uchar *output = (uchar *)_output;
    long outputSize = samples * channels * 2;
    long outputBytesRemaining = outputSize;

    if ( outputSize > 1000 )
	outputSize -= 1000;

    timeStampMutex.lock();
    int64_t newTimeStamp = lastAudioTimeStamp;
    long bufferedOffsetFromTimeStamp = 0;
    timeStampMutex.unlock();

    while ( bufferedBytesRemaining < outputBytesRemaining + outputSize ) {

	MediaPacket *pkt = packetBuffer->getAnotherAudioPacket();
	if ( !pkt ) {
	    if ( !samplesRead ) {
	        samplesRead = -1;
		qDebug("Audio EOF");
		return false; // EOF
	    } else {
		qDebug("Audio just about EOF");
		// updateTimeStamps(lastAudioTimeStamp, newTimeStamp, samplesRead); 
		// Play the buffered samples, then next time this is called, it will be the real EOF
		return true;
	    }
	}
	if ( pkt->msecOffset ) {
	    newTimeStamp = pkt->msecOffset * multiplier;
	    bufferedOffsetFromTimeStamp = 0;
	}
	int len = pkt->pkt.size;
	unsigned char *ptr = pkt->pkt.data;
	int bytesRead = 0;

	while ( len ) {
	    int ret = 0;

	    if ( pkt ) {
		if ( ptr <  pkt->pkt.data ) {
		    qDebug("inconsistancy error");
		    return FALSE;
		}

		if ( ptr ) {
		    ret = avcodec_decode_audio(audioCodecContext, (short*)(bufferedSamples + bufferedBytesRemaining), &bytesRead, ptr, len);
		    // sched_yield();
		    if ( bytesRead > 0 ) {
			bufferedBytesRemaining += bytesRead;
			bufferedOffsetFromTimeStamp += bytesRead;
		    } else if ( bytesRead < 0 ) {
			qDebug("read count < 0, %i", bytesRead );
		    }
		}
	    }

	    if ( ret < 0 ) {
		qDebug("Error while decoding audio stream");
		delete pkt;
		return FALSE;
	    }
	    ptr += ret;
	    len -= ret;
	}

        totalAudioBytesRead += pkt->pkt.size;
	delete pkt;
    }

    samplesRead = samples;
    bufferedOffsetFromTimeStamp -= bufferedBytesRemaining; // bytes not copied over yet
    bufferedOffsetFromTimeStamp += qssBufferSize;	   // bytes queued
    memcpy( output, bufferedSamples, outputBytesRemaining );
    memmove( bufferedSamples, bufferedSamples + outputBytesRemaining, bufferedBytesRemaining - outputBytesRemaining );
    bufferedBytesRemaining -= outputBytesRemaining;
    samplesRead = samples;


#define NO_DEBUG
#ifndef NO_DEBUG
// simulate slow device
//usleep( samples * 200 / audioCodecContext->sample_rate ); // simulate 20% load for audio decoding
#endif

    updateTimeStamps(lastAudioTimeStamp, newTimeStamp, samplesRead, bufferedOffsetFromTimeStamp );

    totalAudioSamplesDecoded += samplesRead;

    // qDebug("AudioCodecContext::audioReadSamples() finished");
    return TRUE;
}


void AudioCodecContext::updateTimeStamps( int64_t prevStamp, int64_t newStamp, long samples, long offset )
{
    FFAutoLockUnlockMutex lock(timeStampMutex);

    int64_t duration = samples * 1000;

    if ( newStamp == prevStamp ) {
	currentAudioTimeStampEnd += duration;
	fakedAudioTimeStampEnd += duration;
    } else {
	if ( bytesToSamplesDivisor )
	    newStamp -= offset / bytesToSamplesDivisor;
	lastAudioTimeStamp = newStamp;
	currentAudioTimeStampStart = newStamp;
	currentAudioTimeStampEnd = newStamp + duration;
	fakedAudioTimeStampStart = fakedAudioTimeStampEnd;
	fakedAudioTimeStampEnd = fakedAudioTimeStampStart + duration;
	audioTime.start();
    }
}


void AudioCodecContext::leadInAudio()
{
    qDebug("AudioCodecContext::leadInAudio()");
    flush();
    short buffer[20000];
    long samplesRead;
    audioReadSamples(buffer, 2, 8000, samplesRead); // Read 8000 samples
    FFAutoLockUnlockMutex audioLock(audioMutex);
    bufferedSamplesCount = 0;
    bufferedBytesRemaining = 0;
    qDebug("AudioCodecContext::leadInAudio() finished");
}


bool AudioCodecContext::syncToTimeStamp(long currentVideoTimeStamp)
{
    qDebug("AudioCodecContext::syncToTimeStamp()");
    FFAutoLockUnlockMutex audioLock(audioMutex);
    FFAutoLockUnlockMutex lock(timeStampMutex);

    bool eof = false;
    bool keepDecoding = true;
    while ( keepDecoding ) {
	MediaPacket *pkt = packetBuffer->getAnotherAudioPacket();
	if ( pkt && pkt->msecOffset )
	    currentAudioTimeStampStart = pkt->msecOffset * multiplier;
	if ( pkt ) {
	    if ( currentVideoTimeStamp != -1 && currentAudioTimeStampStart != -1 && multiplier ) {
		// qDebug("keep decoding audio, case A");
		keepDecoding = ( currentVideoTimeStamp > currentAudioTimeStampStart / multiplier );
	    } else {
		// qDebug("keep decoding audio, case B");
		keepDecoding = ( packetBuffer->distanceBetweenAudioAndVideo() < 0 );
	    }
	    delete pkt;
	} else {
	    keepDecoding = false;
	    eof = true;
	}
	qDebug("AudioCodecContext::syncToTimeStamp() removed packet");
    }

    // Flush any libavcodec buffers
    if ( audioCodecContext )
	avcodec_flush_buffers( audioCodecContext );

    lastAudioTimeStamp       = currentAudioTimeStampStart;
    currentAudioTimeStampEnd = currentAudioTimeStampStart + 1000 * multiplier;
    fakedAudioTimeStampStart = 0;
    fakedAudioTimeStampEnd = 0;

    // Reset these after seeking till we start to get new ones again
    bufferedSamplesCount = 0;
    bufferedBytesRemaining = 0;

    qDebug("AudioCodecContext::syncToTimeStamp() finished");
    return !eof;
}


void AudioCodecContext::codecInit()
{
    FFAutoLockUnlockMutex lock(audioMutex);
    FFAutoLockUnlockMutex audioStreamsLock(audioStreamsMutex);
    audioCodecContext = packetBuffer->audioContext();
    if ( audioCodecContext && audioCodecContext->sample_rate )
        multiplier = audioCodecContext->sample_rate;
    else
        multiplier = 1;
    if ( audioCodecContext && audioCodecContext->channels)
	bytesToSamplesDivisor = audioCodecContext->channels * 2;
    else
	bytesToSamplesDivisor = 2;
    if ( bytesToSamplesDivisor )
	fakedAudioTimeStampStart -= qssBufferSize / bytesToSamplesDivisor;
    fakedAudioTimeStampEnd = fakedAudioTimeStampStart;
    audioStreamsCount = (audioCodecContext) ? 1 : 0;
}


