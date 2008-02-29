#include <stdio.h>
#include <sys/time.h>

#include "sipuri.h"
#include "sipuser.h"
#include "sipclient.h"
#include "sipmessage.h"
#include "siptransaction.h"
#include "sipcall.h"
#include "sipdebug.h"

#include <qalgorithms.h>


SipCallMember::SipCallMember( SipCall *parent, const SipUri &uri )
{
	authstate = authState_Authentication;
	callMemberType = unknown;
	call = parent;
	memberuri = uri;
	contacturi = uri;
	local = 0;
	remote = 0;
	localExpiresTime = -1;
	state = state_Disconnected;
	parent->addMember( this );
	timer = new QTimer( this );
	timer->setSingleShot( true );
	connect( timer, SIGNAL(timeout()), this, SLOT(call_timeout()) );
}

SipCallMember::~SipCallMember( void )
{
	delete timer;
}

void SipCallMember::setContactUri( const SipUri &newcontact )
{
	contacturi = newcontact;
}

void SipCallMember::setUri( const SipUri &newuri )
{
	memberuri = newuri;
}

// Subscribe

void SipCallMember::requestSubscribe( int expiresTime, const QString &body, const MimeContentType &bodytype )
{
	callMemberType = Subscribe;
	state = state_Idle;
	if( expiresTime > 0 ) {
		localExpiresTime = expiresTime;
		localsessiondesc = body;
		localsessiontype = bodytype;
	}
	sendRequestSubscribe();
}

void SipCallMember::sendRequestSubscribe( QString username, QString password )
{
	qDeleteAll(call->transactions.begin(), call->transactions.end());
	call->transactions.clear();
	if( !username.isEmpty() && !password.isEmpty() &&
			( authstate == authState_AuthenticationRequired ||
			authstate == authState_AuthenticationRequiredWithNewPassword ) ) {
		if( authtype == ProxyDigestAuthenticationRequired ) {
			proxyauthresponse = Sip::getDigestResponse(
				username, password, "SUBSCRIBE", getContactUri().uri(), proxyauthstr );
		} else if( authtype == ProxyBasicAuthenticationRequired ) {
			proxyauthresponse = Sip::getBasicResponse( username, password );
		}
		local = call->newRequest( this, Sip::SUBSCRIBE, localsessiondesc, localsessiontype,
			SipUri::null, proxyauthresponse, localExpiresTime );
	} else {
		local = call->newRequest( this, Sip::SUBSCRIBE, localsessiondesc, localsessiontype,
			SipUri::null, QString::null, localExpiresTime );
	}
	if( localExpiresTime > 0 ) {
		timer->start( localExpiresTime * 900 );
	}
	if( local ) {
		op = opRequest;
		if( authstate == authState_AuthenticationRequired ||
		    authstate == authState_AuthenticationRequiredWithNewPassword ) {
			authstate = authState_AuthenticationTryingWithPassword;
		} else {
			authstate = authState_AuthenticationTrying;
		}
		connect( local, SIGNAL(statusUpdated()), this, SLOT(localStatusUpdated()) );
	}
}

void SipCallMember::requestAuthSubscribe( void )
{
	statusUpdated( this );
}

void SipCallMember::requestClearSubscribe( void )
{
	localExpiresTime = 0;
	call->setCallStatus( SipCall::callDisconneting );
	sendRequestSubscribe();
}

void SipCallMember::handlingSubscribeResponse( void )
{
	QString authstrtemp;
	if( local->wasCancelled() ) {
		state = state_Disconnected;
		op = opIdle;
		local = 0;
	} else if( local->getStatus().getCode() >= 200 ) {  // final response
		if( local->getStatus().getCode() < 300 ) {
			if( call->getCallStatus() == SipCall::callDisconneting ) {
				call->setCallStatus( SipCall::callUnconnected );
			} else {
				call->setCallStatus( SipCall::callInProgress );
			}
			authstate = authState_AuthenticationOK;
		} else if( local->getStatus().getCode() == 401) {
			proxyauthstr = local->getFinalWWWAuthString();
			authstrtemp = proxyauthstr.toLower();
			if( authstrtemp.contains( "digest" ) ) {
				authtype = ProxyDigestAuthenticationRequired;
			} else {
				authtype = ProxyBasicAuthenticationRequired;
			}
			if( authstate == authState_AuthenticationTryingWithPassword ) {
				authstate = authState_AuthenticationRequiredWithNewPassword;
			} else {
				authstate = authState_AuthenticationRequired;
			}
			requestAuthSubscribe();

			return;

		} else if( local->getStatus().getCode() == 407) {
			proxyauthstr = local->getFinalProxyAuthString();
			authstrtemp = proxyauthstr.toLower();
			if( authstrtemp.contains( "digest" ) ) {
				authtype = ProxyDigestAuthenticationRequired;
			} else {
				authtype = ProxyBasicAuthenticationRequired;
			}
			if( authstate == authState_AuthenticationTryingWithPassword ) {
				authstate = authState_AuthenticationRequiredWithNewPassword;
			} else {
				authstate = authState_AuthenticationRequired;
			}
			requestAuthSubscribe();

			return;

		} else if( local->getStatus().getCode() < 500 ) {
			if( call->getCallStatus() == SipCall::callInProgress ) {
				contactUpdate( false );
				call->updateSubscribes();
			}
		}
		op = opIdle;
		local = 0;
	} else {
		state = state_EarlyDialog;
	}
}

