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
#ifndef DSPOUTRTP_H_INCLUDED
#define DSPOUTRTP_H_INCLUDED

#include <qtimer.h>

#include "../gsm/gsm.h"
#include "../gsm/private.h"
#include "../ilbc/iLBC_define.h"
#include "../ilbc/iLBC_encode.h"
#include "../ilbc/iLBC_decode.h"
#include "../dissipate2/udpmessagesocket.h"
#include "dspout.h"
#include "callaudio.h"

#define GSM_ENC_BYTES		33
#define GSM_DEC_SAMPLES		160

#define ILBCNOOFWORDS   (NO_OF_BYTES/2)
#define TIME_PER_FRAME  30

/**
 * @short RTP implementation of DspOut.
 * @author Billy Biggs <vektor@div8.net>
 *
 * This is the RTP implementation of DspOut.
 */
class DspOutRtp : public DspOut
{
public:
	/**
	 * Constructs a DspOutRtp object outputting to the given
	 * hostname.
	 */
	DspOutRtp( const codecType newCodec, int newCodecNum,
		const QString &hostName = QString::null, UDPMessageSocket *s = NULL );

	/**
	 * Destructor.  Will close the device if it is open.
	 */
	virtual ~DspOutRtp( void );

	bool openDevice( DeviceMode mode );
	bool writeBuffer( void );
	bool setPortNum( int newport );
	int getPortNum( void ) const { return portnum; }
	int getVideoPortNum( void ) const { return videoPortnum; }
	unsigned int readableBytes( void );
	bool readBuffer( int bytes = 0 );
	void setPayload( int payload ) { fixedrtplen = (size_t)payload; }
	bool sendStunRequest( UDPMessageSocket *socket );
	unsigned int receiveStunResponse( UDPMessageSocket *socket );
	void setStunSrv( QString newStunSrv );
	void setCodec( const codecType newCodec, int newCodecNum );

private:
	int writeGSMBuffer( gsm Gsm_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		unsigned char *tmp_buf, unsigned char *queue,
		int *qlen, int size );
	int readGSMBuffer( gsm Gsm_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		int ignore );
	int writeILBCBuffer_20( iLBC_Enc_Inst_t *Enc_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		unsigned char *tmp_buf, unsigned char *queue,
		int *qlen, int size );
	int readILBCBuffer_20( iLBC_Dec_Inst_t *Dec_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		short mode, int ignore);
	int writeILBCBuffer_30( iLBC_Enc_Inst_t *Enc_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		unsigned char *tmp_buf, unsigned char *queue,
		int *qlen, int size );
	int readILBCBuffer_30( iLBC_Dec_Inst_t *Dec_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		short mode, int ignore);

	UDPMessageSocket *socket;      // UDP Socket
	int portnum;
	int videoPortnum;
	int output_fd;                // The fd of the audio output
	unsigned char *packetbuf;     // Buffer for the packet data
	double lasttime;
	short curseq;
	unsigned char *bufunsend;

	unsigned char *inbuf;
	unsigned char *outbuf;
	unsigned char *tmpbuf;
	unsigned char *quebuf;

//codec
	codecType codec;
	int codecNum;
	gsm gsmInstEnc;
	gsm gsmInstDec;
	int qlen;
	iLBC_Enc_Inst_t ilbcEncInst_20;
	iLBC_Dec_Inst_t ilbcDecInst_20;
	iLBC_Enc_Inst_t ilbcEncInst_30;
	iLBC_Dec_Inst_t ilbcDecInst_30;

	size_t numunsend;
	size_t fixedrtplen;
	unsigned long deb_frag;
	unsigned long deb_rtp;
	int ts;
	int ssrc;
	int ref_sec;
	int ref_usec;
	int dsize;
	bool useStun;
	QString stunSrv;
	bool destroySocket;
};

#endif  // DSPOUTRTP_H_INCLUDED
