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
#include "mediapacketbuffer.h"


#define PACKET_BUFFER_SIZE    1000


MediaPacketBuffer::MediaPacketBuffer() : avFormatMutex(), bufferMutex()
{
    setJoin(false);

    FFAutoLockUnlockMutex streamLock(avFormatMutex);
    FFAutoLockUnlockMutex bufferLock(bufferMutex);
    avFormatContext = 0;
    openFlag.assign(0);
    pthread_create(&threadId, 0, (void *(*)(void *))MediaPacketBuffer::startThreadLoop, this);
}


MediaPacketBuffer::~MediaPacketBuffer()
{
    // qDebug("MediaPacketBuffer::~MediaPacketBuffer()");
    join();
    // qDebug("MediaPacketBuffer::~MediaPacketBuffer() finished");
}


void *MediaPacketBuffer::startThreadLoop(void *arg)
{
    MediaPacketBuffer *context = (MediaPacketBuffer *)arg;
    qDebug("media packet buffer thread started");
    while ( !context->isJoin() ) {
	// qDebug("media packet buffer waiting for file open");
	context->bufferMutex.wait(); // EOF or stopped, wait until we are signaled again (another file opened or seeked)
	qDebug("media packet buffer thread resumed");
	while ( context->isOpen() ) {
	    // qDebug("buffering packets");
	    context->startBufferingPackets();
	    context->bufferMutex.timedWait(200000); // Buffer has been filled, wait 200ms for buffer to get used a bit
	    // qDebug("media packet buffer waiting for packets to be used");
	}
	qDebug("media packet buffer thread paused");
    }
    sched_yield(); // So we can wait for the pthread_join()
    qDebug("media packet buffer thread ended");
    return 0;
}


bool MediaPacketBuffer::open(const QString &fileName)
{
    FFAutoLockUnlockMutex streamLock(avFormatMutex);
    FFAutoLockUnlockMutex bufferLock(bufferMutex);

    // open the input file with generic libav function
    if ( av_open_input_file(&avFormatContext, QFile::encodeName(fileName).data(), NULL, 0, 0) < 0 ) {
	// strInfo = qApp->translate( "LibFFMpegPlugin", "Error: Could not open stream: " ) + fileName;
	qDebug("  Error opening %s", fileName.latin1() );
	avFormatContext = 0;
	openFlag.assign(0);
	return false;
    }
    qDebug("  Opened %s", fileName.latin1() );

    // Decode first frames to get stream parameters (for some stream types like mpeg)
    if ( !avFormatContext || av_find_stream_info(avFormatContext) < 0 ) {
	qDebug("  Error getting parameters for %s", fileName.latin1() );
	if ( avFormatContext )
	    av_close_input_file( avFormatContext );
	avFormatContext = 0;
	openFlag.assign(0);
	return false;
    }

    audioCodecContext = 0;
    videoCodecContext = 0;

    // update the current parameters so that they match the one of the input stream
    for ( int i = 0; i < avFormatContext->nb_streams; i++ ) {
	AVCodecContext *enc = &avFormatContext->streams[i]->codec;
	AVCodec *c = avcodec_find_decoder( enc->codec_id );
	if ( !c ) {
	    qDebug("  Unsupported codec for stream[%i], with codec id %i", i, enc->codec_id);
	} else if ( avcodec_open( enc, c ) < 0 ) {
	    qDebug("  Error opening codec for stream[%i], with codec id %i, codec: %s", i, enc->codec_id, c->name);
	} else {
	    switch (enc->codec_type) {
		case CODEC_TYPE_AUDIO:
		    qDebug("  Found audio stream[%i], with codec id %2i, codec: %s", i, enc->codec_id, c->name);
		    audioStream.assign(i);
		    audioCodecContext = enc;
		    break;
		case CODEC_TYPE_VIDEO:
		    qDebug("  Found video stream[%i], with codec id %2i, codec: %s", i, enc->codec_id, c->name);
		    videoStream.assign(i);
		    videoCodecContext = enc;
		    break;
		default:
		    qDebug("  Found unknown stream type, stream[%i], with codec id %2i, codec: %s", i, enc->codec_id, c->name);
		    break;
	    }
	}
    }

    openFlag.assign(1);

    return true;
}


void MediaPacketBuffer::start()
{
    bufferMutex.signal();
    bufferMutex.signal();
}