// Notify

void SipCallMember::requestNotify( const QString &body, const MimeContentType &bodytype )
{
	callMemberType = Notify;
	localExpiresTime = -1;
	localsessiondesc = body;
	localsessiontype = bodytype;
	sendRequestNotify();
}

void SipCallMember::sendRequestNotify( QString username, QString password )
{
	qDeleteAll(call->transactions.begin(), call->transactions.end());
	call->transactions.clear();
	if( !username.isEmpty() && !password.isEmpty() &&
	    ( authstate == authState_AuthenticationRequired ||
	    authstate == authState_AuthenticationRequiredWithNewPassword ) ) {
		if( authtype == ProxyDigestAuthenticationRequired ) {
			proxyauthresponse = Sip::getDigestResponse(
				username, password, "NOTIFY", getContactUri().uri(), proxyauthstr );
		} else if( authtype == ProxyBasicAuthenticationRequired ) {
			proxyauthresponse = Sip::getBasicResponse( username, password );
		}
		local = call->newRequest( this, Sip::NOTIFY, localsessiondesc, localsessiontype,
			SipUri::null, proxyauthresponse, localExpiresTime );
	} else {
		local = call->newRequest( this, Sip::NOTIFY, localsessiondesc, localsessiontype,
			SipUri::null, QString::null, localExpiresTime );
	}
	if( local ) {
		op = opRequest;
		if( authstate == authState_AuthenticationRequired ||
		    authstate == authState_AuthenticationRequiredWithNewPassword ) {
			authstate = authState_AuthenticationTryingWithPassword;
		} else {
			authstate = authState_AuthenticationTrying;
		}
		connect( local, SIGNAL(statusUpdated()), this, SLOT(localStatusUpdated()) );
	}
}

void SipCallMember::requestAuthNotify( void )
{
	statusUpdated( this );
}

void SipCallMember::requestClearNotify( const  QString &body, const  MimeContentType &bodytype )
{
	callMemberType = Notify;
	localExpiresTime = 0;
	localsessiondesc = body;
	localsessiontype = bodytype;
	sendRequestNotify();
}

void SipCallMember::handlingNotifyResponse( void )
{
	QString authstrtemp;
	if( local->wasCancelled() ) {
		state = state_Disconnected;
		op = opIdle;
		local = 0;
	} else if( local->getStatus().getCode() >= 200 ) {  // final response
		if( local->getStatus().getCode() < 300 ) {
			call->setCallStatus( SipCall::callInProgress );
			local = 0;
		} else {
			if( local->getStatus().getCode() == 407) {
				proxyauthstr = local->getFinalProxyAuthString();
				authstrtemp = proxyauthstr.toLower();
				if( authstrtemp.contains( "digest" ) ) {
					authtype = ProxyDigestAuthenticationRequired;
				} else {
					authtype = ProxyBasicAuthenticationRequired;
				}
				if( authstate == authState_AuthenticationTryingWithPassword ) {
					authstate = authState_AuthenticationRequiredWithNewPassword;
				} else {
					authstate = authState_AuthenticationRequired;
				}
				requestAuthNotify();

				return;

			} else if( local->getStatus().getCode() == 401) {
				proxyauthstr = local->getFinalWWWAuthString();
				authstrtemp = proxyauthstr.toLower();
				if( authstrtemp.contains( "digest" ) ) {
					authtype = ProxyDigestAuthenticationRequired;
				} else {
					authtype = ProxyBasicAuthenticationRequired;
				}
				if( authstate == authState_AuthenticationTryingWithPassword ) {
					authstate = authState_AuthenticationRequiredWithNewPassword;
				} else {
					authstate = authState_AuthenticationRequired;
				}
				requestAuthNotify();

				return;

			} else if( local->getStatus().getCode() == 488) {
			} else if( local->getStatus().getCode() < 500 ) {
				state = state_Disconnected;
				call->setCallStatus( SipCall::callDead );
			}
		}
		op = opIdle;
		local = 0;
	} else {
		state = state_EarlyDialog;
	}
}

// Message

void SipCallMember::requestMessage( const QString &body, const MimeContentType &bodytype )
{
	callMemberType = Message;
	localsessiondesc = body;
	localsessiontype = bodytype;
	sendRequestMessage();
}

