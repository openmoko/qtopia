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
#include <assert.h>
#include <unistd.h>
#include "../config.h"
#include "audiobuffer.h"
#include "dspout.h"
#include "dspoutrtp.h"
#include "dspaudioout.h"

DspAudioOut::DspAudioOut( DspOut *in, DspOut *out )
	: DspAudio( in, out )
{
	curpos = 0;
}

DspAudioOut::~DspAudioOut( void )
{
}

void DspAudioOut::timerTick( void )
{
	if( broken ){
		printf("(b)");
		fflush(stdout);
		return;
	}
	short *curbuf;
	unsigned int outputsize = output->getBuffer().getSize() / sizeof( short );

	if ( !broken ) {
		int numBytes;
		if( input->getDeviceState() == DspOut::DeviceClosed ) {
			return;
		}
		numBytes = input->readableBytes();
		unsigned int samplesleft = input->getBuffer().getSize() / sizeof( short );
		short *inbuf = (short *)input->getBuffer().getData();
		if (numBytes > 0) {
			input->readBuffer( numBytes );
		}
		if (!numBytes) {
			if (!currentTone && !newTone) {
				return;
			}
		} else if (dtmfSamples > 0) { // This is complete bullshit. dtmfSamples can never have a value other than 0
			if (dtmfSamples - samplesleft > 0) {
				dtmfSamples -= 80;
				return;
			} else {
				inbuf += dtmfSamples;
				samplesleft -= dtmfSamples;
				dtmfSamples = 0;
			}
		}
//		printf( "[%s->%s] read a buffer, size %d, samples left %d\n",
//			input->getDeviceName().ascii(),
//			output->getDeviceName().ascii(),
//			input->getBuffer().getSize(),
//			samplesleft );

		while( samplesleft > 0 ) {
			if( isCanceled() ) break;
			if( samplesleft >= ( outputsize - curpos ) ) {
				curbuf = (short *) copybuffer.getData();
				curbuf += curpos;
				if (!generateDTMF(curbuf, outputsize - curpos)) {
					memcpy( curbuf, inbuf, ( outputsize - curpos ) * sizeof( short ) );
				}
				memcpy( output->getBuffer().getData(), copybuffer.getData(),
					output->getBuffer().getSize() );


//				printf( "[%s->%s] writing buffer, output size %d, copy buffer size %d, cwriteBufferurpos was %d\n",
//					input->getDeviceName().ascii(),
//					output->getDeviceName().ascii(),
//					output->getBuffer().getSize(),
//					copybuffer.getSize(),
//					curpos );

				// If the write fails, mark the pipe as broken and stop sending!
				if( !output->writeBuffer() ) {
					printf( QObject::tr("DspAudio: Broken pipe") + "\n" );
					broken = true;
				}
				samplesleft -= ( outputsize - curpos );
				inbuf += ( outputsize - curpos );
				curpos = 0;

			} else {

//				printf( "[%s->%s] filling in %d samples\n",
//					input->getDeviceName().ascii(),
//					output->getDeviceName().ascii(),
//					samplesleft );

				curbuf = (short *) copybuffer.getData();
				curbuf += curpos;
				if (!generateDTMF(curbuf, samplesleft)) {
					memcpy( curbuf, inbuf, samplesleft * sizeof( short ) );
				}
				curpos += samplesleft;
				samplesleft = 0;
			}
		}
	}
}


#ifndef QT_THREAD_SUPPORT
void DspAudioOut::start( void )
{
	timer = new QTimer();
	connect( timer, SIGNAL( timeout() ), this, SLOT( timeout() ) );
	timer->start( 1 );
}

void DspAudioOut::timeout()
{
	timerTick();
}

#endif

