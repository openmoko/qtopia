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
#include "libffmpegplugin.h"


LibFFMpegPlugin::LibFFMpegPlugin()
{
    openFlag = FALSE;
    pluginMutex = 0;
    audioCodec = 0;
    videoCodec = 0;
    audioCodecContext = 0;
    videoCodecContext = 0;
    avFormatContext = 0;
    resampleContext = 0;
    frame = 0;
    videoStream = -1;
    audioStream = -1;
    avcodec_init();
    avcodec_register_all();
    av_register_all();
    YUVFactory = yuv2rgb_factory_init( MODE_16_RGB, 0, 0 );
}


LibFFMpegPlugin::~LibFFMpegPlugin()
{
    close();
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
    QString ext = fileName.right(4);
    return (( ext == ".ogg" ) || ( ext == ".vob" ) || ( ext == ".mp3" ) || ( ext == ".avi" ) || ( ext == ".mpg" ) || ( ext == ".asf" ) || ( fileName.right(5) == ".mpeg" ));
}


bool LibFFMpegPlugin::open( const QString& fileName )
{
    if ( internalOpen( fileName ) ) {
	openFlag = TRUE;

//	return TRUE;
	indexStream();
	url_fseek( &avFormatContext->pb, 0, SEEK_SET );

	frame = 0;
	if ( videoCodecContext )
	    videoCodecContext->frame_number = 0;
	
	return TRUE;

/*
	if ( avFormatContext->pb.seek ) {
	    avFormatContext->pb.seek( avFormatContext, 0, SEEK_SET );
	}
	AVInputFormat *inputs = avFormatContext->iformat;
	while ( inputs ) {
	    if (inputs->read_seek)
		inputs->read_seek( avFormatContext, 0 );
	    inputs = inputs->next;
	}
*/
/*
	close();
	return internalOpen( fileName );
*/
	
    }
    return FALSE;
}


bool LibFFMpegPlugin::internalOpen( const QString& fileName )
{
    qDebug("opening file %s", fileName.latin1() );

    audioCodecContext = 0;
    videoCodecContext = 0;
    videoStream = -1;
    audioStream = -1;
    frame = 0;
    configured = FALSE;
    skipNext = 0;

    if ( !pluginMutex ) {
	pluginMutex = new pthread_mutex_t;
	pthread_mutexattr_t attr;
	pthread_mutexattr_init( &attr );
	pthread_mutex_init( pluginMutex, &attr );
    }

    // open the input file with generic libav function
    if (av_open_input_file(&avFormatContext, fileName.latin1(), NULL, 0, 0) < 0) {
        strInfo = "Could not open file: " + fileName;
        qDebug( "%s", strInfo.latin1() );
	return FALSE;
    }
    
    // Decode first frames to get stream parameters (for some stream types like mpeg)
    if (av_find_stream_info(avFormatContext) < 0 ) {
	qDebug("Error getting parameters for file %s", fileName.latin1() );
	return FALSE;
    }

    // update the current parameters so that they match the one of the input stream
    for ( int i = 0; i < avFormatContext->nb_streams; i++ ) {
        AVCodecContext *enc = &avFormatContext->streams[i]->codec;
	enc->codec = avcodec_find_decoder( enc->codec_id );
	printf( "decoder found: %s\n", enc->codec->name );
	if ( !enc->codec )
	    qDebug("Unsupported codec for input stream");
        else if ( avcodec_open( enc, enc->codec ) < 0 )
            qDebug("Error while opening codec for input stream");
        switch (enc->codec_type) {
	    case CODEC_TYPE_AUDIO:
		//qDebug("setting audio stream with id: %i", i);
		audioStream = i;
		audioCodecContext = enc;
		break;
	    case CODEC_TYPE_VIDEO:
		//qDebug("setting video stream with id: %i", i);
		videoStream = i;
		videoCodecContext = enc;
		break;
	    default:
		qDebug("unknown stream type");
		break;
        }
    }

    if ( audioCodecContext )
        resampleContext = audio_resample_init( 2, audioCodecContext->channels, 44100, audioCodecContext->sample_rate );
    
    if ( audioCodecContext )
    	printf("sample rate: %i\n",  audioCodecContext->sample_rate );

    strInfo = fileName;
    return true;
}


