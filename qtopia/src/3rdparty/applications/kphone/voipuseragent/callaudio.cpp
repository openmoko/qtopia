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
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <qsettings.h>

#include "../config.h"
#include "../dissipate2/sipprotocol.h"
#include "../dissipate2/sipcall.h"
#include "../dissipate2/sdp.h"
#include "dspaudioout.h"
#include "dspaudioin.h"
#include "dspoutrtp.h"
#include "dspoutoss.h"
#include "kphoneview.h"

#ifdef ALSA_SUPPORT
#include "dspoutalsa.h"
#endif

#ifdef JACK_SUPPORT
#include "dspaudiojackout.h"
#include "dspaudiojackin.h"
#endif

#include "callaudio.h"

CallAudio::CallAudio(  KPhoneView *phoneview )
{
	view = phoneview;
	if( view->useStunSrv() ) {
		setStunSrv( view->getStunSrv() );
	}
	jack_audioout = 0;
	jack_audioin = 0;
	output = 0;
	input = 0;
	curcall = 0;
	curmember = 0;
	audiomode = oss;
	setOSSFilename( QString::null );
	setOSSFilename2( QString::null );
	setVideoSW( QString::null );
	local.setIpAddress( "0.0.0.0" );
	QSettings settings;
	payload = settings.readNumEntry( "/kphone/dsp/SizeOfPayload", 160 );
	codec = codecILBC_20;
	rtpCodec = codecUnknown;
	videoCodec = codecUnknown;
	videoRtpCodec = codecUnknown;
	bodyMask = QString::null;
	useStun = false;
	symMedia = false;
	pidVideo = 0;
	audio_fd = -1;
}

CallAudio::~CallAudio( void )
{
	if( output ) {
		if( output->running() ) {
			output->setCancel();
			output->wait();
		}
		delete output;
	}
	if( input ) {
		if( input->running() ) {
			input->setCancel();
			input->wait();
		}
		delete input;
	}
}

void CallAudio::setAudiomode( QString str )
{
	if( str.upper() == "OSS" ) {
		if( audiomode == oss ) return;
		audiomode = oss;
	} else if( str.upper() == "ALSA" ) {
		if( audiomode == alsa ) return;
		audiomode = alsa;
	} else if( str.upper() == "JACK" ) {
		if( audiomode == jack ) return;
		audiomode = jack;
	}
	renegotiateCall();
}


void CallAudio::setOSSFilename( const QString &devname )
{
	if( devname == QString::null ) {
		ossfilename = "/dev/dsp";
	} else {
		ossfilename = devname;
	}
}

void CallAudio::setOSSFilename2( const QString &devname )
{
	if( devname == QString::null ) {
		ossfilename2 = "/dev/dsp";
	} else {
		ossfilename2 = devname;
	}
}

void CallAudio::setVideoSW( const QString &sw )
{
	if( sw == QString::null ) {
		videoSW = "vic";
	} else {
		videoSW = sw;
	}
}