void SipCallMember::sendRequestMessage( QString username, QString password )
{
	if( !username.isEmpty() && !password.isEmpty() &&
	    ( authstate == authState_AuthenticationRequired ||
	    authstate == authState_AuthenticationRequiredWithNewPassword ) ) {
		if( authtype == ProxyDigestAuthenticationRequired ) {
			proxyauthresponse = Sip::getDigestResponse(
				username, password, "MESSAGE", getContactUri().uri(), proxyauthstr );
		} else if( authtype == ProxyBasicAuthenticationRequired ) {
			proxyauthresponse = Sip::getBasicResponse( username, password );
		}
		local = call->newRequest( this, Sip::MESSAGE, localsessiondesc, localsessiontype,
			SipUri::null, proxyauthresponse, localExpiresTime );
	} else {
		local = call->newRequest( this, Sip::MESSAGE, localsessiondesc, localsessiontype,
			SipUri::null, QString::null, localExpiresTime );
	}
	if( localExpiresTime > 0 ) {
		timer->start( localExpiresTime * 900 );
	}
	if( local ) {
		op = opRequest;
		if( authstate == authState_AuthenticationRequired ||
		    authstate == authState_AuthenticationRequiredWithNewPassword ) {
			authstate = authState_AuthenticationTryingWithPassword;
		} else {
			authstate = authState_AuthenticationTrying;
		}
		connect( local, SIGNAL(statusUpdated()), this, SLOT(localStatusUpdated()) );
	}
}

void SipCallMember::requestAuthMessage( void )
{
	statusUpdated( this );
}

void SipCallMember::handlingMessageResponse( void )
{
	QString authstrtemp;
	if( local->wasCancelled() ) {
		state = state_Disconnected;
		op = opIdle;
		local = 0;
	} else if( local->getStatus().getCode() >= 200 ) {  // final response
		if( local->getStatus().getCode() < 300 ) {
		} else {
			if(local->getStatus().getCode() == 407){
				proxyauthstr = local->getFinalProxyAuthString();
				authstrtemp = proxyauthstr.toLower();
				if( authstrtemp.contains( "digest" ) ) {
					authtype = ProxyDigestAuthenticationRequired;
				} else {
					authtype = ProxyBasicAuthenticationRequired;
				}
				if( authstate == authState_AuthenticationTryingWithPassword ) {
					authstate = authState_AuthenticationRequiredWithNewPassword;
				} else {
					authstate = authState_AuthenticationRequired;
				}
				requestAuthMessage();

				return;

			} else {
				state = state_Disconnected;
			}
		}
		op = opIdle;
		local = 0;
	} else {
		state = state_EarlyDialog;
	}
}

// Invite

void SipCallMember::requestInvite( const QString &body, const MimeContentType &bodytype )
{
	callMemberType = Invite;
	localsessiondesc = body;
	localsessiontype = bodytype;
	state = state_InviteRequested;
	sendRequestInvite();
}

void SipCallMember::sendRequestInvite( QString username, QString password )
{
	if( !username.isEmpty() && !password.isEmpty() &&
	    ( authstate == authState_AuthenticationRequired ||
	    authstate == authState_AuthenticationRequiredWithNewPassword ) ) {
		if( authtype == ProxyDigestAuthenticationRequired ) {
			proxyauthresponse = Sip::getDigestResponse(
				username, password, "INVITE", getContactUri().uri(), proxyauthstr );
		} else if( authtype == ProxyBasicAuthenticationRequired ) {
			proxyauthresponse = Sip::getBasicResponse( username, password );
		}
		local = call->newRequest( this, Sip::INVITE, localsessiondesc, localsessiontype,
			SipUri::null, proxyauthresponse, localExpiresTime );
	} else {
		local = call->newRequest( this, Sip::INVITE, localsessiondesc, localsessiontype,
			SipUri::null, QString::null, localExpiresTime );
	}
	if( localExpiresTime > 0 ) {
		timer->start( localExpiresTime * 900 );
	}
	if( local ) {
		op = opRequest;
		if( authstate == authState_AuthenticationRequired ||
		    authstate == authState_AuthenticationRequiredWithNewPassword ) {
			authstate = authState_AuthenticationTryingWithPassword;
		} else {
			authstate = authState_AuthenticationTrying;
		}
		connect( local, SIGNAL(statusUpdated()), this, SLOT(localStatusUpdated()) );
	}
}

void SipCallMember::requestAuthInvite( void )
{
	statusUpdated( this );
}

