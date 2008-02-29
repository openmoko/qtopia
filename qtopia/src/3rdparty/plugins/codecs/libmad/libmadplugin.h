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
#ifndef LIBMAD_PLUGIN_H 
#define LIBMAD_PLUGIN_H


#include <qstring.h>
#include <qtopia/mediaplayerplugininterface.h>


// #define OLD_MEDIAPLAYER_API


class LibMadPluginData;

class LibMadPlugin : public MediaPlayerDecoder {

public:
    LibMadPlugin();
    ~LibMadPlugin();

// QDOC_SKIP_BEGIN
    const char *pluginName() { return "LibMadPlugin"; }
    const char *pluginComment() { return "This is the libmad library that has been wrapped as a plugin"; }
    double pluginVersion() { return 1.0; }

    bool isFileSupported( const QString& );
    bool open( const QString& );
    bool close();
    bool isOpen();
    const QString &fileInfo() { return info; }

    // If decoder doesn't support audio then return 0 here
    int audioStreams();
    int audioChannels( int stream ); 
    int audioFrequency( int stream );
    int audioSamples( int stream );
    bool audioSetSample( long sample, int stream );
    long audioGetSample( int stream );
#ifdef OLD_MEDIAPLAYER_API
    bool audioReadMonoSamples( short *output, long samples, long& samplesRead, int stream );
    bool audioReadStereoSamples( short *output, long samples, long& samplesRead, int stream );
    bool audioReadSamples( short *output, int channel, long samples, int stream );
    bool audioReReadSamples( short *output, int channel, long samples, int stream );
#else
    bool audioReadSamples( short *output, int channels, long samples, long& samplesRead, int stream );
#endif

    bool read();
    bool decode( short *output, long samples, long& samplesRead );

    // If decoder doesn't support video then return 0 here
    int videoStreams() { return 0; }
    int videoWidth( int ) { return 0; }
    int videoHeight( int ) { return 0; }
    double videoFrameRate( int ) { return 0.0; }
    int videoFrames( int ) { return 0; }
    bool videoSetFrame( long, int ) { return FALSE; }
    long videoGetFrame( int ) { return 0; }
    bool videoReadFrame( unsigned char **, int, int, int, int, ColorFormat, int ) { return FALSE; }
    bool videoReadScaledFrame( unsigned char **, int, int, int, int, int, int, ColorFormat, int ) { return FALSE; }
    bool videoReadYUVFrame( char *, char *, char *, int, int, int, int, int ) { return FALSE; }

    // Profiling
    double getTime();

    // Ignore if these aren't supported
    bool setSMP( int ) { return FALSE; }
    bool setMMX( bool ) { return FALSE; }

    // Capabilities
    bool supportsAudio() { return TRUE; }
    bool supportsVideo() { return FALSE; }
    bool supportsYUV() { return FALSE; }
    bool supportsMMX() { return TRUE; }
    bool supportsSMP() { return FALSE; }
    bool supportsStereo() { return TRUE; }
    bool supportsScaling() { return FALSE; }

    long getPlayTime() { return -1; }

private:
    LibMadPluginData *d;
    QString info;

// QDOC_SKIP_END
};


#endif