void CallAudio::audioIn( void )
{
	QString hostname = remote.getIpAddress();
	unsigned int portnum = remote.getPort();
	if( hostname == QString::null || portnum == 0 ) {
		printf( tr("CallAudio: SendToRemote called but we don't have a valid session description yet") + "\n" );
		return;
	}
	if( remote.isOnHold() ) {
		printf( tr("CallAudio: Remote is currently putting us on hold, waiting patiently") + "\n" );
		return;
	}
	printf( tr("CallAudio: Sending to remote site %s:%d") + "\n", hostname.latin1(), portnum );
	if( input ) {
		stopListeningAudio();
	}
	DspOut *out;
	DspOut *in;
	if( audiomode == oss ) {
		DspOutRtp *outrtp;
		if (symMedia) {
			outrtp = new DspOutRtp( getRtpCodec(), getRtpCodecNum(), hostname, &socket );
		} else {
			outrtp = new DspOutRtp( getRtpCodec(), getRtpCodecNum(), hostname );
		}
		outrtp->setPortNum( portnum );
		outrtp->openDevice( DspOut::WriteOnly );
		outrtp->setPayload( payload );
		DspOutOss *inaudio = new DspOutOss( ossfilename2 );
		QSettings settings;
		if( settings.readEntry( "/kphone/audio/mode", "readwrite" ) == "readwrite" ) {
			if( !inaudio->openDevice( audio_fd ) ) {
				printf( tr("** audioIn: openDevice Failed.") + "\n" );
			}
		} else {
			printf( tr("CallAudio: Opening OSS device %s for Input") + "\n", ossfilename2.latin1() );
			if( !inaudio->openDevice( DspOut::ReadOnly ) ) {
				printf( tr("** audioIn: openDevice Failed.") + "\n" );
			}
		}
		inaudio->readBuffer(2);
		out = outrtp;
		in = inaudio;
		printf( tr("CallAudio: Creating OSS->RTP Diverter") + "\n" );
		input = new DspAudioIn( in, out );

#ifdef ALSA_SUPPORT
	} else if( audiomode == alsa ) {
		DspOutRtp *outrtp;
		if (symMedia) {
			outrtp = new DspOutRtp( getRtpCodec(), getRtpCodecNum(), hostname, &socket );
		} else {
			outrtp = new DspOutRtp( getRtpCodec(), getRtpCodecNum(), hostname );
		}
		outrtp->setPortNum( portnum );
		outrtp->openDevice( DspOut::WriteOnly );
		outrtp->setPayload( payload );
		DspOutAlsa *inaudio = new DspOutAlsa();
		QSettings settings;
		printf( "CallAudio: Opening ALSA device for Input \n" );
		if( !inaudio->openDevice( DspOut::ReadOnly ) ) {
			printf( "** audioIn: openDevice Failed.\n" );
		}
		out = outrtp;
		in = inaudio;
		printf( "CallAudio: Creating ALSA->RTP Diverter\n" );
		input = new DspAudioIn( in, out );
#endif

#ifdef JACK_SUPPORT
	} else if( audiomode == jack ) {
		DspOutRtp *outrtp;
		if (symMedia) {
			outrtp = new DspOutRtp( getRtpCodec(), getRtpCodecNum(), hostname, &socket );
		} else {
			outrtp = new DspOutRtp( getRtpCodec(), getRtpCodecNum(), hostname );
		}
		outrtp->setPortNum( portnum );
		outrtp->openDevice( DspOut::WriteOnly );
		outrtp->setPayload( payload );
		
		printf( "CallAudio: Creating JACK->RTP Diverter\n" );
		QString s = curcall->getCallId();
		QString name = "Kphone-" + s.left( s.find( '@' )) + "-in";
		jack_audioin = new DspAudioJackIn( outrtp, name );
#endif
	}

	if( curcall->getCallType() == SipCall::videoCall ) {
		printf( tr("CallAudio: Opening SW for video input and output") + "\n" );
		pidVideo = fork();
		if( !pidVideo ) {
			QSettings settings;
			QString videoSW = settings.readEntry(
				"/kphone/video/videoSW", "/usr/local/bin/vic" );
			QString videoCodec = getVideoRtpCodecName();
			QString SW = videoSW;
			if( SW.contains( "/" ) ) {
				SW = SW.mid( SW.findRev( '/' ) + 1 );
			}
			QString videoSWParam = hostname + "/" +
				QString::number( remote.getVideoPort() ) + "/";
			videoSWParam += videoCodec + "/16/" +
				QString::number( local.getVideoPort() );
			printf( "CallAudio: execlp( %s, %s, %s, 0)\n",
				videoSW.latin1(), SW.latin1(), videoSWParam.latin1() );
			execlp( videoSW.latin1(), SW.latin1(), videoSWParam.latin1(), 0 );
			printf( tr("error executing ", "error in executing the following program") + videoSW + "\n" );
			exit(1);
		}
	}
}

void CallAudio::stopListeningAudio( void )
{
	if( input ) {
		if( input->running() ) {
			input->setCancel();
			input->wait();
		}
		delete input;
	}
	input = 0;

#ifdef JACK_SUPPORT
	if( jack_audioin ) {
		jack_audioin->exit();
//		delete jack_audioin;
	}
	jack_audioin = 0;
#endif

	remote = SdpMessage::null;
}