void SipCallMember::handlingInviteResponse( void )
{
	QString authstrtemp;

	if( state == state_Connected ) {
		if( local->getStatus().getCode() >= 200 ) {
			if( local->getStatus().getCode() == 200 ) {
				statusdesc = tr("Response: ") + tr(local->getStatus().getReasonPhrase().toLatin1().constData());
				statusUpdated( this );
			} else if( local->getStatus().getCode() == 407 ) {
				proxyauthstr = local->getFinalProxyAuthString();
				authstrtemp = proxyauthstr.toLower();
				if( authstrtemp.contains( "digest" ) ) {
					authtype = ProxyDigestAuthenticationRequired;
				} else {
					authtype = ProxyBasicAuthenticationRequired;
				}
 				requestAuthInvite();

				return;
			} else {
				state = state_Disconnected;
				statusdesc = tr("Response: ") + tr(local->getStatus().getReasonPhrase().toLatin1().constData());
			}
			op = opIdle;
			local = 0;
		}
	} else if( state == state_Disconnecting ) {
		if( local->getStatus().getCode() >= 200 ) {
			if( local->getStatus().getCode() == 407 ) {
				proxyauthstr = local->getFinalProxyAuthString();
				authstrtemp = proxyauthstr.toLower();
				if( authstrtemp.contains( "digest" ) ) {
					authtype = ProxyDigestAuthenticationRequired;
				} else {
					authtype = ProxyBasicAuthenticationRequired;
				}
 				requestDisconnect();

				return;

			} else {
				state = state_Disconnected;
				statusdesc = tr("Response: ") + tr(local->getStatus().getReasonPhrase().toLatin1().constData());
			}
			op = opIdle;
			local = 0;
		}
	} else if( state == state_CancelPending ) {
		if( local->getStatus().getCode() == 200 ) {
			local = 0;
			requestDisconnect();
		} else if( local->getStatus().getCode() > 100 ) {
			local->cancelRequest();
			local = 0;
		}
		state = state_Disconnected;
	} else {
		if( local->wasCancelled() ) {
			state = state_Disconnected;
			op = opIdle;
			local = 0;
		} else if( local->getStatus().getCode() >= 200 ) {  // final response
			if( local->getStatus().getCode() < 300 ) {
				call->setCallStatus( SipCall::callInProgress );
				state = state_Connected;
				statusdesc = tr("Connected: ") + tr(local->getStatus().getReasonPhrase().toLatin1().constData());
			} else if( local->getStatus().getCode() < 400 ) {
				state = state_Redirected;
				statusdesc = tr("Redirected: ") + tr(local->getStatus().getReasonPhrase().toLatin1().constData());
				redirectlist = local->getFinalContactList();
			} else {
				if(local->getStatus().getCode() == 407){
					proxyauthstr = local->getFinalProxyAuthString();
					authstrtemp = proxyauthstr.toLower();
					if( authstrtemp.contains( "digest" ) ) {
						authtype = ProxyDigestAuthenticationRequired;
					} else {
						authtype = ProxyBasicAuthenticationRequired;
					}
					if( authstate == authState_AuthenticationTryingWithPassword ||
					    authstate == authState_AuthenticationRequiredWithNewPassword ) {
						authstate = authState_AuthenticationRequiredWithNewPassword;
					} else {
						authstate = authState_AuthenticationRequired;
					}
					requestAuthInvite();

					return;

				} else {
					state = state_Disconnected;
					statusdesc = "!!" + tr("Call Failed: ") + tr(local->getStatus().getReasonPhrase().toLatin1().constData());
				}
			}
			op = opIdle;
			local = 0;
		} else {
			statusdesc = tr(local->getStatus().getReasonPhrase().toLatin1().constData());
			if( local->getStatus().getCode() != 100 ) {
				state = state_EarlyDialog;
			}
		}
	}
}

// Bye

void SipCallMember::requestDisconnect( void )
{
	if( local ) {
		local->cancelRequest();
		disconnect( local, 0, this, 0 );
	}
	state = state_Disconnecting;
	statusdesc = tr("Disconnecting");
	local = call->newRequest( this, Sip::BYE );
	if( local ) {
		connect( local, SIGNAL(statusUpdated()), this, SLOT(localStatusUpdated()) );
	}
}

void SipCallMember::requestTransfer( const SipUri &transferto,
	const QString &body, const MimeContentType &bodytype )
{
	if( state == state_Disconnected ) return;
	state = state_Disconnecting;
	statusdesc = tr("Transfering");
	local = call->newRequest( this, Sip::REFER, body, bodytype, transferto );
	connect( local, SIGNAL(statusUpdated()), this, SLOT(localStatusUpdated()) );
	state = state_Disconnecting;
	statusdesc = tr("Disconnecting");
	local = call->newRequest( this, Sip::BYE );
	if( local ) {
		connect( local, SIGNAL(statusUpdated()), this, SLOT(localStatusUpdated()) );
	}
	statusUpdated( this );
}

void SipCallMember::requestOptions( const QString &body, const MimeContentType &bodytype )
{
	if( local ) {
		local->cancelRequest();
		disconnect( local, 0, this, 0 );
	}
	statusdesc = tr("Querying options");
	local = call->newRequest( this, Sip::OPTIONS, body, bodytype );
	connect( local, SIGNAL(statusUpdated()), this, SLOT(localStatusUpdated()) );
	statusUpdated( this );
}

