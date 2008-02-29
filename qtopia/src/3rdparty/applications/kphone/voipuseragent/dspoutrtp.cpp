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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <g711.h>
#include <qsettings.h>
#include <qtimer.h>

#include "../dissipate2/sipuri.h"
#include "../dissipate2/sipclient.h"
#include "audiobuffer.h"
#include "rtpdataheader.h"
#include "dspoutrtp.h"
#include "kphone.h"


short ILBCencode_20( iLBC_Enc_Inst_t *iLBCenc_inst,short *encoded_data, short *data);
short ILBCdecode_20( iLBC_Dec_Inst_t *iLBCdec_inst,short *decoded_data, short *encoded_data, short mode);
short ILBCencode_30( iLBC_Enc_Inst_t *iLBCenc_inst,short *encoded_data, short *data);
short ILBCdecode_30( iLBC_Dec_Inst_t *iLBCdec_inst,short *decoded_data, short *encoded_data, short mode);


DspOutRtp::DspOutRtp( const codecType newCodec, int newCodecNum, const QString &hostName, UDPMessageSocket *s )
	: portnum(0)
	, curseq(0)
	, qlen(0)
	, numunsend(0)
	, fixedrtplen(160)
	, deb_frag(0L)
	, deb_rtp(0L)
	, ssrc(10)
	, ref_sec(0)
	, ref_usec(0)
	, dsize(4096)
	, useStun(false)
	, destroySocket(false)
{
	codec = newCodec;
	codecNum = newCodecNum;
	if (s) {
		socket = s;
	} else {
		socket = new UDPMessageSocket();
		destroySocket = true;
		if (socket == 0) {
			printf( QObject::tr("DspOutRtp:: Can't create socket") + "\n" );
			abort();
		}
	}
	if( hostName != QString::null && !socket->setHostname( hostName.latin1() ) ) {
		printf( QObject::tr("DspOutRtp::Hostname lookup failed") + "\n" );
	}
	if( !( gsmInstEnc = gsm_create() ) ) {
		printf( "DspOutRtp::GSM_CREATE -error !\n" );
	}
	if( !( gsmInstDec = gsm_create() ) ) {
		printf( "DspOutRtp::GSM_CREATE -error !\n" );
	}
	packetbuf = new unsigned char[ sizeof( rtp_hdr_t ) + dsize ];
	bufunsend = new unsigned char[ dsize ];
	outbuf = new unsigned char[ dsize ];
	tmpbuf = new unsigned char[ dsize ];
	quebuf = new unsigned char[ dsize ];
	initEncode(&ilbcEncInst_20, 20 );
	initDecode(&ilbcDecInst_20, 20, 1);
	initEncode(&ilbcEncInst_30, 30 );
	initDecode(&ilbcDecInst_30, 30, 1);
	audio_buf.resize( dsize * sizeof( short )  );
	setDeviceName("rtp");
}

DspOutRtp::~DspOutRtp( void )
{
	if( gsmInstEnc ) gsm_destroy( gsmInstEnc );
	if( gsmInstDec ) gsm_destroy( gsmInstDec );
	delete[] static_cast<unsigned char *>( packetbuf );
	delete[] static_cast<unsigned char *>( bufunsend );
	delete[] static_cast<unsigned char *>( outbuf );
	delete[] static_cast<unsigned char *>( tmpbuf );
	delete[] static_cast<unsigned char *>( quebuf );
	if (destroySocket) delete socket;
}