SdpMessage CallAudio::audioOut( void )
{
	printf( "CallAudio: listening for incomming RTP\n" );
	if( output ) {
		stopSendingAudio();
	}
	DspOut *out;
	DspOut *in;
	local.setIpAddress( Sip::getLocalAddress() );
	local.setName( "The Funky Flow" );
	if( audiomode == oss ) {
		DspOutRtp *inrtp;
		if (symMedia) {
			inrtp = new DspOutRtp( getRtpCodec(),
				getRtpCodecNum(), QString::null, &socket );
		} else {
			inrtp = new DspOutRtp( getRtpCodec(), getRtpCodecNum() );
		}
		inrtp->setPayload( payload );
		if( useStun ) {
			inrtp->setStunSrv( stunSrv );
		}
		inrtp->openDevice( DspOut::ReadOnly );
		local.setPort( inrtp->getPortNum() );
		local.setVideoPort( inrtp->getVideoPortNum() );
		DspOutOss *outoss = new DspOutOss( ossfilename );
		QSettings settings;
		if( settings.readEntry( "/kphone/audio/mode", "readwrite" ) == "readwrite" ) {
			printf( tr("CallAudio: Opening OSS device %s for Input and Output") + "\n", ossfilename.latin1() );
			if( !outoss->openDevice( DspOut::ReadWrite ) ) {
				printf( tr("** audioOut: openDevice Failed.") + "\n" );
			} else {
				audio_fd = outoss->audio_fd;
			}
		} else {
			printf( tr("CallAudio: Opening OSS device %s for Output") + "\n", ossfilename.latin1() );
			if( !outoss->openDevice( DspOut::WriteOnly )){
				printf( tr("** audioOut: openDevice Failed.") + "\n" );
			}
		}
		in = inrtp;
		out = outoss;
		printf( tr("CallAudio: Creating RTP->OSS Diverter") + "\n" );
		output = new DspAudioOut( in, out );

#ifdef ALSA_SUPPORT
	} else if( audiomode == alsa ) {

		DspOutRtp *inrtp;
		if (symMedia) {
			inrtp = new DspOutRtp( getRtpCodec(),
				getRtpCodecNum(), QString::null, &socket );
		} else {
			inrtp = new DspOutRtp( getRtpCodec(), getRtpCodecNum() );
		}
		inrtp->setPayload( payload );
		if( useStun ) {
			inrtp->setStunSrv( stunSrv );
		}
		inrtp->openDevice( DspOut::ReadOnly );
		local.setPort( inrtp->getPortNum() );
		local.setVideoPort( inrtp->getVideoPortNum() );
		DspOutAlsa *outalsa = new DspOutAlsa();
		QSettings settings;
		printf( "CallAudio: Opening ALSA device for Output\n" );
		if( !outalsa->openDevice( DspOut::WriteOnly )){
			printf( "** audioOut: openDevice Failed.\n" );
		}

		in = inrtp;
		out = outalsa;
		printf( tr("CallAudio: Creating RTP->ALSA Diverter") + "\n" );
		output = new DspAudioOut( in, out );
#endif

#ifdef JACK_SUPPORT
	} else if( audiomode == jack ) {
		DspOutRtp *inrtp;
		if (symMedia) {
			inrtp = new DspOutRtp( getRtpCodec(),
				getRtpCodecNum(), QString::null, &socket );
		} else {
			inrtp = new DspOutRtp( getRtpCodec(), getRtpCodecNum() );
		}
		inrtp->setPayload( payload );
		if( useStun ) {
			inrtp->setStunSrv( stunSrv );
		}
		inrtp->openDevice( DspOut::ReadOnly );
		local.setPort( inrtp->getPortNum() );
		local.setVideoPort( inrtp->getVideoPortNum() );


		printf( "CallAudio: Creating RTP->JACK Diverter\n" );
		QString s = curcall->getCallId();
		QString name = "Kphone-" + s.left( s.find( '@' )) + "-out";
		jack_audioout = new DspAudioJackOut( inrtp, name );
		if ( !jack_audioout->ok() ) {
		}
#endif

	}
	return local;
}

void CallAudio::stopSendingAudio( void )
{
	if( output ) {
		if( output->running() ) {
			output->setCancel();
			output->wait();
		}
		delete output;
	}
	output = 0;

#ifdef JACK_SUPPORT
	if( jack_audioout ) {
		jack_audioout->exit();
		delete jack_audioout;
	}
	jack_audioout = 0;
#endif

	local.setIpAddress( "0.0.0.0" );
	local.setName( "Idle" );
	local.setPort( 0 );
	local.setVideoPort( 0 );
}