void SipCallMember::acceptInvite( const QString &body, const MimeContentType &bodytype )
{
	if( !remote ) return;
	if( body != QString::null ) {
		localsessiondesc = body;
		localsessiontype = bodytype;
	}
	remote->sendResponse( SipStatus( 200 ), body, bodytype );
	state = state_Connected;
	statusdesc = "Connected";
	statusUpdated( this );
	call->setCallStatus( SipCall::callInProgress );
}

void SipCallMember::notAcceptableHere( void )
{
	if( !remote ) return;
	remote->sendResponse( SipStatus( 488 ) );
	state = state_Disconnected;
	statusdesc = "Not Acceptable Here";
	statusUpdated( this );
}

void SipCallMember::declineInvite( const QString &body, const MimeContentType &bodytype )
{
	if( !remote ) return;
	remote->sendResponse( SipStatus( 603 ), body, bodytype );
	state = state_Disconnected;
	statusdesc = "Rejecting call invitation";
	statusUpdated( this );
}

void SipCallMember::localStatusUpdated( void )
{
	QString authstrtemp;
	MimeContentType mtype;
	SipStatus sipstatus;
	if( !local ) {
		sip_printf( "SipCallMember: Received what was likely a retransmission, badly ignoring...\n" );
		return;
	}
	sip_printf( "SipCallMember: localStatusUpdated: %d\n", local->getStatus().getCode() );
	mtype = local->getFinalContentType();
	if( mtype == MimeContentType( "application/sdp" ) ) {
		sessiondesc = local->getFinalMessageBody();
		localsessiontype = mtype;
	} else {
		recentbody = local->getFinalMessageBody();
		recentbodytype = mtype;
	}

	if( callMemberType == Invite ) {
		handlingInviteResponse();
	} else if( callMemberType == Message ) {
		handlingMessageResponse();
	} else if( callMemberType == Subscribe ) {
		handlingSubscribeResponse();
	} else if( callMemberType == Notify ) {
		handlingNotifyResponse();
	} else {
		return;
	}
	statusUpdated( this );
}

void SipCallMember::incomingTransaction( SipTransaction *newtrans )
{
	MimeContentType mtype;
	remote = newtrans;
	if( remote->getRequest()->getMethod() == Sip::INVITE ) {
		connect( remote, SIGNAL(statusUpdated()), this, SLOT(remoteStatusUpdated()) );
		if( state == state_Disconnected ) {
			state = state_RequestingInvite;
			statusdesc = "Invitation received";
			remote->sendResponse( SipStatus( 180 )); 
		} else {
			state = state_RequestingReInvite;
			statusdesc = "Session update requested";
		}
		mtype = remote->getRequestMessageContentType();
		if( mtype == MimeContentType( "application/sdp" ) ) {
			sessiontype = mtype;
			sessiondesc = remote->getRequestMessageBody();
		} else {
			recentbodytype = mtype;
			recentbody = remote->getRequestMessageBody();
		}
	} else if( remote->getRequest()->getMethod() == Sip::BYE ) {
		state = state_Disconnected;
		statusdesc = "Remote end disconnected";
		call->hideCallWidget();
	} else if( remote->getRequest()->getMethod() == Sip::REFER ) {
		if( state == state_Refer ) {
			sip_printf( "SipCallMember: Received what was likely a retransmission, badly ignoring...\n" );
			return;
		}
		remote->sendResponse( SipStatus( 202 ) );
		state = state_Refer;
		statusdesc = "Refer";
		redirectlist = SipUriList( remote->getRequest()->getHeaderData( SipHeader::Refer_To ) );
	}
	statusUpdated( this );
}

void SipCallMember::remoteStatusUpdated( void )
{
	if( state == state_RequestingInvite ) {
		if( remote->wasCancelled() ) {
			state = state_Disconnected;
			statusdesc = "Request cancelled";
			statusUpdated( this );
		}
	}
}

void SipCallMember::cancelTransaction( void )
{
	if( local ) {
		if( state == state_EarlyDialog ) {
			local->cancelRequest();
			state = state_Disconnected;
		} else {
			state = state_CancelPending;
		}
	}
}

QString SipCallMember::getSubject( void )
{
	return call->getSubject();
}

void SipCallMember::call_timeout()
{
	if( call->getCallType() == SipCall::outSubscribeCall ) {
		requestSubscribe();
	} else if( call->getCallType() == SipCall::inSubscribeCall ) {
		contactUpdate( false );
	};
}

