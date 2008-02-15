#include <stdio.h>
#include <sys/time.h>
#include <qtimer.h>

#include "sdp.h"
#include "sipuri.h"
#include "sipcall.h"
#include "sipmessage.h"
#include "siptransaction.h"
#include "sipdebug.h"

SipTransaction::SipTransaction( unsigned int seqn, SipCallMember *farend, SipCall *call )
{
	parent = call;
	requestmessage = NULL;
	direction = None;
	remote = farend;
	seqnum = seqn;
	int uniqid = rand();
	branch.sprintf( "z9hG4bK%X", uniqid );
	cancelled = false;
	timer_202 = new QTimer( this );
	connect( timer_202, SIGNAL(timeout()), this, SLOT(send_202()) );
}

SipTransaction::~SipTransaction( void )
{
	if( requestmessage ) {
		delete requestmessage;
	}
	qDeleteAll(responses.begin(), responses.end());
	responses.clear();
}

QString SipTransaction::getCSeq( void ) const
{
	QString cseq;

	cseq = QString::number( seqnum )
		+ " " + Sip::getMethodString( requestmessage->getMethod() );

	return cseq;
}

QString SipTransaction::getSeqMethod( void ) const
{ 
	return Sip::getMethodString( requestmessage->getMethod() );
}

void SipTransaction::setStatus( const SipStatus &stat )
{
	laststatus = stat;
	statusUpdated();
}

bool SipTransaction::sendRequest( Sip::Method meth, const QString &body,
		const MimeContentType &bodytype, const SipUri &transferto,
		const QString &proxyauthentication, const int expiresTime )
{
	if( meth == Sip::INVITE ) {
		if( parent->getCallType() != SipCall::videoCall ) {
			parent->setCallType( SipCall::StandardCall );
		}
	}

	if ( requestmessage == NULL ) {
		direction = LocalRequest;
	}

	// Construct message
	requestmessage = new SipMessage;
	requestmessage->setType( SipMessage::Request );
	requestmessage->setMethod( meth );

	// Set Request Uri
	if( meth != Sip::SUBSCRIBE ||
	    remote->getCall()->getCallStatus() == SipCall::callDisconneting ||
	    remote->getCall()->getCallStatus() == SipCall::callInProgress ) {
		requestmessage->setRequestUri( remote->getContactUri() );
	} else {
		requestmessage->setRequestUri( remote->getUri() );
	}
	requestmessage->insertHeader( SipHeader::CSeq, getCSeq() );
	if( ( meth != Sip::INVITE && meth != Sip::MESSAGE && meth != Sip::SUBSCRIBE && meth != Sip::CANCEL ) ||
	    remote->getCall()->getCallStatus() == SipCall::callDisconneting ||
	    remote->getCall()->getCallStatus() == SipCall::callInProgress ) {
		requestmessage->insertHeader( SipHeader::To, remote->getUri().nameAddr() );
	} else {
		requestmessage->insertHeader( SipHeader::To, remote->getUri().nameAddr_noTag() );
	}

	if( meth == Sip::REFER && transferto != SipUri::null ) {
		requestmessage->insertHeader( SipHeader::Refer_To, transferto.uri() );
		requestmessage->insertHeader( SipHeader::Referred_By, parent->getContactUri().nameAddr() );
	}

	if( proxyauthentication != QString::null ) {
		requestmessage->insertHeader( SipHeader::Proxy_Authorization, proxyauthentication );
	}

	if( expiresTime >= 0 ) {
		QString strExpiresTime;
		requestmessage->insertHeader( SipHeader::Expires, strExpiresTime.setNum( expiresTime ) );
	}

	if( bodytype != MimeContentType::null ) {
		requestmessage->insertHeader( SipHeader::Content_Type, bodytype.type() );
	}
	requestmessage->setBody( body );

	return parent->sendRequest( requestmessage, true, SipUri::null, branch );
}

