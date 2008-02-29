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
#include "videocodeccontext.h"


VideoCodecContext::VideoCodecContext(MediaPacketBuffer *buffer)
{
    FFAutoLockUnlockMutex videoLock(videoMutex);
    packetBuffer = buffer;
    videoScaleContext.init();
    videoCodecContext = 0;
    currentVideoPacket = 0;
    resetTimeStamps();
}


VideoCodecContext::~VideoCodecContext()
{
    //qDebug("VideoCodecContext::~VideoCodecContext()");
    close();
    delete currentVideoPacket;
    //qDebug("VideoCodecContext::~VideoCodecContext() done");
}


void VideoCodecContext::fileInit()
{
    FFAutoLockUnlockMutex videoLock(videoMutex);
    resetTimeStamps();
    videoCodec = 0;
    videoCodecContext = 0;
    frame = 0;
    skipNext = 0;
    droppedFrames = 0;
    framesInLastPacket = 0;
    totalFrames = 0;
    totalVideoBytesRead = 0;
    totalVideoFramesDecoded = 0;
}


void VideoCodecContext::codecInit()
{
    FFAutoLockUnlockMutex videoLock(videoMutex);
    videoCodecContext = packetBuffer->videoContext();
    if ( videoCodecContext )
	videoCodecContext->hurry_up = 0;

    FFAutoLockUnlockMutex timeStampLock(timeStampMutex);
    //qDebug("videoCodecContext->time_base=%d/%d",videoCodecContext->time_base.num,videoCodecContext->time_base.den);
    if ( videoCodecContext ) {
	long rate = videoCodecContext->time_base.num;
	frameRateNumerator = ( rate ) ? rate * 1000LL : 1000;
	rate = videoCodecContext->time_base.den;
	frameRateDenominator = ( rate ) ? rate : 25;
    } else {
	frameRateDenominator = 1;
	frameRateNumerator = 1;
    }
}


long VideoCodecContext::absoluteTimeStamp()
{
    FFAutoLockUnlockMutex timeStampLock(timeStampMutex);
    return currentVideoTimeStamp + currentFrameOffsetFromTimeStamp * frameRateNumerator / frameRateDenominator;
}


long VideoCodecContext::timeStamp()
{
    FFAutoLockUnlockMutex timeStampLock(timeStampMutex);
    if ( currentVideoTimeStamp == -1 )
	return fakedVideoTimeStamp + fakedFrameOffsetFromTimeStamp * frameRateNumerator / frameRateDenominator;
    return currentVideoTimeStamp + currentFrameOffsetFromTimeStamp * frameRateNumerator / frameRateDenominator;
}


void VideoCodecContext::close()
{
    FFAutoLockUnlockMutex videoLock(videoMutex);
    if ( videoCodecContext )
	avcodec_close( videoCodecContext );
    videoCodecContext = 0;
}


int VideoCodecContext::decodeFrame()
{
    int gotPicture = 0;
    while (!gotPicture) {
	if ( !currentVideoPacket || videoPacketLen <= 0 ) {
	    if ( currentVideoPacket )
		framesInLastPacket = frameInPacket;
	    delete currentVideoPacket;
	    currentVideoPacket = packetBuffer->getAnotherVideoPacket();
	    if ( !currentVideoPacket )
		return 0; // EOF
	    videoPacketPtr = currentVideoPacket->pkt.data;
	    videoPacketLen = currentVideoPacket->pkt.size;
	    frameInPacket = 0;
	    if ( currentVideoPacket->msecOffset ) {
		FFAutoLockUnlockMutex timeStampLock(timeStampMutex);
		currentVideoTimeStamp = currentVideoPacket->msecOffset;
		currentFrameOffsetFromTimeStamp = 0;
	    }
	}

	int ret = avcodec_decode_video(videoCodecContext, &picture, &gotPicture, videoPacketPtr, videoPacketLen);
	//sched_yield();
	if ( ret >= 0 ) {
	    totalVideoBytesRead += ret;
	    videoPacketPtr += ret;
	    videoPacketLen -= ret;
	} else {
	    qDebug("Error while decoding video stream");
	    delete currentVideoPacket;
	    currentVideoPacket = 0;
	    return -1;
	}
	if ( gotPicture ) {
#define NO_DEBUG
#ifndef NO_DEBUG
// Fake a slow device
//usleep(30);
#endif
	    FFAutoLockUnlockMutex timeStampLock(timeStampMutex);
	    totalVideoFramesDecoded++;
	    frameInPacket++;
	    if ( currentVideoTimeStamp != -1 )
		currentFrameOffsetFromTimeStamp++;
	    fakedFrameOffsetFromTimeStamp++;
	}
    }
    return 1;
}


