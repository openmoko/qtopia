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
*      File             : preemph.h
*      Purpose          : Preemphasis filtering
*      Description      : Filtering through 1 - g z^-1 
*
********************************************************************************
*/
#ifndef preemph_h
#define preemph_h "$Id $"
 
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
typedef struct {
  Word16 mem_pre;          /* filter state */
} preemphasisState;
 
/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/

int preemphasis_init (preemphasisState **st);
/* initialize one instance of the preemphasis filter
   Stores pointer to filter status struct in *st. This pointer has to
   be passed to preemphasis in each call.
   returns 0 on success
 */

int preemphasis_reset (preemphasisState *st);
/* reset of preemphasis filter (i.e. set state memory to zero)
   returns 0 on success
 */
void preemphasis_exit (preemphasisState **st);
/* de-initialize preemphasis filter (i.e. free status struct)
   stores NULL in *st
 */
 
int preemphasis (
    preemphasisState *st, /* (i/o): preemphasis filter state                  */
    Word16 *signal,    /* (i/o): input signal overwritten by the output     */
    Word16 g,          /* (i)  : preemphasis coefficient                    */
    Word16 L           /* (i)  : size of filtering                          */
);
 
#endif