void CallAudio::saveAudioSettings( void )
{
	QSettings settings;
	settings.writeEntry( "/kphone/audio/oss-filename", getOSSFilename() );
	settings.writeEntry( "/kphone/audio/oss-filename2", getOSSFilename2() );
}

void CallAudio::readAudioSettings( void )
{
	QSettings settings;
	setOSSFilename( settings.readEntry( "/kphone/audio/oss-filename" ) );
	setOSSFilename2( settings.readEntry( "/kphone/audio/oss-filename2" ) );
	if( settings.readEntry( "/kphone/Symmetric/Media", "Yes" ) == "Yes" ) {
		setSymMediaMode( true );
	}

	QString audiomodestr = settings.readEntry( "/kphone/audio/audio", "oss" );
	setAudiomode( audiomodestr );

	QString codec = settings.readEntry( "/kphone/audio/Codec", "PCMU" );
	if( codec == "PCMU" ) {
		setCodec( codecPCMU );
	} else if( codec == "GSM" ) {
		setCodec( codecGSM );
	} else if( codec == "PCMA" ) {
		setCodec( codecPCMA );
	} else if( codec == "ILBC_20" ) {
		setCodec( codecILBC_20 );
	} else if( codec == "ILBC" ) {
		setCodec( codecILBC_30 );
	}
}

void CallAudio::readVideoSettings( void )
{
	QSettings settings;
	setVideoSW( settings.readEntry( "/kphone/video/videoSW", "/usr/local/bin/vic"  ) );
	QString videoCodec = settings.readEntry( "/kphone/video/codec", "h261" );
	if( videoCodec == "h261" ) {
		setVideoCodec( codecH261 );
	} else if( videoCodec == "h263" ) {
		setVideoCodec( codecH263 );
	}
}

void CallAudio::setCurrentCall( SipCall *newcall )
{
	curcall = newcall;
	statusUpdated();
}

void CallAudio::attachToCallMember( SipCallMember *newmember )
{
	if( curmember ) {
		detachAndHold();
	}
	curmember = newmember;
	connect( curmember, SIGNAL( statusUpdated(SipCallMember *) ),
		this, SLOT( memberStatusUpdated(SipCallMember *) ) );
	statusUpdated();
}

void CallAudio::detachAndHold( void )
{
	if( local.isOnHold() ) {
		printf( "CallAudio: Call already on hold\n" );
	} else {
		toggleOnHold();
	}
}

void CallAudio::toggleOnHold( void )
{
	if( local.isOnHold() ) {
		printf( "CallAudio: Resuming call\n" );
		curmember->requestInvite(
			audioOut().message( getRtpCodec(), getVideoRtpCodec(), getBodyMask() ),
			MimeContentType( "application/sdp" ) );
	} else {
		printf( "CallAudio: Putting call on hold\n" );
		local.setIpAddress( "0.0.0.0" );
		local.setName( "Whoa there dewd" );
		local.setPort( 0 );
		local.setVideoPort( 0 );
		curmember->requestInvite(
			local.message( getRtpCodec(), getVideoRtpCodec(), getBodyMask() ),
			MimeContentType( "application/sdp" ) );
		detachFromCall();
	}
}

