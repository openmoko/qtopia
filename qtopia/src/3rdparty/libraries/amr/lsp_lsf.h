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
*      File             : lsp_lsf.h
*      Purpose          : Lsp_lsf:   Transformation lsp to lsf
*                       : Lsf_lsp:   Transformation lsf to lsp
*
********************************************************************************
*/
#ifndef lsp_lsf_h
#define lsp_lsf_h "$Id $"
 
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
void Lsf_lsp (
    Word16 lsf[],      /* (i)    : lsf[m] normalized (range: 0.0<=val<=0.5) */
    Word16 lsp[],      /* (o)    : lsp[m] (range: -1<=val<1)                */
    Word16 m           /* (i)    : LPC order                                */
);
void Lsp_lsf (
    Word16 lsp[],      /* (i)    : lsp[m] (range: -1<=val<1)                */
    Word16 lsf[],      /* (o)    : lsf[m] normalized (range: 0.0<=val<=0.5) */
    Word16 m           /* (i)    : LPC order                                */
);

#endif