bool DspOutRtp::openDevice( DeviceMode mode )
{
	int min, max;
	QSettings settings;
	min = settings.readNumEntry("/kphone/Media/MinPort", constMinPort);
	max = settings.readNumEntry("/kphone/Media/MaxPort", constMaxPort);
	devmode = mode;
	if( devmode == ReadOnly ) {
		socket->listenOnEvenPort(min, max);
		portnum = socket->getPortNumber();
		/* Use STUN to rewrite port number only if the user
		 * didn't set a static port range which is being forwarded
		 * on the NAT
		 */
		if ((min == 0) && (max == 0)) {
			if ( sendStunRequest( socket ) ) {
				portnum = receiveStunResponse( socket );
				if( portnum > 0 ) {
					socket->forcePortNumber( portnum );
				}
			}
		}

//------------------------------
// Linux kernels 2.4.x
// Do not allow any double binds even when same user and REUSEADDR.
/*
		UDPMessageSocket *socketVideo = new UDPMessageSocket() ;
		socketVideo->forcePortNumber( videoPortnum );
		socketVideo->listenOnEvenPortForVideo();
		videoPortnum = socketVideo->getPortNumber();
		socketVideo->forcePortNumber( videoPortnum );
		if( sendStunRequest( *socketVideo ) ) {
			videoPortnum = receiveStunResponse( *socketVideo );
		}
		delete socketVideo;
*/
		videoPortnum = portnum + 20;
//------------------------------

	} else {
          if( socket->connect( portnum ) ) {
            if ( socket->SetTOS() )
              return false ;  
          }
          else 
            return false;
        }
	devstate = DeviceOpened;
	return true;
}

#define PCMUCODEC 0
#define GSMCODEC 3
#define PCMACODEC 8

typedef struct cb {
unsigned char (*func)( int frombuf );
} cb_t;

bool DspOutRtp::writeBuffer( void )
{
	short *s;
	short *frombuf;
	unsigned char *databuf;
	rtp_hdr_t *h = (rtp_hdr_t *) packetbuf;
	size_t fromsize;
	unsigned char *wlbuf;
	size_t bytesnew;
	size_t bytesgot;
	int tmpsize;
	unsigned int i;
	bytesnew = audio_buf.getSize() / sizeof( short );
	fromsize = audio_buf.getSize() / sizeof( short );
	inbuf = (unsigned char*)audio_buf.getData();
	frombuf = (short *)inbuf;
	if( codec == codecGSM ) {
		short *frombuf_test;
		frombuf_test = frombuf;
		s = (short *)inbuf;
		for( i=0; i<fromsize; ++i) {
			s[i] = (short)(*frombuf_test);
			++frombuf_test;
		}
		tmpsize = writeGSMBuffer( gsmInstEnc, inbuf, outbuf, tmpbuf, quebuf, &qlen, fromsize );
		h->version = 2;
		h->p = 0;
		h->x = 0;
		h->cc = 0;
		h->m = 0;
		h->pt = GSMCODEC;
		while( tmpsize > 0 ) {
			h->seq = htons( ++curseq );
			h->ts = htonl( ts );
			ts += GSM_DEC_SAMPLES;
			h->ssrc = ssrc;
			databuf = packetbuf + sizeof( rtp_hdr_t );
			for( int i = 0; i < tmpsize; ++i ) {
				databuf[ i ] = outbuf[ i ];
			}
			if( socket->send( (char *) packetbuf, sizeof( rtp_hdr_t ) + tmpsize ) < 0 ) {
				printf("DspOutRtp::writeBuffer: %s\n", strerror(errno));
				return false;
			}
			tmpsize = writeGSMBuffer( gsmInstEnc, inbuf, outbuf, tmpbuf, quebuf, &qlen, 0 );
		}
	}
	else if( codec == codecILBC_20 || codec == codecILBC_30 ) {
		short *frombuf_test;
		frombuf_test = frombuf;
		s = (short *)inbuf;
		for( i=0; i<fromsize; ++i) {
			s[i] = (short)(*frombuf_test);
			++frombuf_test;
		}
		if( codec == codecILBC_20 ) {
			tmpsize = writeILBCBuffer_20( &ilbcEncInst_20, inbuf, outbuf, tmpbuf, quebuf, &qlen, fromsize );
		} else {
			tmpsize = writeILBCBuffer_30( &ilbcEncInst_30, inbuf, outbuf, tmpbuf, quebuf, &qlen, fromsize );
		}
		h->version = 2;
		h->p = 0;
		h->x = 0;
		h->cc = 0;
		h->m = 0;
		h->pt = codecNum;
		while( tmpsize > 0 ) {
			h->seq = htons( ++curseq );
			h->ts = htonl( ts );
			if( codec == codecILBC_20 ) {
				ts += BLOCKL_20MS;
			} else {
				ts += BLOCKL_30MS;
			}
			h->ssrc = ssrc;
			databuf = packetbuf + sizeof( rtp_hdr_t );
			for( int i = 0; i < tmpsize; ++i ) {
				databuf[ i ] = outbuf[ i ];
			}
			if( socket->send( (char *) packetbuf, sizeof( rtp_hdr_t ) + tmpsize ) < 0 ) {
				printf("DspOutRtp::writeBuffer: %s\n", strerror(errno));
				return false;
			}
			if( codec == codecILBC_20 ) {
				tmpsize = writeILBCBuffer_20( &ilbcEncInst_20, inbuf, outbuf, tmpbuf, quebuf, &qlen, 0 );
			} else {
				tmpsize = writeILBCBuffer_30( &ilbcEncInst_30, inbuf, outbuf, tmpbuf, quebuf, &qlen, 0 );
			}
		}
	}
	else if( codec == codecPCMA || codec == codecPCMU ) {
		cb_t callb;
		wlbuf = bufunsend;
		h->version = 2;
		h->p = 0;
		h->x = 0;
		h->cc = 0;
		h->m = 0;
		switch ( codec ) {
          case codecPCMU:
			  h->pt = PCMUCODEC;
			  callb.func = &linear2ulaw;
			  break;
		  case codecPCMA:
			  h->pt = PCMACODEC;
			  callb.func = &linear2pcma;
		  default:
			  break;
		}
		while(bytesnew+numunsend >= fixedrtplen){
			++deb_rtp;
			h->seq = htons( ++curseq );
			h->ts = htonl( ts );
			ts += fixedrtplen;
			h->ssrc = ssrc;
			databuf = packetbuf + sizeof( rtp_hdr_t );
			bytesgot = 0;
			while( numunsend > 0 ) {
				*databuf = *wlbuf;
				++databuf;
				++wlbuf;
				--numunsend;
				++bytesgot;
			}
			wlbuf = bufunsend;
			while( bytesgot < fixedrtplen ) {
				*databuf = callb.func( *frombuf );
				++databuf;
				++frombuf;
				--bytesnew;
				++bytesgot;
			}
			if( socket->send( (char *) packetbuf, sizeof( rtp_hdr_t ) + fixedrtplen ) < 0 ) {
				printf("DspOutRtp::writeBuffer: %s\n", strerror(errno));
				return false;
			}
		}
		if( bytesnew > 0 ) {
			wlbuf = bufunsend + numunsend;
			numunsend += bytesnew;
			while( bytesnew > 0 ){
				*wlbuf = callb.func( *frombuf );
				++wlbuf;
				++frombuf;
				--bytesnew;
			}
		}
		++deb_frag;
	}
	else {
       printf("DspOutRtp::writeBuffer: unknown Codec %i\n",codec);
	   return false;
	}

	return true;
}

