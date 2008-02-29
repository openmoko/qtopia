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
/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /usr/local/qtopiaroot/Source/qtopia-phone-2.1.1/src/3rdparty/applications/kphone/gsm/toast_audio.cpp,v 1.1 2005/09/05 10:57:00 pankaj Exp $ */

#include	"toast.h"

/*  toast_audio -- functions to manipulate SunOS audio files.
 *
 *  This is reverse engineered from our present soundfiles
 *  and in no way portable, durable or aesthetically pleasing.
 */

extern FILE	* in, 	  * out;
extern char	* inname;
extern char	* progname;

extern int	(*output) P((gsm_signal *)),
		(*input ) P((gsm_signal *));

extern int	alaw_input   P((gsm_signal *)),
		ulaw_input   P((gsm_signal *)),
		linear_input P((gsm_signal *));

extern int	ulaw_output P((gsm_signal *));

static int put_u32 P2((f, u), FILE * f, unsigned long u)
{
	/*  Write a 32-bit unsigned value msb first. 
	 */
	if (  putc( (char)((u>>24) & 0x0FF), f) == EOF
	   || putc( (char)((u>>16) & 0x0FF), f) == EOF
	   || putc( (char)((u>> 8) & 0x0FF), f) == EOF
	   || putc( (char)( u      & 0x0FF), f) == EOF) return -1;

	return 0;
}

static int get_u32 P2((f, up), FILE * f, unsigned long * up)
{
	/*  Read a 32-bit unsigned value msb first.
	 */
	int		i;
	unsigned long 	u;

	if (  				      (i = getc(f)) == EOF
	   || ((u =        (unsigned char)i), (i = getc(f)) == EOF)
	   || ((u = (u<<8)|(unsigned char)i), (i = getc(f)) == EOF)
	   || ((u = (u<<8)|(unsigned char)i), (i = getc(f)) == EOF)) return -1;
	*up = 	    (u<<8)|(unsigned char)i;
	return 0;
}

int audio_init_input P0()
{
	unsigned long	len, enc;	/* unsigned 32 bits	*/

	if (  fgetc(in) != '.' 
	   || fgetc(in) != 's'
	   || fgetc(in) != 'n'
	   || fgetc(in) != 'd'
	   || get_u32( in, &len )
	   || get_u32( in, &enc )	/* skip this */
	   || get_u32( in, &enc )) {
		fprintf(stderr, 
	"%s: bad (missing?) header in Sun audio file \"%s\";\n\
	Try one of -u, -a, -l instead (%s -h for help).\n",
			progname, inname ? inname : "stdin", progname);
		return -1;
	}

	switch (enc) {
	case 1:	input = ulaw_input;  	break;
	case 2: input = alaw_input;  	break;
	case 3: input = linear_input; 	break;
	default:
		fprintf(stderr,
"%s: warning: file format #%lu for %s not implemented, defaulting to u-law.\n",
			progname, enc, inname);
		input = ulaw_input;
		break;
	}

	while (len > 4*4)
		if (getc(in) == EOF) {
			fprintf(stderr, 
			"%s: EOF in header of Sun audio file \"%s\";\n\
			Try one of -u, -a, -l instead (%s -h for help).\n",
				progname, inname ? inname : "stdin", progname);
			return -1;
		}
		else len--;

	return 0;
}

int audio_init_output P0()
{
	if (  fputs(".snd", out) == EOF
	   || put_u32(out, 32)
	   || put_u32(out, ~(unsigned long)0)
	   || put_u32(out, 1)
	   || put_u32(out, 8000)
	   || put_u32(out, 1)
	   || put_u32(out, 0)
	   || put_u32(out, 0)) return -1;

	return 0;
}

