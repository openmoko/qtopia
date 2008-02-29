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
#include <qobject.h>

#include "dspoutoss.h"

DspOutOss::DspOutOss( const QString &fileName )
	: DspOut()
{
	filename = fileName;
	setDeviceName( "oss" );
	audio_fd = -1;
	audio_stereo = false;
}

DspOutOss::~DspOutOss( void )
{
	close( audio_fd );
}

bool DspOutOss::openDevice( DeviceMode mode )
{
	int oflag;
	if( mode == ReadOnly ) {
		oflag = O_RDONLY;
	} else if( mode == WriteOnly ) {
		oflag = O_WRONLY;
	} else {
		oflag = O_RDWR;
	}
	if( devstate == DeviceOpened ) {
		lasterror = "Device Already Open";
		printf( "ERROR: %s\n", lasterror.ascii() );
		return false;
	}

	// Open immediately or else
	audio_fd = open( filename.ascii(), oflag | O_NONBLOCK );
	if( audio_fd == -1 ) {
		lasterror = "Open Failed";
		printf( "ERROR: %s\n", lasterror.ascii() );
		return false;
	}

	// Remove O_NONBLOCK
	int flags = fcntl( audio_fd, F_GETFL );
	flags &= ~O_NONBLOCK;
	fcntl( audio_fd, F_SETFL, flags );

	// keep fragsize less than 20ms !!
	int frag = ( ( 32767 << 16 ) | 7 );
	if( ioctl( audio_fd, SNDCTL_DSP_SETFRAGMENT, &frag ) ) {
		lasterror = QString( "SETFRAG" ) + QString( strerror( errno ) );
		printf( "ERROR: %s\n", lasterror.ascii() );
		return false;
	}

	int format = AFMT_S16_LE;

	if( ioctl( audio_fd, SNDCTL_DSP_SETFMT, &format ) == -1 ) {
		lasterror = QString( "SETFMT" ) + QString( strerror( errno ) );
		printf( "ERROR: %s\n", lasterror.ascii() );
		return false;
	}

	if( format != AFMT_S16_LE ) {
		lasterror = "Format not supported";
		printf( "ERROR: %s\n", lasterror.ascii() );
		return false;
	}

	int channels = 1;

	if( ioctl( audio_fd, SNDCTL_DSP_CHANNELS, &channels ) == -1 ) {
		lasterror = QString( "DSP_STEREO" ) + QString( strerror( errno ) );
		printf( "ERROR: %s\n", lasterror.ascii() );
		return false;
	}

	if( channels != 1 && channels != 2 ) {
		lasterror = QObject::tr("Unsupported Number of Channels");
		printf( "ERROR: %s\n", lasterror.ascii() );
		return false;
	}

	if( channels == 2 ) {
		printf( QObject::tr("WARNING: nonmonophonic driver, forced into stereo mode (experimental)") + "\n" );
		audio_stereo=true;
	} else {
		audio_stereo=false;
	}

	rate = 8000;

	if( ioctl( audio_fd, SNDCTL_DSP_SPEED, &rate ) == -1 ) {
		lasterror = QString( "DSP_SPEED" ) + QString( strerror( errno ) );
		printf( "ERROR: %s\n", lasterror.ascii() );
		return false;
	}

	if( rate != 8000 ) {
		printf( QObject::tr("WARNING: driver rounded %d Hz request to %d Hz, off by %f%%", "1st param:rounded rate, 2nd param:original rate, 3rd param:rounded percentage") + "\n",
			8000, rate, 100*((rate-8000)/8000.0) );
	}


	audio_buf_info info;

	if( mode == WriteOnly ) {
		if( ioctl( audio_fd, SNDCTL_DSP_GETOSPACE, &info ) == -1 ) {
			lasterror = QString( "GETISPACE" ) + QString( strerror( errno ) );
			printf( "ERROR: %s\n", lasterror.ascii() );
			return false;
		}
	} else {
		if( ioctl( audio_fd, SNDCTL_DSP_GETISPACE, &info ) == -1 ) {
			lasterror = QString( "GETOSPACE" ) + QString( strerror( errno ) );
			printf( "ERROR: %s\n", lasterror.ascii() );
			return false;
		}
	}
	audio_buf.resize( info.fragsize * sizeof( short ) );

	lasterror = QString::null;
	devstate = DeviceOpened;
	return true;

}

