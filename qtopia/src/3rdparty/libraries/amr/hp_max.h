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
*      File             : hp_max.h
*      Purpose          : Find the maximum correlation of scal_sig[] in a given
*                         delay range.
*
********************************************************************************
*/
#ifndef hp_max_h
#define hp_max_h "$Id $"
 
/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"

/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/
/*************************************************************************
 *
 *  FUNCTION:  hp_max
 *
 *  PURPOSE: Find the maximum high-pass filtered correlation of
 *           signal scal_sig[] in a given delay range.
 *
 *  DESCRIPTION:
 *      The correlation is given by
 *           corr[t] = <scal_sig[n],scal_sig[n-t]>,  t=lag_min,...,lag_max
 *      The functions outputs the maximum high-pass filtered correlation
 *      after normalization.
 *
 *************************************************************************/
Word16 hp_max (   
    Word32 corr[],      /* i   : correlation vector.                      */
    Word16 scal_sig[],  /* i   : scaled signal.                           */
    Word16 L_frame,     /* i   : length of frame to compute pitch         */
    Word16 lag_max,     /* i   : maximum lag                              */
    Word16 lag_min,     /* i   : minimum lag                              */
    Word16 *cor_hp_max  /* o   : max high-pass filtered norm. correlation */
    );
#endif