bool DspOutRtp::setPortNum( int newport )
{
	portnum = newport;
	return true;
}

unsigned int DspOutRtp::readableBytes( void )
{
	struct timeval timeout;
	fd_set read_fds;
	int highest_fd;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	FD_ZERO( &read_fds );
	FD_SET( socket->getFileDescriptor(), &read_fds );
	highest_fd = socket->getFileDescriptor() + 1;
retry:
	if ( select( highest_fd, &read_fds, NULL, NULL, &timeout ) == -1 ) {
		if ( errno == EINTR ) goto retry;
		perror( "DspOutRtp::doSelect(): select() punted" );
		exit( 1 );
	}
	if ( FD_ISSET( socket->getFileDescriptor(), &read_fds ) ) {
		return 1;
	}
	return 0;
}

bool DspOutRtp::readBuffer( int bytes )
{
        Q_UNUSED(bytes)

	unsigned char *inbuf;
	short *outbuf;
	int recvsize;
	int i;
	int size;
	recvsize = socket->receive( (char *) packetbuf, sizeof( rtp_hdr_t ) + dsize );
	rtp_hdr_t *h = (rtp_hdr_t *) packetbuf;
	if( h->pt == PCMUCODEC ) {
		audio_buf.resize( ( recvsize - (int) sizeof( rtp_hdr_t ) ) * sizeof( short ) );
		outbuf = (short *) audio_buf.getData();
		inbuf = &packetbuf[ sizeof( rtp_hdr_t ) ];
		for( i = 0; i < recvsize - (int) sizeof( rtp_hdr_t ); ++i ) {
			*outbuf = (short) ulaw2linear( inbuf[ i ] );
			++outbuf;
		}
	} else if( h->pt == GSMCODEC ) {
		audio_buf.resize( GSM_DEC_SAMPLES * sizeof( short ) );
		outbuf = (short *) audio_buf.getData();
		inbuf = &packetbuf[ sizeof( rtp_hdr_t ) ];
		readGSMBuffer( gsmInstDec, inbuf, tmpbuf, false );
		short *s = (short *)tmpbuf;
		for( i = 0; i < GSM_DEC_SAMPLES; ++i) {
			*outbuf = (short)(s[i]);
			++outbuf;
		}
	} else if( h->pt == PCMACODEC ) {
		audio_buf.resize( ( recvsize - (int) sizeof( rtp_hdr_t ) ) * sizeof( short ) );
		outbuf = (short *) audio_buf.getData();
		inbuf = &packetbuf[ sizeof( rtp_hdr_t ) ];
		for( i = 0; i < recvsize - (int) sizeof( rtp_hdr_t ); ++i ) {
			*outbuf = (short) pcma2linear( inbuf[ i ] );
			++outbuf;
		}
	} else if( codec == codecILBC_20 && (int)h->pt == codecNum ) {
		audio_buf.resize( BLOCKL_20MS * sizeof( short ) );
		outbuf = (short *) audio_buf.getData();
		inbuf = &packetbuf[ sizeof( rtp_hdr_t ) ];
		size = readILBCBuffer_20( &ilbcDecInst_20, inbuf, tmpbuf, 1, false );
		short *s = (short *)tmpbuf;
		for( i = 0; i < size; ++i) {
			*outbuf = (short)(s[i]);
			++outbuf;
		}
	} else if( codec == codecILBC_30 && (int)h->pt == codecNum ) {
		audio_buf.resize( BLOCKL_30MS * sizeof( short ) );
		outbuf = (short *) audio_buf.getData();
		inbuf = &packetbuf[ sizeof( rtp_hdr_t ) ];
		size = readILBCBuffer_30( &ilbcDecInst_30, inbuf, tmpbuf, 1, false );
		short *s = (short *)tmpbuf;
		for( i = 0; i < size; i++) {
			*outbuf++ = (short)(s[i]);
		}
	}
	return true;
}

