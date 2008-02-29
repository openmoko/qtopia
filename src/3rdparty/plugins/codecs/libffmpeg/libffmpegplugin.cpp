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
#include <qfileinfo.h>
#include "libffmpegplugin.h"


static const int AVSyncSlack = 200;


LibFFMpegPlugin::LibFFMpegPlugin()
{
    scaleContextDepth = -1;
    scaleContextInputWidth = -1;
    scaleContextInputHeight = -1;
    scaleContextPicture1Width = -1;
    scaleContextPicture2Width = -1;
    scaleContextOutputWidth = -1;
    scaleContextOutputHeight = -1;
    scaleContextLineStride = -1;

    audioCodecContext = 0;
    videoCodecContext = 0;
    audioScaleContext = 0;
    strInfo = "";

    openFlag = FALSE;
    streamingFlag = FALSE;
    needPluginInit = TRUE;
}


LibFFMpegPlugin::~LibFFMpegPlugin()
{
    close();
    flushAudioPackets();
    flushVideoPackets();
}


const char *LibFFMpegPlugin::pluginName()
{
    return "LibFFMpegPlugin";
}


const char *LibFFMpegPlugin::pluginComment()
{
    return "This plugin uses ffmpeg, libavcodec, libav libraries written by Fabrice Bellard and others. "
	   "The Qtopia plugin interface adaptor is created by Trolltech.";
}


double LibFFMpegPlugin::pluginVersion()
{
    return 1.0;
}


bool LibFFMpegPlugin::isFileSupported( const QString& fileName )
{
    qDebug("checking if file is supported %s", fileName.latin1() );
    QString ext2 = fileName.right(3).lower();
    QString ext3 = fileName.right(4).lower();
    QString ext4 = fileName.right(5).lower();
    return  (
		( ext3 == ".asf" ) ||
		( ext3 == ".avi" ) ||
		( ext3 == ".mov" ) ||
		( ext3 == ".mp2" ) ||
		( ext3 == ".mp3" ) ||
		( ext4 == ".mpeg") ||
		( ext3 == ".mpg" ) ||
		//( ext3 == ".ogg" ) ||
		( ext2 == ".rm" ) ||
		//( ext3 == ".vob" ) || 
		//( ext3 == ".wav" ) ||
		( ext3 == ".wma" ) ||
		( ext3 == ".wmf" ) ||
		( ext3 == ".wmv" )
	    );
}


void LibFFMpegPlugin::pluginInit()
{
    avcodec_init();
    avcodec_register_all();
    av_register_all();
    needPluginInit = FALSE;
}


void LibFFMpegPlugin::fileInit()
{
    if ( needPluginInit )
	pluginInit();
    audioCodec = 0;
    videoCodec = 0;
    audioCodecContext = 0;
    audioScaleContext = 0;
    videoCodecContext = 0;
    streamContext = 0;
    frame = 0;
    videoStream = -1;
    audioStream = -1;
    fileLength = 0;
    skipNext = 0;
    currentPacketTimeStamp = 0;
    currentVideoTimeStamp = 0;
    currentAudioTimeStamp = 0;
    haveTotalTimeCache = FALSE;
    totalTimeCache = TRUE;
    droppedFrames = 0;
    framesInLastPacket = 0;
    streamingFlag = FALSE;
    strInfo = "";
    totalFrames = 0;
    openFlag = TRUE;
}


