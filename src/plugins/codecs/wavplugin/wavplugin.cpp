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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <qfile.h>
#include "wavplugin.h"
#ifndef Q_OS_WIN32
#include <unistd.h>
#endif

extern "C" {
#include "gsm.h"
};


//#define debugMsg(a)	    qDebug(a)
#define debugMsg(a)

#ifndef Q_OS_WIN32
// Win32 has a different name for the same function
# define strncasecmp strncmp	

# include <endian.h>
# if __BYTE_ORDER == __BIG_ENDIAN
#  define IS_BIG_ENDIAN
# endif
#endif

struct RiffChunk {
    char id[4];
    Q_UINT32 size;
    char data[4];
};


struct ChunkData {
    Q_INT16 formatTag;
    Q_INT16 channels;
    Q_INT32 samplesPerSec;
    Q_INT32 avgBytesPerSec;
    Q_INT16 blockAlign;
    Q_INT16 wBitsPerSample;
};


struct GsmExtHeader {
    Q_INT16 wExtSize;
    Q_INT16 samplesPerBlock;
};


#ifdef IS_BIG_ENDIAN
Q_INT32 swap32( Q_INT32 w )
{
    unsigned char *c = (unsigned char *)&w;

    // Swap byte 1 and 4
    unsigned char t = *c;
    *c = *(c+3);
    *(c+3) = t;

    // Swap byte 2 and 3
    c++;
    t = *c;
    *c = *(c+1);
    *(c+1) = t;

    return w;
}

inline Q_INT16 swap16( Q_INT16 s )
{
    unsigned char *c = (unsigned char *)&s;
    unsigned char t = *c;
    *c = *(c+1);
    *(c+1) = t;
    
    return s;
}

void swapRiffChunk( RiffChunk *r )
{
    r->size = swap32(r->size);
}

void swapChunkData( ChunkData *d )
{
    d->formatTag = swap16(d->formatTag);
    d->channels = swap16(d->channels);
    d->samplesPerSec = swap32(d->samplesPerSec);
    d->avgBytesPerSec = swap32(d->avgBytesPerSec);
    d->blockAlign = swap16(d->blockAlign);
    d->wBitsPerSample = swap16(d->wBitsPerSample);
}

void swapGsmExtHeader( GsmExtHeader *h )
{
    h->wExtSize = swap16(h->wExtSize);
    h->samplesPerBlock = swap16(h->samplesPerBlock);
}
#else
inline Q_INT32 swap32( Q_INT32 w ) { return w; }
inline Q_INT16 swap16( Q_INT16 s ) { return s; }
inline void swapRiffChunk( RiffChunk * ) {}
inline void swapChunkData( ChunkData * ) {}
inline void swapGsmExtHeader( GsmExtHeader * ) {}
#endif


const int sound_buffer_size = 4096;


#define WAVE_FORMAT_PCM    1
#define WAVE_FORMAT_GSM610 0x31


class WavPluginData {
public:
    QFile *input;

    int wavedata_remaining;
    ChunkData chunkdata;
    GsmExtHeader gsmext;
    RiffChunk chunk;
    uchar data[sound_buffer_size+32]; // +32 to handle badly aligned input data
    int out,max;
    int samples_due;
    int samples;
    gsm gsmhandle;
    gsm_signal gsmsamples[320];
    int gsmnext;

    WavPluginData() {
	input = 0;
	max = out = sound_buffer_size;
	wavedata_remaining = 0;
	samples_due = 0;
	samples = -1;
	gsmhandle = 0;
	gsmnext = 320;
    }
    ~WavPluginData() {
	if ( gsmhandle ) {
	    gsm_destroy( gsmhandle );
	}
    }

    // expands out samples to the frequency of 44kHz
    bool add( short *output, long count, long& done, bool stereo )
    {
	done = 0;

	if ( input == 0 ) {
	    qDebug("no input");
	    return FALSE;
	}

	if ( chunkdata.formatTag == WAVE_FORMAT_PCM ) {
	    // read PCM samples
	    while ( count ) {
		int l,r;
		if ( getSample(l, r) == FALSE ) {
		    // Make the loopcontrol think we played to the end
		    done = count;
		    return FALSE;
		}
		samples_due += 44100;
		while ( count && ( samples_due >= chunkdata.samplesPerSec ) ) {
		    *output++ = l;
		    if ( stereo )
			*output++ = r;
		    samples_due -= chunkdata.samplesPerSec;
		    count--;
		    done++;
		}
	    }
        } else {
	    // read GSM samples
	    while ( count ) {
		int samp;
		if ( gsmnext < 320 ) {
		    samp = gsmsamples[gsmnext++];
		} else if ( nextGsmBlock() ) {
		    samp = gsmsamples[gsmnext++];
		} else {
		    // Make the loopcontrol think we played to the end
		    done = count;
		    return FALSE;
		}
		samples_due += 44100;
		while ( count && ( samples_due >= chunkdata.samplesPerSec ) ) {
		    *output++ = samp;
		    if ( stereo )
			*output++ = samp;
		    samples_due -= chunkdata.samplesPerSec;
		    count--;
		    done++;
		}
	    }
	}

        return TRUE;
    }

