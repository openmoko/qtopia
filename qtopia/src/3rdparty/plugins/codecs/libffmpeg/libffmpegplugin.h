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
#ifndef LIBFFMPEG_PLUGIN_H
#define LIBFFMPEG_PLUGIN_H

#include "libffmpegplugin.h"

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
#include <qapplication.h>
#include <qtopia/mediaplayerplugininterface.h>

#include "ffmutex.h"
#include "mediapacket.h"
#include "mediapacketbuffer.h"
#include "videocodeccontext.h"
#include "audiocodeccontext.h"


class LibFFMpegPlugin : public MediaPlayerDecoder_1_6 {

public:
    LibFFMpegPlugin();
    ~LibFFMpegPlugin();
    const char *pluginName();
    const char *pluginComment();
    double pluginVersion();

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
    FFMutex pluginMutex;
    QString strInfo;
    bool streamingFlag;
    long fileLength;
    int lengthScaleFactor;
    bool haveTotalTimeCache;
    long totalTimeCache;
    bool needPluginInit;
    long totalBitRate;
    bool haveTotalBitRate;
    ThreadSafeInteger savedSeekPos;

    MediaPacketBuffer	packetBuffer;
    VideoCodecContext	videoContext;
    AudioCodecContext	audioContext;

    bool init(const QString& fileName);
    bool realSeek();
    void flushBuffers();
};


#endif