void SipTransaction::sendResponse( const SipStatus &status, const QString &body, const MimeContentType &bodytype )
{
	SipMessage *response = new SipMessage;

	setStatus( status ); // Update our status
	responses.append( response ); // Save this response

	response->setType( SipMessage::Response );
	response->setStatus( status );

	// Copy via list exactly
	response->setViaList( requestmessage->getViaList() );

	response->insertHeader( SipHeader::From, remote->getUri().nameAddr() );
	response->insertHeader( SipHeader::CSeq, getCSeq() );
	if( bodytype != MimeContentType::null ) {
		response->insertHeader( SipHeader::Content_Type, bodytype.type() );
	}
	response->setBody( body );

	parent->sendResponse( response );
}

void SipTransaction::sendRegister( const SipUri &registerserver,
	int expiresTime, const QString &authentication,
	const QString &proxyauthentication, const QString &body,
	const MimeContentType &bodytype, const QString &qvalue )
{
	if ( requestmessage == NULL ) {
		direction = LocalRequest;
	}

	// Create message
	requestmessage = new SipMessage;
	requestmessage->setType( SipMessage::Request );
	requestmessage->setMethod( Sip::REGISTER );
	requestmessage->setQvalue( qvalue );

	// Set request URI
	requestmessage->setRequestUri( registerserver );

	requestmessage->insertHeader( SipHeader::CSeq, getCSeq() );
	requestmessage->insertHeader( SipHeader::To, remote->getUri().nameAddr() );

	if( authentication != QString::null ) {
		requestmessage->insertHeader( SipHeader::Authorization, authentication );
	}

	if( proxyauthentication != QString::null ) {
		requestmessage->insertHeader( SipHeader::Proxy_Authorization, proxyauthentication );
	}

	if( bodytype != MimeContentType::null ) {
		requestmessage->insertHeader( SipHeader::Content_Type, bodytype.type() );
	}

	requestmessage->setBody( body );

	if( expiresTime >= 0 ) {
		QString strExpiresTime;
		requestmessage->insertHeader( SipHeader::Expires, strExpiresTime.setNum( expiresTime ) );
	}

	parent->sendRequest( requestmessage, true, registerserver );
}

void SipTransaction::incomingResponse( SipMessage *message )
{
	sip_printf( "SipTransaction: Incoming Response\n" );

	responses.append( message );

	// Grab tags
	SipUri newto( message->getHeaderData( SipHeader::To ) );
	remote->setUri( newto );
	if( ( requestmessage->getMethod() == Sip::INVITE ) && ( message->getStatus().getCode() >= 200 ) ) {
		sendRequest( Sip::ACK );
	} else if( ( requestmessage->getMethod() == Sip::ACK ) && ( message->getStatus().getCode() >= 200 ) ) {
		sendRequest( Sip::ACK );
	}

	setStatus( message->getStatus() );
}


QString SipTransaction::stateText( QString state )
{
	QString text;
	if( state == "online" ) {
		text = tr("Online");
	} else if( state == "busy" ) {
		text = tr("Busy");
	} else if( state == "berightback" ) {
		text = tr("Be Right Back");
	} else if( state == "away" ) {
		text = tr("Away");
	} else if( state == "onthephone" ) {
		text = tr("On The Phone");
	} else if( state == "outtolunch" ) {
		text = tr("Out To Lunch");
	} else {
		text = state;
	}
	return text;
}

