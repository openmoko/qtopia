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

       gainquant.h

       Copyright (C) The Internet Society (2004).
       All Rights Reserved.

   ******************************************************************/

   #ifndef __iLBC_GAINQUANT_H
   #define __iLBC_GAINQUANT_H

   float gainquant(/* (o) quantized gain value */
       float in,       /* (i) gain value */
       float maxIn,/* (i) maximum of gain value */
       int cblen,      /* (i) number of quantization indices */
       int *index      /* (o) quantization index */
   );

   float gaindequant(  /* (o) quantized gain value */
       int index,      /* (i) quantization index */
       float maxIn,/* (i) maximum of unquantized gain */
       int cblen       /* (i) number of quantization indices */
   );

   #endif