bool LibFFMpegPlugin::open( const QString& fileName )
{
    qDebug("opening file %s", fileName.latin1() );

    fileInit();

    if ( !fileLength ) {
	QFileInfo fi( fileName );
	fileLength = fi.size();
    }

    // open the input file with generic libav function
    if ( av_open_input_file(&streamContext, fileName.latin1(), NULL, 0, 0) < 0 ) {
	strInfo = qApp->translate( "LibFFMpegPlugin", "Error: Could not open file, File: " ) + fileName;
        qDebug( "%s", strInfo.latin1() );
	return FALSE;
    }

    qDebug("opened file %s", fileName.latin1() );

    // Decode first frames to get stream parameters (for some stream types like mpeg)
    if ( av_find_stream_info(streamContext) < 0 ) {
	qDebug("Error getting parameters for file %s", fileName.latin1() );
	return FALSE;
    }

    qDebug("initing file %s", fileName.latin1() );
 
    // update the current parameters so that they match the one of the input stream
    for ( int i = 0; i < streamContext->nb_streams; i++ ) {
	//printf( "searching: %i\n", i );
        AVCodecContext *enc = &streamContext->streams[i]->codec;
	enc->codec = avcodec_find_decoder( enc->codec_id );
	//printf( "decoder found: %s\n", enc->codec->name );
	if ( !enc->codec )
	    qDebug("Unsupported codec for input stream");
        else if ( avcodec_open( enc, enc->codec ) < 0 )
            qDebug("Error while opening codec for input stream");
	else {
	    switch (enc->codec_type) {
		case CODEC_TYPE_AUDIO:
		    qDebug("setting audio stream with id: %i", i);
		    audioStream = i;
		    audioCodecContext = enc;
		    break;
		case CODEC_TYPE_VIDEO:
		    qDebug("setting video stream with id: %i", i);
		    videoStream = i;
		    videoCodecContext = enc;
		    break;
		default:
		    qDebug("unknown stream type");
		    break;
	    }
	}
    }

    if ( audioCodecContext )
        audioScaleContext = audio_resample_init( 2, audioCodecContext->channels, 44100, audioCodecContext->sample_rate );
    
    // Try to determine the total play time if possible
    if ( !lengthAvailable() || !tellAvailable() )
	haveTotalTimeCache = FALSE;
    else {
	// Jump to near the end of the file
	url_fseek( &streamContext->pb, length() - 100000, SEEK_SET );
	AVPacket pkt;
	pkt.pts = 0;
	totalTimeCache = 0;
	// Read packets till we get to the end to try and get the last timestamp available
	while ( av_read_packet(streamContext, &pkt) >= 0 ) {
	    if ( pkt.pts > totalTimeCache )
		totalTimeCache = pkt.pts / 100;
	    pkt.pts = 0;
	    av_free_packet(&pkt);
	}
	// Jump back to the beginning so we are ready to decode
	url_fseek( &streamContext->pb, 0, SEEK_SET );
	haveTotalTimeCache = totalTimeCache > 1;

	if ( !haveTotalTimeCache ) {
	    // Calculate the play time of constant bit rate files
	    int totalBitRate = 0;
	    if ( audioStream != -1 && audioCodecContext ) 
		totalBitRate += audioCodecContext->bit_rate;
	    if ( videoStream != -1 && videoCodecContext ) 
		totalBitRate += videoCodecContext->bit_rate;
	    if ( totalBitRate ) {
		// The 8000 multiplier is because the rate is in bits per second and
		// there are 8 bits to a byte and we want the time in milliseconds.
		totalTimeCache = ((long long)length() * 8000) / totalBitRate;
		haveTotalTimeCache = TRUE;
	    }
	}
    }

    if ( videoCodecContext )
	videoCodecContext->hurry_up = 0;

    if ( videoCodecContext && videoCodecContext->frame_rate )
	msecPerFrame = (1000 * FRAME_RATE_BASE) / videoCodecContext->frame_rate;
    else
	msecPerFrame = 1000 / 25;

    qDebug("finished opening %s", fileName.latin1() );

    return true;
}


const QString &LibFFMpegPlugin::fileInfo()
{
    if ( strInfo == "" ) {
	if ( haveTotalTimeCache ) {
	    int seconds = totalTimeCache / 1000;
	    strInfo += qApp->translate( "LibFFMpegPlugin", "Play Time: " ) + QString::number( seconds / 60 ) + ":" +
		 QString::number( (seconds % 60) / 10 ) + QString::number( (seconds % 60) % 10 ) + ",";
	}

	if ( audioCodecContext ) {
	    strInfo += qApp->translate( "LibFFMpegPlugin", "Audio Tracks: 1," );
	    if ( audioCodecContext->codec )
		strInfo += qApp->translate( "LibFFMpegPlugin", "Audio Format: " ) +  audioCodecContext->codec->name + ",";
	    strInfo += qApp->translate( "LibFFMpegPlugin", "Audio Bit Rate: " ) + QString::number( audioCodecContext->bit_rate ) + ",";
	    strInfo += qApp->translate( "LibFFMpegPlugin", "Audio Channels: " ) + QString::number( audioCodecContext->channels ) + ",";
	    strInfo += qApp->translate( "LibFFMpegPlugin", "Audio Frequency: " ) + QString::number( audioCodecContext->sample_rate ) + ",";
	}

	if ( videoCodecContext ) {
	    strInfo += qApp->translate( "LibFFMpegPlugin", "Video Tracks: 1," );
	    if ( videoCodecContext->codec )
		strInfo += qApp->translate( "LibFFMpegPlugin", "Video Format: " ) + videoCodecContext->codec->name + ",";
	    strInfo += qApp->translate( "LibFFMpegPlugin", "Video Bit Rate: " ) + QString::number( videoCodecContext->bit_rate ) + ",";
	    strInfo += qApp->translate( "LibFFMpegPlugin", "Video Width: " ) + QString::number( videoCodecContext->width ) + ",";
	    strInfo += qApp->translate( "LibFFMpegPlugin", "Video Height: " ) + QString::number( videoCodecContext->height ) + ",";
	}
    }

    return strInfo;
}


static uchar bufferedSamples[AVCODEC_MAX_AUDIO_FRAME_SIZE * 16];
static int bufferedSamplesCount = 0;