bool DspOutOss::openDevice( int exist_fs )
{
	audio_fd = exist_fs;
	if( audio_fd == -1 ) {
		lasterror = "Open Failed";
		printf( "ERROR: %s\n", lasterror.ascii() );
		return false;
	}

	audio_buf_info info;
	if( ioctl( audio_fd, SNDCTL_DSP_GETISPACE, &info ) == -1 ) {
		lasterror = QString( "GETISPACE" ) + QString( strerror( errno ) );
		printf( "ERROR: %s\n", lasterror.ascii() );
		return false;
	}
	audio_buf.resize( info.fragsize * sizeof( short ) );

	lasterror = QString::null;
	devstate = DeviceOpened;
	return true;
}



// count is in 16-bit words in the smaller of the two buffers
void mono_to_stereo(void *b_from, void *b_to, size_t count)
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
void stereo_to_mono(void *b_from, void *b_to, size_t count)
{
	int8_t *b1 = reinterpret_cast<int8_t*>(b_from);
	int8_t *b2 = reinterpret_cast<int8_t*>(b_to);
	for( size_t i=0; i < count; i++ ) {
		b2[2*i]   = b1[4*i];
		b2[2*i+1] = b1[4*i+1];
	}
}

bool DspOutOss::writeBuffer( void )
{
	if( devstate != DeviceOpened ) {
		lasterror = "Device Not Open";
		return false;
	}
	size_t count = audio_buf.getSize();
	void *buf;
	if( audio_stereo ) {
		buf = alloca(2*count);
		mono_to_stereo(audio_buf.getData(), buf, count/2);
		count *= 2;
	} else {
		buf = audio_buf.getData();
	}
	audio_buf_info info;
	if( ioctl( audio_fd, SNDCTL_DSP_GETOSPACE, &info ) == 0 ) {
		if( info.fragstotal - info.fragments > 15 ) {
			// drop the fragment if the buffer starts to fill up
			return true;
		}
	}
	for(;;) {
		if( write( audio_fd, buf, count ) != -1 ) {
			break;
		}
	}
	return true;
}

unsigned int DspOutOss::readableBytes( void )
{
	audio_buf_info info;
	struct timeval timeout;
	fd_set read_fds;

	if( devstate != DeviceOpened ) {
		return 0;
	}
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	FD_ZERO( &read_fds );
	FD_SET( audio_fd, &read_fds );
	if( select( audio_fd + 1, &read_fds, NULL, NULL, &timeout ) == -1 ) {
		return 0;
	}
	if( !FD_ISSET( audio_fd, &read_fds ) ) {
		return 0;
	}
	if( ioctl( audio_fd, SNDCTL_DSP_GETISPACE, &info ) == -1 ) {
		lasterror = strerror( errno );
		return 0;
	}

	return info.bytes;
}


bool DspOutOss::readBuffer( int bytes )
{
	if( devstate != DeviceOpened ) {
		lasterror = "Device Not Open";
		return false;
	}
	audio_buf.resize( bytes );
	size_t count = bytes;

	void *buf;
	if( audio_stereo ) {
		count *= 2;
		buf = alloca(count);
	} else {
		buf = audio_buf.getData();
	}
	ssize_t rc = read( audio_fd, buf, count );
	if( rc < 0 ) {
		lasterror = strerror(errno);
		return false;
	}
	if( (size_t)rc != count ) {
		/* should we treat rc == 0 (EOF) specially? */
		printf("warning: asked microphone for %lu bytes but got %lu\n",
			(unsigned long)count, (unsigned long)rc);
		/* assert: rc < count */
		/* Does count have to be a multiple of 2 or even 4? */
		count = rc;
		audio_buf.resize( audio_stereo? count/2 : count );
	}
	if( (size_t)rc != count ) {
		/* should we treat rc == 0 (EOF) specially? */
		printf("warning: asked microphone for %lu bytes but got %lu\n",
			(unsigned long)count, (unsigned long)rc);
		/* assert: rc < count */
		/* Does count have to be a multiple of 2 or even 4? */
		count = rc;
		audio_buf.resize( audio_stereo? count/2 : count );
	}
	if( (size_t)rc != count ) {
		/* should we treat rc == 0 (EOF) specially? */
		printf("warning: asked microphone for %lu bytes but got %lu\n",
			(unsigned long)count, (unsigned long)rc);
		/* assert: rc < count */
		/* Does count have to be a multiple of 2 or even 4? */
		count = rc;
		audio_buf.resize( audio_stereo? count/2 : count );
	}

	if( audio_stereo ) {
		stereo_to_mono(buf, audio_buf.getData(), count/4);
	}
	return true;
}