bool DspOutRtp::sendStunRequest( UDPMessageSocket *socketStun )
{
	SipUri stun;
	if( useStun ) {
		stun = SipUri( stunSrv );
		if( !socketStun->setHostname( stun.getHostname() ) ) {
			return false;
		}
		socketStun->connect( stun.getPortNumber() );
		printf( "DspOutRtp: STUN request\n" );
		StunRequestSimple req;
		req.msgHdr.msgType = htons(BindRequestMsg);
		req.msgHdr.msgLength = htons( sizeof(StunRequestSimple)-sizeof(StunMsgHdr) );
		for ( int i=0; i<16; ++i ) {
			req.msgHdr.id.octet[i]=0;
		}
		int id = rand();
		req.msgHdr.id.octet[0] = id;
		req.msgHdr.id.octet[1] = id>>8;
		req.msgHdr.id.octet[2] = id>>16;
		req.msgHdr.id.octet[3] = id>>24;
		socketStun->send( (char *)&req, sizeof( req ) );
	} else {
		return false;
	}
	return true;
}

unsigned int DspOutRtp::receiveStunResponse( UDPMessageSocket *socketStun )
{
	unsigned int port = 0;
	char inputbuf[ 8000 ];
	int bytesread;
	struct timeval timeout;
	fd_set read_fds;
	int highest_fd;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	FD_ZERO( &read_fds );
	FD_SET( socketStun->getFileDescriptor(), &read_fds );
	highest_fd = socketStun->getFileDescriptor() + 1;
retry:
	if( select( highest_fd, &read_fds, NULL, NULL, &timeout ) == -1 ) {
		if( errno == EINTR ) goto retry;
		perror( "SipClient::doSelect(): select() punted" );
		exit( 1 );
	}
	if( FD_ISSET( socketStun->getFileDescriptor(), &read_fds ) ) {
		printf( QObject::tr("SipClient: Receiving message...") + "\n" );
		bytesread = read( socketStun->getFileDescriptor(), inputbuf, 8000 - 1 );
		StunMsgHdr* hdr = reinterpret_cast<StunMsgHdr*>( inputbuf );
		if( hdr->msgType == BindResponseMsg ) {
			printf( QObject::tr("SipClient: STUN response for RTP") + "\n" );
			char* body = inputbuf + sizeof( StunMsgHdr );
			unsigned int size = ntohs( hdr->msgLength );
			while( size > 0 ) {
				StunAtrHdr* attr = reinterpret_cast<StunAtrHdr*>( body );
				unsigned int hdrLen = ntohs( attr->length );
				if( ntohs( attr->type ) == MappedAddress ) {
					StunAtrAddress* attribute =
						reinterpret_cast<StunAtrAddress*>( body );
					if ( attribute->address.addrHdr.family == IPv4Family ) {
						StunAtrAddress4* atrAdd4 =
							reinterpret_cast<StunAtrAddress4*>( body );
						if ( hdrLen == sizeof( StunAtrAddress4 ) - 4 ) {
							port = ntohs( atrAdd4->addrHdr.port );
						}
					}
				}
				body += hdrLen+4;
				size -= hdrLen+4;
			}
		}
	}
	return port;
}