void SipCallMember::contactUpdate( bool active, QString presence )
{
	call->setPresenceStatus( presence );
	if( active ) {
		call->setCallStatus( SipCall::callInProgress );
	} else {
		if( call->getCallStatus() != SipCall::callDead ) {
			call->setCallStatus( SipCall::callUnconnected );
		}
		memberuri.setTag( QString::null );
	}
	statusUpdated( this );
}

void SipCallMember::timerStart( int time )
{
	timer->start( time );
}

//------------------------------
//           SipCall
//------------------------------

SipCall::SipCall( SipUser *local, const QString &id, SipCall::CallType ctype )
{
	callstatus = callUnconnected;
	if ( id == QString::null ) {
		callid = SipMessage::createCallId();
	} else {
		callid = id;
	}

	// Remember to nuke all members and transactions
	struct timeval tv;
	gettimeofday( &tv, NULL );
	srand( tv.tv_usec );
	lastseq = rand() % 8000;
	parent = local->parent();
	calltype = ctype;
	hasroute = false;
	localuri = local->getUri();
	localuri.generateTag();
	parent->addCall( this );
	presenceStatus = "";
	bodyMask = QString::null;
	contactstr = "";
	hasrecordroute = false;
}

SipCall::~SipCall( void )
{
	parent->deleteCall( this );
	qDeleteAll(transactions.begin(), transactions.end());
	transactions.clear();
	qDeleteAll(members.begin(), members.end());
	members.clear();
}

void SipCall::addMember( SipCallMember *newmember )
{
	members.append( newmember );
}

SipTransaction *SipCall::newRequest( SipCallMember *member, Sip::Method meth,
	const QString &body, const MimeContentType &bodytype, const SipUri &transferto,
	const QString &proxyauthentication, int expiresTime )
{
	SipTransaction *trans = new SipTransaction( lastseq++, member, this );
	if( trans->sendRequest( meth, body, bodytype, transferto, proxyauthentication, expiresTime ) ) {
		qDeleteAll(transactions.begin(), transactions.end());
		transactions.clear();
		transactions.append( trans );

		// Audit the call
		auditCall();

		// Return the transaction object for tracking
		return trans;
	} else {
		delete trans;
		return 0;
	}
}

SipTransaction *SipCall::newRegister( const SipUri &registerserver, int expiresTime, const QString &authentication,
	const QString &proxyauthentication, const QString &qvalue, const QString &body, const MimeContentType &bodytype )
{
	qDeleteAll(transactions.begin(), transactions.end());
	transactions.clear();
	localuri.setTag( QString::null );
	SipTransaction *trans = new SipTransaction( lastseq++, new SipCallMember( this, localuri ), this );
	transactions.append( trans );
	trans->sendRegister( registerserver, expiresTime, authentication, proxyauthentication, body, bodytype, qvalue );
	return trans;
}

bool SipCall::sendRequest( SipMessage *reqmsg, bool contact, const SipUri &proxy, const QString &branch )
{
	reqmsg->insertHeader( SipHeader::From, localuri.nameAddr() );
	reqmsg->insertHeader( SipHeader::Call_ID, callid );
	if( ( reqmsg->getMethod() == Sip::INVITE ) || ( reqmsg->getMethod() == Sip::MSG ) ) {
		reqmsg->insertHeader( SipHeader::Subject, getSubject() );
	}
	if( hasroute ) {
		if( route.getHead().uri().contains( ";lr" ) ) {
			reqmsg->insertHeader( SipHeader::Route, route.getUriList() );
		} else {
			reqmsg->setRequestUri( route.getHead() );
			SipUriList routewithouthead = route;
			routewithouthead.removeHead();
			reqmsg->insertHeader( SipHeader::Route, routewithouthead.getUriList() );
		}
	}
	if( parent->getExplicitProxyMode() ) {
		if( reqmsg->getMethod() == Sip::REGISTER || reqmsg->getMethod() == Sip::INVITE ||
				reqmsg->getMethod() == Sip::SUBSCRIBE || reqmsg->getMethod() == Sip::ACK ) {
			if( parent->isLooseRoute() ) {
				if( reqmsg->getMethod() == Sip::REGISTER ) {
					reqmsg->setRequestUri(
						SipUri( reqmsg->getHeaderData( SipHeader::To ) ).getRegisterUri() );
				}
				reqmsg->insertHeader( SipHeader::Route, parent->getExplicitProxyAddress() );
			} else if( parent->isStrictRoute() ) {
				if( reqmsg->getMethod() == Sip::REGISTER ) {
					reqmsg->insertHeader(
						SipHeader::Route, SipUri(
						reqmsg->getHeaderData( SipHeader::To ) ).getRouteUri() );
				} else {
					reqmsg->insertHeader(
						SipHeader::Route, "<" + reqmsg->getRequestUri().reqUri() + ">" );
				}
				reqmsg->setRequestUri( parent->getExplicitProxyAddress() );
			} else {
				if( reqmsg->getMethod() == Sip::REGISTER ) {
					reqmsg->setRequestUri(
						SipUri( reqmsg->getHeaderData( SipHeader::To ) ).getRegisterUri() );
				}
			}
		}
	}
	return parent->sendRequest( reqmsg, contact, proxy, branch );
}

