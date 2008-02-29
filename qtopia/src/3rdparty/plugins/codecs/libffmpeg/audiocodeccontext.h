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
#ifndef AUDIO_CODEC_CONTEXT_H
#define AUDIO_CODEC_CONTEXT_H 


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
#include <qdatetime.h>

#include "ffmutex.h"
#include "mediapacket.h"
#include "mediapacketbuffer.h"


class AudioCodecContext {
public:
    AudioCodecContext(MediaPacketBuffer *buffer);   // thread safe
    ~AudioCodecContext();			    // ??

    void fileInit();				    // thread safe
    void codecInit();				    // thread safe
    void close();				    // thread safe

    int audioStreams();				    // thread safe
    int audioChannels(int stream);		    // thread safe
    int audioFrequency(int stream);		    // thread safe

    long bitRate();				    // thread safe
    QString fileInfo();				    // thread safe
    long timeStamp();				    // thread safe
    long absoluteTimeStamp();			    // thread safe

    bool audioReadSamples(short *output, int channels, long samples, long& samplesRead);    // thread safe
    bool syncToTimeStamp(long currentVideoTimeStamp);  // thread safe
    void leadInAudio();				    // thread safe
    void flush();				    // thread safe

private:
    void updateTimeStamps(int64_t start, int64_t prevStart, long samples, long offset); // thread safe

    // has it's own mutex
    MediaPacketBuffer *packetBuffer;

    // audioStreamsMutex mutex protects the audioStreamsCount variable
    FFMutex audioStreamsMutex;
    int audioStreamsCount;	// only updated when both audioStreamsMutex annd audioMutex are locked,
				// but can be read when only audioStreamsMutex is locked.

    // timeStampMutex protects the following variables
    FFMutex timeStampMutex;		// needed to not lock out the sync() function while decoding audio
    int64_t lastAudioTimeStamp;
    int64_t currentAudioTimeStampStart;
    int64_t currentAudioTimeStampEnd;
    int64_t fakedAudioTimeStampStart;
    int64_t fakedAudioTimeStampEnd;
    int64_t multiplier;
    int bytesToSamplesDivisor;
    QTime audioTime;

    // audioMutex protects the following variables
    FFMutex audioMutex;
    uchar *bufferedSamples;
    uchar *tmpSamples;
    int bufferedSamplesCount;
    long bufferedBytesRemaining;
    int64_t totalAudioBytesRead;
    int64_t totalAudioSamplesDecoded;
    AVCodec *audioCodec;
    AVCodecContext *audioCodecContext;
    //ReSampleContext *audioScaleContext;
};


#endif // AUDIO_CODEC_CONTEXT_H 

