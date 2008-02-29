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
#include <qfileinfo.h>
#include "libffmpegplugin.h"


LibFFMpegPlugin::LibFFMpegPlugin() : videoContext(&packetBuffer), audioContext(&packetBuffer)
{
    FFAutoLockUnlockMutex lock(pluginMutex);
    strInfo = "";
    streamingFlag = FALSE;
    needPluginInit = TRUE;
}


LibFFMpegPlugin::~LibFFMpegPlugin()
{
    // qDebug("LibFFMpegPlugin::~LibFFMpegPlugin()");
    close();
    // qDebug("LibFFMpegPlugin::~LibFFMpegPlugin() finished");
}


bool LibFFMpegPlugin::isFileSupported( const QString& fileName )
{
    qDebug("Checking if %s is supported", fileName.latin1() );
    QString ext2 = fileName.right(3).lower();
    QString ext3 = fileName.right(4).lower();
    QString ext4 = fileName.right(5).lower();
    if	( ( ext3 == ".asf" ) || ( ext3 == ".amr" ) || ( ext3 == ".avi" ) ||
	//( ext3 == ".ogg" ) || ( ext3 == ".vob" ) || ( ext3 == ".wav" ) ||
	  ( ext3 == ".mov" ) || ( ext3 == ".mp2" ) || ( ext3 == ".mp3" ) ||
	  ( ext3 == ".mp4" ) || ( ext4 == ".mpeg") || ( ext3 == ".mpg" ) ||
	  ( ext2 == ".rm"  ) || ( ext3 == ".wma" ) || ( ext3 == ".wmf" ) ||
	  ( ext3 == ".wmv" ))
	return true;
    // Wav files need to be handled by the wav plugin
    if ( ext3 == ".wav" )
	return false;
    bool okay = open( fileName );
    close();
    return okay;
}


bool LibFFMpegPlugin::open( const QString& fileName )
{
    if ( !init( fileName ) )
	return false;
    return true;
}


bool LibFFMpegPlugin::openURL( const QUrl& url, const QString& )
{
    if ( !init(url.toString(false, false)) )
	return false;
    FFAutoLockUnlockMutex lock(pluginMutex);
    streamingFlag = true;
    return true;
}


bool LibFFMpegPlugin::init( const QString& fileName )
{
    FFAutoLockUnlockMutex lock(pluginMutex);
    qDebug("LibFFMpegPlugin::init()");

    if ( needPluginInit ) {
	avcodec_init();
	avcodec_register_all();
	av_register_all();
	needPluginInit = false;
    }

    fileLength = 0;
    totalTimeCache = 0;
    haveTotalTimeCache = false;
    streamingFlag = false;
    strInfo = "";
    savedSeekPos.assign(-1);

    packetBuffer.init();
    videoContext.fileInit();
    audioContext.fileInit();

    if (!packetBuffer.open(fileName))
	return false;

    audioContext.codecInit();
    videoContext.codecInit();

    // Get the file length
    fileLength = packetBuffer.fileLength();

    // Get the total bit rate
    totalBitRate = packetBuffer.bitRate();
    if ( !totalBitRate ) {
	// Add together the audio and video bit rates
	long audioBitRate = audioContext.bitRate();
	long videoBitRate = videoContext.bitRate();
	if ( audioStreams() && audioBitRate > 0 )
	    totalBitRate += audioBitRate;
	if ( videoStreams() && videoBitRate > 0 )
	    totalBitRate += videoBitRate;
    }
    haveTotalBitRate = (bool)totalBitRate;

    // Get the total time
    totalTimeCache = packetBuffer.duration();
    if ( !totalTimeCache ) {
	if ( totalBitRate && fileLength ) {
	    // The 8000 multiplier is because the rate is in bits per second and
	    // there are 8 bits to a byte and we want the time in milliseconds.
	    totalTimeCache = ((long long)fileLength * 8000) / totalBitRate;
	}
    }
#if FIND_LAST_PACKET_TIMESTAMP_TO_WORK_OUT_PLAY_TIME
    // Last resort attempt to determine the total play time
    // Seeks towards the end of the file, reads packets looking for timestamps
    if ( !totalTimeCache )
	totalTimeCache = packetBuffer.getLastPacketTime();
#endif
    haveTotalTimeCache = (bool)totalTimeCache;

    qDebug("  File length: %li bytes", fileLength);
    qDebug("  Total time: %li ms", totalTimeCache);
    qDebug("  Total bitrate: %li bits/sec", totalBitRate);

    qDebug("LibFFMpegPlugin::init() finished");

    // Buffer initial packets
    packetBuffer.start();
    
    return true;
}