bool CallAudio::checkCodec( SipCallMember *member )
{
	bool status = true;
	QString mstr = member->getSessionDescription();
	rtpCodec = codecUnknown;
	videoRtpCodec = codecUnknown;
	if( mstr.contains( "m=audio" ) ) {
		QString ilbc = "";
		QString m = mstr.mid( mstr.find( "m=audio" ) );
		m = m.left( m.find( "\n" ) );
		m = m.mid( m.find( "RTP/AVP" ) + 7 );
		m += ' ';

		// dynamic port 96-127 
		if( mstr.lower().contains( "ilbc/8000" ) ) {
			ilbc = mstr.mid( mstr.lower().find( "ilbc/8000" ) - 7, 6 );
			if( ilbc.contains( ":" ) ) {
				ilbc = ilbc.mid( ilbc.find( ":" ) + 1 );
			}
			ilbc = ilbc.simplifyWhiteSpace();
		}
		int posPCMU = m.find( " 0 " );
		int posGSM  = m.find( " 3 " );
		int posPCMA = m.find( " 8 " );
		int posILBC = m.lower().find( " " + ilbc + " " );
		if( posPCMU < 0 ) posPCMU = 101;
		if( posGSM  < 0 ) posGSM  = 101;
		if( posPCMA < 0 ) posPCMA = 101;
		if( posILBC < 0 ) posILBC = 101;

#define MIN(a,b) (a)<(b)?(a):(b)
		
		int winner = MIN ( posPCMU , posGSM );
		    winner = MIN ( winner , posPCMA );
			winner = MIN ( winner , posILBC );
 
	    if ( winner == posPCMU ) {
			rtpCodec = codecPCMU;
			rtpCodecNum = 0;
		}
		else 
		if ( winner == posGSM ) {
			rtpCodec = codecGSM;
			rtpCodecNum = 3;
		}
		if ( winner == posPCMA ) {
			rtpCodec = codecPCMA;
			rtpCodecNum = 8;
		}
		else 
		if( winner == posILBC ) {
			rtpCodec = codecILBC_30;
			rtpCodecNum = ilbc.toInt();
			if( mstr.contains( "a=fmtp:" ) ) {
				QString m = mstr.mid( mstr.find( "a=fmtp:" ) );
				m = m.left( m.find( "\n" ) );
				if( m.lower().contains( "mode=20" ) ) {
					rtpCodec = codecILBC_20;
				}
			}
		} 
	}
	if( mstr.contains( "m=video" ) ) {
		QString ilbc = "";
		QString m = mstr.mid( mstr.find( "m=video" ) );
		m = m.left( m.find( "\n" ) );
		m = m.mid( m.find( "RTP/AVP" ) + 7 );
		m += ' ';
		int posH261 = m.find( " 31 " );
		int posH263 = m.find( " 34 " );
		if( posH261 < 0 ) posH261 = 101;
		if( posH263 < 0 ) posH263 = 101;
#ifdef MESSENGER
		if( videoCodec == codecH261 ) {
			videoRtpCodec = codecH261;
			videoRtpCodecNum = 31;
		} else {
			videoRtpCodec = codecH263;
			videoRtpCodecNum = 34;
		}
#else
		if( posH261 < posH263 ) {
			videoRtpCodec = codecH261;
			videoRtpCodecNum = 31;
		} else if( posH263 < posH261 ) {
			videoRtpCodec = codecH263;
			videoRtpCodecNum = 34;
		}
#endif
	}

	if( rtpCodec == codecILBC_20 ) {
		printf( "CallAudio: Using iLBC 20ms for output\n" );
	} else if( rtpCodec == codecILBC_30 ) {
		printf( "CallAudio: Using iLBC 30ms for output\n" );
	} else if( rtpCodec == codecPCMA ) {
		printf( "CallAudio: Using G711a for output\n" );
	} else if( rtpCodec == codecGSM ) {
		printf( "CallAudio: Using GSM for output\n" );
	} else if( rtpCodec == codecPCMU ) {
		printf( "CallAudio: Using G711u for output\n" );
	} else if( rtpCodec == codecUnknown ) {
		status = false;
	}
	if( rtpCodec != codecUnknown ) {
		if( videoRtpCodec == codecH261 ) {
			printf( "CallAudio: Using H261 for video output\n" );
		} else if( videoRtpCodec == codecH263 ) {
			printf( "CallAudio: Using H263 for video output\n" );
		} else {
			if( curcall ) {
				curcall->setCallType( SipCall::StandardCall ); 
			}
		}
	}
	
	return status;
}

