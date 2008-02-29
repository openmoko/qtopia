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
#include <stdio.h>
#include "sdp.h"


const SdpMessage SdpMessage::null;

SdpMessage::SdpMessage( void )
{
	valid = true;
	port = 0;
	videoPort = 0;
}

SdpMessage::SdpMessage( const QString &parseinput )
{
	parseInput( parseinput );
}

SdpMessage::~SdpMessage( void )
{
}

void SdpMessage::setIpAddress( const QString &newaddr )
{
	ipaddress = newaddr;
}

void SdpMessage::setPort( unsigned int newport )
{
	port = newport;
}

void SdpMessage::setVideoPort( unsigned int newport )
{
	videoPort = newport;
}

void SdpMessage::setName( const QString &newname )
{
	sessionname = newname;
}

QString SdpMessage::message( codecType c, codecType v, QString body ) const
{
	QString msg;

	// Version
	msg += "v=0\r\n";

	// Session Id
	msg += "o=username 0 0 IN IP4 " + ipaddress + "\r\n";

	// Session Name
	msg += "s=" + sessionname + "\r\n";

	// Connection Info
	msg += "c=IN IP4 " + ipaddress + "\r\n";

	// Time active
	msg += "t=0 0\r\n";

	// Media information
	QString ulaw = " 0";
	QString gsm  = " 3";
	QString pcma = " 8";  // alaw
	QString ilbc = "97";
	QString h261 = "31";
	QString h263 = "34";
	QString rtpmode = "30";
	if( c == codecILBC_20 ) {
		rtpmode = "20";
	}
	if( !body.isEmpty() ) {
		if( body.contains( "m=audio" ) ) {
			QString m = body.mid( body.find( "m=audio" ) + 7 );
			m = m.left( m.find( "\n" ) );
			m = m.mid( m.find( "RTP/AVP" ) + 7 );
			m += " ";
			if( !m.contains( " 0 " ) ) {
				ulaw = "";
			}
			if( !m.contains( " 3 " ) ) {
				gsm = "";
			}
			if( !m.contains( " 8 " ) ) {
				pcma = "";
			}
			ilbc = "";
			if( body.lower().contains( "ilbc/8000" ) ) {
				ilbc = body.lower().mid( body.lower().find( "ilbc/8000" ) - 7, 6 );
				if( ilbc.contains( ":" ) ) {
					ilbc = ilbc.mid( ilbc.find( ":" ) + 1 );
				}
				ilbc = ilbc.simplifyWhiteSpace();
			}
			if( body.contains( "a=fmtp:" ) ) {
				m = body.mid( body.find( "a=fmtp:" ) );
				m = m.left( m.find( "\n" ) );
				rtpmode = "30";
				if( m.lower().contains( "mode=20" ) ) {
					rtpmode = "20";
				}
			}
		}
		if( body.contains( "m=video" ) ) {
			QString m = body.mid( body.find( "m=video" ) + 7 );
			m = m.left( m.find( "\n" ) );
			m = m.mid( m.find( "RTP/AVP" ) + 7 );
			m += " ";
			if( !m.contains( " 31 " ) ) {
				h261 = "";
			}
			if( !m.contains( " 34 " ) ) {
				h263 = "";
			}
		}
	}
	switch( c ) {
		case codecGSM:
			msg += "m=audio " + QString::number( port ) + " RTP/AVP 3 " + ilbc + ulaw + pcma + "\r\n";
			break;
		case codecPCMA:
			msg += "m=audio " + QString::number( port ) + " RTP/AVP 8 " + ilbc + gsm + ulaw + "\r\n";
		case codecILBC_20:
		case codecILBC_30:
			msg += "m=audio " + QString::number( port ) + " RTP/AVP " + ilbc + gsm + ulaw + "\r\n";
			break;
		default:
#ifdef MESSENGER
			msg += "m=audio " + QString::number( port ) + " RTP/AVP 0 " + "\r\n";
#else
			msg += "m=audio " + QString::number( port ) + " RTP/AVP 0 " + ilbc + pcma + gsm + "\r\n";
#endif
	}
	if( !ulaw.isEmpty() ) {
		msg += "a=rtpmap:0 PCMU/8000\r\n";
		//more fixes for nortel
		//msg += "a=ptime:20\r\n";
	}
#ifdef MESSENGER
	if( c == codecGSM || c == codecPCMA || c == codecILBC_20 || c == codecILBC_30 ) {
		if( !gsm.isEmpty() ) {
			msg += "a=rtpmap:3 GSM/8000\r\n";
		}
		if( !pcma.isEmpty() ) {
			msg += "a=rtpmap:8 PCMA/8000\r\n";
		}
		if( !ilbc.isEmpty() ) {
			msg += "a=rtpmap:" + ilbc + " iLBC/8000\r\n";
			msg += "a=fmtp:" + ilbc + " mode=" + rtpmode + "\r\n";
		}
	}
#else
	if( !gsm.isEmpty() ) {
		msg += "a=rtpmap:3 GSM/8000\r\n";
	}
	if( !pcma.isEmpty() ) {
		msg += "a=rtpmap:8 PCMA/8000\r\n";
	}
	if( !ilbc.isEmpty() ) {
		msg += "a=rtpmap:" + ilbc + " iLBC/8000\r\n";
		msg += "a=fmtp:" + ilbc + " mode=" + rtpmode + "\r\n";
	}
#endif

	switch( v ) {
		case codecH261:
#ifdef MESSENGER
			msg += "m=video " + QString::number( videoPort ) + " RTP/AVP 31 " + "\r\n";
#else
			msg += "m=video " + QString::number( videoPort ) + " RTP/AVP 31 " + h263 + "\r\n";
#endif
			break;
		case codecH263:
			msg += "m=video " + QString::number( videoPort ) + " RTP/AVP 34 " + h261 + "\r\n";
			break;
		default:
			break;
	}
	if( v != codecUnknown ) {
		if( !h261.isEmpty() ) {
			msg += "a=rtpmap:31 H261/90000\r\n";
		}
#ifdef MESSENGER
		if( c == codecH263 )
#endif
		if( !h263.isEmpty() ) {
			msg += "a=rtpmap:34 H263/90000\r\n";
		}
	}

	return msg;
}