bool LibFFMpegPlugin::close()
{
    if ( openFlag ) {
	flushAudioPackets();
	flushVideoPackets();

	AutoLockUnlockMutex lock( &pluginMutex );

	qDebug("close");

	if ( audioScaleContext )
	    audio_resample_close( audioScaleContext );
	if ( audioCodecContext )
            avcodec_close( audioCodecContext );
	if ( videoCodecContext )
            avcodec_close( videoCodecContext );
	if ( streamContext )
	    av_close_input_file( streamContext );

	audioCodecContext = 0;
	videoCodecContext = 0;
	audioScaleContext = 0;
	streamContext = 0;
	fileLength = 0;
	bufferedSamplesCount = 0;
	openFlag = FALSE;

	return TRUE;
    }

    return FALSE;
}


bool LibFFMpegPlugin::isOpen()
{
    return openFlag;
}


int LibFFMpegPlugin::audioStreams()
{
    return (audioCodecContext) ? 1 : 0;
}


int LibFFMpegPlugin::audioChannels( int )
{
    return 2;
}


int LibFFMpegPlugin::audioFrequency( int )
{
    return 44100;
}


int LibFFMpegPlugin::audioSamples( int )
{
    return -1;
}


bool LibFFMpegPlugin::audioSetSample( long, int )
{
    return FALSE;
}


long LibFFMpegPlugin::audioGetSample( int )
{
    return 0;
}


bool LibFFMpegPlugin::audioReadSamples( short *output, int channels, long samples, long& samplesRead, int )
{
    AutoLockUnlockMutex lock( &audioMutex );

    if ( !audioCodecContext || !audioCodecContext->codec ) {
	qDebug("No audio decoder for stream");
	samplesRead = 0;
	return FALSE;
    }

    if ( samples > AVCODEC_MAX_AUDIO_FRAME_SIZE ) {
	qDebug("Decoder can not buffer that much data at a time!!!");
	samples = AVCODEC_MAX_AUDIO_FRAME_SIZE;
    }

    uchar tmpSamples[AVCODEC_MAX_AUDIO_FRAME_SIZE * 8];
    long long tmpSamplesRead = 0;
    long long fact = audioCodecContext->sample_rate * 2 * audioCodecContext->channels;
    long long bufferedSamplesCount_fact = bufferedSamplesCount * fact;
    long long samples_fact = samples * fact; // Buffer ahead
    long long tmpBufCount = bufferedSamplesCount_fact;

    bool haveBothTimeStamps = ( currentVideoTimeStamp && currentAudioTimeStamp );
    if ( !haveBothTimeStamps )
	samples_fact += 9200 * fact; // With DivX AVI streams I have tested against, libavcodec doesn't
	// return any time stamps and the audio needs to be slightly behind the video so buffering longer achieves this.

    while ( tmpBufCount < samples_fact ) {

	MediaPacket *pkt = getAnotherPacket( audioStream );
	if ( !pkt ) {
	    samplesRead = -1;
	    return FALSE; // EOF
	}
	int len = pkt->pkt.size;
	unsigned char *ptr = pkt->pkt.data;
	int bytesRead = 0;

	while ( len ) {
	    int ret = 0;

	    if ( pkt ) {
		if ( ptr <  pkt->pkt.data ) {
		    qDebug("inconsistancy error");
		    return FALSE;
		}
		if ( ptr && tmpSamplesRead < AVCODEC_MAX_AUDIO_FRAME_SIZE*4 ) {
		    ret = avcodec_decode_audio(audioCodecContext, (short*)(tmpSamples + tmpSamplesRead), &bytesRead, ptr, len);
		}
	    }

	    if ( bytesRead > 0 )
		tmpSamplesRead += bytesRead;
	    else if ( bytesRead < 0 )
		qDebug("read count < 0, %i", bytesRead );

	    if ( ret < 0 ) {
		qDebug("Error while decoding audio stream");
		if ( pkt ) {
		    av_free_packet(&pkt->pkt);
		    delete pkt;
		}
		return FALSE;
	    }
	    ptr += ret;
	    len -= ret;
	}

	if ( pkt ) {
	    av_free_packet(&pkt->pkt);
	    delete pkt;
	}

	tmpBufCount = bufferedSamplesCount_fact + tmpSamplesRead * 44100;
    }


/*
    // Attempt to reduce a memcpy by doing the audio_resample directly in to the output 
    // buffer and then with any remaining output, resample it in to a buffer for later
    tmpSamplesRead /= 2 * audioCodecContext->channels;
    if ( bufferedSamplesCount ) {
	if ( bufferedSamplesCount <= samples ) {
	    memcpy( output, bufferedSamples, bufferedSamplesCount*2*channels );
	    output += bufferedSamplesCount * channels;
	    samples -= bufferedSamplesCount;
	    bufferedSamplesCount = 0;
	    int srcSampleCount = samples * audioCodecContext->sample_rate / 44100;
	    audio_resample( audioScaleContext, (short*)output, (short*)tmpSamples, srcSampleCount );
	    audio_resample( audioScaleContext, (short*)bufferedSamples, (short*)tmpSamples + srcSampleCount * 2, tmpSamplesRead - srcSampleCount );
	    bufferedSamplesCount += (tmpSamplesRead - srcSampleCount) * 44100 / audioCodecContext->sample_rate;
	} else {
	    samplesRead = samples;
	    memcpy( output, bufferedSamples, samples*2*channels );
	    bufferedSamplesCount -= samples;
	    int blength = bufferedSamplesCount*2*channels;
	    memmove( bufferedSamples, bufferedSamples + samples*2*channels, blength );
	    audio_resample( audioScaleContext, (short*)bufferedSamples + bufferedSamplesCount*channels, (short*)tmpSamples, tmpSamplesRead );
	    bufferedSamplesCount += tmpSamplesRead * 44100 / audioCodecContext->sample_rate;
	}
    }
*/

    tmpSamplesRead /= 2 * audioCodecContext->channels;
    bufferedSamplesCount += audio_resample( audioScaleContext, (short*)bufferedSamples + bufferedSamplesCount*channels, (short*)tmpSamples, tmpSamplesRead );

    samplesRead = samples;
    int slength = samples*2*channels;
    memcpy( output, bufferedSamples, slength );
    if ( currentAudioTimeStamp > 1 )
	currentAudioTimeStamp += samples * 1000 / 44100;

    bufferedSamplesCount -= samples;
    int blength = bufferedSamplesCount*2*channels;
    memmove( bufferedSamples, bufferedSamples + slength, blength );

    return TRUE;
}


