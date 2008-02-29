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
#include "libflashplugin.h"

/*!
  \class LibFlashPlugin libflashplugin.h
  \brief The LibFlashPlugin class is a wrapper for the libflash library.

  \legalese
  Libflash renders Flash movies for the MediaPlayer.

  The libflash library is distributed under the terms of the GNU General Public License.
  The primary copyright holders are <a href="mailto:odebon@club-internet.fr">
    Olivier Debon</a> and <a href="mailto:fabrice.bellard@netgem.com">
    Fabrice Bellard</a>. More information about the Flash library can be found at <a href="http://www.swift-tools.com/Flash">http://www.swift-tools.com/Flash</a>.
*/
/*
    \fn virtual LibFlashPlugin::LibFlashPlugin()
    Constructs a libflash plugin decoder object.
*/
/*
    \fn virtual LibFlashPlugin::~LibFlashPlugin()
    \reimp
*/
/*
    \fn virtual const char *LibFlashPlugin::pluginName()
    \reimp
*/
/*
    \fn virtual const char *LibFlashPlugin::pluginComment()
    \reimp
*/
/*
    \fn virtual double LibFlashPlugin::pluginVersion()
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::isFileSupported( const QString& file )
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::open( const QString& file )
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::close()
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::isOpen()
    \reimp
*/
/*
    \fn virtual const QString &LibFlashPlugin::fileInfo()
    \reimp
*/
/*
    \fn virtual int LibFlashPlugin::audioStreams()
    \reimp
*/
/*
    \fn virtual int LibFlashPlugin::audioChannels( int stream )
    \reimp
*/
/*
    \fn virtual int LibFlashPlugin::audioFrequency( int stream )
    \reimp
*/
/*
    \fn virtual int LibFlashPlugin::audioSamples( int stream )
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::audioSetSample( long sample, int stream )
    \reimp
*/
/*
    \fn virtual long LibFlashPlugin::audioGetSample( int stream )
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::audioReadSamples( short *samples, int channels, long sampleCount, long& samplesRead, int stream )
    \reimp
*/
/*
    \fn virtual int LibFlashPlugin::videoStreams()
    \reimp
*/
/*
    \fn virtual int LibFlashPlugin::videoWidth( int stream )
    \reimp
*/
/*
    \fn virtual int LibFlashPlugin::videoHeight( int stream )
    \reimp
*/
/*
    \fn virtual double LibFlashPlugin::videoFrameRate( int stream )
    \reimp
*/
/*
    \fn virtual int LibFlashPlugin::videoFrames( int stream )
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::videoSetFrame( long frame, int stream )
    \reimp
*/
/*
    \fn virtual long LibFlashPlugin::videoGetFrame( int stream )
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::videoReadFrame( unsigned char **, int, int, int, int, ColorFormat, int )
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::videoReadYUVFrame( char *, char *, char *, int, int, int, int, int )
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::videoReadScaledFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, int out_w, int out_h, ColorFormat color_model, int stream )
    \reimp
*/
/*
    \fn virtual double LibFlashPlugin::getTime()
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::setSMP( int CPUs )
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::setMMX( bool useMMX )
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::supportsAudio()
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::supportsVideo()
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::supportsYUV()
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::supportsSMP()
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::supportsMMX()
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::supportsStereo()
    \reimp
*/
/*
    \fn virtual bool LibFlashPlugin::supportsScaling()
    \reimp
*/


#if 0

bool LibFlashPlugin::audioReadSamples( short *output, int channel, long samples, int stream ) {
}


bool LibFlashPlugin::audioReReadSamples( short *output, int channel, long samples, int stream ) {
}


bool LibFlashPlugin::audioReadMonoSamples( short *output, long samples, long& samplesRead, int stream ) {
    samplesRead = samples;
}


bool LibFlashPlugin::audioReadStereoSamples( short *output, long samples, long& samplesRead, int stream ) {
}


bool LibFlashPlugin::videoReadFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, ColorFormat color_model, int stream ) {
}


bool LibFlashPlugin::videoReadScaledFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, int out_w, int out_h, ColorFormat color_model, int stream ) {
/*
    int format = MPEG3_RGB565;
    switch ( color_model ) {
	case RGB565:	format = MPEG3_RGB565;   break;
	case RGBA8888:	format = MPEG3_RGBA8888; break;
	case BGRA8888:	format = MPEG3_BGRA8888; break;
    }
*/
}


bool LibFlashPlugin::videoReadYUVFrame( char *y_output, char *u_output, char *v_output, int in_x, int in_y, int in_w, int in_h, int stream ) {
}


FlashHandle file;
FlashDisplay *fd;

#endif


LibFlashPlugin::LibFlashPlugin() {
    file = NULL;
    fd = 0;
}
#include <stdio.h>
#include <stdlib.h>
static int readFile(const char *filename, char **buffer, long *size)
{
    FILE *in;
    char *buf;
    long length;

    printf("read files\n");

    in = fopen(filename,"r");
    if (in == 0) {
	perror(filename);
	return -1;
    }
    fseek(in,0,SEEK_END);
    length = ftell(in);
    rewind(in);
    buf = (char *)malloc(length);
    fread(buf,length,1,in);
    fclose(in);

    *size = length;
    *buffer = buf;

    return length;
}

