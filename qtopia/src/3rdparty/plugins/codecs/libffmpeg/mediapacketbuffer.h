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
#ifndef MEDIA_PACKET_BUFFER_H
#define MEDIA_PACKET_BUFFER_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>

extern "C" {
#include "avcodec.h"
#include "avformat.h"
};

#include <qlist.h>
#include <qstring.h>
#include <qfile.h>

#include "ffmutex.h"
#include "mediapacket.h"


//
// Thread Safe QList of MediaPackets
//
// Contains a QList instead of inheriting from QList so that the API is restricted to only
// these functions which have been made thread safe.
//
// thread safe, contention safe
class MediaPacketList {
public:
    MediaPacketList() {
	FFAutoLockUnlockMutex lock(mutex);
	list = new QList<MediaPacket>();
	list->setAutoDelete(true);
    }
    ~MediaPacketList() {
	FFAutoLockUnlockMutex lock(mutex);
	qDebug("deleting list");
	delete list;
	qDebug("deleted list");
    }
    void append(MediaPacket *pkt) {
	FFAutoLockUnlockMutex lock(mutex);
	list->append(pkt);
    }
    long fileOffset() {
	FFAutoLockUnlockMutex lock(mutex);
	if ( !list->first() )
	    return 0;
	return list->first()->fileOffset;
    }
    MediaPacket *first() {
	FFAutoLockUnlockMutex lock(mutex);
	return list->first();
    }
    int count() {
	FFAutoLockUnlockMutex lock(mutex);
	return list->count();
    }
    void clear() {
	FFAutoLockUnlockMutex lock(mutex);
	list->clear();
    }
    MediaPacket *next() {
	FFAutoLockUnlockMutex lock(mutex);
	if ( list->count() )
	    return list->take(0);
	return 0;
    }
private:
    FFMutex mutex;
    QList<MediaPacket> *list;
}; 


class MediaPacketBuffer {
public:
    // thread safe, contention safe
    MediaPacketBuffer();

    // thread ??, contention ??
    virtual ~MediaPacketBuffer();

    // thread safe, contention safe
    bool open(const QString &fileName);

    // thread safe, contention safe
    void start(); // begins the buffering after a file has been opened

    // thread safe, contention safe
    long duration() {
	FFAutoLockUnlockMutex streamLock(avFormatMutex);
	if ( avFormatContext )
	    return avFormatContext->duration / (AV_TIME_BASE/1000); // Convert to milliseconds
	return 0;
    }

    // thread safe, contention safe
    long bitRate() {
	FFAutoLockUnlockMutex streamLock(avFormatMutex);
	if ( avFormatContext )
	    return avFormatContext->bit_rate;
	return 0;
    }

    // thread safe, contention safe
    long fileLength() {
	FFAutoLockUnlockMutex streamLock(avFormatMutex);
	if ( avFormatContext )
	    return avFormatContext->file_size;
	return 0;
    }

    // thread safe, contention safe
    long pos() {
	long audioFileOffset = waitingAudioPackets.fileOffset();
	long videoFileOffset = waitingVideoPackets.fileOffset();
	//FFAutoLockUnlockMutex streamLock(avFormatMutex);
	if ( audioFileOffset > fileOffset )
	    fileOffset.assign(audioFileOffset);
	if ( videoFileOffset > fileOffset )
	    fileOffset.assign(videoFileOffset);
	return (int)fileOffset;
    }

    // thread safe, contention unsafe (accesses packet lists inside the mutex, probably okay)
    bool seek(long pos) {
	qDebug("MediaPacketBuffer::seek(%li)", pos);

	if ( !isOpen() ) {
	    qDebug("Can not seek, no file open");
	    return false;
	}

	// Seek in to the file
	FFAutoLockUnlockMutex streamLock(avFormatMutex);
        int64_t file_size = avFormatContext->file_size;
        av_seek_frame( avFormatContext, -1, avFormatContext->start_time + ( file_size ? pos*avFormatContext->duration/avFormatContext->file_size : 0 ), 0 );
	fileOffset.assign(pos);
	waitingAudioPackets.clear();
	waitingVideoPackets.clear();
	//bufferMutex.signal();
	//startBufferingPackets();
	//qDebug("seek start I");
	return true;
    }