MediaPacket *MediaPacketBuffer::getPacket()
{
    FFAutoLockUnlockMutex streamLock(avFormatMutex);

    // read a packet from input
    AVPacket avPkt;
    int readFailures = 0;

    // Wait for file to be opened and successful read
    while (!avFormatContext || av_read_frame(avFormatContext, &avPkt) < 0) {
	if ( avFormatContext && url_feof( &avFormatContext->pb ) )
	    return 0; // EOF
	readFailures++;
	if ( readFailures > 20 ) {
	    qDebug("too many read failures, assuming EOF");
	    return 0; // EOF, clearly we can't get any more packets out, and we aren't getting an eof, something wrong
	}
	avFormatMutex.unlock();
	avFormatMutex.timedWait(100000); // wait a moment, this could be streamed data which takes time to read
	avFormatMutex.lock();
    }
    av_dup_packet( &avPkt );

    MediaPacket *pkt = new MediaPacket(avPkt);
    pkt->fileOffset = url_ftell( &avFormatContext->pb );
    
    // convert pts (presentation timestamp) to milliseconds
    AVRational time_base = avFormatContext->streams[avPkt.stream_index]->time_base;
    if ((unsigned long long)avPkt.pts != AV_NOPTS_VALUE && time_base.den)
	pkt->msecOffset = avPkt.pts * 1000 * time_base.num / time_base.den;
    else
        pkt->msecOffset = 0;

    return pkt;
}


bool MediaPacketBuffer::startBufferingPackets()
{
    int bufferedPackets = waitingAudioPackets.count() + waitingVideoPackets.count();
    while ( isOpen() && bufferedPackets <= PACKET_BUFFER_SIZE ) {
	MediaPacket *pkt = getPacket();
	if ( pkt ) {
	    if ( pkt->pkt.stream_index == (int)audioStream ) {
		waitingAudioPackets.append( pkt );
	    } else if ( pkt->pkt.stream_index == (int)videoStream ) {
		waitingVideoPackets.append( pkt );
	    }
	} else {
	    FFAutoLockUnlockMutex streamLock(avFormatMutex);
	    MediaPacket *pkt1 = new MediaPacket();
            pkt1->fileOffset = pkt1->msecOffset = 0;
	    MediaPacket *pkt2 = new MediaPacket();
            pkt2->fileOffset = pkt2->msecOffset = 0;
	    waitingAudioPackets.append(pkt1);
	    waitingVideoPackets.append(pkt2);
	    qDebug("media packet buffering stopped, EOF");
	    return false;
	}
	bufferedPackets = waitingAudioPackets.count() + waitingVideoPackets.count();
    }
    // qDebug("media packet buffers refilled");
    avFormatMutex.signal();
    return isOpen();
}


MediaPacket *MediaPacketBuffer::getAnotherVideoPacket()
{
    MediaPacket *pkt = 0;
    while ( !pkt && (int)videoStream != -1 ) {
	pkt = waitingVideoPackets.next();
	if ( pkt ) {
	    if ( pkt->eof ) {
		qDebug("Video EOF");
		delete pkt;
		return 0;
	    }
	    return pkt;
	}
	if ( waitingAudioPackets.count() >= PACKET_BUFFER_SIZE )
	    waitingAudioPackets.clear();
	bufferMutex.signal(); // restart buffering thread
	qDebug("Video packet buffer empty");
	avFormatMutex.timedWait(10000); // wait a moment for more packets to get buffered
    }
    qDebug("No video stream");
    return 0;
}


MediaPacket *MediaPacketBuffer::getAnotherAudioPacket()
{
    MediaPacket *pkt = 0;
    while ( !pkt && (int)audioStream != -1 ) {
	pkt = waitingAudioPackets.next();
	if ( pkt ) {
	    if ( pkt->eof ) {
		qDebug("Audio EOF");
		delete pkt;
		return 0;
	    }
	    return pkt;
	}
	if ( waitingVideoPackets.count() >= PACKET_BUFFER_SIZE ) // There might be video packets encoded with a video codec we
	    waitingVideoPackets.clear();			 // don't support and therefore the packets aren't getting consumed
	bufferMutex.signal(); // restart buffering thread
	qDebug("Audio packet buffer empty");
	avFormatMutex.timedWait(10000); // wait a moment for more packets to get buffered
    }
    qDebug("No audio stream");
    return 0;
}