int LibFFMpegPlugin::videoStreams()
{
    return (videoCodecContext) ? 1 : 0;
}


int LibFFMpegPlugin::videoWidth( int )
{
    return (videoCodecContext) ? videoCodecContext->width : 0;
}


int LibFFMpegPlugin::videoHeight( int )
{
    return (videoCodecContext) ? videoCodecContext->height : 0;
}


double LibFFMpegPlugin::videoFrameRate( int )
{
    return (videoCodecContext) ? (double)videoCodecContext->frame_rate / FRAME_RATE_BASE : 1.0;
}


int LibFFMpegPlugin::videoFrames( int )
{
    return -1;
}


bool LibFFMpegPlugin::videoSetFrame( long, int )
{
    return FALSE;
}


long LibFFMpegPlugin::videoGetFrame( int )
{
    return -1;
}


bool LibFFMpegPlugin::videoReadFrame( unsigned char **, int, int, int, int, ColorFormat, int )
{
    return FALSE;
}


bool LibFFMpegPlugin::videoReadScaledFrame( unsigned char **output_rows, int, int, int in_w, int in_h, int out_w, int out_h, ColorFormat fmt, int )
{
    AutoLockUnlockMutex lock( &videoMutex );

    int colorMode = -1;
    switch ( fmt ) {
	case RGB565:   colorMode = MODE_16_RGB; break;
	case BGR565:   colorMode = MODE_16_BGR; break;
	case RGBA8888: colorMode = MODE_32_RGB; break;
	case BGRA8888: colorMode = MODE_32_BGR; break;
    };

    if ( colorMode != scaleContextDepth ) {
	scaleContextDepth = colorMode;
	videoScaleContext = yuv2rgb_factory_init( colorMode, 0, 0 );
    }

    int lineStride = (uchar*)output_rows[1] - (uchar*)output_rows[0];

    if ( !videoCodecContext || !videoCodecContext->codec ) {
	qDebug("No video decoder for stream");
	return 1;
    }

    if ( skipNext ) {
	skipNext--;
	return 0;
    }

    int got_picture = 0;
    while ( !got_picture ) {
	MediaPacket *pkt = getAnotherPacket( videoStream );

	if ( !pkt ) {
	    qDebug("Video EOF");
	    return 1; // EOF
	}

	while (pkt->len > 0 && !got_picture) {
	    int ret = avcodec_decode_video(videoCodecContext, &picture, &got_picture, pkt->ptr, pkt->len);

	    if ( got_picture ) {
		pkt->frameInPacket++;
		if ( currentVideoTimeStamp )
		    currentVideoTimeStamp += msecPerFrame;
		frame = videoCodecContext->frame_number;
//		qDebug("got picture: %i", frame );

		// Check if any colour space conversion variables have changed
		// since the last decoded frame which will require
		// re-initialising the colour space tables 
		if ( scaleContextInputWidth != in_w ||
		    scaleContextInputHeight != in_h ||
		    scaleContextPicture1Width != picture.linesize[0] ||
		    scaleContextPicture2Width != picture.linesize[1] ||
		    scaleContextOutputWidth != out_w ||
		    scaleContextOutputHeight != out_h ||
		    scaleContextLineStride != lineStride ||
		    scaleContextFormat != videoCodecContext->pix_fmt ) {

		    scaleContextInputWidth = in_w;
		    scaleContextInputHeight = in_h;
		    scaleContextPicture1Width = picture.linesize[0];
		    scaleContextPicture2Width = picture.linesize[1];
		    scaleContextOutputWidth = out_w;
		    scaleContextOutputHeight = out_h;
		    scaleContextLineStride = lineStride;
		    scaleContextFormat = videoCodecContext->pix_fmt;

		    int format = 0;
		    switch ( videoCodecContext->pix_fmt ) {
			case PIX_FMT_YUV444P:
			    format = FORMAT_YUV444;
			    break;
			case PIX_FMT_YUV422P:
			    format = FORMAT_YUV422;
			    break;
			case PIX_FMT_YUV420P:
			    format = FORMAT_YUV420;
			    break;
		    };

		    qDebug("reconfiguring scale context");
		    videoScaleContext->converter->configure( videoScaleContext->converter,
			in_w, in_h, picture.linesize[0], picture.linesize[1], out_w, out_h, lineStride, format );
//		    qDebug("configured yuv convert context with - input: %i x %i  pic lines: %i %i, output: %i x %i, linestride: %i", in_w, in_h, picture.linesize[0], picture.linesize[1], out_w, out_h, lineStride );
		}

		videoScaleContext->converter->yuv2rgb_fun( videoScaleContext->converter, (uint8_t*)output_rows[0], picture.data[0], picture.data[1], picture.data[2] );
	    }

	    if ( ret < 0 ) {
		qDebug("Error while decoding stream");
		removeCurrentVideoPacket();
		return 1;
	    }

	    pkt->ptr += ret;
	    pkt->len -= ret;
	}
	if ( pkt->len == 0 ) 
	    removeCurrentVideoPacket(); // Remove from list when done with it
    }

    return 0;
}


