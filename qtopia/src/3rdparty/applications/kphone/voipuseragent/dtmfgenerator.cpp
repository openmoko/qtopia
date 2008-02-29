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
 * $Id: dtmfgenerator.cpp,v 1.1 2005/09/05 11:02:37 pankaj Exp $
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

#include <math.h>
#include "dtmfgenerator.h"


/*
 * Tone frequencies
 */
const DTMFGenerator::DTMFTone DTMFGenerator::tones[NUM_TONES] = {
	{'0', 941, 1336},
	{'1', 697, 1209},
	{'2', 697, 1336},
	{'3', 697, 1477},
	{'4', 770, 1209},
	{'5', 770, 1336},
	{'6', 770, 1477},
	{'7', 852, 1209},
	{'8', 852, 1336},
	{'9', 852, 1477},
	{'A', 697, 1633},
	{'B', 770, 1633},
	{'C', 852, 1633},
	{'D', 941, 1633},
	{'*', 941, 1209},
	{'E', 941, 1209},
	{'e', 941, 1209},
	{'#', 941, 1477},
	{'F', 941, 1477},
	{'f', 941, 1477},
	{'a', 697, 1633},
	{'b', 770, 1633},
	{'c', 852, 1633},
	{'d', 941, 1633},
	{'X', 20, 20},
	{'x', 20, 20},
	{'Y', 100, 100},
	{'y', 100, 100},
	{'Z', 500, 500},
	{'z', 500, 500},
};


DTMFException::DTMFException(const char* _reason) throw()
{
	reason = _reason;
}


DTMFException::~DTMFException() throw()
{
}


const char* DTMFException::what() const throw()
{
	return reason;
}



/*
 * Initialize the generator
 */
DTMFGenerator::DTMFGenerator()
{
	samplingRate = 8000;
	amplitude = 8192;

	state.offset = 0;
	state.sample = 0;

	for(int i = 0; i < NUM_TONES; i++) {
		samples[i] = generateSample(i);
	}
}
	

DTMFGenerator::~DTMFGenerator()
{
	for(int i = 0; i < NUM_TONES; i++) {
		delete[] samples[i];
	}
}


/*
 * Get n samples of the signal of code code
 */
void DTMFGenerator::getSamples(short* buffer, size_t n, unsigned char code) throw(DTMFException)
{
	size_t i;

	if (!buffer) {
		     //	throw DTMFException("Invalid parameter value");
	}

	switch(code) {
	case '0': state.sample = samples[0];  break;
	case '1': state.sample = samples[1];  break;
	case '2': state.sample = samples[2];  break;
	case '3': state.sample = samples[3];  break;
	case '4': state.sample = samples[4];  break;
	case '5': state.sample = samples[5];  break;
	case '6': state.sample = samples[6];  break;
	case '7': state.sample = samples[7];  break;
	case '8': state.sample = samples[8];  break;
	case '9': state.sample = samples[9];  break;
	case 'A':
	case 'a': state.sample = samples[10]; break;
	case 'B':
	case 'b': state.sample = samples[11]; break;
	case 'C':
	case 'c': state.sample = samples[12]; break;
	case 'D':
	case 'd': state.sample = samples[13]; break;
	case '*': state.sample = samples[14]; break;
	case '#': state.sample = samples[15]; break;
	default:
		//		throw DTMFException("Invalid code");
		return;
		break;
	}

	for(i = 0; i < n; i++) {
		buffer[i] = state.sample[i % samplingRate];
	}

	state.offset = i % samplingRate;
}


/*
 * Get next n samples (continues where previous call to
 * genSample or genNextSamples stopped
 */
void DTMFGenerator::getNextSamples(short* buffer, size_t n) throw(DTMFException)
{
	size_t i;

	if (!buffer) {
		//		throw DTMFException("Invalid parameter");
		return;
	}

	if (state.sample == 0) {
		//		throw DTMFException("DTMF generator not initialized");
		return;
	}

	for(i = 0; i < n; i++) {
		buffer[i] = state.sample[(state.offset + i) % samplingRate];
	}
	
	state.offset = (state.offset + i) % samplingRate;
}


/*
 * Generate a tone sample
 */
short* DTMFGenerator::generateSample(unsigned char code) throw (DTMFException)
{
	short* ptr;
	double arg1, arg2;

	//	try {
		ptr = new short[samplingRate];
		if (!ptr) {
			     //throw new DTMFException("No memory left");
			return 0;
		}

		arg1 = (double)2 * (double)M_PI * (double)tones[code].higher / (double)samplingRate; 
		arg2 = (double)2 * (double)M_PI * (double)tones[code].lower / (double)samplingRate;

		for(unsigned int i = 0; i < samplingRate; i++) {
			ptr[i] = (short)((double)(amplitude >> 2) * sin(arg1 * i) + (double)(amplitude >> 2) * sin(arg2 * i));
		};
		
		return ptr;
		//	} catch(...) {
		//		throw new DTMFException("No memory left");
		//		return 0;
		//	}

}
