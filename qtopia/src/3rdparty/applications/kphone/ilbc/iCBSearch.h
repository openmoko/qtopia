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

       iCBSearch.h

       Copyright (C) The Internet Society (2004).
       All Rights Reserved.

   ******************************************************************/

   #ifndef __iLBC_ICBSEARCH_H
   #define __iLBC_ICBSEARCH_H






   void iCBSearch(
       iLBC_Enc_Inst_t *iLBCenc_inst,
                           /* (i) the encoder state structure */
       int *index,         /* (o) Codebook indices */
       int *gain_index,/* (o) Gain quantization indices */
       float *intarget,/* (i) Target vector for encoding */
       float *mem,         /* (i) Buffer for codebook construction */
       int lMem,           /* (i) Length of buffer */
       int lTarget,    /* (i) Length of vector */
       int nStages,    /* (i) Number of codebook stages */
       float *weightDenum, /* (i) weighting filter coefficients */
       float *weightState, /* (i) weighting filter state */
       int block           /* (i) the sub-block number */
   );

   #endif

