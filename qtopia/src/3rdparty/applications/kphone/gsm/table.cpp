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

/* $Header: /usr/local/qtopiaroot/Source/qtopia-phone-2.1.1/src/3rdparty/applications/kphone/gsm/table.cpp,v 1.1 2005/09/05 10:57:00 pankaj Exp $ */

/*  Most of these tables are inlined at their point of use.
 */

/*  4.4 TABLES USED IN THE FIXED POINT IMPLEMENTATION OF THE RPE-LTP
 *      CODER AND DECODER
 *
 *	(Most of them inlined, so watch out.)
 */

#define	GSM_TABLE_C
#include "private.h"
#include	"gsm.h"

/*  Table 4.1  Quantization of the Log.-Area Ratios
 */
/* i 		     1      2      3        4      5      6        7       8 */
word gsm_A[8]   = {20480, 20480, 20480,  20480,  13964,  15360,   8534,  9036};
word gsm_B[8]   = {    0,     0,  2048,  -2560,     94,  -1792,   -341, -1144};
word gsm_MIC[8] = { -32,   -32,   -16,    -16,     -8,     -8,     -4,    -4 };
word gsm_MAC[8] = {  31,    31,    15,     15,      7,      7,      3,     3 };


/*  Table 4.2  Tabulation  of 1/A[1..8]
 */
word gsm_INVA[8]={ 13107, 13107,  13107, 13107,  19223, 17476,  31454, 29708 };


/*   Table 4.3a  Decision level of the LTP gain quantizer
 */
/*  bc		      0	        1	  2	     3			*/
word gsm_DLB[4] = {  6554,    16384,	26214,	   32767	};


/*   Table 4.3b   Quantization levels of the LTP gain quantizer
 */
/* bc		      0          1        2          3			*/
word gsm_QLB[4] = {  3277,    11469,	21299,	   32767	};


/*   Table 4.4	 Coefficients of the weighting filter
 */
/* i		    0      1   2    3   4      5      6     7   8   9    10  */
word gsm_H[11] = {-134, -374, 0, 2054, 5741, 8192, 5741, 2054, 0, -374, -134 };


/*   Table 4.5 	 Normalized inverse mantissa used to compute xM/xmax 
 */
/* i		 	0        1    2      3      4      5     6      7   */
word gsm_NRFAC[8] = { 29128, 26215, 23832, 21846, 20165, 18725, 17476, 16384 };


/*   Table 4.6	 Normalized direct mantissa used to compute xM/xmax
 */
/* i                  0      1       2      3      4      5      6      7   */
word gsm_FAC[8]	= { 18431, 20479, 22527, 24575, 26623, 28671, 30719, 32767 };
