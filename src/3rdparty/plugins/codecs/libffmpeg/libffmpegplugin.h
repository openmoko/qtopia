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
#include "libavcodec/avcodec.h"
#include "libav/avformat.h"
};

#include "yuv2rgb.h"

#include <qlist.h>
#include <qstring.h>
#include <qapplication.h>

#include <qpe/mediaplayerplugininterface.h>


class LibFFMpegPlugin : public MediaPlayerDecoder {

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

private:
    bool openFlag;
    int frame;

    yuv2rgb_factory_t *YUVFactory;
    bool configured;


    pthread_mutex_t *pluginMutex;
    ReSampleContext *resampleContext;

    int skipNext; // Number of frames to skip

    bool internalOpen( const QString& fileName );

    AVPacket *getAnotherPacket( int stream );

    int totalFrames;
//    int framePosition[ 32000 ];
//    int frameNumber[ 32000 ];
    int totalKeyFrames;
    int keyFramePosition[ 16000 ];
    int keyFrameNumber[ 16000 ];
    void indexStream();

    QList<AVPacket> waitingVideoPackets;
    QList<AVPacket> waitingAudioPackets;

    AVFormatContext *avFormatContext;

    AVCodec *audioCodec;
    AVCodec *videoCodec;

    AVCodecContext *audioCodecContext;
    AVCodecContext *videoCodecContext;

    int videoStream;
    int audioStream;

    QString strInfo;
};


#endif

