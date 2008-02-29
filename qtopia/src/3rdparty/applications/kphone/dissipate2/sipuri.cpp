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
#include <stdlib.h>
#include <qregexp.h>

#include "sipuri.h"

const SipUri SipUri::null;

SipUri::SipUri( void )
{
	clear();
}

SipUri::SipUri( const QString &parseinput )
{
	clear();
	parseUri( parseinput );
}

SipUri::~SipUri( void )
{
}

void SipUri::clear( void )
{
	protocolname = "sip";
	isvalid = true;
	islooserouting = false;
	haslr = false;
	hasftag = false;
	hasuserinfo = false;
	hasproxyusername = false;
	haspassword = false;
	port = getSipPort();
	hastransparam = false;
	hasuserparam = false;
	hasmethodparam = false;
	hasttlparam = false;
	ttl = 0;
	hasmaddrparam = false;
	hastag = false;
	hasport = false;
	hasq = false;
}

void SipUri::parseUri( const QString &parseinput )
{
	QString inputline;
	QString userinfo;
	QString hostport;
	QString parameters;

	isvalid = false;

	inputline = parseinput.simplifyWhiteSpace();

	// Squeeze out blah<uri>blah
	if( inputline.contains( '<' ) ) {

		// Validity check (possibly bogus)
		if( !inputline.contains( '>' ) ) {
			return;
		}

		// Parse out the squeezed uri
		QString beforeangle = inputline.left( inputline.find( '>' ) );
		parseUri( beforeangle.mid( beforeangle.find( '<' ) + 1 ) );

		// Parse the fullname, if any
		setFullname( inputline.left( inputline.find( '<' ) ) );

		// Parse the parameters, if any
		parseParameters( inputline.mid( inputline.find( '>' ) + 1 ), false );

		return;
	}
	if( inputline.left( 4 ).lower() == "sip:" ) {
		setProtocolName( "sip" );
		inputline.remove( 0, 4 );
	}
	if( inputline.left( 4 ).lower() == "tel:" ) {
		setProtocolName( "tel" );
		inputline.remove( 0, 4 );
	}

	// If we have user info
	if( inputline.contains( '@' ) ) {
		userinfo = inputline.left( inputline.find( '@' ) );
		if( userinfo.contains( ':' ) ) {
			setUsername( userinfo.left( userinfo.find( ':' ) ) );
			setPassword( userinfo.mid( userinfo.find( ':' ) + 1 ) );
		} else {
			setUsername( userinfo );
		}
		inputline.remove( 0, inputline.find( '@' ) + 1 );
	}

	// If we have any parameters
	if( inputline.contains( ';' ) ) {
		hostport = inputline.left( inputline.find( ';' ) ).stripWhiteSpace();
		inputline.remove( 0, inputline.find( ';' ) );
	} else if( inputline.contains( '?' ) ) {
		hostport = inputline.left( inputline.find( '?' ) ).stripWhiteSpace();
		inputline.remove( 0, inputline.find( '?' ) );
	} else {
		hostport = inputline;
		inputline = QString::null;
	}

	if( hostport.contains( ':' ) ) {
		setHostname( hostport.left( hostport.find( ':' ) ) );
		setPortNumber( hostport.mid( hostport.find( ':' ) + 1 ).toUInt() );
		hasport = true;
	} else {
		setHostname( hostport );
		setPortNumber( getSipPort() );
	}
	if( protocolname != "tel" ) {
		if( !hostname.contains( "." ) ) {
			return;
		}
	}
	if( inputline.contains( '?' ) ) {
		parameters = inputline.left( inputline.find( '?' ) );
		inputline.remove( 0, inputline.find( '?' ) + 1 );
	} else {
		parameters = inputline;
		inputline = QString::null;
	}

	// Parse the parameters at the end
	parseParameters( parameters, true );

	// If we make it this far
	isvalid = true;
}

