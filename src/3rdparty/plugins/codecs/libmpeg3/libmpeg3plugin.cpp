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
#include <qapplication.h>
#include "libmpeg3plugin.h"

/*!
  \class LibMpeg3Plugin libmpeg3plugin.h
  \brief The LibMpeg3Plugin class is a wrapper for the libmpeg3 library.

  \legalese

  The libMPEG3 library is distributed under the terms of the GNU General Public License.
  The primary copyright holder is Adam Williams &lt;broadcast@earthling.net&gt;.
*/
/*!
    \fn virtual LibMpeg3Plugin::LibMpeg3Plugin()
    Constructs a libmpeg3 plugin decoder object.
*/
/*!
    \fn virtual LibMpeg3Plugin::~LibMpeg3Plugin()
    \reimp
*/
/*!
    \fn virtual const char *LibMpeg3Plugin::pluginName()
    \reimp
*/
/*!
    \fn virtual const char *LibMpeg3Plugin::pluginComment()
    \reimp
*/
/*!
    \fn virtual double LibMpeg3Plugin::pluginVersion()
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::isFileSupported( const QString& file )
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::open( const QString& file )
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::close()
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::isOpen()
    \reimp
*/
/*!
    \fn virtual const QString &LibMpeg3Plugin::fileInfo()
    \reimp
*/
/*!
    \fn virtual int LibMpeg3Plugin::audioStreams()
    \reimp
*/
/*!
    \fn virtual int LibMpeg3Plugin::audioChannels( int stream )
    \reimp
*/
/*!
    \fn virtual int LibMpeg3Plugin::audioFrequency( int stream )
    \reimp
*/
/*!
    \fn virtual int LibMpeg3Plugin::audioSamples( int stream )
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::audioSetSample( long sample, int stream )
    \reimp
*/
/*!
    \fn virtual long LibMpeg3Plugin::audioGetSample( int stream )
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::audioReadSamples( short *samples, int channels, long sampleCount, long& samplesRead, int stream )
    \reimp
*/
/*!
    \fn virtual int LibMpeg3Plugin::videoStreams()
    \reimp
*/
/*!
    \fn virtual int LibMpeg3Plugin::videoWidth( int stream )
    \reimp
*/
/*!
    \fn virtual int LibMpeg3Plugin::videoHeight( int stream )
    \reimp
*/
/*!
    \fn virtual double LibMpeg3Plugin::videoFrameRate( int stream )
    \reimp
*/
/*!
    \fn virtual int LibMpeg3Plugin::videoFrames( int stream )
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::videoSetFrame( long frame, int stream )
    \reimp
*/
/*!
    \fn virtual long LibMpeg3Plugin::videoGetFrame( int stream )
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::videoReadFrame( unsigned char **, int, int, int, int, ColorFormat, int )
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::videoReadYUVFrame( char *, char *, char *, int, int, int, int, int )
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::videoReadScaledFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, int out_w, int out_h, ColorFormat color_model, int stream )
    \reimp
*/
/*!
    \fn virtual double LibMpeg3Plugin::getTime()
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::setSMP( int CPUs )
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::setMMX( bool useMMX )
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::supportsAudio()
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::supportsVideo()
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::supportsYUV()
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::supportsSMP()
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::supportsMMX()
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::supportsStereo()
    \reimp
*/
/*!
    \fn virtual bool LibMpeg3Plugin::supportsScaling()
    \reimp
*/
/*!
    \fn virtual long LibMpeg3Plugin::getPlayTime()
    \reimp
*/



#ifdef OLD_MEDIAPLAYER_API


bool LibMpeg3Plugin::audioReadSamples( short *output, int channel, long samples, int stream ) {
    return file ? mpeg3_read_audio( file, 0, output, 0, channel, samples, stream ) == 1 : FALSE; 
}


bool LibMpeg3Plugin::audioReReadSamples( short *output, int channel, long samples, int stream ) {
    return file ? mpeg3_reread_audio( file, 0, output, 0, channel, samples, stream ) == 1 : FALSE; 
}


bool LibMpeg3Plugin::audioReadMonoSamples( short *output, long samples, long& samplesRead, int stream ) {
    samplesRead = samples;
    return file ? mpeg3_read_audio( file, 0, output, 0, 0, samples, stream ) == 1 : FALSE; 
}