//extern "C" offset_t url_ftell_real(ByteIOContext *s);
void LibFFMpegPlugin::indexStream()
{
    totalFrames = 0;
    totalKeyFrames = 0;

    if ( !videoCodecContext ) 
	return;

    // Add key frame at 0
    keyFramePosition[ totalKeyFrames ] = 0;
    keyFrameNumber[ totalKeyFrames ] = 0;
    totalKeyFrames++;

//    int firstPos = avFormatContext->pb.pos; // url_ftell( &avFormatContext->pb );
    int bufferSize = url_fget_max_packet_size( &avFormatContext->pb );

    AVPicture picture;
    while ( TRUE ) {
	AVPacket pkt;
//	int lastPos = avFormatContext->pb.pos;
//	int lastPos = url_ftell_real( &avFormatContext->pb );
	int lastPos = url_ftell( &avFormatContext->pb );

#define IO_BUFFER_SIZE 32768
	if ( lastPos > bufferSize )
	    lastPos -= bufferSize;//IO_BUFFER_SIZE;
	else
	    lastPos = 0;

	if (av_read_packet(avFormatContext, &pkt) < 0) 
	    return; // EOF
	if ( pkt.stream_index == videoStream ) {
	    int len = pkt.size;
	    unsigned char *ptr = pkt.data;
	    while (len > 0) {
		int got_picture;
		videoCodecContext->hurry_up = 2;
		videoCodecContext->parse_only = 1;
		int ret = avcodec_decode_video(videoCodecContext, &picture, &got_picture, ptr, len);
		videoCodecContext->parse_only = 0;
		videoCodecContext->hurry_up = 0;
		// Just parse (this appears to be in avcodec.h but isn't defined anywhere.)
//		int ret = avcodec_parse_frame(videoCodecContext, &ptr, &len, ptr, len);
		if ( got_picture ) {
//		    framePosition[ totalFrames ] = lastPos;
		    totalFrames++;
		    if ( videoCodecContext->key_frame ) {
			keyFramePosition[ totalKeyFrames ] = lastPos;
			keyFrameNumber[ totalKeyFrames ] = videoCodecContext->frame_number;
			totalKeyFrames++;
			printf("adding KeyFrame index, frame: %i, pos: %i\n",
			    videoCodecContext->frame_number, lastPos );
		    }
		}
		ptr += ret;
		len -= ret;
		if ( ret < 0 ) {
		    qDebug("Error while decoding stream");
		    len = 0;
		}
	    }
	}
	av_free_packet(&pkt);
    }
    return;
}


bool LibFFMpegPlugin::close()
{
    if ( pluginMutex ) {
	pthread_mutex_lock( pluginMutex );

	while ( waitingVideoPackets.first() ) {
	    AVPacket *pkt = waitingVideoPackets.take();
	    if ( pkt ) {
		av_free_packet(pkt);
		delete pkt;
	    }
	}

	while ( waitingAudioPackets.first() ) {
	    AVPacket *pkt = waitingAudioPackets.take();
	    if ( pkt ) {
		av_free_packet(pkt);
		delete pkt;
	    }
	}

	if ( audioCodecContext )
	    avcodec_flush_buffers( audioCodecContext );
	if ( videoCodecContext )
            avcodec_flush_buffers( videoCodecContext );
	if ( resampleContext )
	    audio_resample_close( resampleContext );
	if ( audioCodecContext )
            avcodec_close( audioCodecContext );
	if ( videoCodecContext )
            avcodec_close( videoCodecContext );
	if ( avFormatContext )
	    av_close_input_file( avFormatContext );

	audioCodecContext = 0;
	videoCodecContext = 0;
	resampleContext = 0;
	avFormatContext = 0;

	pthread_mutex_unlock( pluginMutex );
	pthread_mutex_destroy( pluginMutex );
	delete pluginMutex;
	pluginMutex = 0;
    }

    return TRUE;
}


