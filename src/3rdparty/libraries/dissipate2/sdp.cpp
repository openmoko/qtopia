#include <stdio.h>
#include "sdp.h"
#include "sipdebug.h"


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
			QString m = body.mid( body.indexOf( "m=audio" ) + 7 );
			m = m.left( m.indexOf( "\n" ) );
			m = m.mid( m.indexOf( "RTP/AVP" ) + 7 );
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
			if( body.toLower().contains( "ilbc/8000" ) ) {
				ilbc = body.toLower().mid( body.toLower().indexOf( "ilbc/8000" ) - 7, 6 );
				if( ilbc.contains( ":" ) ) {
					ilbc = ilbc.mid( ilbc.indexOf( ":" ) + 1 );
				}
				ilbc = ilbc.simplified();
			}
			if( body.contains( "a=fmtp:" ) ) {
				m = body.mid( body.indexOf( "a=fmtp:" ) );
				m = m.left( m.indexOf( "\n" ) );
				rtpmode = "30";
				if( m.toLower().contains( "mode=20" ) ) {
					rtpmode = "20";
				}
			}
		}
		if( body.contains( "m=video" ) ) {
			QString m = body.mid( body.indexOf( "m=video" ) + 7 );
			m = m.left( m.indexOf( "\n" ) );
			m = m.mid( m.indexOf( "RTP/AVP" ) + 7 );
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
	if( input.left( input.indexOf( '\n' ) ) != "v=0" ) {
		return;
	}
	input.remove( 0, input.indexOf( '\n' ) + 1 );
	while( input.length() > 0 ) {
		curline = input.left( input.indexOf( '\n' ) );
		input.remove( 0, input.indexOf( '\n' ) + 1 );

		if( curline.left( curline.indexOf( '=' ) ) == "c" ) {
			ipaddress = curline.mid( curline.indexOf( "IP4" ) + 4 ).trimmed();
		}
		if( curline.left( curline.indexOf( "=audio" ) ) == "m" ) {
			portstr = curline.mid( curline.indexOf( "audio" ) + 5 );
			portstr = portstr.left( portstr.indexOf( "RTP" ) ).trimmed();

			// fix for ericsson
			if( portstr.contains( '/' ) ) {
				sip_printf( "SdpMessage: Uh-oh, ericsson is calling again\n" );
				portstr = portstr.left( portstr.indexOf( '/' ) ).trimmed();
				sip_printf( "SdpMessage: portstr is '%s'\n", portstr.toLatin1().constData() );
			}
			port = portstr.toUInt();
		}
		if( curline.left( curline.indexOf( "=video" ) ) == "m" ) {
			portstr = curline.mid( curline.indexOf( "video" ) + 5 );
			portstr = portstr.left( portstr.indexOf( "RTP" ) ).trimmed();

			// fix for ericsson
			if( portstr.contains( '/' ) ) {
				sip_printf( "SdpMessage: Uh-oh, ericsson is calling again\n" );
				portstr = portstr.left( portstr.indexOf( '/' ) ).trimmed();
				sip_printf( "SdpMessage: portstr is '%s'\n", portstr.toLatin1().constData() );
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
	return ( ipaddress.simplified() == "0.0.0.0" );
}