void SipCall::sendResponse( SipMessage *responsemsg, bool contact )
{
	responsemsg->insertHeader( SipHeader::Call_ID, callid );
	responsemsg->insertHeader( SipHeader::To, localuri.nameAddr() );
	if( hasrecordroute ) {
		responsemsg->setRecordRoute( recordroute );
	}
	parent->sendResponse( responsemsg, contact );
}

void SipCall::sendRaw( SipMessage *msg )
{
	parent->sendRaw( msg );
}

SipCallMember *SipCall::incomingMessage( SipMessage *message )
{
	if( message->getType() == SipMessage::Request ) {
		return incomingRequest( message );
	} else if( message->getType() == SipMessage::Response ) {
		incomingResponse( message );
	} else {
		sip_printf( "SipCall: Incoming message dropped (bad message type)\n" );
		delete message;
	}
	return 0;
}


void SipCall::incomingResponse( SipMessage *message )
{
	SipUri incominguri( message->getHeaderData( SipHeader::To ) );
	QString cseq = message->getHeaderData( SipHeader::CSeq );
	unsigned int seqnum = cseq.left( cseq.indexOf( " " ) ).toUInt();
	sip_printf( "SipCall: Incoming response\n" );
	SipTransaction *curtrans;
	QList<SipTransaction *>::Iterator it;
	for ( it = transactions.begin(); it != transactions.end(); ++it ) {
		curtrans = *it;
		if (( incominguri == curtrans->getCallMember()->getUri()) &&
		    ( seqnum == curtrans->getSeqNum() ) &&
		    ( curtrans->getDirection() == SipTransaction::LocalRequest ) ) {
			SipCallMember *member = getMember( incominguri );
			if( member == NULL ) {
				sip_printf( "SipCall: Billy, you really messed something up\n" );
			} else if( message->getStatus().getCode() == 200 || message->getStatus().getCode() == 202 ) {
				sip_printf( "SipCall: Checking for Contact and Record-Route\n" );

				// Update the Contact for this member
				if( message->getContactList().getListLength() > 0 ) {
					sip_printf( "SipCall: Setting Contact for this Call Member\n" );
					member->setContactUri( message->getContactList().getHead() );
				}

				// Update the route
				if( message->getRecordRoute().getListLength() > 0 ) {
					hasroute = true;
					route = message->getRecordRoute();
					route.reverseList();
					if( !route.getHead().uri().contains( ";lr" ) ) {
						route.addToEnd( member->getContactUri() );
					}
				}
			}
			curtrans->incomingResponse( message );
			return;
		}
	}
	sip_printf( "SipCall: Response dropped: No transaction found\n" );
	delete message;
}


SipCallMember *SipCall::incomingRequest( SipMessage *message )
{
	SipUri incominguri( message->getHeaderData( SipHeader::From ) );
	QString cseq = message->getHeaderData( SipHeader::CSeq );
	unsigned int seqnum = cseq.left( cseq.indexOf( " " ) ).toUInt();
	QString seqmethod = cseq.right( cseq.indexOf( " " ) + 1 );
	SipTransaction *curtrans;
	QList<SipTransaction *>::Iterator it;
	sip_printf( "SipCall: Incoming request\n" );

	// Update our identity if necessary
	SipUri touri( message->getHeaderData( SipHeader::To ) );
	if( touri != localuri ) {
		localuri = touri;
		if( !localuri.hasTag() ) {
			localuri.generateTag();
		}
	}

	if( ( message->getMethod() == Sip::ACK ) || ( message->getMethod() == Sip::CANCEL ) ) {
		for( it = transactions.begin(); it != transactions.end(); ++it ) {
			curtrans = *it;
			if( ( incominguri == curtrans->getCallMember()->getUri() ) &&
			    ( seqnum == curtrans->getSeqNum() ) &&
			    ( curtrans->getDirection() == SipTransaction::RemoteRequest ) ) {
				curtrans->incomingRequest( message );
				return 0;
			}
		}
		sip_printf( "SipCall: ACK/CANCEL recieved, but no matching transaction found\n" );
		delete message;
		return 0;
	}
	for( it = transactions.begin(); it != transactions.end(); ++it ) {
		curtrans = *it;
		if( ( incominguri == curtrans->getCallMember()->getUri() ) && ( seqnum == curtrans->getSeqNum() ) && 
		    ( seqmethod == curtrans->getSeqMethod() ) &&
		    ( curtrans->getDirection() == SipTransaction::RemoteRequest ) ) {
			curtrans->incomingRequestRetransmission( message );
			return 0;
		}
	}

	// Find or create a member for this request
	SipCallMember *member = getMember( incominguri );
	if ( member == NULL ) {
          sip_printf("SipCall: Create a new incominguri \n");
		member = new SipCallMember( this, incominguri );
		if ( message->getMethod() == Sip::INVITE ) {
			member->setCallMemberType( SipCallMember::Invite );
		} else if ( message->getMethod() == Sip::SUBSCRIBE ) {
			member->setCallMemberType( SipCallMember::Subscribe );
		} else if ( message->getMethod() == Sip::NOTIFY ) {
			member->setCallMemberType( SipCallMember::Notify );
		} else if ( message->getMethod() == Sip::MESSAGE ) {
			member->setCallMemberType( SipCallMember::Message );
		}
	}

	// Update the Contact for this member
	if( message->getContactList().getListLength() > 0 ) {
		member->setContactUri( message->getContactList().getHead() );
	}

	// Update the route
	if( message->getRecordRoute().getListLength() > 0 ) {
		hasrecordroute = true;
		recordroute = message->getRecordRoute();
		hasroute = true;
		route = recordroute;
		if( !route.getHead().uri().contains( ";lr" ) ) {
			route.addToEnd( member->getContactUri() );
		}
	}

	// Create a new transaction and process it
	sip_printf( "SipCall: New transaction created\n" );
	SipTransaction *transaction = new SipTransaction( seqnum, member, this );
	transactions.append( transaction );
	transaction->incomingRequest( message );

	// Update member status based on this transaction
	member->incomingTransaction( transaction );
	return member;
}

