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

       StateSearchW.h

       Copyright (C) The Internet Society (2004).
       All Rights Reserved.

   ******************************************************************/

   #ifndef __iLBC_STATESEARCHW_H
   #define __iLBC_STATESEARCHW_H

   void AbsQuantW(
       iLBC_Enc_Inst_t *iLBCenc_inst,
                           /* (i) Encoder instance */
       float *in,          /* (i) vector to encode */
       float *syntDenum,   /* (i) denominator of synthesis filter */
       float *weightDenum, /* (i) denominator of weighting filter */
       int *out,           /* (o) vector of quantizer indexes */
       int len,        /* (i) length of vector to encode and
                                  vector of quantizer indexes */
       int state_first     /* (i) position of start state in the
                                  80 vec */
   );

   void StateSearchW(
       iLBC_Enc_Inst_t *iLBCenc_inst,
                           /* (i) Encoder instance */
       float *residual,/* (i) target residual vector */
       float *syntDenum,   /* (i) lpc synthesis filter */
       float *weightDenum, /* (i) weighting filter denuminator */
       int *idxForMax,     /* (o) quantizer index for maximum
                                  amplitude */
       int *idxVec,    /* (o) vector of quantization indexes */
       int len,        /* (i) length of all vectors */
       int state_first     /* (i) position of start state in the
                                  80 vec */
   );


   #endif








