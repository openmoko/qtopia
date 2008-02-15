//#include <qmessagebox.h>
#include <stdio.h>

#include "sipclient.h"
#include "sipcall.h"
#include "sipstatus.h"
#include "siptransaction.h"
#include "sipregister.h"
#include "sipuser.h"
#include "sipdebug.h"

SipRegister::SipRegister( SipUser *user, const SipUri &serveruri, int expires, QString qvalue )
{
	qValue = qvalue;
	autoregister = false;
	if( expires > 0 && expires < minValidTime ) {
		expires = minValidTime;
	} else if( expires <= 0) {
		expires = -1;
	}
	expiresTime = expires;
	rstate = NotConnected;
	regop = NoRegOpPending;
	authtype = DigestAuthenticationRequired;
	regUser = user;
	regUser->addServer( this );
	cleanRegister = false;
	timer = new QTimer( this );
	timer->setSingleShot( true );
	connect( timer, SIGNAL(timeout()), this, SLOT(register_timeout()) );
	regcall = new SipCall( user, QString::null, SipCall::RegisterCall );
	regcall->setSubject( "Register Call" );
	outboundProxy = "";
	if( !serveruri.getHostname().isEmpty() ) {
		regserver = SipUri( serveruri.getHostname() );
		setOutboundProxyUri( serveruri ); 
	} else {
		if( regserver.getHostname().isEmpty() ) {
			regserver = SipUri( regUser->getMyUri()->getHostname() );
		}
	}
}

SipRegister::~SipRegister( void )
{
	regUser->removeServer( this );
	delete regcall;
}

void SipRegister::setOutboundProxyUri( const SipUri &newregserver )
{
	outboundProxy = newregserver.proxyUri();
}

void SipRegister::requestRegister( const QString &username, const QString &password )
{
	if ( regop == NoRegOpPending ) {
		regop = RegRequest;
		if( !username.isEmpty() && !password.isEmpty() &&
				( rstate == AuthenticationRequired ||
				rstate == AuthenticationRequiredWithNewPassword ) ) {
			sip_printf( "SipRegister: Authentication required\n" );
			if( authtype == DigestAuthenticationRequired ) {
				authresponse = Sip::getDigestResponse(
					username, password, "REGISTER", regserver.uri(), authstr );
			} else if( authtype == BasicAuthenticationRequired ) {
				authresponse = Sip::getBasicResponse( username, password );
			} else if( authtype == ProxyDigestAuthenticationRequired ) {
				proxyauthresponse = Sip::getDigestResponse(
					username, password, "REGISTER", regserver.uri(), proxyauthstr );
			} else if( authtype == ProxyBasicAuthenticationRequired ) {
				proxyauthresponse = Sip::getBasicResponse( username, password );
			}
			rstate = TryingServerWithPassword;
		} else {
			rstate = TryingServer;
		}
		sip_printf( "SipRegister: Auth is '%s'\n", authresponse.toLatin1().constData() );
		sip_printf( "SipRegister: Proxy Auth is '%s'\n", proxyauthresponse.toLatin1().constData() );
		if( cleanRegister ) {
			regop = RegClear;
			curtrans = regcall->newRegister( regserver, 0, authresponse, proxyauthresponse );
		} else {
			curtrans = regcall->newRegister( regserver, expiresTime, authresponse, proxyauthresponse, qValue );
		}
		cleanRegister = false;
		connect( curtrans, SIGNAL(statusUpdated()), this, SLOT(localStatusUpdated()) );
	}
	statusUpdated();
}

void SipRegister::requestRegister( const SipUri & )
{
	statusUpdated();
}

void SipRegister::requestClearRegistration( void )
{
	if ( regop == NoRegOpPending ) {
		timer->stop();      /* reregistration */
		rstate = Disconnecting;
		regop = RegClear;
		cleanRegister = true;
		curtrans = regcall->newRegister( regserver, 0 );
		connect( curtrans, SIGNAL(statusUpdated()), this, SLOT(localStatusUpdated()) );
	}
	statusUpdated();
}

