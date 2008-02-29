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
*      File             : d_gain_p.h
*      Purpose          : Decodes the pitch gain using the received index.
*
********************************************************************************
*/
#ifndef d_gain_p_h
#define d_gain_p_h "$Id $"
 
/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "mode.h"
 
/*
**************************************************************************
*
*  Function    : d_gain_pitch
*  Purpose     : Decodes the pitch gain using the received index.
*  Description : In case of no frame erasure, the gain is obtained 
*                from the quantization table at the given index; 
*                otherwise, a downscaled past gain is used.
*  Returns     : Quantized pitch gain
*
**************************************************************************
*/
Word16 d_gain_pitch (      /* return value: gain (Q14)                */
    enum Mode mode,        /* i : AMR mode                            */
    Word16 index           /* i   : index of quantization             */
);
 
#endif
