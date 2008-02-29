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

/* $Header: /usr/local/qtopiaroot/Source/qtopia-phone-2.1.1/src/3rdparty/applications/kphone/gsm/debug.cpp,v 1.1 2005/09/05 10:57:00 pankaj Exp $ */

#include "private.h"

#ifndef	NDEBUG

/* If NDEBUG _is_ defined and no debugging should be performed,
 * calls to functions in this module are #defined to nothing
 * in private.h.
 */

#include <stdio.h>
#include "proto.h"

void gsm_debug_words P4( (name, from, to, ptr), 
	char 	      * name,
	int		from,
	int		to,
	word		* ptr)
{
	int 	nprinted = 0;

	fprintf( stderr, "%s [%d .. %d]: ", name, from, to );
	while (from <= to) {
		fprintf(stderr, "%d ", ptr[ from ] );
		from++;
		if (nprinted++ >= 7) {
			nprinted = 0;
			if (from < to) putc('\n', stderr);
		}
	}
	putc('\n', stderr);
}

void gsm_debug_longwords P4( (name, from, to, ptr),
	char 	      * name,
	int		from,
	int		to,
	longword      * ptr)
{
	int 	nprinted = 0;

	fprintf( stderr, "%s [%d .. %d]: ", name, from, to );
	while (from <= to) {

		fprintf(stderr, "%ld ", ptr[ from ] );
		from++;
		if (nprinted++ >= 7) {
			nprinted = 0;
			if (from < to) putc('\n', stderr);
		}
	}
	putc('\n', stderr);
}

void gsm_debug_longword P2(  (name, value),
	char		* name,
	longword	  value	)
{
	fprintf(stderr, "%s: %ld\n", name, (long)value );
}

void gsm_debug_word P2(  (name, value),
	char	* name,
	word	  value	)
{
	fprintf(stderr, "%s: %ld\n", name, (long)value);
}

#endif