const QString &LibFFMpegPlugin::fileInfo()
{
    FFAutoLockUnlockMutex lock(pluginMutex);
    if ( strInfo == "" ) {
	if ( haveTotalTimeCache ) {
	    int seconds = totalTimeCache / 1000;
	    strInfo += qApp->translate( "LibFFMpegPlugin", "Length: " ) + QString::number( seconds / 60 ) + ":" +
		 QString::number( (seconds % 60) / 10 ) + QString::number( (seconds % 60) % 10 ) + ",";
	}
	strInfo += audioContext.fileInfo();
	strInfo += videoContext.fileInfo();
    }
    return strInfo;
}


bool LibFFMpegPlugin::close()
{
    qDebug("LibFFMpegPlugin::close()");

    FFAutoLockUnlockMutex lock(pluginMutex);

    flushBuffers();

    if ( packetBuffer.isOpen() ) {
	fileLength = 0;
	audioContext.close();
	videoContext.close();
	packetBuffer.close();
	return TRUE;
    }

    return FALSE;
}


bool LibFFMpegPlugin::audioReadSamples( short *output, int channels, long samples, long& samplesRead, int )
{
/*
    qDebug("LibFFMpegPlugin::audioReadSamples()     decoded pts, video: %5li audio: %5li  faked pts, video: %5li audio: %5li",
	    videoContext.absoluteTimeStamp(), audioContext.absoluteTimeStamp(), videoContext.timeStamp(), audioContext.timeStamp());
*/
    return (realSeek()) ? audioContext.audioReadSamples(output, channels, samples, samplesRead) : false;
}


bool LibFFMpegPlugin::videoReadScaledFrame( unsigned char **output_rows, int, int, int in_w, int in_h, int out_w, int out_h, ColorFormat fmt, int )
{
/*
    qDebug("LibFFMpegPlugin::videoReadScaledFrame() decoded pts, video: %5li audio: %5li  faked pts, video: %5li audio: %5li",
	    videoContext.absoluteTimeStamp(), audioContext.absoluteTimeStamp(), videoContext.timeStamp(), audioContext.timeStamp());
*/
    return (realSeek()) ? videoContext.videoReadScaledFrame(output_rows, in_w, in_h, out_w, out_h, fmt) : 1;
}


bool LibFFMpegPlugin::sync()
{
    if ( realSeek() && packetBuffer.isOpen() && audioStreams() && videoStreams() )
        return videoContext.syncToTimeStamp(audioContext.timeStamp()); // relative time stamp
    //qDebug("sync needs both audio and video contexts to sync together");
    return false;
}


bool LibFFMpegPlugin::seek( long pos )
{
    qDebug("LibFFMpegPlugin::seek()");
    FFAutoLockUnlockMutex lock(pluginMutex);
    if ( pos > 1000 )
	pos = pos - 1000;
    else
	pos = 0;
    savedSeekPos.assign(pos);
    qDebug("LibFFMpegPlugin::seek() finished");
    return true;
}

//
// Seek and realSeek are split in two because realSeek can take a reasonable
// amount of CPU when both video and audio are involved.
// The mediaplayer calls seek as the user moves the slider, and as they
// move the slider it updates the UI with where in the file they are.
// This is queried with a call to currentTime()
//
bool LibFFMpegPlugin::realSeek()
{
    // throw away any outstanding packets we aren't consuming
    if ( !audioStreams() )
	packetBuffer.flushAudioPackets();
    if ( !videoStreams() )
	packetBuffer.flushVideoPackets();

    if ( savedSeekPos == -1 )
	return true;

    FFAutoLockUnlockMutex lock(pluginMutex);
    long pos = (int)savedSeekPos;
    savedSeekPos.assign(-1);
    
    // Seek in to the file
    if ( packetBuffer.seek(pos) ) {
	if ( videoStreams() ) {
	    videoContext.leadInVideo();
	    if ( audioStreams() )
		// Throw away audio packets till they are in time with the video packets
		audioContext.syncToTimeStamp(videoContext.absoluteTimeStamp()); // absolute value
	}
	if ( audioStreams() ) {
	    audioContext.leadInAudio();
	    if ( videoStreams() )
		videoContext.syncToTimeStamp(audioContext.absoluteTimeStamp());
	}
	return true;
    }

    // qDebug("LibFFMpegPlugin::realSeek() finished");
    return false;
}


long LibFFMpegPlugin::tell()
{
    if (savedSeekPos != -1)
	return (int)savedSeekPos;
    return packetBuffer.pos();
}


bool LibFFMpegPlugin::currentTimeAvailable()
{
    // Use audio context first, using its lock
    long audioTimeStamp = audioContext.absoluteTimeStamp();
    if (audioTimeStamp != -1)
	return true;
    // Then seperately lock plugin without audio context locked
    FFAutoLockUnlockMutex lock(pluginMutex);
    return (totalBitRate || (fileLength && haveTotalTimeCache));
}