void SipUri::parseParameters( const QString &parseinput, bool beforeangle )
{
	QString curparm;
	QString parmname;
	QString parmvalue;
	QString parameters;

	// NOTE: Calling this function will never change validity of the URI

	parameters = parseinput.simplifyWhiteSpace();

	while( parameters[ 0 ] == ';' ) {
		curparm = parameters.mid( 1, parameters.find( ';', 1 ) - 1 ).stripWhiteSpace();
		parameters.remove( 0, parameters.find( ';', 1 ) );
		parmname = curparm.left( curparm.find( '=' ) ).lower().stripWhiteSpace();
		if( curparm.contains( '=' ) ) {
			parmvalue = curparm.mid( curparm.find( '=' ) + 1 ).stripWhiteSpace();
		} else {
			parmvalue = QString::null;
		}

		// Check if the URI has a tag
		if( parmname.compare( "tag" ) == 0 ) {
			hastag = true;
			tag = parmvalue;
		}

		// maddr param
		else if( parmname.compare( "maddr" ) == 0 ) {
			hasmaddrparam = true;
			maddrhostname = parmvalue.lower();
		}

		// ttl param
		else if( parmname.compare( "ttl" ) == 0 ) {
			hasttlparam = true;
			ttl = parmvalue.toUInt();
		}

		// transport param
		else if( parmname.compare( "transport" ) == 0 ) {
			if( parmvalue.lower() == "udp" ) {
				transparam = UDP;
				hastransparam = true;
			} else if( parmvalue.lower() == "tcp" ) {
				transparam = TCP;
				hastransparam = true;
			}
		}

		// Loose Routing param
		else if( parmname.compare( "lr" ) == 0 ) {
			islooserouting = true;
			lr = parmvalue;
			haslr = !lr.isEmpty();
		}

		// ftag
		else if( parmname.compare( "ftag" ) == 0 ) {
			hasftag = true;
			ftag = parmvalue;
		}
		
		// q
		else if( parmname.compare( "q" ) == 0 ) {
			hasq = true;
			qValue = parmvalue;
		}
		
		//save the rest of params.
		else if( beforeangle ) {
			rOfParamsUri.append( SimpleParameter(parmname,parmvalue));
		} else {
			rOfParams.append( SimpleParameter(parmname,parmvalue));
		}
	}
}

void SipUri::setHostname( const QString &hname )
{
	hostname = hname.lower().stripWhiteSpace();
}

const QString SipUri::getUserParamString( UserParam u )
{
	switch ( u ) {
		case Phone: return "phone";
		case IP: return "ip";
		case NoUserParam: return "BAD";
	};
	return QString::null;
}

const QString SipUri::getTransportParamString( TransportParam t )
{
	switch ( t ) {
		case UDP: return "udp";
		case TCP: return "tcp";
		case NoTransportParam: return "BAD"; // Should return udp
	};
	return QString::null;
}

QString SipUri::user( void ) const
{
	QString usertext;
	if ( hasUserInfo() ) {
		usertext += username + "@";
		if ( hasPassword() ) {
			usertext += password;
		}
	}
	usertext += hostname;
	return usertext;
}

QString SipUri::nameAddr( void ) const
{
	QString nameaddr;
	if( fullname.length() > 0 ) {
		nameaddr = "\"" + fullname + "\" ";
	}
	nameaddr += "<" + uri() + ">";
	if( hasTag() ) {
		nameaddr += ";tag=" + getTag();
	}
	ParameterListConstIterator it;
	for(it=rOfParams.begin();it != rOfParams.end();++it) {
		nameaddr += ";";
		nameaddr += (*it).getName();
		nameaddr += "=";
		nameaddr += (*it).getValue();
	}
	return nameaddr;
}

QString SipUri::nameAddr_noTag( void ) const
{
	QString nameaddr;

	if( fullname.length() > 0 ) {
		nameaddr = "\"" + fullname + "\" ";
	}
	nameaddr += "<" + uri() + ">";
	ParameterListConstIterator it;
	for(it=rOfParams.begin();it != rOfParams.end();++it) {
		nameaddr += ";";
		nameaddr += (*it).getName();
		nameaddr += "=";
		nameaddr += (*it).getValue();
	}
	return nameaddr;
}

QString SipUri::proxyUri( void ) const
{
	QString uritext;
	uritext = hostname;
	if( hasport || port != getSipPort() ) {
		uritext += ":" + QString::number( port );
	}

	return uritext;
}

QString SipUri::uri( void ) const
{
	QString uritext;
	if( hasUserInfo() ) {
		uritext = username;
		uritext += "@";
	}
	uritext += hostname;
	if( hasport || port != getSipPort() ) {
		uritext += ":" + QString::number( port );
	}

	if( hasTransportParam() ) {
		uritext += ";transport=" + getTransportParamString( transparam );
	}
	if( hasUserParam() ) {
		uritext += ";user=" + getUserParamString( userparam );
	}
	if( hasMethodParam() ) {
		uritext += ";method=" + Sip::getMethodString( meth );
	}
	if( hasTtlParam() ) {
		uritext += ";ttl=" + QString::number( (int) ttl );
	}
	if( hasMaddrParam() ) {
		uritext += ";maddr=" + maddrhostname;
	}
	if( hasFtag() ) {
		uritext += ";ftag=" + ftag;
	}
	if( isLooseRouting() ) {
		uritext += ";lr";
		if( haslrparam() ) {
			uritext += "=" + lr;
		}
	}

	if( !uritext.isEmpty() ) {
		uritext = getProtocolName() + ":" + uritext;
	}

	//add the rest of params to the generated uri
	ParameterListConstIterator it;
	for(it=rOfParamsUri.begin();it != rOfParamsUri.end();++it) {
		uritext += ";";
		uritext += (*it).getName();
		if( !(*it).getValue().isEmpty() ) {
			uritext += "=";
			uritext += (*it).getValue();
		}
	}

	return uritext;
}

