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

   /******************************************************************

       iLBC Speech Coder ANSI-C Source Code

       LPC_decode.h

       Copyright (C) The Internet Society (2004).
       All Rights Reserved.

   ******************************************************************/

   #ifndef __iLBC_LPC_DECODE_H
   #define __iLBC_LPC_DECODE_H

   void LSFinterpolate2a_dec(
       float *a,           /* (o) lpc coefficients for a sub-frame */
       float *lsf1,    /* (i) first lsf coefficient vector */
       float *lsf2,    /* (i) second lsf coefficient vector */
       float coef,         /* (i) interpolation weight */
       int length          /* (i) length of lsf vectors */
   );

   void SimplelsfDEQ(
       float *lsfdeq,      /* (o) dequantized lsf coefficients */
       int *index,         /* (i) quantization index */
       int lpc_n           /* (i) number of LPCs */
   );

   void DecoderInterpolateLSF(
       float *syntdenum,   /* (o) synthesis filter coefficients */
       float *weightdenum, /* (o) weighting denumerator
                                  coefficients */
       float *lsfdeq,      /* (i) dequantized lsf coefficients */
       int length,         /* (i) length of lsf coefficient vector */
       iLBC_Dec_Inst_t *iLBCdec_inst
                           /* (i) the decoder state structure */
   );

   #endif