void SipTransaction::incomingRequest( SipMessage *message )
{
	SdpMessage optsdp;
	QString p;

	sip_printf( "SipTransaction: Incoming Request\n" );
	if( ( message->getMethod() == Sip::ACK ) ) {
		sip_printf( "SipTransaction: Received an ACK...\n" );

		// Make sure listeners know in case there was a new session description
		statusUpdated();
		delete message;
	} else if( ( message->getMethod() == Sip::CANCEL ) ) {
		sip_printf( "SipTransaction: CANCEL Received\n" );
		timer_202->stop();
		if( !cancelled && requestmessage->getMethod() == Sip::INVITE ) {
			sendResponse( SipStatus( 487 ) );
		}
		requestmessage->setMethod( Sip::CANCEL );
		if( cancelled ) {
			sendResponse( SipStatus( 481 ) );
		} else {
			sendResponse( SipStatus( 200 ) );
		}

		// Mark request as cancelled and inform listeners
		cancelled = true;
		statusUpdated();
		delete message;

	} else {
		direction = RemoteRequest;
		if( requestmessage ) {
			delete requestmessage;
		}
		requestmessage = message;
		switch( message->getMethod() ) {
			case Sip::INVITE:
				if( parent->getCallType() != SipCall::videoCall ) {
					parent->setCallType( SipCall::StandardCall );
				}
				sendResponse( SipStatus( 100 ) );
				break;
			case Sip::BYE:
				if( parent->getCallType() == SipCall::UnknownCall ) {
					parent->setCallType( SipCall::BrokenCall );
				}
				sendResponse( SipStatus( 200 ) );
				break;
			case Sip::OPTIONS:
				if( parent->getCallType() == SipCall::UnknownCall ) {
					parent->setCallType( SipCall::OptionsCall );
				}
				optsdp.setIpAddress( "0.0.0.0" );
				optsdp.setName( "session" );
				optsdp.setPort( 0 );
				optsdp.setVideoPort( 0 );
				sendResponse( SipStatus( 200 ), optsdp.message( codecPCMU, codecUnknown ),
					MimeContentType( "application/sdp" ) );
				break;
			case Sip::MSG:
				parent->setCallType( SipCall::MsgCall );
				if( message->hasHeader( SipHeader::Subject ) ) {
					parent->setSubject( message->getHeaderData( SipHeader::Subject ) );
				}
				sendResponse( SipStatus( 200 ) );
				break;
			case Sip::SUBSCRIBE:
				if( parent->getCallType() == SipCall::inSubscribeCall ) {
					sendResponse( SipStatus( 202 ) );
				} else if( parent->getCallType() == SipCall::inSubscribeCall_2 ) {
//					timer_202->start( 250, TRUE );
					sendResponse( SipStatus( 403 ) );
					cancelled = true;
//---
				}
				break;
			case Sip::NOTIFY:
				sendResponse( SipStatus( 200 ) );
				if( message->getMethod() == Sip::NOTIFY ) {
					QString m = message->messageBody();
					int idx = m.indexOf("substatus=");
					if (idx != -1) {
						m = m.mid( idx + 11 );
						parent->setPresenceStatus( stateText( m.left( m.indexOf('"') ) ) );
					} else {
						m = message->messageBody();
						idx = m.indexOf("status status=");
						if (idx != -1) {
							m = m.mid( idx + 15 );
							parent->setPresenceStatus( m.left( m.indexOf('"') ) );
						} else {
							parent->setPresenceStatus( "" );
						}
					}
				}
				break;
			default:
				break;
		}
	}
}

void SipTransaction::incomingRequestRetransmission( SipMessage *message )
{
	SdpMessage optsdp;
	QString p;
	sip_printf( "SipTransaction: Incoming Request Retransmission\n" );
	sip_printf( "SipTransaction: Response Retransmission\n" );
	if( ( message->getMethod() == Sip::CANCEL ) ) {
		sendResponse( SipStatus( 481 ) );
	} else {
		switch( message->getMethod() ) {
			case Sip::INVITE:
				sendResponse( SipStatus( 100 ) );
				break;
			case Sip::BYE:
				sendResponse( SipStatus( 200 ) );
				break;
			case Sip::OPTIONS:
				optsdp.setIpAddress( "0.0.0.0" );
				optsdp.setName( "session" );
				optsdp.setPort( 0 );
				optsdp.setVideoPort( 0 );
				sendResponse( SipStatus( 200 ), optsdp.message(), MimeContentType( "application/sdp" ) );
				break;
			case Sip::MSG:
				sendResponse( SipStatus( 200 ) );
				break;
			case Sip::SUBSCRIBE:
				if( parent->getCallType() == SipCall::inSubscribeCall ) {
					sendResponse( SipStatus( 202 ) );
				} else if( parent->getCallType() == SipCall::inSubscribeCall_2 ) {
					sendResponse( SipStatus( 403 ) );
				}
				break;
			case Sip::NOTIFY:
				sendResponse( SipStatus( 200 ) );
				break;
			default:
				break;
		}
	}
	delete message;
}


