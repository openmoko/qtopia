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
/*
 * $Id: dtmfgenerator.h,v 1.1 2005/09/05 11:02:56 pankaj Exp $
 *
 * (c) 2003 iptel.org
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 */

#ifndef DTMFGENERATOR_H
#define DTMFGENERATOR_H

#include <string.h>

#define NUM_TONES 30	// was 16

/*
 * DMTF Generator Exception
 */
class DTMFException
{
private:
	const char* reason;
public:
	DTMFException(const char* _reason) throw();
	virtual ~DTMFException() throw();
	virtual const char* what() const throw();
};


/*
 * DTMF Tone Generator
 */
class DTMFGenerator
{
private:
	unsigned int samplingRate; // Sampling rate used, default is 8000 Hz
	short amplitude;           // Amplitude of the resulting signal

	struct DTMFTone {
		unsigned char code; // Code of the tone
		int lower;          // Lower frequency
		int higher;         // Higher frequency
	};

/*
 * State of the DTMF generator
 */
	struct DTMFState {
		unsigned int offset;   // Offset in the sample currently being played
		short* sample;         // Currently generated code
	};

	DTMFState state;
	static const DTMFTone tones[NUM_TONES];

	short* samples[NUM_TONES];        // Generated samples

public:
	DTMFGenerator();
	~DTMFGenerator();

/*
 * Get n samples of the signal of code code
 */
	void getSamples(short* buffer, size_t n, unsigned char code) throw (DTMFException);

/*
 * Get next n samples (continues where previous call to
 * genSample or genNextSamples stopped
 */
	void getNextSamples(short* buffer, size_t n) throw (DTMFException);

private:
	short* generateSample(unsigned char code) throw (DTMFException);

};


#endif // DTMFGENERATOR_H
