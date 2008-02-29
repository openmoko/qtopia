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
*      File             : convolve.h
*      Purpose          : Perform the convolution between two vectors x[]
*                       : and h[] and write the result in the vector y[].
*                       : All vectors are of length L and only the first
*                       : L samples of the convolution are computed.
*
********************************************************************************
*/
#ifndef convolve_h
#define convolve_h "$Id $"
 
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
void Convolve (
    Word16 x[],        /* (i)  : input vector                               */
    Word16 h[],        /* (i)  : impulse response                           */
    Word16 y[],        /* (o)  : output vector                              */
    Word16 L           /* (i)  : vector size                                */
);
 
#endif