    void flushAudioPackets() {
	waitingAudioPackets.clear();
    }

    void flushVideoPackets() {
	waitingVideoPackets.clear();
    }

    // thread safe, contention unsafe (accesses packet lists inside the mutex, probably okay)
    void flush() {
	FFAutoLockUnlockMutex streamLock(avFormatMutex);
	waitingAudioPackets.clear();
	waitingVideoPackets.clear();
    }

    // thread safe, contention safe
    long distanceBetweenAudioAndVideo() {
	long audioFileOffset = waitingAudioPackets.fileOffset();
	long videoFileOffset = waitingVideoPackets.fileOffset();
	if ( !audioFileOffset || !videoFileOffset )
	    return 0;
	return audioFileOffset - videoFileOffset;
    }

    // thread safe, contention safe
    AVCodecContext *audioContext() {
	FFAutoLockUnlockMutex streamLock(avFormatMutex);
	return audioCodecContext;
    }

    // thread safe, contention safe
    AVCodecContext *videoContext() {
	FFAutoLockUnlockMutex streamLock(avFormatMutex);
	return videoCodecContext;
    }

    // thread safe, contention safe
    void init() {
	FFAutoLockUnlockMutex streamLock(avFormatMutex);
	fileOffset.assign(0);
	audioCodecContext = 0;
	videoCodecContext = 0;
	audioStream.assign(-1);
	videoStream.assign(-1);
	avFormatContext = 0;
	openFlag.assign(0);
    }

    bool isJoin() {
	FFAutoLockUnlockMutex streamLock(bufferMutex);
	return joined;
    }

    void setJoin(bool j) {
	FFAutoLockUnlockMutex streamLock(bufferMutex);
	joined = j;
    }
    void join() {
	setJoin(true);
	bufferMutex.signal();
	pthread_yield();
	bufferMutex.signal();
	pthread_yield();
	bufferMutex.signal();
	pthread_join(threadId, 0);
    }

    // thread safe, contention safe
    void close() {
	{
	    FFAutoLockUnlockMutex streamLock(avFormatMutex);
	    audioCodecContext = 0;
	    videoCodecContext = 0;
	    if ( avFormatContext )
		av_close_input_file( avFormatContext );
	    avFormatContext = 0;
	    openFlag.assign(0);
	}
	bufferMutex.signal();
    }

    // thread safe, contention safe
    bool isOpen() {
	return (bool)openFlag;
    }

/*
    long getLastPacketTime() {
	// Jump to near the end of the file
	setPos(length() - 10000);
	long lastTimeStamp = 0;

	// Read packets till we get to the end to try and get the last timestamp available
	MediaPacket *pkt;
	while ( (pkt = getPacket()) ) {
	    long long pktTime = pkt.msecOffset;
	    if ( pktTime > lastTimeStamp )
		lastTimeStamp = pktTime;
	    delete pkt;
	}

	// Jump back to the beginning so we are ready to decode
	setPos(0);
	return lastTimeStamp;
    }
*/

    // thread safe, contention safe
    MediaPacket *getAnotherVideoPacket();

    // thread safe, contention safe
    MediaPacket *getAnotherAudioPacket();

    // semi-thread safe, it reads the openFlag without locking
    bool startBufferingPackets();

private:
    static void *startThreadLoop(void *arg);
    MediaPacket *getPacket();

    ThreadSafeInteger openFlag;

    MediaPacketList waitingVideoPackets;
    MediaPacketList waitingAudioPackets;
    FFMutex avFormatMutex;
    FFMutex bufferMutex;
    AVFormatContext *avFormatContext;
    AVCodecContext *audioCodecContext;
    AVCodecContext *videoCodecContext;
    ThreadSafeInteger videoStream;
    ThreadSafeInteger audioStream;
    ThreadSafeInteger fileOffset;
    bool joined;
    pthread_t threadId;
};


#endif // MEDIA_PACKET_BUFFER_H

