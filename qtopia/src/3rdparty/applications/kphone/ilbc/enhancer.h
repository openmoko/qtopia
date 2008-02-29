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

       enhancer.h

       Copyright (C) The Internet Society (2004).
       All Rights Reserved.





   ******************************************************************/

   #ifndef __ENHANCER_H
   #define __ENHANCER_H

   #include "iLBC_define.h"

   float xCorrCoef(
       float *target,      /* (i) first array */
       float *regressor,   /* (i) second array */
       int subl        /* (i) dimension arrays */
   );

   int enhancerInterface(
       float *out,         /* (o) the enhanced recidual signal */
       float *in,          /* (i) the recidual signal to enhance */
       iLBC_Dec_Inst_t *iLBCdec_inst
                           /* (i/o) the decoder state structure */
   );

   #endif

