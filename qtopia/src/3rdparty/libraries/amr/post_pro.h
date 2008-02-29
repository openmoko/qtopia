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
*      File             : post_pro.h
*      Purpose          : Postprocessing of output speech.
*
*                         - 2nd order high pass filtering with cut
*                           off frequency at 60 Hz.
*                         - Multiplication of output by two.
*
********************************************************************************
*/
#ifndef post_pro_h
#define post_pro_h "$Id $"
 
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
} Post_ProcessState;
 
/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/
 
int Post_Process_init (Post_ProcessState **st);
/* initialize one instance of the Post processing state.
   Stores pointer to filter status struct in *st. This pointer has to
   be passed to Post_Process in each call.
   returns 0 on success
 */
 
int Post_Process_reset (Post_ProcessState *st);
/* reset of Post processing state (i.e. set state memory to zero)
   returns 0 on success
 */
void Post_Process_exit (Post_ProcessState **st);
/* de-initialize Post processing state (i.e. free status struct)
   stores NULL in *st
 */
 
int Post_Process (
    Post_ProcessState *st,  /* i/o : post process state                   */
    Word16 signal[],        /* i/o : signal                               */
    Word16 lg               /* i   : lenght of signal                     */
    );

#endif