void SipRegister::localStatusUpdated( void )
{
	QString authstrtemp;
	SipStatus status;
	int validTime;
	if( !curtrans ) {
		sip_printf( "SipCallMember: Received what was likely a retransmission, badly ignoring...\n" );
		return;
	}
	if( curtrans->getStatus().getCode() >= 200 ) { // final response
		if( curtrans->getStatus().getCode() == 200 ) {
			switch ( regop ) {
				case RegRequest:
					rstate = Connected;
					status = curtrans->getStatus();
					validTime = status.getValidTime();
					sip_printf( "ReRegistrationTimer (ms): %d\n", validTime );
					timer->start( validTime );
					break;
				case RegClear:
					rstate = NotConnected;
					break;
				default:
					break;
			}
		} else if( curtrans->getStatus().getCode() == 401 ) {
			if (rstate == TryingServerWithPassword) {
				rstate = AuthenticationRequiredWithNewPassword;
			} else {
				rstate = AuthenticationRequired;
			}
			authstr = curtrans->getFinalWWWAuthString();
			authstrtemp = authstr.toLower();
			if( authstrtemp.contains( "digest" ) ) {
				authtype = DigestAuthenticationRequired;
			} else {
				authtype = BasicAuthenticationRequired;
			}
			disconnect( curtrans, 0, this, 0 );
			curtrans = 0;
		} else if( curtrans->getStatus().getCode() == 407 ) {
			if (rstate == TryingServerWithPassword) {
				rstate = AuthenticationRequiredWithNewPassword;
			} else {
				rstate = AuthenticationRequired;
			}
			proxyauthstr = curtrans->getFinalProxyAuthString();
			sip_printf( "SipRegister: Proxy auth string: '%s'\n", proxyauthstr.toLatin1().constData() );
			authstrtemp = proxyauthstr.toLower();
			if( authstrtemp.contains( "digest" ) ) {
				authtype = ProxyDigestAuthenticationRequired;
			} else {
				authtype = ProxyBasicAuthenticationRequired;
			}
			disconnect( curtrans, 0, this, 0 );
			curtrans = 0;
		} else if( curtrans->getStatus().getCode() >= 400 ) {
//			QMessageBox mb( "KPhone",
//				"Registration Failed: " + SipStatus::codeString( curtrans->getStatus().getCode() ),
//				QMessageBox::Information,
//				QMessageBox::Ok | QMessageBox::Default,0 ,0 );
//			mb.exec();
			rstate = NotConnected;
			
		} else {
			switch ( regop ) {
				case RegRequest:
					rstate = NotConnected;
					break;
				case RegClear:
					rstate = Connected;
					break;
				default:
					break;
			}
		}
		regop = NoRegOpPending;
	}
	statusUpdated();
}


QString SipRegister::getAuthRealm( void )
{
	QString realm;

	if( authstr.contains( "realm=\"" ) ) {
		realm = authstr.mid( authstr.indexOf( "realm=\"" ) + 7 );
		realm = realm.left( realm.indexOf( '\"' ) );
		return realm;
	}

	return QString::null;
}

QString SipRegister::getProxyAuthRealm( void )
{
	QString realm;

	if( proxyauthstr.contains( "realm=\"" ) ) {
		realm = proxyauthstr.mid( authstr.indexOf( "realm=\"" ) + 7 );
		sip_printf( "SipRegister: Realm is '%s'\n", realm.toLatin1().constData() );
		realm = realm.left( realm.indexOf( '\"' ) );
		return realm;
	}

	return QString::null;
}


void SipRegister::register_timeout()
{
	authresponse = QString::null;
	rstate = Reconnecting;
	requestRegister();
}

void SipRegister::updateRegister()
{
	authresponse = QString::null;
	rstate = Reconnecting;
	requestRegister();
}