static void showUrl(char *url, char * /*target*/, void * /*client_data*/) {
    printf("get url\n");
    printf("GetURL : %s\n", url);
}

static void getSwf(char *url, int level, void *client_data) {
    FlashHandle flashHandle = (FlashHandle) client_data;
    char *buffer;
    long size;

    printf("get swf\n");

    printf("LoadMovie: %s @ %d\n", url, level);
    if (readFile(url, &buffer, &size) > 0) {
	FlashParse(flashHandle, level, buffer, size);
    }
}

bool LibFlashPlugin::open( const QString& fileName ) {

    printf("opening file\n");

    delete fd;
    fd = new FlashDisplay;
    fd->pixels = new int[320*240*4];
    fd->width = 200;
    fd->bpl = 320*2;
    fd->height = 300;
    fd->depth = 16;
    fd->bpp = 2;
    fd->flash_refresh = 25;
    fd->clip_x = 0;
    fd->clip_y = 0;
    fd->clip_width = 0;
    fd->clip_height = 0;

    char *buffer;
    long size;
    int status;
    struct FlashInfo fi;

    if (readFile(fileName.latin1(), &buffer, &size) < 0)
	    exit(2);

    if (!(file = FlashNew()))
	    exit(1);

    do
	status = FlashParse(file, 0, buffer, size);
    while (status & FLASH_PARSE_NEED_DATA);

    free(buffer);
    FlashGetInfo(file, &fi);
    //FlashSettings(flashHandle, PLAYER_LOOP);
    FlashGraphicInit(file, fd);
    FlashSoundInit(file, "/dev/dsp");
    FlashSetGetUrlMethod(file, showUrl, 0);
    FlashSetGetSwfMethod(file, getSwf, (void*)file);

    printf("opened file\n");
}

// If decoder doesn't support audio then return 0 here
bool LibFlashPlugin::audioSetSample( long sample, int stream ) { return TRUE; }
long LibFlashPlugin::audioGetSample( int stream ) { return 0; }
//bool LibFlashPlugin::audioReadMonoSamples( short *output, long samples, long& samplesRead, int stream ) { return TRUE; }
//bool LibFlashPlugin::audioReadStereoSamples( short *output, long samples, long& samplesRead, int stream ) { return FALSE; }
bool LibFlashPlugin::audioReadSamples( short *output, int channels, long samples, long& samplesRead, int stream ) { return FALSE; }
//bool LibFlashPlugin::audioReadSamples( short *output, int channel, long samples, int stream ) { return TRUE; }
//bool LibFlashPlugin::audioReReadSamples( short *output, int channel, long samples, int stream ) { return TRUE; }

// If decoder doesn't support video then return 0 here
int LibFlashPlugin::videoStreams() { return 1; }
int LibFlashPlugin::videoWidth( int stream ) { return 300; }
int LibFlashPlugin::videoHeight( int stream ) { return 200; }
double LibFlashPlugin::videoFrameRate( int stream ) { return 25.0; }
int LibFlashPlugin::videoFrames( int stream ) { return 1000000; }
bool LibFlashPlugin::videoSetFrame( long frame, int stream ) { return TRUE; }
long LibFlashPlugin::videoGetFrame( int stream ) { return 0; }
bool LibFlashPlugin::videoReadFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, ColorFormat color_model, int stream ) { return TRUE; }
#include <sys/time.h>
bool LibFlashPlugin::videoReadScaledFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, int out_w, int out_h, ColorFormat color_model, int stream ) {
        struct timeval wd;
        FlashEvent fe;

/*
    delete fd;
    fd = new FlashDisplay;
    fd->pixels = output_rows[0];
    fd->width = 300; // out_w;
    fd->bpl = 640; // out_w*2;
    fd->height = 200;//out_h;
    fd->depth = 16;
    fd->bpp = 2;
    fd->flash_refresh = 50;
    fd->clip_x = 0;//in_x;
    fd->clip_y = 0;//in_y;
    fd->clip_width = 300;//in_w;
    fd->clip_height = 200;//in_h;
    FlashGraphicInit(file, fd);
*/

        long cmd = FLASH_WAKEUP;
        FlashExec(file, cmd, 0, &wd);

	fe.type = FeRefresh;
	cmd = FLASH_EVENT;
        FlashExec(file, cmd, &fe, &wd);
/*
	for (int i = 0; i < out_h; i++)
	    memcpy( output_rows[i], (char*)fd->pixels + i*fd->bpl, QMIN( fd->width * fd->bpp, out_w * fd->bpp ) );
*/	
	memcpy( output_rows[0], (char*)fd->pixels, out_w * out_h * 2 );
}

bool LibFlashPlugin::videoReadYUVFrame( char *y_output, char *u_output, char *v_output, int in_x, int in_y, int in_w, int in_h, int stream ) { return TRUE; }

// Profiling
double LibFlashPlugin::getTime() { return 0.0; }

// Ignore if these aren't supported
bool LibFlashPlugin::setSMP( int cpus ) { return TRUE; }
bool LibFlashPlugin::setMMX( bool useMMX ) { return TRUE; }


