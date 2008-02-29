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

       iCBConstruct.h

       Copyright (C) The Internet Society (2004).
       All Rights Reserved.






   ******************************************************************/

   #ifndef __iLBC_ICBCONSTRUCT_H
   #define __iLBC_ICBCONSTRUCT_H

   void index_conv_enc(
       int *index          /* (i/o) Codebook indexes */
   );

   void index_conv_dec(
       int *index          /* (i/o) Codebook indexes */
   );

   void iCBConstruct(
       float *decvector,   /* (o) Decoded vector */
       int *index,         /* (i) Codebook indices */
       int *gain_index,/* (i) Gain quantization indices */
       float *mem,         /* (i) Buffer for codevector construction */
       int lMem,           /* (i) Length of buffer */
       int veclen,         /* (i) Length of vector */
       int nStages         /* (i) Number of codebook stages */
   );

   #endif

