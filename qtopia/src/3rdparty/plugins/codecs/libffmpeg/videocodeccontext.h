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
#ifndef VIDEO_CODEC_CONTEXT_H
#define VIDEO_CODEC_CONTEXT_H 


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
#include "mediapacketbuffer.h"
#include "videoscalecontext.h"


class VideoCodecContext {
public:
    VideoCodecContext(MediaPacketBuffer *buffer);
    ~VideoCodecContext();

    void fileInit();
    void codecInit();
    void close();

    int videoStreams();
    int videoWidth( int stream );
    int videoHeight( int stream );
    double videoFrameRate( int stream );

    long bitRate();
    long timeStamp();
    long absoluteTimeStamp();
    void resetTimeStamps();
    QString fileInfo();

    bool videoReadScaledFrame( unsigned char **output_rows, int in_w, int in_h, int out_w, int out_h, ColorFormat fmt );
    bool syncToTimeStamp( long AudioTS );
    void leadInVideo();
    void flush();

private:
    // Helper function, keeps decoding packets until a complete video frame is decoded
    int decodeFrame();

    // Has it's own mutex, this is the packet provider
    MediaPacketBuffer *packetBuffer;

    // Timing variables
    FFMutex timeStampMutex;
    long currentVideoTimeStamp;
    long currentFrameOffsetFromTimeStamp;
    long fakedVideoTimeStamp;
    long fakedFrameOffsetFromTimeStamp;
    int64_t frameRateDenominator;
    int64_t frameRateNumerator;
    long picturePtsOffset;

    FFMutex videoMutex;
    int frame;
    int skipNext; // Number of frames to skip
    int totalFrames;
    int droppedFrames;
    int framesInLastPacket;
    long totalVideoBytesRead;
    long totalVideoFramesDecoded;

    // Contexts
    AVCodec *videoCodec;
    AVCodecContext *videoCodecContext;
    VideoScaleContext videoScaleContext;

    // State associated with decoding the current frame and packet
    AVFrame picture;
    MediaPacket *currentVideoPacket;
    int frameInPacket;
    unsigned char *videoPacketPtr;
    int videoPacketLen;
};


#endif // VIDEO_CODEC_CONTEXT_H 

