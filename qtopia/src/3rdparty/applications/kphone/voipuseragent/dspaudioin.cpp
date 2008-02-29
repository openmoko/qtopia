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
#include <unistd.h>

#include "../config.h"
#include "audiobuffer.h"
#include "dspout.h"
#include "dspoutrtp.h"
#include "dspaudioin.h"

DspAudioIn::DspAudioIn( DspOut *in, DspOut *out )
	: DspAudio( in, out )
{
	udp_failures = 0;
}

DspAudioIn::~DspAudioIn( void )
{
}

void DspAudioIn::timerTick( void )
{
	if( broken ) {
		printf("(B)");
		fflush(stdout);
		return;
	}

	int bytes = input->readableBytes();
	
	// Fixed problem with ALSA or the OSS emulation layer
	bytes=bytes&0xfff0;
	
	if( bytes > 0 ) {

		// Read in a buffer
		input->readBuffer( bytes );

		// Make sure there's room in the output
		output->getBuffer().resize( input->getBuffer().getSize() );
		if (!generateDTMF((short*)output->getBuffer().getData(),
			input->getBuffer().getSize() / sizeof(short))) {

			// Copy the data
			memcpy( output->getBuffer().getData(),
				input->getBuffer().getData(),
				input->getBuffer().getSize() );
		}

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
	}
}

#ifndef QT_THREAD_SUPPORT
void DspAudioIn::start( void )
{
	timer = new QTimer();
	connect( timer, SIGNAL( timeout() ), this, SLOT( timeout() ) );
	timer->start( 1 );
}

void DspAudioIn::timeout()
{
	timerTick();
}

#endif
