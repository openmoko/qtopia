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
/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0                
*                                REL-4 Version 4.1.0                
*
********************************************************************************
*
*      File             : autocorr.h
*      Purpose          : Compute autocorrelations of signal with windowing
*
********************************************************************************
*/
#ifndef autocorr_h
#define autocorr_h "$Id $"
 
/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
 
/*
********************************************************************************
*                         DEFINITION OF DATA TYPES
********************************************************************************
*/
 
/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/
/*
**************************************************************************
*
*  Function    : autocorr
*  Purpose     : Compute autocorrelations of signal with windowing
*  Description : - Windowing of input speech:   s'[n] = s[n] * w[n]
*                - Autocorrelations of input speech:
*                  r[k] = sum_{i=k}^{239} s'[i]*s'[i-k]    k=0,...,10
*                The autocorrelations are expressed in normalized 
*                double precision format.
*  Returns     : Autocorrelation
*
**************************************************************************
*/
Word16 Autocorr (
    Word16 x[],        /* (i)    : Input signal (L_WINDOW)             */
    Word16 m,          /* (i)    : LPC order                           */
    Word16 r_h[],      /* (o)    : Autocorrelations  (msb)  (MP1)      */
    Word16 r_l[],      /* (o)    : Autocorrelations  (lsb)  (MP1)      */
    const Word16 wind[]/* (i)    : window for LPC analysis. (L_WINDOW) */
);
 
#endif