bool LibFFMpegPlugin::isOpen()
{
    return openFlag;
}


const QString &LibFFMpegPlugin::fileInfo()
{
    return strInfo;
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
    return 10000000;
}


bool LibFFMpegPlugin::audioSetSample( long, int )
{
    return FALSE;
}


long LibFFMpegPlugin::audioGetSample( int )
{
    return 0;
}


uchar bufferedSamples[AVCODEC_MAX_AUDIO_FRAME_SIZE * 4];
int bufferedSamplesCount = 0;


bool LibFFMpegPlugin::audioReadSamples( short *output, int channels, long samples, long& samplesRead, int )
{
    if ( !audioCodecContext || !audioCodecContext->codec ) {
	qDebug("No audio decoder for stream");
	samplesRead = -1;
	return FALSE;
    }

    if ( samples > AVCODEC_MAX_AUDIO_FRAME_SIZE ) {
	printf("this decoder can not buffer that much data!!!\n");
    }

    while ( bufferedSamplesCount < samples ) {
	AVPacket *pkt = getAnotherPacket( audioStream );
	if ( !pkt ) {
	    samplesRead = -1;
//	    qDebug("Audio EOF");
	    return FALSE;
	}
	int len = pkt->size;
	unsigned char *ptr = pkt->data;
	short tmpSamples[AVCODEC_MAX_AUDIO_FRAME_SIZE * 4];
	int tmpSamplesRead = 0;
	int lastSamplesRead = 0;

	while ( len ) {
	    int ret = avcodec_decode_audio(audioCodecContext, tmpSamples + tmpSamplesRead * audioCodecContext->channels, &lastSamplesRead, ptr, len);
	    if ( lastSamplesRead )
		tmpSamplesRead += lastSamplesRead / 2 / audioCodecContext->channels;
	    if ( ret < 0 ) {
		qDebug("Error while decoding audio stream");
		av_free_packet(pkt);
		delete pkt;
		return FALSE;
	    }
	    ptr += ret;
	    len -= ret;
	}
/*	
	if ( tmpSamplesRead < 0 ) {
	    printf("read count < 0, %i\n", tmpSamplesRead );
	    tmpSamplesRead = 0;
	}
*/

	audio_resample( resampleContext, (short*)(bufferedSamples + bufferedSamplesCount*2*channels), tmpSamples, tmpSamplesRead );

	bufferedSamplesCount += tmpSamplesRead;
	av_free_packet(pkt);
	delete pkt;
    }

    int slength = samples*2*channels;
    memcpy( output, bufferedSamples, slength );
    samplesRead = samples;

    int leftOver = bufferedSamplesCount - samples;
    memmove( bufferedSamples, bufferedSamples + slength, leftOver*channels*2 );
    bufferedSamplesCount = leftOver;

    return TRUE;
}


// If decoder doesn't support video then return 0 here
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
    return totalFrames;
}