bool LibMpeg3Plugin::audioReadStereoSamples( short *output, long samples, long& samplesRead, int stream ) {
    bool err = FALSE;
    if ( file ) {
#if 1
	err = mpeg3_read_audio  ( file, 0,  output, 1, 0, samples, stream ) == 1;
	if ( err == FALSE ) {
	    err = mpeg3_reread_audio( file, 0, output + 1, 1, 1, samples, stream ) == 1;
#else
	short left[samples];
	short right[samples];
	err = mpeg3_read_audio  ( file, 0,  left, 0, samples, stream ) == 1;
	if ( !err )
	    err = mpeg3_reread_audio( file, 0, right, 1, samples, stream ) == 1;
	for ( int j = 0; j < samples; j++ ) {
	    output[j*2+0] =  left[j];
	    output[j*2+1] = right[j];
#endif
	}
    }
    samplesRead = samples;
    return err;
}


#else


bool LibMpeg3Plugin::audioReadSamples( short *output, int channels, long samples, long& samplesRead, int stream ) {
    bool retVal = FALSE;
    samplesRead = samples;
    switch ( channels ) {
	case 1:
	    retVal = file ? mpeg3_read_audio( file, 0, output, 0, 0, samples, stream ) != 1 : FALSE;
	    break;
	case 2:
	    if ( ( file ) && ( mpeg3_read_audio( file, 0, output, 1, 0, samples, stream ) != 1 ) &&
		    ( mpeg3_reread_audio( file, 0, output + 1, 1, 1, samples, stream ) != 1 ) )
		retVal = TRUE;
	    else
		retVal = FALSE;
	    break;
    }

    // Blank the samples if there was an error
    if ( !retVal ) 
	memset( output, 0, samples * channels * 2 );

    return retVal;
}


#endif


bool LibMpeg3Plugin::videoReadFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, ColorFormat color_model, int stream ) {
    int format = MPEG3_RGB565;
    switch ( color_model ) {
	case RGB565:	format = MPEG3_RGB565;   break;
	case BGR565:    /*format = MPEG3_BGR565;*/   break;
	case RGBA8888:	format = MPEG3_RGBA8888; break;
	case BGRA8888:	format = MPEG3_BGRA8888; break;
    }
    return file ? mpeg3_read_frame( file, output_rows, in_x, in_y, in_w, in_h, in_w, in_h, format, stream ) == 1 : FALSE;
}


bool LibMpeg3Plugin::videoReadScaledFrame( unsigned char **output_rows, int in_x, int in_y, int in_w, int in_h, int out_w, int out_h, ColorFormat color_model, int stream ) {
    int format = MPEG3_RGB565;
    switch ( color_model ) {
	case RGB565:	format = MPEG3_RGB565;   break;
	case BGR565:	/*format = MPEG3_BGR565;*/   break;
	case RGBA8888:	format = MPEG3_RGBA8888; break;
	case BGRA8888:	format = MPEG3_BGRA8888; break;
    }
    return file ? mpeg3_read_frame( file, output_rows, in_x, in_y, in_w, in_h, out_w, out_h, format, stream ) == 1 : FALSE;
}


bool LibMpeg3Plugin::videoReadYUVFrame( char *y_output, char *u_output, char *v_output, int in_x, int in_y, int in_w, int in_h, int stream ) {
    return file ? mpeg3_read_yuvframe( file, y_output, u_output, v_output, in_x, in_y, in_w, in_h, stream ) == 1 : FALSE;
}


QString LibMpeg3Plugin::tr( const char *text, const char *comment )
{
    return qApp->translate( "LibMpeg3Plugin", text, comment );
}


const QString &LibMpeg3Plugin::fileInfo()
{
    QString info;

    if ( file ) {

	if ( file->has_audio ) {
	    info += tr("Audio Tracks: ") + QString::number( file->total_astreams ) + ",";
	    if ( file->atrack[0] ) {
		info += tr("Channels: ") + QString::number( file->atrack[0]->channels ) + ",";
		int frequency = file->atrack[0]->sample_rate;
		if ( frequency ) {
		    info += tr("Frequency: ") + QString::number( frequency ) + ",";
		    int seconds = file->atrack[0]->total_samples / frequency;
		    info += tr("Play Time: ") + QString::number( seconds / 60 ) + ":" +
			 QString::number( (seconds % 60) / 10 ) + 
			 QString::number( (seconds % 60) % 10 ) + ",";
		}
		if ( file->atrack[0]->audio ) {
		    info += tr("Format: ");
		    switch ( file->atrack[0]->audio->format ) {
			default:
			case AUDIO_UNKNOWN: info += tr("Unknown"); break;
			case AUDIO_MPEG:    info += tr("MPEG");
			    info += tr(" Layer ") + QString::number( file->atrack[0]->audio->layer );
			    break;
			case AUDIO_AC3:	    info += tr("AC3"); break;
			case AUDIO_PCM:	    info += tr("PCM"); break;
			case AUDIO_AAC:	    info += tr("AAC"); break;
			case AUDIO_JESUS:   info += tr("Jesus"); break;
		    }
		    info += ",";
		}
	    }
	}
    
	if ( file->has_video ) {
	    info += tr("Video Tracks: ") + QString::number( file->total_vstreams ) + ",";
	    if ( file->vtrack[0] ) {
		if ( file->vtrack[0]->video ) {
		    info += tr("Format: ");
		    if ( file->vtrack[0]->video->mpeg2 )
			info += tr("MPEG2");
		    else
			info += tr("MPEG1");
		    info += ",";
		}
	    }
	}

    } else {
	info = tr("File not open");
    }

    return strInfo = info;
}