bool LibFFMpegPlugin::videoReadYUVFrame( char *, char *, char *, int, int, int, int, int )
{
    return FALSE;
}


double LibFFMpegPlugin::getTime()
{
    return -1;
}


bool LibFFMpegPlugin::setSMP( int )
{
    return FALSE;
}


bool LibFFMpegPlugin::setMMX( bool )
{
    return FALSE;
}


bool LibFFMpegPlugin::supportsAudio()
{
    return TRUE;
}


bool LibFFMpegPlugin::supportsVideo()
{
    return TRUE;
}


bool LibFFMpegPlugin::supportsYUV()
{
    return FALSE;
}


bool LibFFMpegPlugin::supportsMMX()
{
    return FALSE;
}


bool LibFFMpegPlugin::supportsSMP()
{
    return FALSE;
}


bool LibFFMpegPlugin::supportsStereo()
{
    return TRUE;
}


bool LibFFMpegPlugin::supportsScaling()
{
    return TRUE;
}


long LibFFMpegPlugin::getPlayTime()
{
    return -1;
}







bool LibFFMpegPlugin::supportsStreaming()
{
    return TRUE;
}
bool LibFFMpegPlugin::canStreamURL( const QUrl& url, const QString& mimetype )
{
    QString fileName = url.toString( true, false );
    // Support file://
    if ( fileName.left(7).lower() == "file://" )
	return true;
    // Support http://
    if ( fileName.left(7).lower() == "http://" )
	return true;
    // Support rtsp://
    if ( fileName.left(7).lower() == "rtsp://" )
	return true;
    // Does not support mms://
    if ( fileName.left(6).lower() == "mms://" )
	return false;
    // All others assumed not supported
    return false;
}
bool LibFFMpegPlugin::openURL( const QUrl& url, const QString& mimetype )
{
    fileInit();

    streamingFlag = TRUE;
    haveTotalTimeCache = FALSE;

    QString fileName = url.toString( false, false );

    qDebug("opening url %s", fileName.latin1() );

    // open the input file with generic libav function
    if ( av_open_input_file(&streamContext, fileName.latin1(), NULL, 0, 0) < 0 ) {
	strInfo = qApp->translate( "LibFFMpegPlugin", "Error: Could not open url, URL: " ) + fileName;
        qDebug( "%s", strInfo.latin1() );
	return FALSE;
    }

    qDebug("opened url %s", fileName.latin1() );

    // Decode first frames to get stream parameters (for some stream types like mpeg)
    if ( av_find_stream_info(streamContext) < 0 ) {
	qDebug("Error getting parameters for file %s", fileName.latin1() );
	return FALSE;
    }

    qDebug("initing url %s", fileName.latin1() );
 
    // update the current parameters so that they match the one of the input stream
    for ( int i = 0; i < streamContext->nb_streams; i++ ) {
	//printf( "searching: %i\n", i );
        AVCodecContext *enc = &streamContext->streams[i]->codec;
	enc->codec = avcodec_find_decoder( enc->codec_id );
	//printf( "decoder found: %s\n", enc->codec->name );
	if ( !enc->codec )
	    qDebug("Unsupported codec for input stream");
        else if ( avcodec_open( enc, enc->codec ) < 0 )
            qDebug("Error while opening codec for input stream");
	else {
	    switch (enc->codec_type) {
		case CODEC_TYPE_AUDIO:
		    qDebug("setting audio stream with id: %i", i);
		    audioStream = i;
		    audioCodecContext = enc;
		    break;
		case CODEC_TYPE_VIDEO:
		    qDebug("setting video stream with id: %i", i);
		    videoStream = i;
		    videoCodecContext = enc;
		    break;
		default:
		    qDebug("unknown stream type");
		    break;
	    }
	}
    }

    if ( audioCodecContext )
        audioScaleContext = audio_resample_init( 2, audioCodecContext->channels, 44100, audioCodecContext->sample_rate );

    if ( videoCodecContext )
	videoCodecContext->hurry_up = 0;

    if ( videoCodecContext && videoCodecContext->frame_rate )
	msecPerFrame = (1000 * FRAME_RATE_BASE) / videoCodecContext->frame_rate;
    else
	msecPerFrame = 1000 / 25;

    qDebug("finished opening %s", fileName.latin1() );

    return true;
}
bool LibFFMpegPlugin::streamed()
{
    return streamingFlag;
}

