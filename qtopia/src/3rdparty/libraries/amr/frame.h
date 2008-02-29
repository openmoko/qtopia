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
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0                
*                                REL-4 Version 4.1.0                
*
*****************************************************************************
*
*      File             : frame.h
*      Purpose          : Declaration of received and transmitted frame types
*
*****************************************************************************
*/
#ifndef frame_h
#define frame_h "$Id $"
 
/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
*/
/*
*****************************************************************************
*                         DEFINITION OF DATA TYPES
*****************************************************************************
* Note: The order of the TX and RX_Type identifiers has been chosen in
*       the way below to be compatible to an earlier version of the
*       AMR-NB C reference program.                                   
*****************************************************************************
*/

enum RXFrameType { RX_SPEECH_GOOD = 0,
                   RX_SPEECH_DEGRADED,
                   RX_ONSET,
                   RX_SPEECH_BAD,
                   RX_SID_FIRST,
                   RX_SID_UPDATE,
                   RX_SID_BAD,
                   RX_NO_DATA,
                   RX_N_FRAMETYPES     /* number of frame types */
};

enum TXFrameType { TX_SPEECH_GOOD = 0,
                   TX_SID_FIRST,
                   TX_SID_UPDATE,
                   TX_NO_DATA,
                   TX_SPEECH_DEGRADED,
                   TX_SPEECH_BAD,
                   TX_SID_BAD,
                   TX_ONSET,
                   TX_N_FRAMETYPES     /* number of frame types */
};


/* Channel decoded frame type */
enum CHDECFrameType { CHDEC_SID_FIRST = 0,
                      CHDEC_SID_FIRST_INCOMPLETE,
                      CHDEC_SID_UPDATE_INCOMPLETE,
                      CHDEC_SID_UPDATE,
                      CHDEC_SPEECH,
                      CHDEC_SPEECH_ONSET,
                      CHDEC_ESCAPE_MARKER,
                      CHDEC_ESCAPE_DATA,
                      CHDEC_NO_DATA };

/* Channel decoded frame quality */
enum CHDECFrameQuality { CHDEC_GOOD = 0,
                         CHDEC_PROBABLY_DEGRADED,
                         CHDEC_PROBABLY_BAD,
                         CHDEC_BAD };

#endif
