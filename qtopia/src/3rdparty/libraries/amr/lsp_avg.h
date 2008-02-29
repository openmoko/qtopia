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
*      File             : lsp_avg.h
*      Purpose:         : LSP averaging and history
*                         The LSPs are averaged over 8 frames
*
********************************************************************************
*/
#ifndef lsp_avg_h
#define lsp_avg_h "$Id $"
 
/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "cnst.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/

#define EXPCONST          5243               /* 0.16 in Q15 */
 
/*
********************************************************************************
*                         DEFINITION OF DATA TYPES
********************************************************************************
*/
typedef struct {
  Word16 lsp_meanSave[M];          /* Averaged LSPs saved for efficiency  */
} lsp_avgState;
 
/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/

/*
**************************************************************************
*
*  Function    : lsp_avg_init
*  Purpose     : Allocates memory and initializes state variables
*
**************************************************************************
*/
int lsp_avg_init (
    lsp_avgState **state
);
 
/*
**************************************************************************
*
*  Function    : lsp_history_reset
*  Purpose     : Resets state memory
*
**************************************************************************
*/
int lsp_avg_reset (
    lsp_avgState *state
);
 
/*
**************************************************************************
*
*  Function    : lsp_avg_exit
*  Purpose     : The memory used for state memory is freed
*
**************************************************************************
*/
void lsp_avg_exit (
    lsp_avgState **state
);

/*
**************************************************************************
*
*  Function    : lsp_avg
*  Purpose     : Calculate the LSP averages
*
**************************************************************************
*/
void lsp_avg (
    lsp_avgState *st,     /* i/o : State struct                 Q15 */
    Word16 *lsp           /* i   : LSP vector                   Q15 */
);
#endif
