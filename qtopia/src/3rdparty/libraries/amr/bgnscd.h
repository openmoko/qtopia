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
*      File             : bgnscd.h
*      Purpose          : Background noise source charateristic detector (SCD)
*
********************************************************************************
*/
#ifndef bgnscd_h
#define bgnscd_h "$Id $"
 
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
#define L_ENERGYHIST 60
#define INV_L_FRAME 102


/* 2*(160*x)^2 / 65536  where x is FLP values 150,5 and 50 */
#define FRAMEENERGYLIMIT  17578         /* 150 */
#define LOWERNOISELIMIT      20         /*   5 */
#define UPPERNOISELIMIT    1953         /*  50 */

/*
********************************************************************************
*                         DEFINITION OF DATA TYPES
********************************************************************************
*/
typedef struct{
   /* history vector of past synthesis speech energy */
   Word16 frameEnergyHist[L_ENERGYHIST];
   
   /* state flags */
   Word16 bgHangover;       /* counter; number of frames after last speech frame */

} Bgn_scdState;
 
/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/
/*
**************************************************************************
*
*  Function    : Bgn_scd_init
*  Purpose     : Allocates initializes state memory
*  Description : Stores pointer to filter status struct in *st. This
*                pointer has to be passed to Bgn_scd in each call.
*  Returns     : 0 on success
*
**************************************************************************
*/
Word16 Bgn_scd_init (Bgn_scdState **st);
 
/*
**************************************************************************
*
*  Function    : Bgn_scd_reset
*  Purpose     : Resets state memory
*  Returns     : 0 on success
*
**************************************************************************
*/
Word16 Bgn_scd_reset (Bgn_scdState *st);
 
/*
**************************************************************************
*
*  Function    : Bgn_scd_exit
*  Purpose     : The memory used for state memory is freed
*  Description : Stores NULL in *s
*  Returns     : void
*
**************************************************************************
*/
void Bgn_scd_exit (Bgn_scdState **st);
 
/*
**************************************************************************
*
*  Function    : Bgn_scd
*  Purpose     : Charaterice synthesis speech and detect background noise
*  Returns     : background noise decision; 0 = bgn, 1 = no bgn
*
**************************************************************************
*/
Word16 Bgn_scd (Bgn_scdState *st,      /* i : State variables for bgn SCD         */
                Word16 ltpGainHist[],  /* i : LTP gain history                    */
                Word16 speech[],       /* o : synthesis speech frame              */
                Word16 *voicedHangover /* o : # of frames after last voiced frame */
);
 
#endif
