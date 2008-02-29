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
#ifndef MEDIA_PLAYER_PLUGIN_INTERFACE_H
#define MEDIA_PLAYER_PLUGIN_INTERFACE_H

#include <qtopia/qcom.h>
#include <qstring.h>
#include <qurl.h>

#ifndef QT_NO_COMPONENT
// {c0093632-b44c-4cf7-a279-d82fe8a8890c}
# ifndef IID_MediaPlayerPlugin
#  define IID_MediaPlayerPlugin QUuid( 0xc0093632, 0xb44c, 0x4cf7, 0xa2, 0x79, 0xd8, 0x2f, 0xe8, 0xa8, 0x89, 0x0c )
# endif
#endif


enum ColorFormat {
    RGB565,
    BGR565,
    RGBA8888,
    BGRA8888
};


class MediaPlayerDecoder {

public:
    virtual ~MediaPlayerDecoder() { };
   
    // About Plugin 
    virtual const char *pluginName() = 0;
    virtual const char *pluginComment() = 0;
    virtual double pluginVersion() = 0;

    virtual bool isFileSupported( const QString& file ) = 0;
    virtual bool open( const QString& file ) = 0;
    virtual bool close() = 0;
    virtual bool isOpen() = 0;
    virtual const QString &fileInfo() = 0;

    // If decoder doesn't support audio then return 0 here
    virtual int audioStreams() = 0;
    virtual int audioChannels( int stream ) = 0;
    virtual int audioFrequency( int stream ) = 0;
    virtual int audioSamples( int stream ) = 0;
    virtual bool audioSetSample( long sample, int stream ) = 0;
    virtual long audioGetSample( int stream ) = 0;
    virtual bool audioReadSamples( short *samples, int channels, long count, long& samplesRead, int stream ) = 0;

    // If decoder doesn't support video then return 0 here
    virtual int videoStreams() = 0;
    virtual int videoWidth( int stream ) = 0;
    virtual int videoHeight( int stream ) = 0;
    virtual double videoFrameRate( int stream ) = 0; // frames per second (this may change to frames/1000secs)
    virtual int videoFrames( int stream ) = 0;
    virtual bool videoSetFrame( long sample, int stream ) = 0;
    virtual long videoGetFrame( int stream ) = 0;
    virtual bool videoReadFrame( unsigned char **, int, int, int, int, ColorFormat, int ) = 0;
    virtual bool videoReadScaledFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, int out_w, int out_h, ColorFormat color_model, int stream ) = 0;
    virtual bool videoReadYUVFrame( char *, char *, char *, int, int, int, int, int ) = 0;

    // Profiling
    virtual double getTime() = 0;

    // Ignore if these aren't supported
    virtual bool setSMP( int cpus ) = 0;
    virtual bool setMMX( bool useMMX ) = 0;

    // Capabilities
    virtual bool supportsAudio() = 0;
    virtual bool supportsVideo() = 0;
    virtual bool supportsYUV() = 0;
    virtual bool supportsMMX() = 0;
    virtual bool supportsSMP() = 0;
    virtual bool supportsStereo() = 0;
    virtual bool supportsScaling() = 0;

    // File Properies
    virtual long getPlayTime() { return -1; }
};


class MediaPlayerEncoder;


#ifndef QT_NO_COMPONENT
// {4193445c-ed33-11d6-93da-0050bad6ea87}
# ifndef IID_MediaPlayerPlugin_1_6
#  define IID_MediaPlayerPlugin_1_6 QUuid( 0x4193445c, 0xed33, 0x11d6, 0x93, 0xda, 0x00, 0x50, 0xba, 0xd6, 0xea, 0x87 )
# endif
#endif


class MediaPlayerDecoder_1_6 : public MediaPlayerDecoder {
public:
    // Streaming and seeking availability extensions for 1.6

    virtual bool supportsStreaming() = 0; // plugin feature
    virtual bool canStreamURL( const QUrl& url, const QString& mimetype ) = 0; // Test if the URL is supported
    virtual bool openURL( const QUrl& url, const QString& mimetype ) = 0; // Opens a URL for streaming
    virtual bool streamed() = 0; // Is the open file a streamed file

    virtual bool syncAvailable() = 0;
    virtual bool sync() = 0; // sync up the video to the catch up to the audio

    virtual bool seekAvailable() = 0; // Availability depends on stream
    virtual bool seek( long pos ) = 0; // seek to byte offset from beginning

    virtual bool tellAvailable() = 0; // Hard to imagine why this wouldn't be possible 
    virtual long tell() = 0; // byte offset from beginning

    virtual bool lengthAvailable() = 0; // Availability depends on stream
    virtual long length() = 0; // in bytes

    virtual bool totalTimeAvailable() = 0; // Availability depends on stream
    virtual long totalTime() = 0; // in milliseconds

    virtual bool currentTimeAvailable() = 0; // Availability depends on stream
    virtual long currentTime() = 0; // in milliseconds

/*
    virtual bool supportsQuality() = 0; // plugin feature
    virtual int quality() = 0; // Value from 0-100
    virtual void setQuality(int) = 0; // Hint to decoder to run faster (loose quality)

    virtual bool playtimeAvailable() = 0; // Availability depends on stream
    virtual long playtime() = 0;

    virtual bool timestampsAvailable() = 0; // Availability depends on stream
    virtual long timestamp() = 0;
*/

};


class MediaPlayerEncoder_1_6;


struct MediaPlayerPluginInterface : public QUnknownInterface
{
    virtual MediaPlayerDecoder *decoder() = 0;
    virtual MediaPlayerEncoder *encoder() = 0;
};


#endif

