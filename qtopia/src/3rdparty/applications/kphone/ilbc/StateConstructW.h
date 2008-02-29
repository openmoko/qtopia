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

       StateConstructW.h






       Copyright (C) The Internet Society (2004).
       All Rights Reserved.

   ******************************************************************/

   #ifndef __iLBC_STATECONSTRUCTW_H
   #define __iLBC_STATECONSTRUCTW_H

   void StateConstructW(
       int idxForMax,      /* (i) 6-bit index for the quantization of
                                  max amplitude */
       int *idxVec,    /* (i) vector of quantization indexes */
       float *syntDenum,   /* (i) synthesis filter denumerator */
       float *out,         /* (o) the decoded state vector */
       int len             /* (i) length of a state vector */
   );

   #endif