bool SipTransaction::auditPending( void )
{
	struct timeval tv;
	struct timeval *reftv;
	unsigned int diff;

	if( ( laststatus.getCode() < 200 ) && ( direction == LocalRequest ) ) {

		// Check time
		gettimeofday( &tv, NULL );
		reftv = requestmessage->getTimestamp();

		// Check difference
		diff = ( tv.tv_sec - reftv->tv_sec ) * 1000 * 1000 + tv.tv_usec - reftv->tv_usec;
		diff = diff / 1000; // ms

		if( requestmessage->getMethod() != Sip::INVITE ) {
			if( diff > requestmessage->lastTimeTick() ) {

				// Timeout
				if( requestmessage->sendCount() == 3 ) {
					if( requestmessage->getMethod() == Sip::SUBSCRIBE ) {
						remote->contactUpdate( false );
					}
					remote->setIdle();
					setStatus( SipStatus( 408 ) );
					return false;
				}
				sip_printf( "SipTransaction: Retransmit %d (%d)\n",
					requestmessage->sendCount(), requestmessage->lastTimeTick() );
				parent->sendRaw( requestmessage );
				requestmessage->setTimestamp();
				requestmessage->incrSendCount();
				if( requestmessage->lastTimeTick() < 4000 ) {
					if( laststatus.getCode() < 100 ) {
						requestmessage->setTimeTick( 2 * requestmessage->lastTimeTick() );
					} else {
						requestmessage->setTimeTick( 4000 );
					}
				}
				return true;
			}
		} else {

			// INVITEs should be retransmitted differently
			if( laststatus.getCode() < 100 ) {
				if( diff > requestmessage->lastTimeTick() ) {

					// Timeout
					if ( requestmessage->sendCount() == 7 ) {
						setStatus( SipStatus( 408 ) );
						return false;
					}
					sip_printf( "SipTransaction: Retransmit %d (%d)\n",
						requestmessage->sendCount(), requestmessage->lastTimeTick() );
					parent->sendRaw( requestmessage );
					requestmessage->setTimestamp();
					requestmessage->incrSendCount();
					requestmessage->setTimeTick( 2 * requestmessage->lastTimeTick() );
					return true;
				}
			}
		}
	}
	return false;
}

void SipTransaction::cancelRequest( const QString &body, const MimeContentType &bodytype )
{
	requestmessage->setMethod( Sip::CANCEL );
	sendRequest( Sip::CANCEL, body, bodytype );
}

QString SipTransaction::getFinalMessageBody( void )
{
	if( responses.last() ) {
		if( responses.last()->haveBody() ) {
			return responses.last()->messageBody();
		}
	}

	return QString::null;
}

MimeContentType SipTransaction::getFinalContentType( void )
{
	if( responses.last() ) {
		if( responses.last()->hasHeader( SipHeader::Content_Type ) ) {
			return MimeContentType( responses.last()->getHeaderData( SipHeader::Content_Type ) );
		}
	}

	return MimeContentType::null;
}

SipUriList SipTransaction::getFinalContactList( void )
{
	if( responses.last() ) {
		return responses.last()->getContactList();
	}

	return SipUriList::null;
}

QString SipTransaction::getFinalWWWAuthString( void )
{
	if( responses.last() ) {
		return responses.last()->getHeaderData( SipHeader::WWW_Authenticate );
	}

	return QString::null;
}

QString SipTransaction::getRequestMessageBody( void ) const
{
	return requestmessage->messageBody();
}

MimeContentType SipTransaction::getRequestMessageContentType( void ) const
{
	if( requestmessage->hasHeader( SipHeader::Content_Type ) ) {
		return MimeContentType( requestmessage->getHeaderData( SipHeader::Content_Type ) );
	}

	return MimeContentType::null;
}

QString SipTransaction::getFinalProxyAuthString( void )
{
	if( responses.last() ) {
		return responses.last()->getHeaderData( SipHeader::Proxy_Authenticate );
	}

	return QString::null;
}

void SipTransaction::send_202( void )
{
	sendResponse( SipStatus( 202 ) );
}