void SdpMessage::parseInput( const QString &parseinput )
{
	QString input = parseinput;
	QString curline;
	QString portstr;
	QString codecstr;
	valid = false;
	port = 0;
	videoPort = 0;
	ipaddress = QString::null;

	// Guarentee termination
	input += '\n';
	if( input.left( input.find( '\n' ) ) != "v=0" ) {
		return;
	}
	input.remove( 0, input.find( '\n' ) + 1 );
	while( input.length() > 0 ) {
		curline = input.left( input.find( '\n' ) );
		input.remove( 0, input.find( '\n' ) + 1 );

		if( curline.left( curline.find( '=' ) ) == "c" ) {
			ipaddress = curline.mid( curline.find( "IP4" ) + 4 ).stripWhiteSpace();
		}
		if( curline.left( curline.find( "=audio" ) ) == "m" ) {
			portstr = curline.mid( curline.find( "audio" ) + 5 );
			portstr = portstr.left( portstr.find( "RTP" ) ).stripWhiteSpace();

			// fix for ericsson
			if( portstr.contains( '/' ) ) {
				printf( "SdpMessage: Uh-oh, ericsson is calling again\n" );
				portstr = portstr.left( portstr.find( '/' ) ).stripWhiteSpace();
				printf( "SdpMessage: portstr is '%s'\n", portstr.latin1() );
			}
			port = portstr.toUInt();
		}
		if( curline.left( curline.find( "=video" ) ) == "m" ) {
			portstr = curline.mid( curline.find( "video" ) + 5 );
			portstr = portstr.left( portstr.find( "RTP" ) ).stripWhiteSpace();

			// fix for ericsson
			if( portstr.contains( '/' ) ) {
				printf( "SdpMessage: Uh-oh, ericsson is calling again\n" );
				portstr = portstr.left( portstr.find( '/' ) ).stripWhiteSpace();
				printf( "SdpMessage: portstr is '%s'\n", portstr.latin1() );
			}
			videoPort = portstr.toUInt();
		}
	}

	valid = true;
}

bool SdpMessage::operator==( const SdpMessage &m ) const
{
	return ( ( ipaddress == m.ipaddress ) && ( port == m.port ) );
}

bool SdpMessage::operator!=( const SdpMessage &m ) const
{
	return !( *this == m );
}

SdpMessage &SdpMessage::operator=( const SdpMessage &m )
{
	valid = m.valid;
	sessionname = m.sessionname;
	ipaddress = m.ipaddress;
	port = m.port;
	videoPort = m.videoPort;

	return *this;
}

bool SdpMessage::isOnHold( void ) const
{
	return ( ipaddress.simplifyWhiteSpace() == "0.0.0.0" );
}