long VideoCodecContext::bitRate()
{
    FFAutoLockUnlockMutex videoLock(videoMutex);
    if ( videoCodecContext ) {
	if ( videoCodecContext->bit_rate )
	    return videoCodecContext->bit_rate;
	if ( totalVideoBytesRead > 100 ) {
	    return (long)((double)totalVideoBytesRead * 8
		    * totalVideoFramesDecoded * videoCodecContext->time_base.num
		    / videoCodecContext->time_base.den );
	}
    }
    return -1;
}


QString VideoCodecContext::fileInfo()
{
    FFAutoLockUnlockMutex videoLock(videoMutex);
    QString videoInfo;
    if ( videoCodecContext ) {
	if ( videoCodecContext->codec )
	    videoInfo += qApp->translate( "LibFFMpegPlugin", "Video: " ) + videoCodecContext->codec->name + ",";
	videoInfo += qApp->translate( "LibFFMpegPlugin", "Rate: " ) + QString::number( videoCodecContext->bit_rate ) + ",";
	videoInfo += qApp->translate( "LibFFMpegPlugin", "Width: " ) + QString::number( videoCodecContext->width ) + ",";
	videoInfo += qApp->translate( "LibFFMpegPlugin", "Height: " ) + QString::number( videoCodecContext->height ) + ",";
    }
    return videoInfo;
}


int VideoCodecContext::videoStreams()
{
    FFAutoLockUnlockMutex lock(videoMutex);
    return (videoCodecContext) ? 1 : 0;
}


int VideoCodecContext::videoWidth( int )
{
    FFAutoLockUnlockMutex lock(videoMutex);
    return (videoCodecContext) ? videoCodecContext->width : 0;
}


int VideoCodecContext::videoHeight( int )
{
    FFAutoLockUnlockMutex lock(videoMutex);
    return (videoCodecContext) ? videoCodecContext->height : 0;
}


double VideoCodecContext::videoFrameRate( int )
{
    FFAutoLockUnlockMutex lock(videoMutex);
    return (videoCodecContext && videoCodecContext->time_base.num) ?
            (double)videoCodecContext->time_base.den / videoCodecContext->time_base.num : 1.0;
}


bool VideoCodecContext::videoReadScaledFrame( unsigned char **output_rows, int in_w, int in_h, int out_w, int out_h, ColorFormat fmt )
{
    FFAutoLockUnlockMutex lock(videoMutex);

    if ( !videoCodecContext || !videoCodecContext->codec ) {
	qDebug("No video decoder for stream");
	return 1;
    }

    static int skipCount = 0;
    if ( skipNext ) {
	skipNext--;
	skipCount++;
	if ( skipCount > 100 ) // EOF or we are decoding way too slow... give up
	    return 1;
	qDebug("skip next frame");
	return 0;
    }
    skipCount = 0;

    int ret = decodeFrame();
    // qDebug("decoded frame");
    if ( ret < 0 )
	return 0;
    if ( ret == 0 )
	return 1;

    frame = videoCodecContext->frame_number;
    // qDebug("got picture: %i", frame );
    int lineStride = (uchar*)output_rows[1] - (uchar*)output_rows[0];
    // qDebug("scale config");
    if (!videoScaleContext.configure(in_w, in_h, out_w, out_h, &picture, lineStride, videoCodecContext->pix_fmt, fmt))
	return 1;
    // qDebug("scale convert");
    videoScaleContext.convert((uint8_t*)output_rows[0], &picture);

    // qDebug("VideoCodecContext::videoReadScaledFrame() finished");
    return 0;
}


void VideoCodecContext::flush()
{
    qDebug("VideoCodecContext::flush()");
    FFAutoLockUnlockMutex videoLock(videoMutex);
    if ( videoCodecContext )
	avcodec_flush_buffers( videoCodecContext );
    delete currentVideoPacket;
    currentVideoPacket = 0;
    qDebug("VideoCodecContext::flush() finished");
}


