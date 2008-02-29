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

#define ALSA_PCM_NEW_HW_PARAMS_API
//#define ALSADEBUG
#include "dspoutalsa.h"


//static snd_output_t *log;
static snd_pcm_uframes_t chunk_size = 0;
static snd_pcm_uframes_t period_frames = 0;
//static snd_pcm_uframes_t buffer_frames = 0;


DspOutAlsa::DspOutAlsa( const QString &devName )
	: DspOut()
{
	devname = devName;
	setDeviceName( "alsa" );
}

DspOutAlsa::~DspOutAlsa( void )
{
	if ((err = snd_pcm_close( handle)) < 0) {
		fprintf (stderr, "cannot close audio device (%s)\n", 
			 snd_strerror (err));
		exit (1);
	}
}

bool DspOutAlsa::openDevice( DeviceMode mode )
{
	unsigned int rate = 8000; 
	unsigned int exact_rate;
	int dir;
	if( mode == ReadOnly ) {
		if ((err = snd_pcm_open( &handle, devname, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
			fprintf (stderr, "cannot open audio device (%s)\n", 
				 snd_strerror (err));
			exit (1);
		}
	} else {

//		devname = "plug:dmix"; //why should we modify the default here??

		if ((err = snd_pcm_open( &handle, devname, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
			fprintf (stderr, "cannot open audio device (%s)\n", 
				 snd_strerror (err));
			exit (1);
		}
	}   

	if ((err = snd_pcm_hw_params_malloc( &hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_any( handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_access( handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_format( handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		fprintf (stderr, "cannot set sample format (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_channels( handle, hw_params, 2 )) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	exact_rate = rate;
	dir = 0;
	if ((err = snd_pcm_hw_params_set_rate_near( handle, hw_params, &exact_rate, &dir )) < 0) {
		fprintf (stderr, "cannot set rate (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
	if (dir != 0) {
		fprintf(stderr, "The rate %d Hz is not supported by your hardware.\n==> Using %d Hz instead.\n", rate, exact_rate);
	}

	period_frames = 32; 
	if ((err = snd_pcm_hw_params_set_period_size_near( handle, hw_params, &period_frames, &dir)) < 0) {
		fprintf (stderr, "cannot set sample format (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params( handle, hw_params )) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	snd_pcm_hw_params_get_period_size( hw_params, &chunk_size, &dir );
	snd_pcm_hw_params_get_buffer_size( hw_params, &buffer_size );

	fprintf (stderr, "\n\n\n----------<%d - %d>--------------\n\n\n", chunk_size, buffer_size );

	audio_buf.resize( chunk_size*2 );


	if (chunk_size == buffer_size) {
		fprintf (stderr, "Can't use period equal to buffer size (%lu == %lu)", chunk_size, buffer_size);
		exit (1);
	}

	snd_pcm_hw_params_free (hw_params);

	lasterror = QString::null;
	devstate = DeviceOpened;
  
	return true;
}


bool DspOutAlsa::writeBuffer( void )
{
	snd_pcm_sframes_t delay;
	unsigned int rate = 8000; 
	if( devstate != DeviceOpened ) {
		lasterror = "Device Not Open";
		return false;
	}

	int rc;
	size_t count = audio_buf.getSize();
	void *buf;
	int8_t x0 = 0;
	int8_t x1 = 0;
	buf = alloca(2*(count+20));
	int8_t *b1 = reinterpret_cast<int8_t*>(audio_buf.getData());
	int8_t *b2 = reinterpret_cast<int8_t*>(buf);
	for( size_t i=0; i < count/2; i++ ) {
		x0 = b1[2*i];
		x1 = b1[2*i+1];
		b2[4*i]   = x0;
		b2[4*i+1] = x1;
		b2[4*i+2] = x0;
		b2[4*i+3] = x1;
	}
	if (snd_pcm_delay(handle, &delay) < 0) { //FIXME: maybe this should go to another location
#ifdef ALSADEBUG
		fprintf(stderr, "snd_pcm_delay failed\n");
#endif
		/* this probably means a buffer underrund, therefore delay = 0 */
		delay = 0; //FIXME: check for buffer underrun
	}
	if (delay > (int)rate/10) { /* more than 0.1 s delay */
#ifdef ALSADEBUG
		fprintf(stderr,"too high delay: %d, dropping data\n", delay);
#endif
		return true;
	}
      
	if ((delay + count/2) < (rate/200)) { /* after writing samples less than 5 ms delay */
#ifdef ALSADEBUG
		fprintf(stderr,"too little delay: %d, underrun likely, inserting additional samples\n", delay+count/2);
#endif
		for( size_t i = count/2; i < count/2 + 10; i++ ) {
			b2[4*i]   = x0;
			b2[4*i+1] = x1;
			b2[4*i+2] = x0;
			b2[4*i+3] = x1;
		}
		count += 20;
	}
	while ((rc = snd_pcm_writei(handle, buf, count/2)) != (int)count/2) {
		if (rc >= 0) {
			/* This should never happen, because we opened the device in blocking mode */
#ifdef ALSADEBUG
			fprintf(stderr,  "write %d instead of %d frames !\n", rc, (int)count/2);
#endif
			(int8_t*) buf += rc*4;
			count -= rc*2;
			continue;
		}
		if (rc == -EPIPE) {
			/* buffer underrun */
#ifdef ALSADEBUG
			fprintf(stderr, "buffer underrun\n");
#endif
			if (snd_pcm_prepare(handle) < 0) {
				fprintf(stderr, "snd_pcm_prepare failed\n");
			}
			continue;
		} else if (rc == - ESTRPIPE) {
			int err;
			fprintf(stderr, "strpipe\n");
			while ((err = snd_pcm_resume(handle)) == -EAGAIN) {
				fprintf(stderr, "resume failed, prepare\n");
				sleep(1); //maybe a shorter time would be better, use usleep or msleep
			}
			if (err < 0) {
				fprintf(stderr, "resume failed, prepare\n");
				if (snd_pcm_prepare(handle) < 0) {
					fprintf(stderr, "snd_pcm_prepare failed\n");
					return false;
				}
			}
			continue;
		}
		fprintf(stderr, "other problem\n");
		if (snd_pcm_prepare(handle) < 0) {
			fprintf(stderr, "snd_pcm_prepare failed\n");
			return false;
		}
	}
	return true;
}

unsigned int DspOutAlsa::readableBytes( void )
{
	if( devstate != DeviceOpened ) {
		return 0;
	}
	return 64;
}

bool DspOutAlsa::readBuffer( int bytes )
{

	if( devstate != DeviceOpened ) {
		lasterror = "Device Not Open";
		return false;
	}

	int rc;
	size_t frames = audio_buf.getSize()/2;
	frames=32;
	audio_buf.resize( frames*2 );
	void *buf;
	buf = alloca(frames*4);
	rc = snd_pcm_readi( handle, buf, frames );
	if (rc == -EPIPE) {
		snd_pcm_prepare( handle );
		return false;
	} else if (rc < 0) {
		fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
		return false;
	} else if (rc != (int)frames) {
		printf("warning: asked microphone for %lu frames but got %lu\n",
			(unsigned long)frames, (unsigned long)rc);
		frames = rc;
		audio_buf.resize( frames*2 );
	}
	int8_t *b1 = reinterpret_cast<int8_t*>(buf);
	int8_t *b2 = reinterpret_cast<int8_t*>(audio_buf.getData());
	for( size_t i=0; i < frames; i++ ) {
		b2[2*i]   = b1[4*i];
		b2[2*i+1] = b1[4*i+1];
	}

	return true;
}
