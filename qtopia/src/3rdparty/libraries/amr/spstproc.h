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
*      File             : subframePostProc.h
*      Purpose          : Subframe post processing
*
********************************************************************************
*/
#ifndef spstproc_h
#define spstproc_h "$Id $"

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "mode.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/

int subframePostProc(
    Word16 *speech,   /* i   : speech segment                        */
    enum Mode mode,   /* i   : coder mode                            */
    Word16 i_subfr,   /* i   : Subframe nr                           */
    Word16 gain_pit,  /* i   : Pitch gain                       Q14  */
    Word16 gain_code, /* i   : Decoded innovation gain               */
    Word16 *Aq,       /* i   : A(z) quantized for the 4 subframes    */
    Word16 synth[],   /* i   : Local snthesis                        */
    Word16 xn[],      /* i   : Target vector for pitch search        */
    Word16 code[],    /* i   : Fixed codebook exitation              */
    Word16 y1[],      /* i   : Filtered adaptive exitation           */
    Word16 y2[],      /* i   : Filtered fixed codebook excitation    */
    Word16 *mem_syn,  /* i/o : memory of synthesis filter            */
    Word16 *mem_err,  /* o   : pointer to error signal               */
    Word16 *mem_w0,   /* o   : memory of weighting filter            */
    Word16 *exc,      /* o   : long term prediction residual         */
    Word16 *sharp     /* o   : pitch sharpening value                */
);

#endif
