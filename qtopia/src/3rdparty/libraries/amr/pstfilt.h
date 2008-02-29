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
*      File             : pstfilt.h
*      Purpose          : Performs adaptive postfiltering on the synthesis
*                       : speech
*
********************************************************************************
*/
#ifndef pstfilt_h
#define pstfilt_h "$Id $"
 
/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "mode.h"
#include "cnst.h"
#include "preemph.h"
#include "agc.h"
/*
********************************************************************************
*                         DEFINITION OF DATA TYPES
********************************************************************************
*/
typedef struct{
  Word16 res2[L_SUBFR];
  Word16 mem_syn_pst[M];
  preemphasisState* preemph_state;
  agcState* agc_state;
  Word16 synth_buf[M + L_FRAME];  
} Post_FilterState;

/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/
 
int Post_Filter_init (Post_FilterState **st);
/* initialize one instance of the post filter
   Stores pointer to filter status struct in *st. This pointer has to
   be passed to Post_Filter in each call.
   returns 0 on success
 */
 
int Post_Filter_reset (Post_FilterState *st);
/* reset post filter (i.e. set state memory to zero)
   returns 0 on success
 */

void Post_Filter_exit (Post_FilterState **st);
/* de-initialize post filter (i.e. free status struct)
   stores NULL in *s
 */

int Post_Filter (
    Post_FilterState *st, /* i/o : post filter states                        */
    enum Mode mode,       /* i   : AMR mode                                  */
    Word16 *syn,          /* i/o : synthesis speech (postfiltered is output) */
    Word16 *Az_4          /* i   : interpolated LPC parameters in all subfr. */
);
/* filters the signal syn using the parameters in Az_4 to calculate filter
   coefficients.
   The filter must be set up using Post_Filter_init prior to the first call
   to Post_Filter. Post_FilterState is updated to mirror the current state
   of the filter
 
   return 0 on success
 */
 
#endif
