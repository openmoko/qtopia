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
#include "amrrecord.h"

extern "C" {
#include "typedef.h"
#include "frame.h"
#include "sp_enc.h"
#include "sid_sync.h"
#include "e_homing.h"
};


// Conversion handler for an audio input format.  Format handlers
// are responsible for converting from the device's encoding to
// normalized 16-bit host byte order samples.
//
// This should really be done in "mediarecorder", but we would need
// to change the plugin interface for that, which we don't want to
// do just yet.
class AmrAudioFormatHandler
{
public:
    virtual ~AmrAudioFormatHandler() { }

    // Convert a buffer of audio data into raw 16-bit sound samples.
    // "length" is the number of bytes read from the device.  Returns
    // the number of raw sound samples.
    virtual int convert( short *buffer, int length ) = 0;

};


// Perform channel averaging to convert stereo samples into mono.
class AmrSToMAudioFormatHandler : public AmrAudioFormatHandler
{
public:

    AmrSToMAudioFormatHandler( AmrAudioFormatHandler *_linked )
	{ linked = _linked; }
    ~AmrSToMAudioFormatHandler() { delete linked; }

    int convert( short *buffer, int length );

private:

    AmrAudioFormatHandler *linked;

};


int AmrSToMAudioFormatHandler::convert( short *buffer, int length )
{
    // Convert the raw samples into their normalized 16-bit form.
    int samples;
    if ( linked )
	samples = linked->convert( buffer, length );
    else
	samples = length;

    // Perform averaging on the the samples.
    int posn = 0;
    int limit = samples / 2;
    while ( posn < limit ) {
	buffer[posn] = (short)(((int)(buffer[posn * 2])) +
			       ((int)(buffer[posn * 2 + 1])) / 2);
	++posn;
    }
    return limit;
}


// Resample an audio stream to a different frequency.
class AmrResampleAudioFormatHandler : public AmrAudioFormatHandler
{
public:

    AmrResampleAudioFormatHandler( AmrAudioFormatHandler *_linked,
				   int _from, int _to, int _channels,
				   int bufferSize );
    ~AmrResampleAudioFormatHandler() { delete linked; delete[] temp; }

    int convert( short *buffer, int length );

private:

    AmrAudioFormatHandler *linked;
    int from, to, channels;
    long samplesDue;
    long rollingLeft;
    long rollingRight;
    int numInRolling;
    short *temp;

};


AmrResampleAudioFormatHandler::AmrResampleAudioFormatHandler
	( AmrAudioFormatHandler *_linked, int _from, int _to,
	  int _channels, int bufferSize )
{
    linked = _linked;
    from = _from;
    to = _to;
    channels = _channels;
    samplesDue = 0;
    rollingLeft = 0;
    rollingRight = 0;
    numInRolling = 0;
    temp = new short [bufferSize];
}

int AmrResampleAudioFormatHandler::convert( short *buffer, int length )
{
    // Convert the raw samples into their normalized 16-bit form.
    int samples;
    if ( linked )
	samples = linked->convert( buffer, length );
    else
	samples = length;
    if ( !samples )
	return 0;

    // Resample the data.  We should probably do some kind of curve
    // fit algorithm, but that can be *very* expensive CPU-wise.
    memcpy( temp, buffer, samples * sizeof(short) );
    int inposn = 0;
    int outposn = 0;
    short left, right;
    long due = samplesDue;
    long rollLeft = rollingLeft;
    long rollRight = rollingRight;
    int num = numInRolling;
    if ( from < to ) {
	// Replicate samples to convert to a higher sample rate.
	if ( channels == 1 ) {
	    while ( inposn < samples ) {
		due += to;
		left = temp[inposn++];
		while ( due >= from ) {
		    buffer[outposn++] = left;
		    due -= from;
		}
	    }
	} else {
	    while ( inposn < samples ) {
		due += to;
		left = temp[inposn++];
		right = temp[inposn++];
		while ( due >= from ) {
		    buffer[outposn++] = left;
		    buffer[outposn++] = right;
		    due -= from;
		}
	    }
	}
    } else {
	// Average samples to convert to a lower sample rate.
	// This may lose a small number (from / to) of samples
	// off the end of the stream.
	if ( channels == 1 ) {
	    while ( inposn < samples ) {
		left = temp[inposn++];
		rollLeft += (long)left;
		due += to;
		++num;
		if ( due >= from ) {
		    buffer[outposn++] = (short)(rollLeft / num);
		    rollLeft = 0;
		    num = 0;
		    due -= from;
		}
	    }
	} else {
	    while ( inposn < samples ) {
		left = temp[inposn++];
		right = temp[inposn++];
		rollLeft += (long)left;
		rollLeft += (long)right;
		due += to;
		++num;
		if ( due >= from ) {
		    buffer[outposn++] = (short)(rollLeft / num);
		    buffer[outposn++] = (short)(rollRight / num);
		    rollLeft = 0;
		    rollRight = 0;
		    num = 0;
		    due -= from;
		}
	    }
	}
    }
    samples = outposn;
    samplesDue = due;
    rollingLeft = rollLeft;
    rollingRight = rollRight;
    numInRolling = num;

    // Done
    return samples;
}


AmrRecorderPlugin::AmrRecorderPlugin()
{
    // Reset the recording state.
    device = 0;
    channels = 0;
    frequency = 0;
    writtenHeader = FALSE;
    blockLen = 0;
    amrState = 0;
    sidSync = 0;
    handler = 0;
}