bool LibFFMpegPlugin::syncAvailable()
{
    return TRUE;
}
bool LibFFMpegPlugin::sync()
{
    if ( !streamContext ) {
	qDebug("No file open");
	return FALSE;
    }

    if ( !videoCodecContext ) {
	printf("no context\n");
	return FALSE;
    }

    if ( !videoStreams() )
	return TRUE;
 
    AutoLockUnlockMutex lock( &videoMutex );

    int packetCount = waitingVideoPackets.count();
    if ( packetCount > 1000 ) {
	// We are way too far behind, need to altogether drop packets we
	// are behind and try to pick up from somewhere reasonable
	for ( int i = 0; i < packetCount - 10; i++ )
	    removeCurrentVideoPacket();

	qDebug("got really far behind");
    }

    bool haveBothTimeStamps = ( currentVideoTimeStamp && currentAudioTimeStamp );
    bool keepDecoding = TRUE;

    if ( haveBothTimeStamps ) {
//	qDebug("have both time stamps %li %li", (long)currentVideoTimeStamp, (long)currentAudioTimeStamp );
	// Are we too far ahead with the video?
	if ( currentVideoTimeStamp > currentAudioTimeStamp + AVSyncSlack ) {
	    printf("slow down video\n");
	    skipNext++;
	    // The w38.mpg example has a crazy video time stamp on the first video packet
	    // which would cause the logic here to think we are 1000s of frames behind.
	    // What is required is to decode more video packets till we get a sane video
	    // time stamp but this doesn't happen if we think we are miles ahead with the
	    // video because of a bad time stamp. The line below ensures if we do get too
	    // ahead with the video we can't stall and stop decoding for more than a 
	    // single frame at a time in case we get crazy time stamps.
//	    currentVideoTimeStamp = currentAudioTimeStamp;
	    return TRUE;
	}
	keepDecoding = ( currentAudioTimeStamp > currentVideoTimeStamp + AVSyncSlack );
	if ( keepDecoding ) {
//	    qDebug("audio ahead, decode more video 1");
	}
    } else {
//	qDebug("packets - video: %i audio: %i", waitingVideoPackets.count(), waitingAudioPackets.count() );
	if ( waitingAudioPackets.count() >= 1 ) {
	    printf("slow down video\n");
	    skipNext++;
	    return TRUE;
	}
	keepDecoding = ( waitingVideoPackets.count() > 1 );
	if ( keepDecoding ) {
//	    qDebug("audio ahead, decode more video 2");
	}
    }

    // Quickly skip over packets if we are *really* far behind (ie one second behind)
    if ( haveBothTimeStamps ) {
       int maxPackets = 10;
       while ( currentAudioTimeStamp > currentVideoTimeStamp + 1000 && maxPackets ) {
           maxPackets--;
           //qDebug("catching up another frame");
           MediaPacket *pkt = getAnotherPacket( videoStream );
           if ( !pkt ) {
               qDebug("Video EOF");
               return FALSE; // EOF
           }
           removeCurrentVideoPacket(); // Remove from list when done with it
       }
    }

    int maxFrames = 10;

    // Try to consume up the waiting video packets so we get back in sync with the audio
    while ( keepDecoding ) {
//	qDebug("catching up another packet");
	MediaPacket *pkt = getAnotherPacket( videoStream );
	if ( !pkt ) {
	    qDebug("Video EOF");
	    return FALSE;
	}
	while ( pkt->len > 0 && maxFrames && (!haveBothTimeStamps || ( currentAudioTimeStamp > currentVideoTimeStamp + 70 )) ) {
	    int got_picture = 0;
	    // when set to 1 during decoding, b frames will be skiped when
	    // set to 2 idct/dequant will be skipped too
	    int oldHurryUp = videoCodecContext->hurry_up;
	    videoCodecContext->hurry_up = 1;
	    int ret = avcodec_decode_video(videoCodecContext, &picture, &got_picture, pkt->ptr, pkt->len);
	    if ( got_picture ) {
		pkt->frameInPacket++;
		if ( currentVideoTimeStamp )
		    currentVideoTimeStamp += msecPerFrame;
		droppedFrames++;
		qDebug("frames dropped: %i", droppedFrames);
		maxFrames--;
	    }
	    videoCodecContext->hurry_up = oldHurryUp;
	    if ( ret < 0 ) {
		qDebug("Error while decoding stream");
		av_free_packet(&pkt->pkt);
		delete pkt;
	    }
	    pkt->ptr += ret;
	    pkt->len -= ret;
	}
	if ( pkt->len == 0 ) 
	    removeCurrentVideoPacket(); // Remove from list when done with it

	if ( haveBothTimeStamps ) {
	    // Guard against crazy audio or video time stamps by limiting the frames to maxFrames
	    // otherwise they could cause this code to want to catch up with an oasis timestamp way in the future
	    if ( maxFrames <= 0 ) {
		currentVideoTimeStamp = currentAudioTimeStamp;
		keepDecoding = FALSE;
	    } else
		keepDecoding = ( currentAudioTimeStamp > currentVideoTimeStamp + AVSyncSlack );

	    if ( keepDecoding ) {
//		qDebug("audio ahead, decode more video 3");
	    }
	} else {
	    if ( maxFrames <= 0 ) {
		keepDecoding = FALSE;
	    } else
		keepDecoding = ( waitingVideoPackets.count() > 1 );
	}
    }

    return TRUE;
}