long LibFFMpegPlugin::currentTime()
{
    // qDebug("LibFFMpegPlugin::currentTime");

    // Use audio context first, using its lock
    long audioTimeStamp = audioContext.absoluteTimeStamp();
    if ( audioTimeStamp != -1 ) {
	if (savedSeekPos == -1) {
	    // qDebug("LibFFMpegPlugin::currentTime end");
	    return audioTimeStamp;
	}
    }

    FFAutoLockUnlockMutex lock(pluginMutex);
    if ( fileLength && haveTotalTimeCache ) {
	// This method is generally more accurate, try it before using the bitrate method
	return ((long long)totalTimeCache * tell()) / fileLength;
    } else if ( totalBitRate ) {
	// The 8000 multiplier is because the rate is in bits per second and
	// there are 8 bits to a byte and we want the time in milliseconds.
	return ((long long)8000 * tell()) / totalBitRate;
    }
    return -1;
}





//
// Only calls thread safe functions
//
void LibFFMpegPlugin::flushBuffers()
{
    audioContext.flush();
    videoContext.flush();
    packetBuffer.flush();
}












//
// Thread safe assuming QString and QUrl functions don't make use of static or global variables.
// Also shouldn't be a problem as this function in reality is only called before decoding
// starts always from the main thread
//
bool LibFFMpegPlugin::canStreamURL( const QUrl& url, const QString& )
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


bool LibFFMpegPlugin::streamed()
{
    FFAutoLockUnlockMutex lock(pluginMutex);
    return streamingFlag;
}


bool LibFFMpegPlugin::lengthAvailable()
{
    FFAutoLockUnlockMutex lock(pluginMutex);
    return !streamingFlag;
}


long LibFFMpegPlugin::length()
{
    FFAutoLockUnlockMutex lock(pluginMutex);
    return fileLength;
}


bool LibFFMpegPlugin::totalTimeAvailable()
{
    FFAutoLockUnlockMutex lock(pluginMutex);
    return haveTotalTimeCache;
}


long LibFFMpegPlugin::totalTime()
{
    FFAutoLockUnlockMutex lock(pluginMutex);
    return totalTimeCache;
}


bool LibFFMpegPlugin::seekAvailable()
{
    FFAutoLockUnlockMutex lock(pluginMutex);
    return !streamingFlag;
}


bool LibFFMpegPlugin::tellAvailable()
{
    FFAutoLockUnlockMutex lock(pluginMutex);
    return true;//!streamingFlag;
}


//
// Clearly thread safe functions
//
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


bool LibFFMpegPlugin::isOpen()
{
    return packetBuffer.isOpen();
}


int LibFFMpegPlugin::audioStreams()
{
    return audioContext.audioStreams();
}


int LibFFMpegPlugin::audioChannels(int stream)
{
    return audioContext.audioChannels(stream);
}


int LibFFMpegPlugin::audioFrequency(int stream)
{
    return audioContext.audioFrequency(stream);
}


int LibFFMpegPlugin::audioSamples( int )
{
    return -1;
}


bool LibFFMpegPlugin::audioSetSample( long, int )
{
    return false;
}


long LibFFMpegPlugin::audioGetSample( int )
{
    return 0;
}


int LibFFMpegPlugin::videoStreams()
{
    return videoContext.videoStreams();
}


int LibFFMpegPlugin::videoWidth(int stream)
{
    return videoContext.videoWidth(stream);
}


int LibFFMpegPlugin::videoHeight(int stream)
{
    return videoContext.videoHeight(stream);
}


double LibFFMpegPlugin::videoFrameRate(int stream)
{
    return videoContext.videoFrameRate(stream);
}


int LibFFMpegPlugin::videoFrames( int )
{
    return -1;
}


bool LibFFMpegPlugin::videoSetFrame( long, int )
{
    return false;
}


long LibFFMpegPlugin::videoGetFrame( int )
{
    return -1;
}


bool LibFFMpegPlugin::videoReadFrame( unsigned char **, int, int, int, int, ColorFormat, int )
{
    return false;
}


bool LibFFMpegPlugin::videoReadYUVFrame( char *, char *, char *, int, int, int, int, int )
{
    return false;
}


double LibFFMpegPlugin::getTime()
{
    return -1;
}


bool LibFFMpegPlugin::setSMP( int )
{
    return false;
}


bool LibFFMpegPlugin::setMMX( bool )
{
    return false;
}


bool LibFFMpegPlugin::supportsAudio()
{
    return true;
}


bool LibFFMpegPlugin::supportsVideo()
{
    return true;
}


bool LibFFMpegPlugin::supportsYUV()
{
    return false;
}


bool LibFFMpegPlugin::supportsMMX()
{
    return false;
}


bool LibFFMpegPlugin::supportsSMP()
{
    return false;
}


bool LibFFMpegPlugin::supportsStereo()
{
    return true;
}


bool LibFFMpegPlugin::supportsScaling()
{
    return true;
}


long LibFFMpegPlugin::getPlayTime()
{
    return -1;
}


bool LibFFMpegPlugin::supportsStreaming()
{
    return true;
}


bool LibFFMpegPlugin::syncAvailable()
{
    return true;
}


