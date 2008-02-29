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
#ifndef _ADPCM_H_
#define _ADPCM_H_

class Adpcm {

	// Destination format - note we always decompress to 16 bit
	long		 stereo;
	int		 nBits;  // number of bits in each sample

	long		 valpred[2]; // Current state
	int		 index[2];

	long 		 nSamples; // number of samples decompressed so far

	// Parsing Info
	unsigned char 	*src;
	long		 bitBuf; // this should always contain at least 24 bits of data
	int		 bitPos;

	void FillBuffer();

	long GetBits(int n);

	long GetSBits(int n);

public:
	Adpcm(unsigned char *buffer, long isStereo);

	void Decompress(short * dst, long n); // return number of good samples
#ifdef DUMP
	void dump(BitStream *bs);
	void Compress(short *pcm, long n, int bits);
#endif
};

#endif /* _ADPCM_H_ */