bool LibFFMpegPlugin::seekAvailable()
{
    return !streamingFlag;
}
bool LibFFMpegPlugin::seek( long pos )
{
    qDebug("LibFFMpegPlugin::seek");

    if ( !streamContext ) {
	qDebug("No file open");
	return FALSE;
    }

    AutoLockUnlockMutex audioLock( &audioMutex );
    AutoLockUnlockMutex videoLock( &videoMutex );

    flushAudioPackets();
    flushVideoPackets();

    {
	AutoLockUnlockMutex lock( &pluginMutex );

	if ( audioCodecContext )
	    avcodec_flush_buffers( audioCodecContext );
	if ( videoCodecContext )
	    avcodec_flush_buffers( videoCodecContext );

	// Seek in to the file
	if ( pos > 1000 )
	    url_fseek( &streamContext->pb, pos - 1000, SEEK_SET );
	else
	    url_fseek( &streamContext->pb, 0, SEEK_SET );

	// Reset these after seeking till we start to get new ones again
	currentVideoTimeStamp = 0;
	currentAudioTimeStamp = 0;
	bufferedSamplesCount = 0;
    }

    if ( !videoStreams() )
	return TRUE;

    // Sync up the input with the packets so
    // we are ready to get the next video frame
    // We have to sync through a keyframe, but not
    // all codecs have keyframes or some have keyframes
    // which are far apart depending on the encoder which created the file.
    // So instead of hoping to find a keyframe, we decode 25 frames so we
    // are reasonably confident we have enough picture complete by then
    // to resume decoding from.
    int framesToDecode = 25;

    while ( framesToDecode > 0 ) {
	MediaPacket *pkt = getAnotherPacket( videoStream );

	if ( !pkt ) {
	    qDebug("Video EOF");
	    return FALSE;
	}

	while (pkt->len > 0) {
	    int got_pic;
	    int ret = avcodec_decode_video(videoCodecContext, &picture, &got_pic, pkt->ptr, pkt->len);

	    if ( got_pic ) {
		pkt->frameInPacket++;
		if ( currentVideoTimeStamp )
		    currentVideoTimeStamp += msecPerFrame;
		framesToDecode--;
	    }

	    if ( ret < 0 ) {
		qDebug("Error while decoding stream");
		av_free_packet(&pkt->pkt);
		delete pkt;
	    }
	    pkt->ptr += ret;
	    pkt->len -= ret;
	}

	if ( pkt->len == 0 ) 
	    removeCurrentVideoPacket(); // Remove from list when done with it
    }

    bool haveBothTimeStamps = ( currentVideoTimeStamp && currentAudioTimeStamp );
    if ( haveBothTimeStamps ) {
	// ### Throw away audio packets till we are in time with the video packets
        flushAudioPackets();
	bufferedSamplesCount = 0;
    } else {
        flushAudioPackets();
	bufferedSamplesCount = 0;
    }

    return TRUE;
}

bool LibFFMpegPlugin::tellAvailable()
{
    return !streamingFlag;
}
long LibFFMpegPlugin::tell()
{
    return url_ftell( &streamContext->pb );
}

bool LibFFMpegPlugin::lengthAvailable()
{
    return !streamingFlag;
}
long LibFFMpegPlugin::length()
{
    return fileLength;
}

bool LibFFMpegPlugin::totalTimeAvailable()
{
    return haveTotalTimeCache;
}
long LibFFMpegPlugin::totalTime()
{
    return totalTimeCache;
}