bool LibFFMpegPlugin::videoSetFrame( long fr, int )
{
    printf("seek %i -> %i, ", frame, fr );

    if ( !videoCodecContext ) {
	printf("no context\n");
	return false;
    }

    if ( fr == frame + 1 || fr == frame ) {
	printf("next frame (automatic)\n");
	return TRUE;
    }
/*
    if ( fr == frame - 1 ) {
	skipNext = 1;
	return TRUE;
    }
*/
    if ( fr <= frame - 1 && fr >= frame - 5 ) {
	printf("back a few (just wait it out)\n");
	skipNext = frame - fr;
	return TRUE;
    }

    bool setFrameNumber = FALSE;
    int nextKeyFrame = 0;
    bool flushAudio = FALSE;

    if ( fr < frame || frame + 50 < fr ) {
	printf("big jump, ");
	for ( int i = 0; i < totalKeyFrames; i++ ) {
	    if ( keyFrameNumber[i] >= fr ) {

		if ( keyFrameNumber[i] != fr )
		    i = ( i <= 0 ) ? 0 : i - 1;

		// If we are already closer to the desired frame than the nearest
		// previous keyframe, don't bother seeking back etc
		if ( frame >= keyFrameNumber[i] && frame < fr )
		    break;


		pthread_mutex_lock( pluginMutex );

		while ( waitingVideoPackets.first() ) {
		    AVPacket *pkt = waitingVideoPackets.take();
		    if ( pkt ) {
			av_free_packet(pkt);
			delete pkt;
		    }
		}
		while ( waitingAudioPackets.first() ) {
		    AVPacket *pkt = waitingAudioPackets.take();
		    if ( pkt ) {
			av_free_packet(pkt);
			delete pkt;
		    }
		}

		if ( audioCodecContext )
		    avcodec_flush_buffers( audioCodecContext );
		if ( videoCodecContext )
		    avcodec_flush_buffers( videoCodecContext );

		printf("previous key frame at: %i, ", keyFrameNumber[i] );
		url_fseek( &avFormatContext->pb, keyFramePosition[ i ], SEEK_SET );
		frame = keyFrameNumber[i] - 1;
		setFrameNumber = TRUE;
		flushAudio = TRUE;
		nextKeyFrame = keyFrameNumber[i];

		pthread_mutex_unlock( pluginMutex );
		break;
//		return TRUE;
	    }
	}
    }
    
    // when set to 1 during decoding, b frames will be skiped when
    // set to 2 idct/dequant will be skipped too
    videoCodecContext->hurry_up = 2;

    printf("quick decode, frames: ");
    while ( (frame < (fr - 1)) || setFrameNumber ) {
	AVPicture picture;
	int got_picture = 0, dummy;
	while ( !got_picture ) {
	    AVPacket *pkt = getAnotherPacket( videoStream );

	    if ( !pkt ) {
		qDebug("Video EOF");
		return 1;
	    }

	    int len = pkt->size;
	    unsigned char *ptr = pkt->data;

	    while (len > 0) {
		int *got_pic_ptr = ( got_picture ) ? &dummy : &got_picture;
		int ret = avcodec_decode_video(videoCodecContext, &picture, got_pic_ptr, ptr, len);

		if ( *got_pic_ptr ) {
		    if ( setFrameNumber && videoCodecContext->key_frame ) {
			videoCodecContext->frame_number = nextKeyFrame;
			setFrameNumber = FALSE;
		    }
		    printf("%i, ", videoCodecContext->frame_number);
		}

		if ( ret < 0 ) {
		    qDebug("Error while decoding stream");
		    av_free_packet(pkt);
		    delete pkt;
		    //return 1;
		}
		ptr += ret;
		len -= ret;
	    }

	    av_free_packet(pkt);
	    delete pkt;
	}

	frame = videoCodecContext->frame_number;
    }
    printf("%i\n", videoCodecContext->frame_number);

    videoCodecContext->hurry_up = 0;

    if ( flushAudio ) {
	pthread_mutex_lock( pluginMutex );
	while ( waitingAudioPackets.first() ) {
	    AVPacket *pkt = waitingAudioPackets.take();
	    if ( pkt ) {
		av_free_packet(pkt);
		delete pkt;
	    }
	}
	if ( audioCodecContext )
	    avcodec_flush_buffers( audioCodecContext );
	pthread_mutex_unlock( pluginMutex );
    }
    return TRUE;
}


long LibFFMpegPlugin::videoGetFrame( int )
{
    return frame;
}


bool LibFFMpegPlugin::videoReadFrame( unsigned char **, int, int, int, int, ColorFormat, int )
{
    return FALSE;
}


