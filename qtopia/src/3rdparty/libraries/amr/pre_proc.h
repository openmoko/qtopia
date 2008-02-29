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
*      File             : pre_proc.h
*      Purpose          : Preprocessing of input speech.
*
********************************************************************************
*/
#ifndef pre_proc_h
#define pre_proc_h "$Id $"
 
/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
 
/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/
 
/*
********************************************************************************
*                         DEFINITION OF DATA TYPES
********************************************************************************
*/
typedef struct {
  Word16 y2_hi;
  Word16 y2_lo;
  Word16 y1_hi;
  Word16 y1_lo;
  Word16 x0;
  Word16 x1;
} Pre_ProcessState;
 
/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/
 
int Pre_Process_init (Pre_ProcessState **st);
/* initialize one instance of the pre processing state.
   Stores pointer to filter status struct in *st. This pointer has to
   be passed to Pre_Process in each call.
   returns 0 on success
 */
 
int Pre_Process_reset (Pre_ProcessState *st);
/* reset of pre processing state (i.e. set state memory to zero)
   returns 0 on success
 */
void Pre_Process_exit (Pre_ProcessState **st);
/* de-initialize pre processing state (i.e. free status struct)
   stores NULL in *st
 */
 
int Pre_Process (
    Pre_ProcessState *st,
    Word16 signal[],   /* Input/output signal                               */
    Word16 lg          /* Lenght of signal                                  */
);

#endif