void DspOutRtp::setCodec( const codecType newCodec, int newCodecNum )
{
	codec = newCodec;
	codecNum = newCodecNum;
}

/************************************************************************/
/* GSM                                                                   */
/************************************************************************/

int DspOutRtp::writeGSMBuffer( gsm gsmInst,
		unsigned char *input_buf, unsigned char *output_buf,
		unsigned char *tmp_buf, unsigned char *queue,
		int *qlen, int size )
{
	int outbufsize = 0;
	int inq, inx;
	short *ibuf;
	short *obuf;
	short *tbuf;
	short *qbuf;
	ibuf = (short *)input_buf;
	obuf = (short *)output_buf;
	qbuf = (short *)queue;
	inq = *qlen;
	inx = 0;
	if( inq >= GSM_DEC_SAMPLES ) {
		tbuf = (short *)tmp_buf;
		while( inx < GSM_DEC_SAMPLES ) {
			*tbuf = *qbuf;
			++tbuf;
			++qbuf;
			--inq;
			++inx;
		}
		tbuf = (short *)tmp_buf;
		gsm_encode( gsmInst, (gsm_signal *)tbuf, (gsm_byte *)obuf);
		*qlen = inq;
		outbufsize = GSM_ENC_BYTES;
	} else if( inq + size >= GSM_DEC_SAMPLES ) {
		tbuf = (short *)tmp_buf;
		while( inq > 0 ) {
			*tbuf = *qbuf;
			++tbuf;
			++qbuf;
			--inq;
			++inx;
		}
		*qlen = 0;
		while( inx < GSM_DEC_SAMPLES ) {
			*tbuf = *ibuf;
			++tbuf;
			++ibuf;
			--size;
			++inx;
		}
		tbuf = (short *)tmp_buf;
		gsm_encode( gsmInst, (gsm_signal *)tbuf, (gsm_byte *)obuf);
		outbufsize = GSM_ENC_BYTES;

	}
	if( size > 0 ) {
		inq = *qlen;
		qbuf = (short *)(queue + ( inq * sizeof(short) ) );
		while( size > 0 ) {
			*qbuf = *ibuf;
			++inq;
			--size;
			++qbuf;
			++ibuf;
		}
		*qlen = inq;
	}
	return outbufsize;
}

