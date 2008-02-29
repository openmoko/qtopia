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

       syntFilter.c

       Copyright (C) The Internet Society (2004).
       All Rights Reserved.

   ******************************************************************/

   #include "iLBC_define.h"

   /*----------------------------------------------------------------*
    *  LP synthesis filter.
    *---------------------------------------------------------------*/

   void syntFilter(
       float *Out,     /* (i/o) Signal to be filtered */
       float *a,       /* (i) LP parameters */
       int len,    /* (i) Length of signal */





       float *mem      /* (i/o) Filter state */
   ){
       int i, j;
       float *po, *pi, *pa, *pm;

       po=Out;

       /* Filter first part using memory from past */

       for (i=0; i<LPC_FILTERORDER; i++) {
           pi=&Out[i-1];
           pa=&a[1];
           pm=&mem[LPC_FILTERORDER-1];
           for (j=1; j<=i; j++) {
               *po-=(*pa++)*(*pi--);
           }
           for (j=i+1; j<LPC_FILTERORDER+1; j++) {
               *po-=(*pa++)*(*pm--);
           }
           po++;
       }

       /* Filter last part where the state is entirely in
          the output vector */

       for (i=LPC_FILTERORDER; i<len; i++) {
           pi=&Out[i-1];
           pa=&a[1];
           for (j=1; j<LPC_FILTERORDER+1; j++) {
               *po-=(*pa++)*(*pi--);
           }
           po++;
       }

       /* Update state vector */

       memcpy(mem, &Out[len-LPC_FILTERORDER],
           LPC_FILTERORDER*sizeof(float));
   }
