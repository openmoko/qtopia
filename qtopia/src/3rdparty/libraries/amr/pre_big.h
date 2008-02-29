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
*      File             : pre_big.h
*      Purpose          : Big subframe (2 subframes) preprocessing
*
********************************************************************************
*/
#ifndef pre_big_h
#define pre_big_h "$Id $"

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "mode.h"
#include "cnst.h"

/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/

int pre_big(
    enum Mode mode,            /* i  : coder mode                             */
    const Word16 gamma1[],     /* i  : spectral exp. factor 1                 */
    const Word16 gamma1_12k2[],/* i  : spectral exp. factor 1 for EFR         */
    const Word16 gamma2[],     /* i  : spectral exp. factor 2                 */
    Word16 A_t[],              /* i  : A(z) unquantized, for 4 subframes, Q12 */
    Word16 frameOffset,        /* i  : Start position in speech vector,   Q0  */
    Word16 speech[],           /* i  : speech,                            Q0  */
    Word16 mem_w[],            /* i/o: synthesis filter memory state,     Q0  */
    Word16 wsp[]               /* o  : weighted speech                    Q0  */
);

#endif