    bool initialise() {
	if ( input == 0 )
	    return FALSE;

	if ( gsmhandle ) {
	    gsm_destroy( gsmhandle );
	    gsmhandle = 0;
	}

	wavedata_remaining = -1;

	while ( wavedata_remaining == -1 ) {
	    // Keep reading chunks...
	    const int n = sizeof(chunk) - sizeof(chunk.data);
	    int t = input->readBlock( (char*)&chunk, n );
	    if ( t != n ) {
		if ( t == -1 )
		    return FALSE;
		return TRUE;
	    }
	    swapRiffChunk(&chunk);
	    if ( qstrncmp(chunk.id,"data",4) == 0 ) { // No tr
		wavedata_remaining = chunk.size;
		// approx. number of 44.1KHz samples
		if ( chunkdata.formatTag == WAVE_FORMAT_GSM610 ) {
		    samples = (wavedata_remaining / 65) * 320;
		    samples = samples * 441 / (chunkdata.samplesPerSec/100);
		} else {
		    samples = wavedata_remaining / chunkdata.channels;
		    if ( chunkdata.wBitsPerSample == 16 )
			samples /= 2;
		    samples = samples * 441 / (chunkdata.samplesPerSec/100);
		}
	    } else if ( qstrncmp(chunk.id,"RIFF",4) == 0 ) {
		char d[4];
		if ( input->readBlock(d,4) != 4 ) {
		    return FALSE;
		}
		if ( qstrncmp(d,"WAVE",4) != 0 ) {
		    // skip
		    if ( chunk.size > 1000000000 || !input->at(input->at()+chunk.size-4) ) {
			return FALSE;
		    }
		}
	    } else if ( qstrncmp(chunk.id,"fmt ",4) == 0 ) {
		int cur_pos = input->at();
		if ( input->readBlock((char*)&chunkdata,sizeof(chunkdata)) != sizeof(chunkdata) ) {
		    return FALSE;
		}
		swapChunkData(&chunkdata);
		if ( chunkdata.formatTag == WAVE_FORMAT_GSM610 ) {
		    // validate the GSM header details.
		    if ( chunk.size < (sizeof(chunkdata) + 4) ) {
			qDebug( "WAV file: BAD GSM HEADER SIZE" );
			return FALSE;
		    }
		    if ( input->readBlock( (char *)&gsmext, sizeof(gsmext) ) != sizeof(gsmext) ) {
			qDebug( "WAV file: TRUNCATED GSM HEADER" );
			return FALSE;
		    }
		    swapGsmExtHeader(&gsmext);
		    if ( gsmext.wExtSize != 2 ||
		         gsmext.samplesPerBlock != 320 ||
			 chunkdata.blockAlign != 65 ) {
			qDebug( "WAV file: INCORRECT GSM PARAMETERS" );
			return FALSE;
		    }
		} else if ( chunkdata.formatTag != WAVE_FORMAT_PCM ) {
		    qDebug("WAV file: UNSUPPORTED FORMAT %d",chunkdata.formatTag);
		    return FALSE;
		}
		if ( chunk.size > 1000000000 || !input->at( cur_pos + chunk.size ) ) {
		    return FALSE;
		}
	    } else {
		// ignored chunk
		if ( chunk.size > 1000000000 || !input->at(input->at()+chunk.size) ) {
		    return FALSE;
		}
	    }
	} // while

	// initialize the GSM decompression code.
	if ( chunkdata.formatTag == WAVE_FORMAT_GSM610 ) {
	    gsmhandle = gsm_create();
	    if ( !gsmhandle ) {
		return FALSE;
	    }
	    int value = 1;
	    gsm_option( gsmhandle, GSM_OPT_WAV49, &value );
	}

	return TRUE;
    }


    // gets a sample from the file
    bool getSample(int& l, int& r)
    {
	l = r = 0;

	if ( input == 0 )
	    return FALSE;

	if ( (wavedata_remaining < 0) || !max )
	    return FALSE;

	if ( out >= max ) {
	    max = input->readBlock( (char*)data, (uint)QMIN(sound_buffer_size,wavedata_remaining) );

	    wavedata_remaining -= max;

	    out = 0;
	    if ( max <= 0 ) {
		max = 0;
		return TRUE;
	    }
	}
	if ( chunkdata.wBitsPerSample == 8 ) {
	    l = (data[out++] - 128) * 128;
	} else {
#ifdef IS_BIG_ENDIAN
	    l = swap16(((short*)data)[out/2]);
#else
	    l = ((short*)data)[out/2];
#endif
	    out += 2;
	}
	if ( chunkdata.channels == 1 ) {
	    r = l;
	} else {
	    if ( chunkdata.wBitsPerSample == 8 ) {
		r = (data[out++] - 128) * 128;
	    } else {
#ifdef IS_BIG_ENDIAN
		r = swap16(((short*)data)[out/2]);
#else
		r = ((short*)data)[out/2];
#endif
		out += 2;
	    }
	}
	return TRUE;
    } // getSample

