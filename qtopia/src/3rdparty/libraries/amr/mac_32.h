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
*      File             : mac_32.h
*      Purpose          : 32 x 32 and 32 x 16 bit DPF multiy & accumulate
*                         (similar as Mpy_32 and Mpy_32_16 in oper_32b.c)
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#ifndef mac_32_h
#define mac_32_h "$Id $"

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "count.h"

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/

/*****************************************************************************
 * Function Mac_32()                                                         *
 *                                                                           *
 *   Multiply two 32 bit integers (DPF) and accumulate with (normal) 32 bit  *
 *   integer. The multiplication result is divided by 2**31                  *
 *                                                                           *
 *   L_32 = L_32 + (hi1*hi2)<<1 + ( (hi1*lo2)>>15 + (lo1*hi2)>>15 )<<1       *
 *                                                                           *
 *   This operation can also be viewed as the multiplication of two Q31      *
 *   number and the result is also in Q31.                                   *
 *                                                                           *
 * Arguments:                                                                *
 *                                                                           *
 *  hi1         hi part of first number                                      *
 *  lo1         lo part of first number                                      *
 *  hi2         hi part of second number                                     *
 *  lo2         lo part of second number                                     *
 *                                                                           *
 *****************************************************************************
*/

Word32 Mac_32 (Word32 L_32, Word16 hi1, Word16 lo1, Word16 hi2, Word16 lo2);

/*****************************************************************************
 * Function Mac_32_16()                                                      *
 *                                                                           *
 *   Multiply a 16 bit integer by a 32 bit (DPF) and accumulate with (normal)*
 *   32 bit integer. The multiplication result is divided by 2**15           *
 *                                                                           *
 *                                                                           *
 *   L_32 = L_32 + (hi1*lo2)<<1 + ((lo1*lo2)>>15)<<1                         *
 *                                                                           *
 * Arguments:                                                                *
 *                                                                           *
 *  hi          hi part of 32 bit number.                                    *
 *  lo          lo part of 32 bit number.                                    *
 *  n           16 bit number.                                               *
 *                                                                           *
 *****************************************************************************
*/

Word32 Mac_32_16 (Word32 L_32, Word16 hi, Word16 lo, Word16 n);

#endif
