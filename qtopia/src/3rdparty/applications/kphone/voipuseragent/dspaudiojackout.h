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
#ifndef CALLAUDIOJACKOUT_H_INCLUDED
#define CALLAUDIOJACKOUT_H_INCLUDED

#include <jack/jack.h>


class DspAudioJackOut
{
public:
	DspAudioJackOut( DspOutRtp *inrtp, const QString &client = "kphone" );

	/**
	 * Destructor.  Will close the device if it is open.
	 */
	virtual ~DspAudioJackOut( void );

	jack_port_t *input_port;
	jack_port_t *output_port;
	void setCodec( const codecType newCodec, int newCodecNum );
	void start( void );
	void exit( void );
	bool ok( void ) { return jack_client != 0; }

private:
	void mono_to_stereo(void *b_from, void *b_to, size_t count);
	void stereo_to_mono(void *b_from, void *b_to, size_t count);
	bool audio_stereo;
//	AudioBuffer output_buf;   // Output buffer
	
//	DspOut *input;

/* JACK data */
	jack_client_t *jack_client;
	QString jack_clientName;
//	jack_thread_info_t thread_info;

};

#endif  // DSPOUTJACK_H_INCLUDED
