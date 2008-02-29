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
#include "dspaudio.h"

DspAudio::DspAudio( DspOut *in, DspOut *out )
	: input( in ), output( out )
{
	broken = false;
	cancel = false;
	copybuffer.resize( out->getBuffer().getSize() );
	newTone = 0;
	currentTone = 0;
	input->getBuffer().resize( 0 );
	dtmfSamples = 0;
#ifndef QT_THREAD_SUPPORT
	timer = 0;
#endif
}

DspAudio::~DspAudio( void )
{
	if( input ) delete input;
	if( output ) delete output;
#ifndef QT_THREAD_SUPPORT
	if( timer ) delete timer;
#endif
}

#ifdef QT_THREAD_SUPPORT
void DspAudio::run( void )
{
	for(;;) {
		if( isCanceled() ) break;
		timerTick();
		msleep( 1 );
	}
#ifdef QTHREAD_TERMINATE
	terminate();  // use this function only if absolutely necessary. (e.g. Redhat9)
#else
	QThread::exit();
#endif
}
#endif

void DspAudio::startTone(char code)
{
	newTone = code;
}

void DspAudio::stopTone(void)
{
	newTone = 0;
}

bool DspAudio::generateDTMF(short* buffer, size_t n)
{
	if (!buffer) return false;

	if (currentTone != 0) {
		// Currently generating a DTMF tone
		if (currentTone == newTone) {
			// Continue generating the same tone
			dtmf.getNextSamples(buffer, n);
			return true;
		} else if (newTone != 0) {
			// New tone requested
			dtmf.getSamples(buffer, n, newTone);
			currentTone = newTone;
			return true;
		} else {
			// Stop requested
			currentTone = newTone;
			return false;
		}
	} else {
		// Not generating any DTMF tone
		if (newTone) {
			// Requested to generate a DTMF tone
			dtmf.getSamples(buffer, n, newTone);
			currentTone = newTone;
			return true;
		}
		return false;
	}
}

void DspAudio::setCodec( const codecType newCodec, int newCodecNum )
{
	((DspOutRtp *)input)->setCodec( newCodec, newCodecNum );
}