QString SipUri::reqUri( void ) const
{
	QString uritext;
	uritext = getProtocolName() + ":";
	if( hasUserInfo() ) {
		uritext += username + "@";
	}
	uritext += hostname;
	if( hasport || port != getSipPort() ) {
		uritext += ":" + QString::number( port );
	}
	if( hasMaddrParam() ) {
		uritext += ";maddr=" + maddrhostname;
	}
	if( hasTransportParam() ) {
		uritext += ";transport=" + getTransportParamString( transparam );
	}

	//add the rest of params to the generated uri
	ParameterListConstIterator it;
	for(it=rOfParamsUri.begin();it != rOfParamsUri.end();++it) {
		uritext += ";";
		uritext += (*it).getName();
		if( !(*it).getValue().isEmpty() ) {
			uritext += "=";
			uritext += (*it).getValue();
		}
	}

	return uritext;
}

QString SipUri::getRouteUri( void ) const
{
	QString uritext;
	uritext = "<" + getProtocolName() + ":";
	uritext += hostname;
	uritext += ">";
	return uritext;
}

QString SipUri::getRegisterUri( void ) const
{
	QString uritext;
	uritext = getProtocolName() + ":";
	uritext += hostname;
	return uritext;
}

void SipUri::setUsername( const QString &u )
{
	if ( u.isNull() ) {
		hasuserinfo = false;
	} else {
		hasuserinfo = true;
		username = u;
	}
}

void SipUri::setProxyUsername( const QString &u )
{
	if ( u.isNull() ) {
		hasproxyusername = false;
	} else {
		hasproxyusername = true;
		proxyusername = u;
	}
}

void SipUri::setPassword( const QString &p )
{
	if ( p.isNull() ) {
		haspassword = false;
	} else {
		haspassword = true;
		password = p;
	}
}

void SipUri::setPortNumber( unsigned int p )
{
	port = p;
}

void SipUri::setTransportParam( TransportParam t )
{
	if ( t == NoTransportParam ) {
		hastransparam = false;
	} else {
		hastransparam = true;
		transparam = t;
	}
}

void SipUri::setUserParam( UserParam u )
{
	if ( u == NoUserParam ) {
		hasuserparam = false;
	} else {
		hasuserparam = true;
		userparam = u;
	}
}

void SipUri::setMethodParam( Sip::Method m )
{
	if ( m == Sip::BadMethod ) {
		hasmethodparam = false;
	} else {
		hasmethodparam = true;
		meth = m;
	}
}

void SipUri::setTtl( unsigned char t )
{
	ttl = t;
}

void SipUri::setFullname( const QString &newfname )
{
	fullname = newfname;
	fullname = fullname.replace( QRegExp( "\"" ), QString::null ).simplifyWhiteSpace();
}

void SipUri::generateTag( void )
{
	int uniqid = rand();
	tag.sprintf( "%X", uniqid );
	hastag = true;
}

bool SipUri::operator==( const SipUri &uri ) const
{
	if( !isValid() || !uri.isValid() ) {
		return false;
	}
	if( username == uri.username && /* password == uri.password && */
		hostname == uri.hostname && port == uri.port ) {
		return true;
	}
	return false;
}

bool SipUri::operator==( const QString &url ) const
{
	SipUri u( url );
	return ( *this == u );
}

bool SipUri::operator!=( const SipUri &uri ) const
{
	return !( *this == uri );
}

bool SipUri::operator!=( const QString &url ) const
{
	return !( *this == url );
}

SipUri &SipUri::operator=( const SipUri &uri )
{
	clear();
	protocolname = uri.protocolname;
	fullname = uri.fullname;
	hasuserinfo = uri.hasuserinfo;
	username = uri.username;
	hasproxyusername = uri.hasproxyusername;
	proxyusername = uri.proxyusername;
	haspassword = uri.haspassword;
	password = uri.password;
	port = uri.port;
	isvalid = uri.isvalid;
	hastransparam = uri.hastransparam;
	transparam = uri.transparam;
	hasuserparam = uri.hasuserparam;
	userparam = uri.userparam;
	hasmethodparam = uri.hasmethodparam;
	meth = uri.meth;
	hasttlparam = uri.hasttlparam;
	maddrhostname = uri.maddrhostname;
	hasmaddrparam = uri.hasmaddrparam;
	hastag = uri.hastag;
	tag = uri.tag;
	hostname = uri.hostname;
	hasport = uri.hasport;
	ftag = uri.ftag;

//copy rest of params
	rOfParams = uri.rOfParams;
	rOfParamsUri = uri.rOfParamsUri;

	return *this;
}

void SipUri::setTag( const QString &newtag )
{
	tag = newtag;
	if( tag == QString::null ) {
		hastag = false;
	} else {
		hastag = true;
	}
}

void SipUri::setMaddrParam( const QString &newmaddr )
{
	if( newmaddr != QString::null ) {
		maddrhostname = newmaddr;
		hasmaddrparam = true;
	} else {
		hasmaddrparam = false;
		maddrhostname = QString::null;
	}
}

void SipUri::updateUri( const QString &u )
{
	parseUri( u );
}