void CallAudio::memberStatusUpdated(SipCallMember *member)
{
	SdpMessage sdpm;
	SdpMessage rsdp;
	curmember = member;
	sdpm.parseInput( curmember->getSessionDescription() );
	if( checkCodec( curmember ) ) {
		if( curmember->getState() == SipCallMember::state_RequestingReInvite ) {
			if( sdpm.isOnHold() ) {
				rsdp.setName( "Accepting on hold" );
				rsdp.setIpAddress( "0.0.0.0" );
				rsdp.setPort( 0 );
				rsdp.setVideoPort( 0 );
				curmember->acceptInvite(
					rsdp.message( getRtpCodec(),
						getVideoRtpCodec(), getBodyMask() ),
					MimeContentType( "application/sdp" ) );
				stopSendingAudio();
				stopListeningAudio();
			} else {
				curmember->acceptInvite(
					audioOut().message( getRtpCodec(),
						getVideoRtpCodec(), getBodyMask() ),
					MimeContentType( "application/sdp" ) );
			}
		}
		if( sdpm != remote ) {
			remote = sdpm;
			if( !sdpm.isOnHold() ) {
				if( output ) {
					output->setCodec( getRtpCodec(), getRtpCodecNum() );
					audioIn();
					output->start();
					input->start();
				} else if( jack_audioout ) {

//					jack_audioout->setCodec( getRtpCodec(), getRtpCodecNum() );
					audioIn();
/*
					jack_audioout->start();
					jack_audioin->start();
*/
				}
			}
			statusUpdated();
		}
	}
}

void CallAudio::detachFromCall( void )
{
	if( curmember ) {
		disconnect( curmember, 0, this, 0 );
	}
	curmember = 0;
	stopSendingAudio();
	stopListeningAudio();
	setCurrentCall( 0 );
	if( pidVideo ) {
		kill( pidVideo, SIGKILL );
		pidVideo = 0;
	}
}

bool CallAudio::isRemoteHold( void )
{
	return remote.isOnHold();
}

void CallAudio::renegotiateCall( void )
{
	if( !curcall ) return;
	stopSendingAudio();
	stopListeningAudio();
	curmember->requestInvite(
		audioOut().message( getRtpCodec(),
			getVideoRtpCodec(), getBodyMask() ),
		MimeContentType( "application/sdp" ) );
}

bool CallAudio::isAudioOn( void )
{
	return (output || input );
}

codecType CallAudio::getRtpCodec( void )
{
	if( rtpCodec != codecUnknown ) {
		return rtpCodec;
	} else {
		return codec;
	}
}

int CallAudio::getRtpCodecNum( void )
{
	int c;
	if( rtpCodec != codecUnknown ) {
		c = rtpCodecNum;
	} else {
		switch( codec ) {
			case codecGSM:
				c = 3;
				break;
			case codecPCMA:
				c = 8;
				break;
			case codecILBC_20:
			case codecILBC_30:
				c = 97;
				break;
			case codecPCMU:
			default:
				c = 0;
				break;
		}
	}
	return c;
}

codecType CallAudio::getVideoRtpCodec( void )
{
	if( videoRtpCodec != codecUnknown ) {
		return videoRtpCodec;
	} else {
		if( curcall ) {
			if( curcall->getCallType() == SipCall::videoCall ) {
				return videoCodec;
			} else {
				return codecUnknown;
			}
		} else {
			return codecUnknown;
		}
	}
}

int CallAudio::getVideoRtpCodecNum( void )
{
	int c;
	if( curcall ) {
		if( curcall->getCallType() != SipCall::videoCall ) {
			return -1;
		}
	}
	if( videoRtpCodec != codecUnknown ) {
		c = videoRtpCodecNum;
	} else {
		switch( videoCodec ) {
			case codecH261:
				c = 31;
				break;
			case codecH263:
				c = 34;
				break;
			default:
				c = -1;
				break;
		}
	}
	return c;
}

QString CallAudio::getVideoRtpCodecName( void )
{
	QString c;
	if( curcall->getCallType() != SipCall::videoCall ) {
		return "";
	}
	if( videoRtpCodec != codecUnknown ) {
		switch( videoRtpCodec ) {
			case codecH263:
				c = "h263";
				break;
			default:
			case codecH261:
				c = "h261";
				break;
		}
	} else {
		switch( videoCodec ) {
			case codecH263:
				c = "h263";
				break;
			default:
			case codecH261:
				c = "h261";
				break;
		}
	}
	return c;
}

void CallAudio::setStunSrv( QString newStunSrv )
{
	useStun = true;
	stunSrv = newStunSrv;
}

void CallAudio::startDTMF(char code)
{
	if (output) {
		output->startTone(code);
	}

	if (input) {
		input->startTone(code);
	}
}

void CallAudio::stopDTMF(void)
{
	if (output) {
		output->stopTone();
	}

	if (input) {
		input->stopTone();
	}
}


