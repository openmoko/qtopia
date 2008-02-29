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

/* $Header: /usr/local/qtopiaroot/Source/qtopia-phone-2.1.1/src/3rdparty/applications/kphone/gsm/gsm_option.cpp,v 1.1 2005/09/05 10:57:00 pankaj Exp $ */

#include "private.h"

#include "gsm.h"
#include "proto.h"

int gsm_option P3((r, opt, val), gsm r, int opt, int * val)
{
	int 	result = -1;

	switch (opt) {
	case GSM_OPT_LTP_CUT:
#ifdef 	LTP_CUT
		result = r->ltp_cut;
		if (val) r->ltp_cut = *val;
#endif
		break;

	case GSM_OPT_VERBOSE:
#ifndef	NDEBUG
		result = r->verbose;
		if (val) r->verbose = *val;
#endif
		break;

	case GSM_OPT_FAST:

#if	defined(FAST) && defined(USE_FLOAT_MUL)
		result = r->fast;
		if (val) r->fast = !!*val;
#endif
		break;

	case GSM_OPT_FRAME_CHAIN:

#ifdef WAV49
		result = r->frame_chain;
		if (val) r->frame_chain = *val;
#endif
		break;

	case GSM_OPT_FRAME_INDEX:

#ifdef WAV49
		result = r->frame_index;
		if (val) r->frame_index = *val;
#endif
		break;

	case GSM_OPT_WAV49:

#ifdef WAV49 
		result = r->wav_fmt;
		if (val) r->wav_fmt = !!*val;
#endif
		break;

	default:
		break;
	}
	return result;
}