bool AmrRecorderPlugin::begin( QIODevice *_device, const QString& )
{
    // Bail out if we are already recording.
    if ( device )
        return FALSE;
    
    // Bail out if the new device is invalid.
    if ( !_device )
        return FALSE;

    // Initialize the AMR encoding routines.
    Speech_Encode_FrameState *state;
    sid_syncState *sid;
    if ( Speech_Encode_Frame_init( &state, 0, "encoder" ) != 0 ) {
	return FALSE;
    }
    if ( sid_sync_init( &sid ) != 0 ) {
	Speech_Encode_Frame_exit( &state );
	return FALSE;
    }
    amrState = (void *)state;
    sidSync = (void *)sid;
    device = _device;
    return TRUE;
}


bool AmrRecorderPlugin::end()
{
    bool result = TRUE;

    // Bail out if we were not recording.
    if ( !device )
        return FALSE;

    // Flush the last AMR block if necessary.  If there were no blocks
    // in the stream at all, then add at least one silence block.
    if ( blockLen > 0 || !writtenHeader ) {
	if ( !writtenHeader ) {
	    if ( !writeHeader() )
		result = FALSE;
	    writtenHeader = TRUE;
	}
	memset( block + blockLen, 0, sizeof(short) * (160 - blockLen) );
	if ( result && !amrFlush() )
	    result = FALSE;
    }

    // Destroy the AMR state record.
    Speech_Encode_FrameState *state = (Speech_Encode_FrameState *)amrState;
    Speech_Encode_Frame_exit( &state );
    sid_syncState *sid = (sid_syncState *)sidSync;
    sid_sync_exit( &sid );

    // Reset the recording state for the next file.
    device = 0;
    channels = 0;
    frequency = 0;
    writtenHeader = FALSE;
    blockLen = 0;
    amrState = 0;
    sidSync = 0;
    if ( handler )
	delete handler;
    handler = 0;
    return result;
}


bool AmrRecorderPlugin::setAudioChannels( int _channels )
{
    if ( device && !writtenHeader &&
         ( _channels == 1 || _channels == 2 ) ) {
        channels = _channels;
        return TRUE;
    } else {
        return FALSE;
    }
}


bool AmrRecorderPlugin::setAudioFrequency( int _frequency )
{
    if ( device && !writtenHeader && _frequency > 0 ) {
        frequency = _frequency;
        return TRUE;
    } else {
        return FALSE;
    }
}


bool AmrRecorderPlugin::writeAudioSamples( const short *samples, long numSamples )
{
    // Bail out if we are not currently recording.
    if ( !device )
        return FALSE;

    // Write the header if necessary.
    if ( !writtenHeader ) {
        if ( !writeHeader() )
            return FALSE;
        writtenHeader = TRUE;
    }

    if ( handler ) {

	// Down-convert the audio data to 8 kHz Mono.
	short buf[512];
	int len, cvtlen, index;
	while ( numSamples > 0 ) {
	    if ( numSamples >= 512 )
		len = 512;
	    else
		len = (int)numSamples;
	    memcpy( buf, samples, len * sizeof(short) );
	    samples += len;
	    numSamples -= len;
	    cvtlen = handler->convert( buf, len );
	    for ( index = 0; index < cvtlen; ++index ) {
		block[blockLen++] = buf[index];
		if ( blockLen >= 160 ) {
		    blockLen = 0;
		    if ( !amrFlush() )
			return FALSE;
		}
	    }
	}

    } else {

	// Encode 8 kHz Mono AMR data directly.
	while ( numSamples > 0 ) {
	    block[blockLen++] = *samples++;
	    if ( blockLen >= 160 ) {
		blockLen = 0;
		if ( !amrFlush() )
		    return FALSE;
	    }
	    --numSamples;
	}

    }

    return TRUE;
}


long AmrRecorderPlugin::estimateAudioBps( int , int , const QString& )
{
    // AMR is always 8 kHz Mono, so ignore the parameters when estimating.
    // 160 input samples are turned into an output frame of 21 bytes.
    return (long)(((8000 + 159) / 160) * 21);
}


bool AmrRecorderPlugin::writeHeader()
{
    // Create the audio format handler that is required to convert
    // the input stream into 8 kHz mono sound.
    handler = 0;
    if ( channels != 1 )
	handler = new AmrSToMAudioFormatHandler( handler );
    if ( frequency > 8000 ) {
	handler = new AmrResampleAudioFormatHandler
	    ( handler, frequency, 8000, 1, 1024 );
    }

    // Write the header.
    static char header[] = "#!AMR\n";
    return ( device->writeBlock( header, 6 ) == 6 );
}


bool AmrRecorderPlugin::amrFlush()
{
    int resetFlag;
    short temp[250];
    unsigned char frame[64];
    Mode usedMode;
    TXFrameType frameType;
    int size;

    // Encode the frame.
    resetFlag = encoder_homing_frame_test( block );
    Speech_Encode_Frame( (Speech_Encode_FrameState *)amrState,
			 MR795 /* 7.95 kHz - closest to 8 kHz */,
			 block, temp, &usedMode );
    sid_sync( (sid_syncState *)sidSync, usedMode, &frameType );
    size = PackBits( usedMode, MR795, frameType, temp, frame );
    if ( resetFlag != 0 ) {
	Speech_Encode_Frame_reset( (Speech_Encode_FrameState *)amrState );
	sid_sync_reset( (sid_syncState *)sidSync );
    }

    // Write the frame to the output device.
    return ( device->writeBlock( (char *)frame, size ) == size );
}