int DspOutRtp::readGSMBuffer( gsm Gsm_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		int ignore )
{
	short *outbuf;
	short *ibuf;
	outbuf = (short *)output_buf;
	ibuf = (short *)input_buf;
	if( !ignore ) {
		gsm_decode( Gsm_Inst, (gsm_byte *)ibuf, (gsm_signal *)outbuf );
	}
	return true;
}



/*************************************************************************/
/* iLBC 20ms                                                                  */
/*************************************************************************/

int DspOutRtp::writeILBCBuffer_20( iLBC_Enc_Inst_t *Enc_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		unsigned char *tmp_buf, unsigned char *queue,
		int *qlen, int size )
{
	int outbufsize = 0;
	int inq, inx;
	short *ibuf;
	short *obuf;
	short *tbuf;
	short *qbuf;
	ibuf = (short *)input_buf;
	obuf = (short *)output_buf;
	qbuf = (short *)queue;
	inq = *qlen;
	inx = 0;
	if( inq >= BLOCKL_20MS ) {
		tbuf = (short *)tmp_buf;
		while( inx < BLOCKL_20MS ) {
			*tbuf = *qbuf;
			++tbuf;
			++qbuf;
			--inq;
			++inx;
		}
		tbuf = (short *)tmp_buf;
		outbufsize = ILBCencode_20( Enc_Inst, obuf, tbuf);
		*qlen = inq;
	} else if( inq + size >= BLOCKL_20MS ) {
		tbuf = (short *)tmp_buf;
		while( inq > 0 ) {
			*tbuf = *qbuf;
			++tbuf;
			++qbuf;
			--inq;
			++inx;
		}
		*qlen = 0;
		while( inx < BLOCKL_20MS ) {
			*tbuf = *ibuf;
			++tbuf;
			++ibuf;
			--size;
			++inx;
		}
		tbuf = (short *)tmp_buf;
		outbufsize = ILBCencode_20( Enc_Inst, obuf, tbuf);
	}
	if( size > 0 ) {
		inq = *qlen;
		qbuf = (short *)(queue + ( inq * sizeof(short) ) );
		while( size > 0 ) {
			*qbuf = *ibuf;
			++inq;
			--size;
			++qbuf;
			++ibuf;
		}
		*qlen = inq;
	}
	return outbufsize;
}


int DspOutRtp::readILBCBuffer_20( iLBC_Dec_Inst_t *Dec_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		short mode, int ignore)
{
	short *outbuf;
	short *ibuf;
	outbuf = (short *)output_buf;
	ibuf = (short *)input_buf;
	if( !ignore ) {
		return ILBCdecode_20( Dec_Inst, outbuf, ibuf, mode );
	}
	return 0;
}


/*----------------------------------------------------------------*
 *  Encoder interface function
 *---------------------------------------------------------------*/

short ILBCencode_20( iLBC_Enc_Inst_t *iLBCenc_inst,
		short *encoded_data,short *data )
{
	float block[BLOCKL_20MS];
	int k;

/* convert signal to float */

	for(k=0;k<BLOCKL_20MS;++k) block[k] = (float)data[k];

/* do the actual encoding */

	iLBC_encode((unsigned char *)encoded_data,block,iLBCenc_inst);

	return (short)(NO_OF_BYTES_20MS);
}


/*----------------------------------------------------------------*
 *  Decoder interface function
 *---------------------------------------------------------------*/

short ILBCdecode_20(iLBC_Dec_Inst_t *iLBCdec_inst,
		short *decoded_data, short *encoded_data, short mode)
{
	int k;
	float decblock[BLOCKL_20MS], dtmp;

/* check if mode is valid */

	if(mode<0 || mode>1){
		printf( "\nERROR - Wrong mode - 0, 1 allowed\n" );
		exit(3);
	}

/* do actual decoding of block */

	iLBC_decode( decblock,
		(unsigned char *)encoded_data, iLBCdec_inst, mode);

/* convert to short */

	for(k=0;k<BLOCKL_20MS;++k){
		dtmp=decblock[k];
		if (dtmp<MIN_SAMPLE)dtmp=MIN_SAMPLE;
		else if (dtmp>MAX_SAMPLE)dtmp=MAX_SAMPLE;
		decoded_data[k] = (short) dtmp;
	}

	return (short)BLOCKL_20MS;
}

