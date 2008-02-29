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
/**********************************************************************
** This file is part of the KPhone project.
** 
** KPhone is a SIP (Session Initiation Protocol) user agent for Linux, with
** which you can initiate VoIP (Voice over IP) connections over the Internet,
** send Instant Messages and subscribe your friends' presence information.
** Read INSTALL for installation instructions, and CHANGES for the latest
** additions in functionality and COPYING for the General Public License
** (GPL).
** 
** More information about Wirlab available at http://www.wirlab.net/
** 
** Note: "This program is released under the GPL with the additional
** exemption that compiling, linking, and/or using OpenSSL is allowed."
***********************************************************************/
/*
 * codec.h
 *
 * RFC 1890 enums
 * 
 */

#ifndef _CODEC_H_
#define _CODEC_H_

enum codecNum {
        PCMU			=0 , 
        _1016			=1 , 
        G721			=2 , 
        GSM				=3 , 
        unassigned_4	=4 , 
        DVI4_8			=5 , 
        DVI4_16			=6 , 
        LPC				=7 , 
        PCMA			=8 , 
        G722			=9 , 
        L16_2c			=10, 
        L16_1c			=11, 
        unassigned_12	=12, 
        unassigned_13	=13, 
        MPA				=14, 
        G728			=15, 
        unassigned_16_24=16,
        CelB			=25, 
        JPEG			=26, 
        unassigned_27	=27, 
        nv				=28, 
        unassigned_29	=29, 
        unassigned_30	=30, 
        H261			=31, 
        MPV				=32, 
        MP2T			=33, 
        unassigned_34_71=34, 
        reserved_72_76	=72, 
        unassigned_77_95=77,
        dynamic_96_127	=96 
};




#endif // _CODEC_H_
