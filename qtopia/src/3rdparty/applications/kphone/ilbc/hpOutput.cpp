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

       hpOutput.c

       Copyright (C) The Internet Society (2004).
       All Rights Reserved.

   ******************************************************************/

   #include "constants.h"

   /*----------------------------------------------------------------*
    *  Output high-pass filter
    *---------------------------------------------------------------*/

   void hpOutput(





       float *In,  /* (i) vector to filter */
       int len,/* (i) length of vector to filter */
       float *Out, /* (o) the resulting filtered vector */
       float *mem  /* (i/o) the filter state */
   ){
       int i;
       float *pi, *po;

       /* all-zero section*/

       pi = &In[0];
       po = &Out[0];
       for (i=0; i<len; i++) {
           *po = hpo_zero_coefsTbl[0] * (*pi);
           *po += hpo_zero_coefsTbl[1] * mem[0];
           *po += hpo_zero_coefsTbl[2] * mem[1];

           mem[1] = mem[0];
           mem[0] = *pi;
           po++;
           pi++;

       }

       /* all-pole section*/

       po = &Out[0];
       for (i=0; i<len; i++) {
           *po -= hpo_pole_coefsTbl[1] * mem[2];
           *po -= hpo_pole_coefsTbl[2] * mem[3];

           mem[3] = mem[2];
           mem[2] = *po;
           po++;
       }
   }