/*************************************************************************/
/* iLBC 30ms                                                                  */
/*************************************************************************/

int DspOutRtp::writeILBCBuffer_30( iLBC_Enc_Inst_t *Enc_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		unsigned char *tmp_buf, unsigned char *queue,
		int *qlen, int size )
{
	int outbufsize = 0;
	int inq, inx;
	short *ibuf;
	short *obuf;
	short *tbuf;
	short *qbuf;
	ibuf = (short *)input_buf;
	obuf = (short *)output_buf;
	qbuf = (short *)queue;
	inq = *qlen;
	inx = 0;
	if( inq >= BLOCKL_30MS ) {
		tbuf = (short *)tmp_buf;
		while( inx < BLOCKL_30MS ) {
			*tbuf = *qbuf;
			++tbuf;
			++qbuf;
			--inq;
			++inx;
		}
		tbuf = (short *)tmp_buf;
		outbufsize = ILBCencode_30( Enc_Inst, obuf, tbuf);
		*qlen = inq;
	} else if( inq + size >= BLOCKL_30MS ) {
		tbuf = (short *)tmp_buf;
		while( inq > 0 ) {
			*tbuf = *qbuf;
			++tbuf;
			++qbuf;
			--inq;
			++inx;
		}
		*qlen = 0;
		while( inx < BLOCKL_30MS ) {
			*tbuf = *ibuf;
			++tbuf;
			++ibuf;
			--size;
			++inx;
		}
		tbuf = (short *)tmp_buf;
		outbufsize = ILBCencode_30( Enc_Inst, obuf, tbuf);
	}
	if( size > 0 ) {
		inq = *qlen;
		qbuf = (short *)(queue + ( inq * sizeof(short) ) );
		while( size > 0 ) {
			*qbuf = *ibuf;
			++inq;
			--size;
			++qbuf;
			++ibuf;
		}
		*qlen = inq;
	}
	return outbufsize;
}


int DspOutRtp::readILBCBuffer_30( iLBC_Dec_Inst_t *Dec_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		short mode, int ignore)
{
	short *outbuf;
	short *ibuf;
	outbuf = (short *)output_buf;
	ibuf = (short *)input_buf;
	if( !ignore ) {
		return ILBCdecode_30( Dec_Inst, outbuf, ibuf, mode );
	}
	return 0;
}


/*----------------------------------------------------------------*
 *  Encoder interface function
 *---------------------------------------------------------------*/

short ILBCencode_30( iLBC_Enc_Inst_t *iLBCenc_inst,
		short *encoded_data,short *data )
{
	float block[BLOCKL_30MS];
	int k;

/* convert signal to float */

	for(k=0;k<BLOCKL_30MS;++k) block[k] = (float)data[k];

/* do the actual encoding */

	iLBC_encode((unsigned char *)encoded_data,block,iLBCenc_inst);

	return (short)(NO_OF_BYTES_30MS);
}


/*----------------------------------------------------------------*
 *  Decoder interface function
 *---------------------------------------------------------------*/

short ILBCdecode_30(iLBC_Dec_Inst_t *iLBCdec_inst,
		short *decoded_data, short *encoded_data, short mode)
{
	int k;
	float decblock[BLOCKL_30MS], dtmp;

/* check if mode is valid */

	if(mode<0 || mode>1){
		printf( "\nERROR - Wrong mode - 0, 1 allowed\n" );
		exit(3);
	}

/* do actual decoding of block */

	iLBC_decode( decblock,
		(unsigned char *)encoded_data, iLBCdec_inst, mode);

/* convert to short */

	for(k=0;k<BLOCKL_30MS;++k){
		dtmp=decblock[k];
		if (dtmp<MIN_SAMPLE)dtmp=MIN_SAMPLE;
		else if (dtmp>MAX_SAMPLE)dtmp=MAX_SAMPLE;
		decoded_data[k] = (short) dtmp;
	}

	return (short)BLOCKL_30MS;
}



void  DspOutRtp::setStunSrv( QString newStunSrv )
{
	useStun = true;
	stunSrv = newStunSrv;
}
