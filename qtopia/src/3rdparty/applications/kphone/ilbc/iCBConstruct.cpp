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

       iCBConstruct.c

       Copyright (C) The Internet Society (2004).
       All Rights Reserved.

   ******************************************************************/

   #include <math.h>

   #include "iLBC_define.h"
   #include "gainquant.h"
   #include "getCBvec.h"

   /*----------------------------------------------------------------*
    *  Convert the codebook indexes to make the search easier
    *---------------------------------------------------------------*/






   void index_conv_enc(
       int *index          /* (i/o) Codebook indexes */
   ){
       int k;

       for (k=1; k<CB_NSTAGES; k++) {

           if ((index[k]>=108)&&(index[k]<172)) {
               index[k]-=64;
           } else if (index[k]>=236) {
               index[k]-=128;
           } else {
               /* ERROR */
           }
       }
   }

   void index_conv_dec(
       int *index          /* (i/o) Codebook indexes */
   ){
       int k;

       for (k=1; k<CB_NSTAGES; k++) {

           if ((index[k]>=44)&&(index[k]<108)) {
               index[k]+=64;
           } else if ((index[k]>=108)&&(index[k]<128)) {
               index[k]+=128;
           } else {
               /* ERROR */
           }
       }
   }

   /*----------------------------------------------------------------*
    *  Construct decoded vector from codebook and gains.
    *---------------------------------------------------------------*/

   void iCBConstruct(
       float *decvector,   /* (o) Decoded vector */
       int *index,         /* (i) Codebook indices */
       int *gain_index,/* (i) Gain quantization indices */
       float *mem,         /* (i) Buffer for codevector construction */
       int lMem,           /* (i) Length of buffer */
       int veclen,         /* (i) Length of vector */
       int nStages         /* (i) Number of codebook stages */
   ){
       int j,k;





       float gain[CB_NSTAGES];
       float cbvec[SUBL];

       /* gain de-quantization */

       gain[0] = gaindequant(gain_index[0], 1.0, 32);
       if (nStages > 1) {
           gain[1] = gaindequant(gain_index[1],
               (float)fabs(gain[0]), 16);
       }
       if (nStages > 2) {
           gain[2] = gaindequant(gain_index[2],
               (float)fabs(gain[1]), 8);
       }

       /* codebook vector construction and construction of
       total vector */

       getCBvec(cbvec, mem, index[0], lMem, veclen);
       for (j=0;j<veclen;j++){
           decvector[j] = gain[0]*cbvec[j];
       }
       if (nStages > 1) {
           for (k=1; k<nStages; k++) {
               getCBvec(cbvec, mem, index[k], lMem, veclen);
               for (j=0;j<veclen;j++) {
                   decvector[j] += gain[k]*cbvec[j];
               }
           }
       }
   }