    // get the next block of GSM data to be played
    bool nextGsmBlock()
    {
	if ( input == 0 )
	    return FALSE;

	if ( (wavedata_remaining < 0) || !max )
	    return FALSE;

	if ( out >= max ) {
	    // read as many 65-byte GSM frames as possible
	    int size = QMIN(sound_buffer_size,wavedata_remaining);
	    size -= size % 65;
	    max = input->readBlock( (char*)data, (uint)size );

	    wavedata_remaining -= max;

	    out = 0;
	    if ( max < 65 ) {
		max = 0;
		return FALSE;
	    }
	}

	// decode the two halves of the 65-byte GSM frame
	gsm_decode( gsmhandle, (gsm_byte *)(data + out), gsmsamples );
	gsm_decode( gsmhandle, (gsm_byte *)(data + out + 33), gsmsamples + 160 );
	out += 65;
	gsmnext = 0;

	return TRUE;
    } // nextGsmBlock

};


WavPlugin::WavPlugin() {
    d = new WavPluginData;
    d->input = 0;
}


WavPlugin::~WavPlugin() {
    close();
    delete d;
}


bool WavPlugin::isFileSupported( const QString& path ) {
    debugMsg( "WavPlugin::isFileSupported" );

    char *ext = strrchr( path.latin1(), '.' );

    // Test file extension
    if ( ext ) {
	if ( strncasecmp(ext, ".raw", 4) == 0 )
	    return TRUE;
	if ( strncasecmp(ext, ".wav", 4) == 0 )
	    return TRUE;
	if ( strncasecmp(ext, ".wave", 4) == 0 )
	    return TRUE;
    }

    return FALSE;
}


bool WavPlugin::open( const QString& path ) {
    debugMsg( "WavPlugin::open" );

    d->max = d->out = sound_buffer_size;
    d->wavedata_remaining = 0;
    d->samples_due = 0;

    d->input = new QFile( path );
    if ( d->input->open(IO_ReadOnly) == FALSE ) {
	qDebug("couldn't open file");
	delete d->input;
	d->input = 0;
	return FALSE;
    }

    d->initialise();

    return TRUE;
}


bool WavPlugin::close() {
    debugMsg( "WavPlugin::close" );

    if (d->input) {
	d->input->close();
    }
    delete d->input;
    d->input = 0;
    return TRUE;
}


bool WavPlugin::isOpen() {
    debugMsg( "WavPlugin::isOpen" );
    return ( d->input != 0 );
}


int WavPlugin::audioStreams() {
    debugMsg( "WavPlugin::audioStreams" );
    return 1;
}


int WavPlugin::audioChannels( int ) {
    debugMsg( "WavPlugin::audioChannels" );
    return 2; // ### Always scale audio to stereo samples
}


int WavPlugin::audioFrequency( int ) {
    debugMsg( "WavPlugin::audioFrequency" );
    return 44100; // ### Always scale to frequency of 44100
}


int WavPlugin::audioSamples( int ) {
    debugMsg( "WavPlugin::audioSamples" );
    return d->samples;
}


bool WavPlugin::audioSetSample( long, int ) {
    debugMsg( "WavPlugin::audioSetSample" );
    return FALSE;
}


long WavPlugin::audioGetSample( int ) {
    debugMsg( "WavPlugin::audioGetSample" );
    return 0;
}


#ifdef OLD_MEDIAPLAYER_API


bool WavPlugin::audioReadSamples( short *, int, long, int ) {
    debugMsg( "WavPlugin::audioReadSamples" );
    return FALSE;
}


bool WavPlugin::audioReReadSamples( short *, int, long, int ) {
    debugMsg( "WavPlugin::audioReReadSamples" );
    return FALSE;
}


bool WavPlugin::audioReadMonoSamples( short *output, long samples, long& samplesMade, int ) {
    debugMsg( "WavPlugin::audioReadMonoSamples" );
    return !d->add( output, samples, samplesMade, FALSE );
}


bool WavPlugin::audioReadStereoSamples( short *output, long samples, long& samplesMade, int ) {
    debugMsg( "WavPlugin::audioReadStereoSamples" );
    return !d->add( output, samples, samplesMade, TRUE );
}


#else


bool WavPlugin::audioReadSamples( short *output, int channels, long samples, long& samplesMade, int ) {
    debugMsg( "WavPlugin::audioReadSamples" );
    return d->add( output, samples, samplesMade, channels != 1 );
}


#endif


double WavPlugin::getTime() {
    debugMsg( "WavPlugin::getTime" );
    return 0.0;
}


