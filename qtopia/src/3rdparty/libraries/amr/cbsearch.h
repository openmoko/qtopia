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
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0                
*                                REL-4 Version 4.1.0                
*
*****************************************************************************
*
*      File             : cbsearch.h
*      Purpose          : Inovative codebook search (find index and gain)
*
*****************************************************************************
*/
#ifndef cbsearch_h
#define cbsearch_h "$Id $"
 
/*
*****************************************************************************
*                         INCLUDE FILES                               
*****************************************************************************
*/                                                                    
#include "typedef.h"                                                  
#include "mode.h"                                                     
                                                                      
/*                                                                    
*****************************************************************************
*                         LOCAL VARIABLES AND TABLES                  
*****************************************************************************
*/                                                                    
                                                                      
/*                                                                    
*****************************************************************************
*                         DEFINITION OF DATA TYPES                    
*****************************************************************************
*/
 
/*
*****************************************************************************
*                         PUBLIC PROGRAM CODE
*****************************************************************************
*/
int cbsearch(Word16 x[],     /* i : target vector, Q0                      */
             Word16 h[],     /* i : impulse response of weighted synthesis */
                             /*     filter h[-L_subfr..-1] must be set to  */
                             /*    zero. Q12                               */
             Word16 T0,      /* i : Pitch lag                              */
             Word16 pitch_sharp, /* i : Last quantized pitch gain, Q14     */
             Word16 gain_pit,/* i : Pitch gain, Q14                        */ 
             Word16 res2[],  /* i : Long term prediction residual, Q0      */
             Word16 code[],  /* o : Innovative codebook, Q13               */
             Word16 y[],     /* o : filtered fixed codebook excitation, Q12 */
             Word16 **anap,  /* o : Signs of the pulses                    */
             enum Mode mode, /* i : coder mode                             */
             Word16 subNr)   /* i : subframe number                        */
;

#endif