bool LibFFMpegPlugin::currentTimeAvailable()
{
    bool haveTimeStamp = (currentAudioTimeStamp != 0) || (currentVideoTimeStamp != 0);
    bool canDerivePosition = lengthAvailable() && tellAvailable() && totalTimeAvailable();
    return haveTimeStamp || canDerivePosition;
}
long LibFFMpegPlugin::currentTime()
{
    if (currentAudioTimeStamp != 0)
       return currentAudioTimeStamp;
    if (currentVideoTimeStamp != 0)
       return currentVideoTimeStamp;
    bool canDerivePosition = lengthAvailable() && tellAvailable() && totalTimeAvailable();
    if ( canDerivePosition && fileLength )
       return ((long long)totalTime() * tell()) / fileLength;
    return -1;
}


// Remove from list when done with it
void LibFFMpegPlugin::removeCurrentVideoPacket()
{
    AutoLockUnlockMutex lock( &pluginMutex );
    MediaPacket *pkt = waitingVideoPackets.take(0);
    framesInLastPacket = pkt->frameInPacket;
    if ( pkt ) {
	av_free_packet(&pkt->pkt);
	delete pkt;
    }
}


void LibFFMpegPlugin::flushVideoPackets()
{
    AutoLockUnlockMutex lock( &pluginMutex );
    if ( videoCodecContext )
	avcodec_flush_buffers( videoCodecContext );
    while ( waitingVideoPackets.first() ) {
	MediaPacket *pkt = waitingVideoPackets.take();
	if ( pkt ) {
	    av_free_packet(&pkt->pkt);
	    delete pkt;
	}
    }
}


void LibFFMpegPlugin::flushAudioPackets()
{
    AutoLockUnlockMutex lock( &pluginMutex );
    if ( audioCodecContext )
	avcodec_flush_buffers( audioCodecContext );
    while ( waitingAudioPackets.first() ) {
	MediaPacket *pkt = waitingAudioPackets.take();
	if ( pkt ) {
	    av_free_packet(&pkt->pkt);
	    delete pkt;
	}
    }
}


MediaPacket *LibFFMpegPlugin::getAnotherPacket( int stream )
{
    AutoLockUnlockMutex lock( &pluginMutex );

    if ( stream == videoStream ) 
	if ( waitingVideoPackets.first() ) {
	    MediaPacket *pkt = waitingVideoPackets.first();
	    if ( pkt->pkt.pts ) {
		currentPacketTimeStamp = pkt->pkt.pts / 100;
		currentVideoTimeStamp = currentPacketTimeStamp + (pkt->frameInPacket * msecPerFrame);
//		qDebug("got time stamp: %li queued video (%i left)", currentPacketTimeStamp, waitingVideoPackets.count() );
	    } else if ( currentVideoTimeStamp ) {
		currentVideoTimeStamp = currentPacketTimeStamp + (framesInLastPacket * msecPerFrame);
	    }
	    return pkt;
	}
    if ( stream == audioStream ) 
	if ( waitingAudioPackets.first() ) {
	    MediaPacket *pkt = waitingAudioPackets.take();
	    if ( pkt->pkt.pts ) {
		currentPacketTimeStamp = pkt->pkt.pts / 100; // convert to milliseconds
		currentAudioTimeStamp = currentPacketTimeStamp;
//		qDebug("got time stamp: %li queued audio (%i left)", currentPacketTimeStamp, waitingAudioPackets.count() );
	    }
	    return pkt;
	}

    // Buffer up some more packets
    for ( int i = 0; i < 100; i++ ) {
	MediaPacket *pkt = new MediaPacket;
	pkt->pkt.pts = 0;
	pkt->frameInPacket = 0;

	// read a packet from input
	if (av_read_packet(streamContext, &pkt->pkt) < 0) {
	    delete pkt;
	    return 0; // EOF
	}

	pkt->len = pkt->pkt.size;
	pkt->ptr = pkt->pkt.data;

	if ( pkt->pkt.stream_index == stream && stream == audioStream ) {
	    if ( pkt->pkt.pts ) {
		currentPacketTimeStamp = pkt->pkt.pts / 100;
		currentAudioTimeStamp = currentPacketTimeStamp;
//		qDebug("got time stamp: %li audio", currentPacketTimeStamp );
	    }
	    return pkt;
	} else if ( pkt->pkt.stream_index == stream && stream == videoStream ) {
	    waitingVideoPackets.append( pkt );
	    if ( pkt->pkt.pts ) {
		currentPacketTimeStamp = pkt->pkt.pts / 100;
		currentVideoTimeStamp = currentPacketTimeStamp;
//		qDebug("got time stamp: %li video", currentPacketTimeStamp );
	    }
	    return pkt;
	}

	if ( pkt->pkt.stream_index == videoStream ) 
	    waitingVideoPackets.append( pkt );
	if ( pkt->pkt.stream_index == audioStream ) 
	    waitingAudioPackets.append( pkt );
    }

    return 0;
}