void SipCall::setSubject( const QString& newsubject )
{
	subject = newsubject;
	subjectChanged();
}

SipUri SipCall::getContactUri( void )
{
	return parent->getContactUri();
}

SipCallMember *SipCall::getMember( const SipUri &uri )
{
	QList<SipCallMember *>::Iterator m;
	for( m = members.begin(); m != members.end(); ++m ) {
		if ( uri == (*m)->getUri() ) {
			return *m;
		}
	}

	return NULL;
}

void SipCall::setPresenceStatus( QString status )
{
	if( presenceStatus != status ) {
		presenceStatus = status;
		parent->callListUpdated();
	}
}

void SipCall::setCallType( CallType newtype )
{
	calltype = newtype;
	parent->callTypeUpdated();
}

void SipCall::auditCall( void )
{
	bool foundmemb = false;

	// If there are no active call members, set the call status to 'Dead'
	SipCallMember *memb;
	QList<SipCallMember *>::Iterator it;
	for( it = members.begin(); it != members.end(); ++it ) {
		memb = *it;
		if( ( memb->getState() != SipCallMember::state_Disconnecting ) &&
		    ( memb->getState() != SipCallMember::state_Disconnected ) ) {
			foundmemb = true;
		}
	}
	if( !foundmemb ) {
		callstatus = callDead;
		callStatusUpdated();
	}
}

QString SipCall::getProxyUsername( void )
{
	SipUser *u = parent->getUser( localuri );
	if( u != NULL ) {
		SipUri *uri = u->getMyUri();
		if( uri->hasProxyUsername() ) {
			return uri->getProxyUsername();
		} else {
			return QString::null;
		}
	} else {
		return QString::null;
	}
}

QString SipCall::getHostname( void )
{
	SipUser *u = parent->getUser( localuri );
	if( u != NULL ) {
		SipUri *uri = u->getMyUri();
		return uri->getHostname();
	} else {
		return QString::null;
	}
}

QString SipCall::getSipProxy( void )
{
	SipUser *u = parent->getUser( localuri );
	if( u != NULL ) {
		SipClient *c = u->parent();
		QString proxy = c->getSipProxy();
		if( !proxy.isEmpty() ) {
			return proxy;
		} else {
			if( c->getProxyMode() ) {
				return c->getSipProxySrv( u->getMyUri()->getHostname() );
			}
		}
	} 
	return QString::null;
}

void SipCall::setProxyUsername( QString newUsername )
{
	SipUser *u = parent->getUser( localuri );
	if( u != NULL ) {
		SipUri *uri = u->getMyUri();
		uri->setProxyUsername( newUsername );
	}
}

QString SipCall::getPassword( void )
{
	SipUser *u = parent->getUser( localuri );
	if( u != NULL ) {
		SipUri *uri = u->getMyUri();
		if( uri->hasPassword() ) {
			return uri->getPassword();
		} else {
			return QString::null;
		}
	} else {
		return QString::null;
	}
}

void SipCall::setPassword( QString newPassword )
{
	SipUser *u = parent->getUser( localuri );
	if( u != NULL ) {
		SipUri *uri = u->getMyUri();
		uri->setPassword( newPassword );
	}
}

void SipCall::hideCallWidget( void )
{
	parent->hideCallWidget( this );
}

void SipCall::updateSubscribes( void )
{
	parent->updateSubscribes();
}
