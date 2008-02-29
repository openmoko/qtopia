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
/**********************************************************************
** This file is part of the KPhone project.
** 
** KPhone is a SIP (Session Initiation Protocol) user agent for Linux, with
** which you can initiate VoIP (Voice over IP) connections over the Internet,
** send Instant Messages and subscribe your friends' presence information.
** Read INSTALL for installation instructions, and CHANGES for the latest
** additions in functionality and COPYING for the General Public License
** (GPL).
** 
** More information about Wirlab available at http://www.wirlab.net/
** 
** Note: "This program is released under the GPL with the additional
** exemption that compiling, linking, and/or using OpenSSL is allowed."
***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "dspoutrtp.h"
#include "dspaudiojackin.h"

typedef struct _thread_info {
    pthread_t thread_id;
    jack_nframes_t duration;
    jack_nframes_t rb_size;
    jack_client_t *client;
    unsigned int channels;
    int bitdepth;
    char *path;
    volatile int ready;
    volatile int running;
    volatile int status;
} jack_thread_info_t;


/* JACK data */
jack_port_t *portin;
jack_default_audio_sample_t *in;
static int udp_failures;
static bool broken;
static DspOutRtp* output;

jack_nframes_t nframes;
const size_t sample_size = sizeof(jack_default_audio_sample_t);


static int jack_callaudio_process (jack_nframes_t nframes, void *arg)
{

	jack_thread_info_t *info = (jack_thread_info_t *) arg;
	if (!info->ready || !info->running) {
		return 0;
	}
	in = (jack_default_audio_sample_t *) jack_port_get_buffer( portin, nframes );

	// Make sure there's room in the output
	int inputSize = sizeof(jack_default_audio_sample_t) * nframes;
	output->getBuffer().resize( inputSize );


//	if (!generateDTMF((short*)output->getBuffer().getData(), inputSize / sizeof(short))) {

		// Copy the data
		memcpy( output->getBuffer().getData(),  in, inputSize );
//	}

	// If the write fails, mark the pipe as broken and stop sending!
	if( !output->writeBuffer() ) {
		udp_failures++;
		if(udp_failures > 5){
			broken = true;
			printf( QObject::tr("DspAudioIn: Broken pipe") + "\n" );
		} else {
			printf( QObject::tr("DspAudioIn: UDP-failure(%d)") + "\n", udp_failures );
		}
	} else {
		udp_failures = 0;
	}

	return 1;
}


static void jack_callaudio_shutdown (void *arg)
{
	fprintf (stderr, "JACK shutdown\n");
	abort();
}


void setup_port_in (int sources, char *source_name, jack_thread_info_t *info)
{
	size_t in_size;

	portin = (jack_port_t *) malloc (sizeof (jack_port_t *));
	in_size =  sizeof (jack_default_audio_sample_t *);
	in = (jack_default_audio_sample_t *) malloc (in_size);

	memset(in, 0, in_size);

	char name[64];

	sprintf (name, "input%d", 1);

	if ((portin = jack_port_register (info->client, name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0)) == 0) {
		fprintf (stderr, "cannot register input port \"%s\"!\n", name);
		jack_client_close (info->client);
		exit (1);
	}
	if (jack_connect (info->client, source_name, jack_port_name( portin ))) {
		fprintf (stderr, "cannot connect input port %s to %s\n", jack_port_name( portin ), source_name );
		jack_client_close (info->client);
		exit (1);
	} 

	info->ready = 1;
}


DspAudioJackIn::DspAudioJackIn( DspOutRtp *outrtp, const QString &name )
{
	jack_clientName = name;
	audio_stereo = false;
	udp_failures = 0;
	broken = false;
	output = outrtp;

	if ((jack_client = jack_client_new( jack_clientName )) == 0 ) {
		printf( "CallAudio: Error, jack server not running?\n" );
		return;
	}
	
	jack_thread_info_t thread_info;
	
	thread_info.client = jack_client;
	thread_info.channels = 1;
	thread_info.ready = 0;
	thread_info.running = 0;
	jack_set_process_callback( jack_client, jack_callaudio_process, &thread_info );
	jack_on_shutdown( jack_client, jack_callaudio_shutdown, &thread_info );

	if (jack_activate( jack_client )) {
		fprintf (stderr, "cannot activate client");
	}
	setup_port_in( 1, "alsa_pcm:capture_1", &thread_info);
}


DspAudioJackIn::~DspAudioJackIn( void )
{
}


void DspAudioJackIn::start( void )
{
//	thread_info.running = 1;
}


void DspAudioJackIn::exit( void )
{
	if (jack_client) {
		jack_client_close( jack_client );
	}
}


// count is in 16-bit words in the smaller of the two buffers
void DspAudioJackIn::mono_to_stereo(void *b_from, void *b_to, size_t count)
{
	int8_t *b1 = reinterpret_cast<int8_t*>(b_from);
	int8_t *b2 = reinterpret_cast<int8_t*>(b_to);
	for( size_t i=0; i < count; i++ ) {
		int8_t x0 = b1[2*i];
		int8_t x1 = b1[2*i+1];
		b2[4*i]   = x0;
		b2[4*i+1] = x1;
		b2[4*i+2] = x0;
		b2[4*i+3] = x1;
	}
}


// count is in 16-bit words in the smaller of the two buffers
void DspAudioJackIn::stereo_to_mono(void *b_from, void *b_to, size_t count)
{
	int8_t *b1 = reinterpret_cast<int8_t*>(b_from);
	int8_t *b2 = reinterpret_cast<int8_t*>(b_to);
	for( size_t i=0; i < count; i++ ) {
		b2[2*i]   = b1[4*i];
		b2[2*i+1] = b1[4*i+1];
	}
}