bool VideoCodecContext::syncToTimeStamp( long audioTS )
{
    FFAutoLockUnlockMutex videoLock(videoMutex);

    int maxFrames = 10;
    
    // Case where we are syncing after seeking, we should make efforts to get it right
    if ( audioTS == -1 || timeStamp() == -1 )
	maxFrames = 50; // 2 seconds

    // Try to consume up the waiting video packets so we get back in sync with the audio
    while ( maxFrames ) {

	long videoTS = timeStamp();

	if ( audioTS != -1 && videoTS != -1 ) {
	    const int AVSyncSlack = 100;    // 100 ms slack
	    long timeStampDiff = audioTS - videoTS;
	    //qDebug("timeStampDiff: %li", timeStampDiff);
	    if ( timeStampDiff > -AVSyncSlack && timeStampDiff < AVSyncSlack ) // Audio is ahead of the video
		return true;
	    if ( timeStampDiff < -AVSyncSlack ) {  // Video is ahead of the audio
		qDebug("slow down video, timeStampDiff: %li", timeStampDiff);
		skipNext++;
		return true;
	    }
	} else {
	    if ( absoluteTimeStamp() == -1 ) {
		FFAutoLockUnlockMutex timeStampLock(timeStampMutex);
		fakedVideoTimeStamp = audioTS;
		fakedFrameOffsetFromTimeStamp = 0;
	    }
	    const int AVSizeSlack = 50;   // 50 bytes slack
	    long fileOffsetDiff = packetBuffer->distanceBetweenAudioAndVideo() + 500; // offset a bit to compensate for buffered audio
	    // qDebug("fileOffsetDiff: %li", fileOffsetDiff);
	    if ( fileOffsetDiff > -AVSizeSlack && fileOffsetDiff < AVSizeSlack ) // Audio is ahead of the video
		return true;
	    if ( fileOffsetDiff < -AVSizeSlack ) { // Video is ahead of the audio
		qDebug("slow down video, fileOffsetDiff: %li", fileOffsetDiff);
		skipNext++;
		return true;
	    } else {
		qDebug("decode more video, fileOffsetDiff: %li", fileOffsetDiff);
	    }
	}

	// when set to 1 during decoding, b frames will be skiped when
	// set to 2 idct/dequant will be skipped too
	int oldHurryUp = videoCodecContext->hurry_up;
	videoCodecContext->hurry_up = 1;
	int ret = decodeFrame();
	videoCodecContext->hurry_up = oldHurryUp;

	if ( ret == 0 )
	    return false;
	if ( ret > 0 ) {
	    droppedFrames++;
	    maxFrames--;
	    qDebug("frames dropped: %i", droppedFrames);
	}
    }

    return true;
}


void VideoCodecContext::resetTimeStamps()
{
    FFAutoLockUnlockMutex timeStampLock(timeStampMutex);
    delete currentVideoPacket;
    currentVideoPacket = 0;
    currentVideoTimeStamp = -1;
    currentFrameOffsetFromTimeStamp = 0;
    fakedVideoTimeStamp = 0;
    fakedFrameOffsetFromTimeStamp = 0;
}


void VideoCodecContext::leadInVideo()
{
    qDebug("VideoCodecContext::leadInVideo()");
    FFAutoLockUnlockMutex videoLock(videoMutex);

    if ( videoCodecContext )
	avcodec_flush_buffers( videoCodecContext );

    resetTimeStamps();

    // Sync up the input with the packets so
    // we are ready to get the next video frame
    // We have to sync through a keyframe, but not
    // all codecs have keyframes or some have keyframes
    // which are far apart depending on the encoder which created the file.
    // So instead of hoping to find a keyframe, we decode 25 frames so we
    // are reasonably confident we have enough picture complete by then
    // to resume decoding from.
    int framesToDecode = 25;

    // XXX clear picture to black perhaps

    while ( framesToDecode > 0 ) {
	int ret = decodeFrame();
	if ( ret == 0 )
	    break; // EOF
	if ( ret > 0 )
	    framesToDecode--;
    }

    qDebug("VideoCodecContext::leadInVideo() finished");
}


