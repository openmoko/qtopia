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
#ifndef RTPDATAHEADER_H
#define RTPDATAHEADER_H

// For 32bit intel machines
typedef short int16;
typedef int   int32;
typedef unsigned int u_int32;
typedef unsigned short u_int16;

#define RTP_BIG_ENDIAN 0
#define RTP_LITTLE_ENDIAN 1

//  RTP data header
typedef __attribute__ ((__packed__)) struct {
#if RTP_BIG_ENDIAN
	unsigned int version:2;         // protocol version
	unsigned int p:1;               // padding flag
	unsigned int x:1;               // header extension flag
	unsigned int cc:4;              // CSRC count
	unsigned int m:1;               // marker bit
	unsigned int pt:7;              // payload type
#elif RTP_LITTLE_ENDIAN
	unsigned int cc:4;              // CSRC count
	unsigned int x:1;               // header extension flag
	unsigned int p:1;               // padding flag
	unsigned int version:2;         // protocol version
	unsigned int pt:7;              // payload type
	unsigned int m:1;               // marker bit
#else
#error Define one of RTP_LITTLE_ENDIAN or RTP_BIG_ENDIAN
#endif
	unsigned int seq:16;            // sequence number
	u_int32 ts;                     // timestamp 32bits
	u_int32 ssrc;                   // synchronization source
} rtp_hdr_t;

#endif