bool LibFFMpegPlugin::videoReadScaledFrame( unsigned char **output_rows, int, int, int in_w, int in_h, int out_w, int out_h, ColorFormat, int )
{
    if ( !videoCodecContext || !videoCodecContext->codec ) {
	qDebug("No video decoder for stream");
	return 1;
    }

    if ( skipNext ) {
	skipNext--;
	return 0;
    }

    AVPicture picture;
    int got_picture = 0, dummy;
    while ( !got_picture ) {
	AVPacket *pkt = getAnotherPacket( videoStream );

	if ( !pkt ) {
	    qDebug("Video EOF");
	    return 1;
	}

	if (pkt->flags & PKT_FLAG_DROPPED_FRAME) {
	    printf("drop frame\n");
	    frame++;
	}

	int len = pkt->size;
	unsigned char *ptr = pkt->data;

	while (len > 0) {
	    int *got_pic_ptr = ( got_picture ) ? &dummy : &got_picture;
	    int ret = avcodec_decode_video(videoCodecContext, &picture, got_pic_ptr, ptr, len);
/*
	    int ret = avcodec_decode_video(videoCodecContext, &picture, &got_picture, ptr, len);

	    if (got_picture) {
		if ( ret != len ) {
		    lastPkt = pkt;
		    lastPktLen = len;
		    lastPktPtr = ptr;
		    ret = len;
		} else {
		    lastPkt = 0;
		    av_free_packet(pkt);
		    delete pkt;
		}
	    }
*/
	    //if (got_picture) {
	    if ( *got_pic_ptr ) {
		frame = videoCodecContext->frame_number;

//		printf("got picture: %i\n", frame );


/*
		qDebug("details - frame size: %i, num: %i, real: %i, was_key: %i, type: %i", 
		   videoCodecContext->frame_size,
		   videoCodecContext->frame_number, 
		   videoCodecContext->real_pict_num,
		   videoCodecContext->key_frame, 
		   videoCodecContext->pict_type );
*/
		if ( !configured ) {
		    configured = TRUE;
		    int lineStride = (uchar*)output_rows[1] - (uchar*)output_rows[0];
		    YUVFactory->converter->configure( YUVFactory->converter,
			in_w, in_h, picture.linesize[0], picture.linesize[1], out_w, out_h, lineStride );
		}

		YUVFactory->converter->yuv2rgb_fun( YUVFactory->converter, (uint8_t*)output_rows[0], picture.data[0], picture.data[1], picture.data[2] ); 
	    }

	    if ( ret < 0 ) {
		qDebug("Error while decoding stream");
		av_free_packet(pkt);
		delete pkt;
		return 1;
	    }

	    ptr += ret;
	    len -= ret;
	}
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


AVPacket *LibFFMpegPlugin::getAnotherPacket( int stream )
{
    pthread_mutex_lock( pluginMutex );

    if ( stream == videoStream ) 
	if ( waitingVideoPackets.first() ) {
	    AVPacket *pkt = waitingVideoPackets.take();
	    pthread_mutex_unlock( pluginMutex );
	    return pkt;
	}
    if ( stream == audioStream ) 
	if ( waitingAudioPackets.first() ) {
	    AVPacket *pkt = waitingAudioPackets.take();
	    pthread_mutex_unlock( pluginMutex );
	    return pkt;
	}

    // Buffer up some more packets
    for ( int i = 0; i < 100; i++ ) {
	AVPacket *pkt = new AVPacket;

	// read a packet from input
	if (av_read_packet(avFormatContext, pkt) < 0) {
//	    qDebug("EOF");
	    delete pkt;
	    pthread_mutex_unlock( pluginMutex );
	    return 0;
	}

	if ( pkt->stream_index == stream ) {
	    pthread_mutex_unlock( pluginMutex );
	    return pkt;
	}

	if ( pkt->stream_index == videoStream )
	    waitingVideoPackets.append( pkt );
	if ( pkt->stream_index == audioStream )
	    waitingAudioPackets.append( pkt );
    }

    pthread_mutex_unlock( pluginMutex );
    return 0;
}



