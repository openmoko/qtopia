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
#ifndef LIBFFMPEG_PLUGIN_H
#define LIBFFMPEG_PLUGIN_H

#include "libffmpegplugin.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

extern "C" {
#include "avcodec.h"
#include "avformat.h"
};

#include "yuv2rgb.h"

#include <qlist.h>
#include <qstring.h>
#include <qapplication.h>

#include <qtopia/mediaplayerplugininterface.h>


class MediaPacket {
    public:
	int len;
	unsigned char *ptr;
	int frameInPacket;
	AVPacket pkt;
};


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


class AutoLockUnlockMutex {
public:
    AutoLockUnlockMutex( Mutex *m ) : mutex( m ) {
	mutex->lock();
    }
    ~AutoLockUnlockMutex() {
	mutex->unlock();
    }
private:
    Mutex *mutex;
};


class LibFFMpegPlugin : public MediaPlayerDecoder_1_6 {

public:
    LibFFMpegPlugin();
    ~LibFFMpegPlugin();
    const char *pluginName();
    const char *pluginComment();
    double pluginVersion();

    void fileInit();
    void pluginInit();

    bool isFileSupported( const QString& fileName );
    bool open( const QString& fileName );

    bool close();
    bool isOpen();
    const QString &fileInfo();

    int audioStreams();
    int audioChannels( int stream );
    int audioFrequency( int stream );
    int audioSamples( int stream );
    bool audioSetSample( long sample, int stream );
    long audioGetSample( int stream );
    bool audioReadSamples( short *output, int channels, long samples, long& samplesRead, int stream );

    // If decoder doesn't support video then return 0 here
    int videoStreams();
    int videoWidth( int stream );
    int videoHeight( int stream );
    double videoFrameRate( int stream );
    int videoFrames( int stream );

    bool videoSetFrame( long fr, int stream );
    long videoGetFrame( int stream );
    bool videoReadFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, ColorFormat color_model, int stream );
    bool videoReadScaledFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, int out_w, int out_h, ColorFormat color_model, int stream );
    bool videoReadYUVFrame( char *y_output, char *u_output, char *v_output, int in_x, int in_y, int in_w, int in_h, int stream );

    // Profiling
    double getTime();

    // Ignore if these aren't supported
    bool setSMP( int cpus );
    bool setMMX( bool useMMX );

    // Capabilities
    bool supportsAudio();
    bool supportsVideo();
    bool supportsYUV();
    bool supportsMMX();
    bool supportsSMP();
    bool supportsStereo();
    bool supportsScaling();

    long getPlayTime();

    // 1.6 Extensions
    bool supportsStreaming(); // plugin feature
    bool canStreamURL( const QUrl& url, const QString& mimetype ); // Tests if the URL can be streamed
    bool openURL( const QUrl& url, const QString& mimetype ); // Opens a URL for streaming
    bool streamed(); // Is the open file a streamed file

    bool syncAvailable();
    bool sync(); // sync up the video to the catch up to the audio

    bool seekAvailable(); // Availability depends on stream
    bool seek( long pos ); // seek to byte

    bool tellAvailable(); // Hard to imagine why this wouldn't be possible
    long tell(); // byte offset from beginning

    bool lengthAvailable(); // Availability depends on stream
    long length(); // in bytes

    bool totalTimeAvailable(); // Availability depends on stream
    long totalTime(); // in milliseconds

    bool currentTimeAvailable(); // Availability depends on stream
    long currentTime(); // in milliseconds

private:
    bool openFlag;
    bool streamingFlag;
    int frame;

    Mutex pluginMutex;
    Mutex audioMutex;
    Mutex videoMutex;

    int skipNext; // Number of frames to skip

    int totalFrames;

    int msecPerFrame;
    int droppedFrames;
    int fileLength;
    int lengthScaleFactor;
    long currentPacketTimeStamp;
    long currentAudioTimeStamp;
    long currentVideoTimeStamp;
    bool haveTotalTimeCache;
    long totalTimeCache;
    bool needPluginInit;
    int framesInLastPacket;

    int scaleContextDepth;
    int scaleContextInputWidth;
    int scaleContextInputHeight;
    int scaleContextPicture1Width;
    int scaleContextPicture2Width;
    int scaleContextOutputWidth;
    int scaleContextOutputHeight;
    int scaleContextLineStride;
    int scaleContextFormat;

    AVPicture picture;
    MediaPacket *getAnotherPacket( int stream );
    QList<MediaPacket> waitingVideoPackets;
    QList<MediaPacket> waitingAudioPackets;
    void removeCurrentVideoPacket();
    void flushVideoPackets();
    void flushAudioPackets();

    AVCodec *audioCodec;
    AVCodec *videoCodec;

    int videoStream;
    int audioStream;

    QString strInfo;

    AVFormatContext	*streamContext;
    AVCodecContext	*audioCodecContext;
    AVCodecContext	*videoCodecContext;
    ReSampleContext	*audioScaleContext;
    yuv2rgb_factory_t	*videoScaleContext;
};


#endif

