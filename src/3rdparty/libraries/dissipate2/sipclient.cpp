#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <qdatetime.h>
#include <qsocketnotifier.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "siputil.h"
#include "sipuser.h"
#include "sipvia.h"
#include "sipcall.h"
#include "tcpmessagesocket.h"
#include "sipmessage.h"
#include "mimecontenttype.h"
#include "sipstatus.h"
#include "siptransaction.h"
#include "sipclient.h"
#include "sipdebug.h"

typedef unsigned int u_int;
typedef unsigned short u_short;
typedef unsigned char u_char;

struct s_SOA
{
	char *mname;
	char *rname;
	u_int serial;
	u_int refresh;
	u_int retry;
	u_int expire;
	u_int minimum;
};

struct s_NULL
{
	char *anything;
	u_short length;         /* Length of valid data */
};

struct s_WKS
{
	struct in_addr address;
	char *bitmap;
	u_int maplength;
	u_char protocol;
};

struct s_HINFO
{
	char *cpu;
	char *os;
};

struct s_MINFO
{
	char *rmailbx;
	char *emailbx;
};

struct s_MX
{
	char *exchange;
	u_short preference;
};

struct s_TXT
{
	char *text;
	struct s_TXT *next;
	u_short len;
};

struct s_SRV
{
	u_short priority;
	u_short weight;
	u_short port;
	char *target;
};

struct s_NAPTR
{
	u_short order;
	u_short pref;
	char *flags;
	char *service;
	char *regexp;
	char *replacement;
};

/* AFS servers */
struct s_AFSDB
{
	u_short subtype;
	char *hostname;
};

/* Responsible Person */
struct s_RP
{
	char *mbox_dname;
	char *txt_dname;
};

/* ISDN Address */
struct s_ISDN
{
	char *address;
	char *sa;    /* optional */
};

/* Route Through */
struct s_RT
{
	u_short preference;
	char *int_host;
};

/* Generic RDATA RR structure */
union u_rdata
{
	char *string;            /* Any simple string record */
	u_int number;            /* Any simple numeric record */
	struct in_addr address; /* Simple address (A record) */

/* other structured RR types */
	struct s_SOA soa;
	struct s_NULL null;
	struct s_WKS wks;
	struct s_HINFO hinfo;
	struct s_MINFO minfo;
	struct s_MX mx;
	struct s_TXT txt;
	struct s_SRV srv;
	struct s_NAPTR naptr;

/* RFC 1183 RR types */
	struct s_AFSDB afsdb;
	struct s_RP rp;
	struct s_ISDN isdn;
	struct s_RT rt;
};

/* Full RR structure */
typedef struct s_rr
{
	char *name;
	u_short type;
	u_short xclass;
	u_int ttl;
	u_int dlen;
	union u_rdata rdata;
} s_rr;

/* DNS Question sctructure */
typedef struct s_question
{
	char *qname;
	u_short qtype;
	u_short qclass;
} s_question;

/* Full DNS message structure */
typedef struct s_res_response
{
	HEADER header;
	s_question **question;
	s_rr **answer;
	s_rr **authority;
	s_rr **additional;
} res_response;


bool const traceMessageSending = true;
bool const traceMessageReceived = true;


SipClient::SipClient( QObject *parent, const char *, unsigned int newListenport,
			bool newLooseRoute, bool newStrictRoute, QString socketStr )
	: QObject( parent )
{
	if( !setupSocketStuff( newListenport, socketStr ) ) {
		sip_printf("SipClient::setupSocketStuff() Failed.\n");
		exit( 1 );
	}
	setupContactUri();
	useProxyDial = false;
	useExplicitProxy = false;
	proxyport = 5060;
	fwmode = false;
	busymode = false;
	user = 0;
	hidemode = DontHideVia;
	symmetricmode = false;
	maxforwards = 0;
	looseRoute = newLooseRoute;
	strictRoute = newStrictRoute;
	testOn = false;
	useStunProxy = false;
	tcpSocket = 0;

}

SipClient::~SipClient( void )
{
    qDeleteAll(calls.begin(), calls.end());
    calls.clear();
    qDeleteAll(tcpSockets.begin(), tcpSockets.end());
    tcpSockets.clear();
}

void SipClient::setupContactUri( SipUser *user )
{
	if( user ) {
		contacturi.setFullname( user->getUri().getFullname() );
		contacturi.setUsername( user->getUri().getUsername() );
	}
	contacturi.setHostname( Sip::getLocalAddress() );
	if( isTcpSocket() ) {
		contacturi.setPortNumber( TCP_listener.getPortNumber() );
		contacturi.setTransportParam( SipUri::TCP );
	} else {
		contacturi.setPortNumber( listener.getPortNumber() );
		contacturi.setTransportParam( SipUri::UDP );
	}
}

bool SipClient::setupSocketStuff( unsigned int newListenport, QString socketStr )
{
	unsigned int listenport;
        QSocketNotifier *notifier;

	if( socketStr == "UDP" ) {
		SocketMode = UDP;
	} else {
		SocketMode = TCP;
	}
	
	listenport = QString::fromUtf8( getenv( "DISSIPATE_PORT" ) ).toUInt();
	if( newListenport ) {
		listenport = newListenport;
	}
	if( listenport == 0 ) {
		listenport = 5060;
	}
	if( isTcpSocket() ) {
		listenport = TCP_listener.listen( listenport );
		if( !listenport ) { return false; }
		TCP_listener.forcePortNumber( listenport );
		sip_printf( "SipClient: Listening TCP on port: %d\n", TCP_listener.getPortNumber() );

                // XXX - avoid the use of "doSelect()".
                notifier = new QSocketNotifier
                    ( TCP_listener.getFileDescriptor(),
                      QSocketNotifier::Read, this );
                connect( notifier, SIGNAL(activated(int)),
                         this, SLOT(incomingTcpConnection()) );
	}
	listenport = listener.listen( listenport );
	if( !listenport ) { return false; }
	listener.forcePortNumber( listenport );
	sip_printf( "SipClient: Listening UDP on port: %d\n", listener.getPortNumber() );
	sip_printf( "SipClient: Our address: %s\n", Sip::getLocalAddress().toUtf8().constData() );

        // XXX - avoid the use of "doSelect()".
        notifier = new QSocketNotifier
            ( listener.getFileDescriptor(), QSocketNotifier::Read, this );
        connect( notifier, SIGNAL(activated(int)),
                 this, SLOT(incomingMessage(int)) );
	return true;
}

void SipClient::doSelect( bool block )
{
	struct timeval timeout;
	fd_set read_fds;
	int highest_fd;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	FD_ZERO( &read_fds );
	FD_SET( listener.getFileDescriptor(), &read_fds );
	highest_fd = listener.getFileDescriptor() + 1;

retry:
	if( select( highest_fd, &read_fds, NULL, NULL, block ? NULL : &timeout ) == -1 ) {
		if( errno == EINTR ) goto retry;
		sip_perror( "SipClient::doSelect(): select() punted" );
		exit( 1 );
	}
	if( FD_ISSET( listener.getFileDescriptor(), &read_fds ) ) {
		incomingMessage( listener.getFileDescriptor() );
	}

	if( isTcpSocket() ) {
		FD_ZERO( &read_fds );
		FD_SET( TCP_listener.getFileDescriptor(), &read_fds );
		highest_fd = TCP_listener.getFileDescriptor() + 1;

retry2:
		if( select( highest_fd, &read_fds, NULL, NULL, block ? NULL : &timeout ) == -1 ) {
			if( errno == EINTR ) goto retry2;
			sip_perror( "SipClient::doSelect(): select() punted" );
			exit( 1 );
		}

		if( FD_ISSET( TCP_listener.getFileDescriptor(), &read_fds ) ) {
			clilen = sizeof(cli_addr);
			newsockfd = ::accept( TCP_listener.getFileDescriptor(), (struct sockaddr *) &cli_addr, (socklen_t*)&clilen);
			incomingMessage( newsockfd );
			::close( newsockfd );
		}
		TCPMessageSocketList sockets = getTcpSocketList();
		TCPMessageSocketList::Iterator it;
		for ( it = sockets.begin(); it != sockets.end(); ++it) {
			tcpSocket = *it;

			if( tcpSocket != 0 ) {
				FD_ZERO( &read_fds );
				FD_SET( tcpSocket->getFileDescriptor(), &read_fds );
				highest_fd = tcpSocket->getFileDescriptor() + 1;

retry3:
				if( select( highest_fd, &read_fds, NULL, NULL, block ? NULL : &timeout ) == -1 ) {
					if( errno == EINTR ) goto retry3;
					sip_perror( "SipClient::doSelect(): select() punted" );
					exit( 1 );
				}
				if( FD_ISSET( tcpSocket->getFileDescriptor(), &read_fds ) ) {
					incomingMessage( tcpSocket->getFileDescriptor() );
				}
			}
		}
	} else {
		auditPending();
	}
}

void SipClient::incomingTcpConnection()
{
    clilen = sizeof(cli_addr);
    newsockfd = ::accept( TCP_listener.getFileDescriptor(), (struct sockaddr *) &cli_addr, (socklen_t*)&clilen);
    incomingMessage( newsockfd );
    ::close( newsockfd );
}

void SipClient::auditPending( void )
{
	SipCall *curcall;
	SipTransaction *curtrans;
	QList<SipCall *>::Iterator it;
	QList<SipTransaction *>::Iterator it2;
	for( it = calls.begin(); it != calls.end(); ++it ) {
		curcall = *it;
		for( it2 = curcall->getTransactionList().begin();
		     it2 != curcall->getTransactionList().end(); ++it2 ) {
			curtrans = *it2;
			if( curtrans->auditPending() ) {
				return;
			}
		}
	}
}

void SipClient::incomingMessage( int socketfd )
{
	QString fullmessage;
	char inputbuf[ 8000 ];
	int bytesread;
	int i1,i2,i3,i4;
	unsigned int port;
	char ip[16];
	int contentLength = 0;

	// Receive the message
	QString tmp = tr("SipClient: Receiving message...");
	sip_printf( "%s\n", tmp.toUtf8().constData() );
	for(;;) {
		bytesread = read( socketfd, inputbuf, 8000 - 1 );
		StunMsgHdr* hdr = reinterpret_cast<StunMsgHdr*>( inputbuf );
		if( hdr->msgType == BindResponseMsg ) {
		
			// check that the size of the header isn't larger than what we've read
			if ((signed int)sizeof(StunMsgHdr) > bytesread)
			{
				sip_printf("Malformed packet (sizeof(StunMsgHdr) > bytesread)\n");
				return;
			}
			sip_printf( "SipClient: STUN response\n" );
			char* body = inputbuf + sizeof( StunMsgHdr );
			unsigned int size = ntohs( hdr->msgLength );
			port = listener.getPortNumber();
			while( size > 0 ) {
				StunAtrHdr* attr = reinterpret_cast<StunAtrHdr*>( body );
				unsigned int hdrLen = ntohs( attr->length );
				// check that our attribute length is not larger than the remaining size
				if (hdrLen+4 > size)
				{
					sip_printf("Malformed packet (hdrLen+4 > size)\n");
					return;
				}
				if( ntohs( attr->type ) == MappedAddress ) {
					StunAtrAddress* attribute = reinterpret_cast<StunAtrAddress*>( body );
					if ( attribute->address.addrHdr.family == IPv4Family ) {
						StunAtrAddress4* atrAdd4 = reinterpret_cast<StunAtrAddress4*>( body );
						if ( hdrLen == sizeof( StunAtrAddress4 ) - 4 ) {
							port = ntohs( atrAdd4->addrHdr.port );
							sip_printf( "   address_port:   %d\n", port );
							i1 = atrAdd4->v4addr & 0xFF;
							i2 = (atrAdd4->v4addr & 0xFF00) >> 8;
							i3 = (atrAdd4->v4addr & 0xFF0000) >> 16;
							i4 = (atrAdd4->v4addr & 0xFF000000) >> 24;
							sprintf( ip, "%d.%d.%d.%d", i1, i2, i3, i4 );
							sip_printf( "   address:        %s\n", ip );
						}
					}
				}
				body += hdrLen+4;
				size -= hdrLen+4;
			}
			SipRegister *current;
			if( !QString(ip).contains( Sip::getLocalAddress() ) ||
					listener.getPortNumber() != port ) {
				Sip::setLocalAddress( ip );
				listener.forcePortNumber( port );
				setupContactUri();
				SipRegisterList reg = user->getSipRegisterList();
				SipRegisterList::Iterator it;
				for( it = reg.begin(); it != reg.end(); ++it ) {
					current = *it;
					if( current->getRegisterState() == SipRegister::Connected ||
					    current->getAutoRegister() ) {
						current->setAutoRegister( false );
						current->requestRegister();
					}
				}
			} else {
				SipRegisterList reg = user->getSipRegisterList();
				SipRegisterList::Iterator it;
				for( it = reg.begin(); it != reg.end(); ++it ) {
					current = *it;
					if( current->getAutoRegister() ) {
						current->setAutoRegister( false );
						current->requestRegister();
					}
				}
			}
			return;
		} else {
			fullmessage.append( QString::fromUtf8( inputbuf, bytesread ) );
		}
		if( bytesread < 0 ) {
			sip_perror( "SipClient::incomingMessage(): read failed" );
			tcpSockets.removeAll( tcpSocket );
			tcpSocket = 0;
			return;
		}
		QString s;
		if( isTcpSocket() ) {
			if( fullmessage.contains( "\r\n\r\n" ) > 0 ) {
				if( fullmessage.lastIndexOf( "Content-Length: " ) < fullmessage.lastIndexOf( "\r\n\r\n" ) ) {
					s = fullmessage.mid( fullmessage.lastIndexOf( "Content-Length: " ) + 16 );
					s = s.left( s.indexOf( '\r' ) );
					if( s.toInt() == 0 ) {
						if( fullmessage.right( 4 ) == "\r\n\r\n" ) {
							break;
						}
					} else {
						contentLength = s.toInt();
						s = fullmessage.mid( fullmessage.lastIndexOf( QString( "\r\n\r\n" ) ) + 4 );
						if( s.toUtf8().size() == contentLength ) {
							break;
						}
					}
				} else if( fullmessage.right( 4 ) == "\r\n\r\n" ) {
					break;
				}
			}
		} else {
			break;
		}

		// Socket closed (so we're done) WRONG
		if( bytesread == 0 ) {
			tcpSockets.removeAll( tcpSocket );
			tcpSocket = 0;
			break;
		}
	}

	// Parse input
	if( isTcpSocket() ) {
		int contentLength;
		QString s;
		while( !fullmessage.isEmpty() ) {
			contentLength = 0;
			if( fullmessage.contains( "Content-Length: " ) > 0 ) {
				if( fullmessage.indexOf( "Content-Length: " ) < fullmessage.indexOf( QString( "\r\n\r\n" ) ) ) {
					s = fullmessage.mid( fullmessage.indexOf( "Content-Length: " ) + 16 );
					s = s.left( s.indexOf( '\r' ) );
					contentLength = s.toInt();
				}
			}
			s = fullmessage.left( fullmessage.indexOf( QString( "\r\n\r\n" ) ) + 4 + contentLength );
			parseMessage( s );
			fullmessage.remove( 0, s.length() );
		}
	} else {
		parseMessage( fullmessage );
	}
}

void SipClient::parseMessage( QString fullmessage )
{
	SipMessage *curmessage = new SipMessage( fullmessage );
	if( traceMessageReceived ) {
		sip_printf( "\nSipClient: Received: %s.%03d\n---------------------------------\n%s\n",
			QTime::currentTime().toString().toUtf8().constData(), QTime::currentTime().msec(), fullmessage.toUtf8().constData() );
	}
	if( !curmessage->isValid() ) {
		delete curmessage;
		return;
	}

//#test
	if( testOn ) {
		incomingTestMessage();
		delete curmessage;
		return;
	}
	QString callid = curmessage->getHeaderData( SipHeader::Call_ID );

	// Grab address in To: header
	SipUri touri( curmessage->getHeaderData( SipHeader::To ) );
	// Grab address in From: header
	SipUri fromuri( curmessage->getHeaderData( SipHeader::From ) );
	if( curmessage->getStatus().getCode() == 487 ) {
		sendAck( curmessage );
		delete curmessage;
		return;
	}
	QString cseq = curmessage->getHeaderData( SipHeader::CSeq );
	if( curmessage->getMethod() == Sip::MESSAGE ) {
		if( callid == messageCID && cseq == messageCSeq ) {
			sip_printf( "SipCient: Received what was likely a retransmission, badly ignoring...\n" );
			delete curmessage;
			return;
		}
		messageCID = callid;
		messageCSeq = cseq;
		sendQuickResponse( curmessage, SipStatus( 200 ) );
		incomingInstantMessage( curmessage );
		delete curmessage;
		return;
	}
	if( curmessage->getMethod() == Sip::INFO ) {
		sendQuickResponse( curmessage, SipStatus( 200 ) );
	}
	if( curmessage->getMethod() == Sip::SUBSCRIBE ) {
		if( callid == subscribeCID && cseq == subscribeCSeq ) {
			sip_printf( "SipCient: Received what was likely a retransmission, badly ignoring...\n" );
			delete curmessage;
			return;
		}
		subscribeCID = callid;
		subscribeCSeq = cseq;
		bool found = false;
		QList<SipCall *>::Iterator it;
		SipCall *curcall;
		for( it = calls.begin(); it != calls.end(); ++it ) {
			curcall = *it;
			if( callid == curcall->getCallId() && curcall->getCallType() == SipCall::inSubscribeCall ) {
				curcall->incomingMessage( curmessage );
				QString expires = curmessage->getHeaderData( SipHeader::Expires );
				SipUri incominguri( curmessage->getHeaderData( SipHeader::From ) );
				if( curcall->getMember( incominguri ) ) {
					if( expires == "0" ) {
						curcall->setCallStatus( SipCall::callUnconnected );
						incomingSubscribe( 0, false );
						delete curcall;
						QList<SipCall *>::Iterator it2;
						for( it2 = calls.begin(); it2 != calls.end(); ++it2 ) {
							SipCall *c = *it2;
							if( c->getCallType() == SipCall::outSubscribeCall ) {
								if( c->getMember( incominguri ) ) {
									if( c->getCallStatus() != SipCall::callDead ) {
										c->setCallStatus(
											SipCall::callUnconnected );
									}
								}
							}
						}
					} else {
						curcall->getMember( incominguri )->timerStart( expires.toInt() * 1000 );
						curcall->setCallStatus( SipCall::callInProgress );
						incomingSubscribe( curcall->getMember( incominguri ), false );
					}
					return;
				}
				found = true;
			}
			if( callid == curcall->getCallId() && curcall->getCallType() == SipCall::inSubscribeCall_2 ) {
				curcall->incomingMessage( curmessage );
				found = true;
			}
			if( found ) {
				return;
			}
		}
		if( touri.hasTag() ) {
			sendQuickResponse( curmessage, SipStatus( 481 ) );
			delete curmessage;
			return;
		}
		SipCall *newcall;
		SipCallMember *member;
		bool sendSubscribe = true;
		if( touri == user->getUri() ) {
			newcall = new SipCall( user, curmessage->getHeaderData( SipHeader::Call_ID ),
				SipCall::inSubscribeCall );
			member = newcall->incomingMessage( curmessage );
		} else {
			newcall = new SipCall( user, curmessage->getHeaderData( SipHeader::Call_ID ),
				SipCall::inSubscribeCall_2 );
			newcall->incomingMessage( curmessage );
			delete newcall;

			return;
		}
		SipUri remoteuri( curmessage->getHeaderData( SipHeader::From ) );
		if( curmessage->getHeaderData( SipHeader::Expires ) == "0" ||
				remoteuri.reqUri() == user->getMyUri()->reqUri() ) {
			delete newcall;
		} else {
			QList<SipCall *>::Iterator it;
			for( it = calls.begin(); it != calls.end(); ++it ) {
				SipCall *curcall = *it;
				if( curcall->getCallType() == SipCall::outSubscribeCall ) {
					if( curcall->getMember( remoteuri ) ) {
						if( curcall->getCallStatus() == SipCall::callInProgress ) {
							sendSubscribe = false;
						}
					}
				}
			}
			member->setContactUri( curmessage->getContactList().getHead() );
			member->setUri( remoteuri );
			if( curmessage->getHeaderData( SipHeader::Expires).toInt() > 0 ) {
				member->timerStart( curmessage->getHeaderData( SipHeader::Expires).toInt() * 1000 );
			}
			connect( member, SIGNAL(statusUpdated(SipCallMember*)),
				this, SLOT(callMemberUpdated()) );
			member->getCall()->setCallStatus( SipCall::callInProgress );
			incomingSubscribe( member, sendSubscribe );
		}
		return;
	}

	// If the CallId exists already, pass the message to that call
	QList<SipCall *>::Iterator it;
	for( it = calls.begin(); it != calls.end(); ++it ) {
		SipCall *curcall = *it;
		if( callid == curcall->getCallId() ) {

			// Check cseq because SUBSCRIBE and NOTIFY call have same CallID
			QString cseq = curmessage->getHeaderData( SipHeader::CSeq );
			if( cseq.contains( "NOTIFY" ) && curmessage->getMethod() != Sip::NOTIFY ) {
				if( curcall->getCallType() == SipCall::inSubscribeCall ) {
					curcall->incomingMessage( curmessage );
					return;
				}
			} else {
				if( curmessage->getType() == SipMessage::Request && 
				    touri.hasTag() && 
				    curmessage->getMethod() != Sip::REFER ) {
					if( touri.getTag() != curcall->localAddress().getTag() ) {
						if( curmessage->getMethod() != Sip::ACK &&
						    curmessage->getMethod() != Sip::CANCEL ) {
							sendQuickResponse( curmessage, SipStatus( 481 ) );
						} else {
							sip_printf( "SipClient: Dropping ACK/CANCEL which deserved a 481\n" );
						}
						delete curmessage;
						return;
					}
				}
				curcall->incomingMessage( curmessage );
				return;
			}
		}
	}

	// Check message type
	if( curmessage->getType() != SipMessage::Request ) {
		sip_printf( "SipClient: No call found for incoming response. Dropping.\n" );
		delete curmessage;
		return;
	}

	// Check method
	if( curmessage->getMethod() == Sip::ACK ) {
		sip_printf( "SipClient: ACK received, but nobody was listening. Dropping.\n" );
		delete curmessage;
		return;
	}
	if( curmessage->getMethod() == Sip::CANCEL ) {
		sip_printf( "SipClient: No listener for this CANCEL, returning a 481.\n" );
		sendQuickResponse( curmessage, SipStatus( 481 ) );
		delete curmessage;
		return;
	}
	if( touri.hasTag() ) {
		sendQuickResponse( curmessage, SipStatus( 481 ) );
		delete curmessage;
		return;
	}
	if( fwmode ) {
		sip_printf( "SipClient: Forwarding call.\n" );
		if( fwbody != QString::null ) {
			sendQuickResponse( curmessage, SipStatus( 302 ), fwbody, MimeContentType( "text/plain" ) );
		} else {
			sendQuickResponse( curmessage, SipStatus( 302 ) );
		}
		delete curmessage;
		return;
	}
	if( busymode ) {
		sip_printf( "SipClient: We're busy.\n" );
		if( busybody != QString::null ) {
			sendQuickResponse( curmessage, SipStatus( 486 ), busybody, MimeContentType( "text/plain" ) );
		} else {
			sendQuickResponse( curmessage, SipStatus( 486 ) );
		}
		delete curmessage;
		return;
	}
	if( curmessage->getMethod() == Sip::REGISTER ) {
		sip_printf( "SipClient:Not Implemented, Returning a 501.\n" );
		sendQuickResponse( curmessage, SipStatus( 501 ) );
		delete curmessage;
		return;
	}
	if( curmessage->getMethod() == Sip::BadMethod ) {
		sip_printf( "SipClient: I don't recognize that method... Returning a 501.\n" );
		sendQuickResponse( curmessage, SipStatus( 501 ) );
		delete curmessage;
		return;
	}
	if( curmessage->hasHeader( SipHeader::Accept ) ) {
		if( curmessage->getMethod() == Sip::INVITE ) {
			if( !curmessage->getHeaderData( SipHeader::Accept ).toLower().contains( "application/sdp" ) ) {
				sendQuickResponse( curmessage, SipStatus( 406 ) );
				delete curmessage;
				return;
			}
		} else if( curmessage->getMethod() == Sip::SUBSCRIBE ) {
			if( !curmessage->getHeaderData( SipHeader::Accept ).toLower().contains( "application/xpidf+xml" ) ) {
				sendQuickResponse( curmessage, SipStatus( 406 ) );
				delete curmessage;
				return;
			}
		}
	}
	if( curmessage->hasHeader( SipHeader::Require ) &&
			curmessage->getHeaderData( SipHeader::Require ) != QString::null ) {
		sip_printf( "SipClient: This messages says it requires '%s', returning 420.\n",
			curmessage->getHeaderData( SipHeader::Require ).toUtf8().constData() );
		sendQuickResponse( curmessage, SipStatus( 420 ) );
		delete curmessage;
		return;
	}

	// Create a new call and pass it the message
	sip_printf( "SipClient: Searching for a user\n" );

	if( curmessage->getMethod() == Sip::OPTIONS ) {
		SipCall *newcall = new SipCall( user, curmessage->getHeaderData( SipHeader::Call_ID ) );
		newcall->incomingMessage( curmessage );
		return;
	}

	sip_printf( "SipClient: Creating new call for user %s\n", user->getUri().nameAddr().toUtf8().constData() );
	SipCall *newcall = new SipCall( user, curmessage->getHeaderData( SipHeader::Call_ID ) );
	SipCallMember *member = newcall->incomingMessage( curmessage );
	if( member == 0 ) {
		return;
	}

	// Signal that we have an incoming call
	if( curmessage->getMethod() == Sip::INVITE ) {
		QString body = curmessage->messageBody();
		if( body.contains( "m=video" ) ) {
			newcall->setCallType( SipCall::videoCall );
		}
		incomingCall( newcall, body );
	} else {
		delete curmessage;
	}
}

bool SipClient::sendRequest( SipMessage *msg, bool contact, const SipUri &regProxy, const QString &branch )
{
	if( regProxy != SipUri::null ) {
		sipProxy = regProxy;
	}

	// Create a Via tag and add it to the message at the top of the list
	SipVia regvia;
	if( isTcpSocket() ) {
		regvia.setTransport( SipVia::TCP );
	} else {
		regvia.setTransport( SipVia::UDP );
	}

	if (symmetricmode) {
		regvia.setRportParam( QString::null );
	}

	regvia.setHostname( Sip::getLocalAddress() );
	if( isTcpSocket() ) {
		regvia.setPortNumber( TCP_listener.getPortNumber() );
	} else {
		regvia.setPortNumber( listener.getPortNumber() );
	}

	if( branch != QString::null ) {
		regvia.setBranchParam( branch );
	} else {
		regvia.generateBranchParam();
	}

	msg->getViaList().insertTopmostVia( regvia );

	// Calculate content length
	msg->insertHeader( SipHeader::Content_Length, QString::number( msg->messageBody().toUtf8().size() ) );

	// Advertise shamelessly
	msg->insertHeader( SipHeader::User_Agent, getUserAgent() );
	if( msg->getMethod() == Sip::REGISTER ) {
		msg->insertHeader( SipHeader::Event, "registration" );
		msg->insertHeader( SipHeader::Allow_Events, "presence" );
	}

	if( msg->getMethod() == Sip::SUBSCRIBE ) {
		msg->insertHeader( SipHeader::Event, "presence" );
		msg->insertHeader( SipHeader::Accept, "application/xpidf+xml" );
	}

	if( msg->getMethod() == Sip::NOTIFY ) {
		msg->insertHeader( SipHeader::Event, "presence" );
	}

	// Set max-forwards
	if( maxforwards != 0 ) {
		msg->insertHeader( SipHeader::Max_Forwards, QString::number( maxforwards ) );
	}

	// Via hiding mode
	if( hidemode != DontHideVia ) {
		if( hidemode == HideHop )
			msg->insertHeader( SipHeader::Hide, "hop" );
		else
			msg->insertHeader( SipHeader::Hide, "route" );
	}

	// If this request requires the contact header, add it
	if( contact ) {
		msg->getContactList().addToHead( contacturi );
	}

	// Retransmission timestamp
	msg->setTimestamp();

	if( msg->getMethod() == Sip::REGISTER || msg->getMethod() == Sip::MESSAGE ) {
		msg->setTimeTick( 4000 );
	} else {
		msg->setTimeTick( 500 ); // T1
	}

	msg->incrSendCount();

	// Error in reguest uri
	if( msg->getRequestUri().reqUri().contains( ' ' ) ) {
		QString s = msg->getRequestUri().reqUri();
		while( s.contains( ' ' ) ) {
			s.remove( s.indexOf( ' ' ), 1 );
		}
		msg->setRequestUri( SipUri( s ) );
		sip_printf( "\nSipClient: Spaces removed from Request Uri\n" );
	}

	// Announce that we're sending a message
	if( traceMessageSending ) {
		sip_printf( "\nSipClient: Sending: %s.%03d\n--------------------------------\n%s\n",
			QTime::currentTime().toString().toUtf8().constData(), QTime::currentTime().msec(),
			msg->message().toUtf8().constData() );
	}

	// Send the message
	TCPMessageSocket *tcpSocket = 0;
	if( isTcpSocket() ) {
		bool createTcpSocket = true;
		TCPMessageSocketList sockets = getTcpSocketList();
		TCPMessageSocketList::Iterator it;
		if( useExplicitProxy ) {
			for (it = sockets.begin(); it != sockets.end(); ++it) {
				tcpSocket = *it;
				if( tcpSocket->cmpSocket( proxy.toUtf8().constData(), proxyport ) ) {
					createTcpSocket = false;
					break;
				}
			}
			if( createTcpSocket ) {
				tcpSocket = new TCPMessageSocket;
				if( !tcpSocket->setHostnamePort( proxy.toUtf8().constData(), proxyport ) ) {
					return false;
				}
				if( tcpSocket->connect( proxyport ) == -1 ) {
					delete tcpSocket;
					tcpSocket = 0;
				} else {
					if( traceMessageSending ) {
						sip_printf( "SipClient: Sending to '%s:%d' (TCP)\n",
							proxy.toUtf8().constData(), proxyport );
					}
					tcpSockets.append( tcpSocket );
				}
			}
		} else {

			// send to the host/port in the request uri
			QString sendtoaddr;
			if( msg->getRequestUri().hasMaddrParam() ) {
				sendtoaddr = msg->getRequestUri().getMaddrParam();
			} else {
				sendtoaddr = msg->getRequestUri().getHostname();
			}
			sendtoaddr = getSipProxySrv( sendtoaddr );
			if( sendtoaddr.contains( ':' ) ) {
				unsigned int port = sendtoaddr.mid( sendtoaddr.indexOf( ':' ) + 1 ).toUInt();
				msg->getRequestUri().setPortNumber( port );
				sendtoaddr = sendtoaddr.left( sendtoaddr.indexOf( ':' ) );
			}
			if( traceMessageSending ) {
				sip_printf( "SipClient: Sending to '%s:%d' (TCP)\n", sendtoaddr.toUtf8().constData(),
					msg->getRequestUri().getPortNumber() );
			}
			for (it = sockets.begin(); it != sockets.end(); ++it) {
				tcpSocket = *it;
				if( tcpSocket->cmpSocket( sendtoaddr.toUtf8().constData(), msg->getRequestUri().getPortNumber() ) ) {
					createTcpSocket = false;
					break;
				}
			}
			if( createTcpSocket ) {
				tcpSocket = new TCPMessageSocket;
				if( !tcpSocket->setHostnamePort( sendtoaddr.toUtf8().constData(), msg->getRequestUri().getPortNumber() ) ) {
					return false;
				}
				if( tcpSocket->connect( msg->getRequestUri().getPortNumber() ) == -1 ) {
					delete tcpSocket;
					tcpSocket = 0;
				} else {
					tcpSockets.append( tcpSocket );
				}
			}
		}
		if( tcpSocket != 0 ) {
			if( tcpSocket->send( msg->message().toUtf8().data(), msg->message().toUtf8().size() ) == -1 ) {
				tcpSockets.removeAll( tcpSocket );
				tcpSocket = 0;
			}
		}
	}
	if( tcpSocket == 0 ) {
		UDPMessageSocket* s;
		UDPMessageSocket sendsocket;

		if (symmetricmode) {
			s = &listener;
		} else {
			s = &sendsocket;
		}

		// Choose destination
		if( useExplicitProxy ) {
			if( !s->setHostname( proxy.toUtf8().constData() ) ) { return false; }
			if( traceMessageSending ) {
				sip_printf( "SipClient: Sending to '%s:%d'\n", proxy.toUtf8().constData(), proxyport );
			}
			s->connect( proxyport );
		} else {

			// send to the host/port in the request uri
			QString sendtoaddr;
			unsigned int sendtoport = msg->getRequestUri().getPortNumber();
			if( msg->getRequestUri().hasMaddrParam() ) {
				sendtoaddr = msg->getRequestUri().getMaddrParam();
			} else {
				SipUri route( msg->getHeaderData( SipHeader::Route ) );
				if( route.uri().contains( ";lr" ) ) {
					sendtoaddr = route.getHostname();
					sendtoport = route.getPortNumber();
				} else {
					sendtoaddr = msg->getRequestUri().getHostname();
					sendtoaddr = getSipProxySrv( sendtoaddr );
					if( sendtoaddr.contains( ':' ) ) {
						unsigned int port =
							sendtoaddr.mid( sendtoaddr.indexOf( ':' ) + 1 ).toUInt();
						msg->getRequestUri().setPortNumber( port );
						sendtoaddr = sendtoaddr.left( sendtoaddr.indexOf( ':' ) );
					}
					sendtoport = msg->getRequestUri().getPortNumber();
				}
			}
			if( traceMessageSending ) {
				sip_printf( "SipClient: Sending to '%s:%d'\n", sendtoaddr.toUtf8().constData(), sendtoport );
			}
			if( !s->setHostname( sendtoaddr.toUtf8().constData() ) ) { return false; }
			s->connect( sendtoport );
		}
		s->send( msg->message().toUtf8().data(), msg->message().toUtf8().size() );
	}
	return true;
}


void SipClient::setSymmetricMode( bool newmode)
{
	symmetricmode = newmode;
}


void SipClient::sendResponse( SipMessage *msg, bool contact )
{
	MessageSocket *outsocket = 0;
	SipVia topvia;
	QString sendaddr;

	// Calculate content length
	msg->insertHeader( SipHeader::Content_Length, QString::number( msg->messageBody().toUtf8().size() ) );

	// Advertise shamelesslysipvialist
	msg->insertHeader( SipHeader::User_Agent, getUserAgent() );

	// If this rquest requires the contact header, add it
	if( contact ) {
		msg->getContactList().addToHead( contacturi );
	}

	// Indicate what methods we allow if this is an answer to an OPTIONS
	if( msg->getHeaderData( SipHeader::CSeq ).contains( "OPTIONS" ) ) {
		msg->insertHeader( SipHeader::Allow,
			"INVITE, OPTIONS, ACK, BYE, MSG, CANCEL, MESSAGE, SUBSCRIBE, NOTIFY, INFO, REFER" );
	}

	// Use via to tell us where to send it and how
	topvia = msg->getViaList().getTopmostVia();
	switch( topvia.getTransport() ) {
		case SipVia::UDP:
			sip_printf( "SipClient: Sending UDP Response\n" );
			if (symmetricmode) {
				outsocket = &listener;
			} else {
				outsocket = new UDPMessageSocket;
			}
			break;
		case SipVia::TCP:
			sip_printf( "SipClient: Sending TCP Response\n" );
			outsocket = new TCPMessageSocket;
			break;
		case SipVia::TLS:
			sip_printf( "SipClient: TLS in top via, not supported (full TLS support not implemented)\n" );
			break;
		case SipVia::BadTransport:
			sip_printf( "SipClient: Bad transport on incoming Via\n" );
			break;
	}

	// If transport was bad, no use sending
	if( !outsocket ) return;

	// maddr, received, sentby
	if( topvia.hasMaddrParam() ) {
		sip_printf( "SipClient: Using address from maddr via parameter\n" );
		sendaddr = topvia.getMaddrParam();
	} else if( topvia.hasReceivedParam() ) {
		sip_printf( "SipClient: Using address from received via parameter\n" );
		sendaddr = topvia.getReceivedParam();
	} else {
		sendaddr = topvia.getHostname();
	}

	// Announce where we're sending
	if( traceMessageSending ) {
		sip_printf( "SipClient: Sending to '%s' port %d\n", sendaddr.toUtf8().data(), topvia.getPortNumber() );
	}
	if( !outsocket->setHostname( sendaddr.toUtf8().data() ) ) {
		if (outsocket != &listener) {
			delete outsocket;
		}
		return;
	}
	outsocket->connect( topvia.getPortNumber() );

	// Announce what we're sending
	if( traceMessageSending ) {
		sip_printf( "\nSipClient: Sending: %s.%03d\n--------------------------------\n%s\n",
			QTime::currentTime().toString().toUtf8().constData(), QTime::currentTime().msec(),
			msg->message().toUtf8().constData() );
	}

	// Send it
	outsocket->send( msg->message().toUtf8().data(), msg->message().toUtf8().size() );
	if (outsocket != &listener) {
		delete outsocket;
	}
}

void SipClient::sendRaw( SipMessage *msg )
{
	if( isTcpSocket() ) {
		return;
	}
	// Announce that we're sending
	if( traceMessageSending ) {
		sip_printf( "\nSipClient: Sending: %s.%03d\n--------------------------------\n%s\n",
			QTime::currentTime().toString().toUtf8().constData(), QTime::currentTime().msec(),
			msg->message().toUtf8().constData() );
	}
	UDPMessageSocket *sendsocket;

	if (symmetricmode) {
		sendsocket = &listener;
	} else {
		sendsocket = new UDPMessageSocket;
	}

	// Choose destination
	if( ( msg->getType() != SipMessage::Response ) && useExplicitProxy ) {
		if( !sendsocket->setHostname( proxy.toUtf8().constData() ) ) { 
			if (sendsocket != &listener) {
				delete sendsocket;
			}
			return; 
		}
		sendsocket->connect( proxyport );
	} else {

		// Send to whatever is in the request uri
		QString sendtoaddr;
		if( msg->getRequestUri().hasMaddrParam() ) {
			sendtoaddr = msg->getRequestUri().getMaddrParam();
		} else {
			sendtoaddr = msg->getRequestUri().getHostname();
		}
		if( !sendsocket->setHostname( sendtoaddr.toUtf8().constData() ) ) { 
			if (sendsocket != &listener) {
				delete sendsocket;
			}
			return; 
		}
		sendsocket->connect( msg->getRequestUri().getPortNumber() );
	}
	sendsocket->send( msg->message().toUtf8().data(), msg->message().toUtf8().size() );
	if (sendsocket != &listener) {
		delete sendsocket;
	}
}

void SipClient::addCall( SipCall *call )
{
	if( !calls.contains( call ) ) {
		calls.append( call );
	}
	callListUpdated();
}

void SipClient::callTypeUpdated( void )
{
	callListUpdated();
}

void SipClient::deleteCall( SipCall *call )
{
	calls.removeAll( call );
	callListUpdated();
}

void SipClient::setExplicitProxyMode( bool eproxy )
{
	useExplicitProxy = eproxy;
}

void SipClient::setExplicitProxyAddress( const QString &newproxy )
{
	if( newproxy.contains( ':' ) ) {
		proxy = newproxy.left( newproxy.indexOf( ':' ) );
		proxyport = newproxy.mid( newproxy.indexOf( ':' ) + 1 ).toUInt();
	} else {
		proxy = newproxy;
		proxyport = 5060;
	}
}

QString SipClient::getExplicitProxyAddress( void )
{
	QString uri = "<sip:" + proxy;
	if( proxyport != 5060 ) {
		uri += ":" + QString::number( proxyport, 10 );
	}
	uri += ";lr>";
	return uri;
}

void SipClient::setUser( SipUser *u )
{
	user = u;
}

void SipClient::sendQuickResponse( SipMessage *origmessage, const SipStatus &status,
	const QString &body, const MimeContentType &bodytype )
{
	MessageSocket *outsocket = 0;
	SipMessage *msg = new SipMessage;
	SipVia topvia;
	QString sendaddr;
	msg->setType( SipMessage::Response );
	msg->setStatus( status );

	// Copy via list exactly
	msg->setViaList( origmessage->getViaList() );
	msg->insertHeader( SipHeader::From, origmessage->getHeaderData( SipHeader::From ) );
	msg->insertHeader( SipHeader::To, origmessage->getHeaderData( SipHeader::To ) );
	msg->insertHeader( SipHeader::CSeq, origmessage->getHeaderData( SipHeader::CSeq ) );
	msg->insertHeader( SipHeader::Call_ID, origmessage->getHeaderData( SipHeader::Call_ID ) );
	if( origmessage->hasHeader( SipHeader::Require ) ) {
		msg->insertHeader( SipHeader::Unsupported, origmessage->getHeaderData( SipHeader::Require ) );
	}
	if( ( status.getCode() >= 300 ) && ( status.getCode() < 400 ) ) {
		msg->getContactList().addToHead( forwarduri );
	}
	if( status.getCode() == 501 ) {
		msg->insertHeader( SipHeader::Allow,
			"INVITE, OPTIONS, ACK, BYE, MSG, CANCEL, MESSAGE, SUBSCRIBE, NOTIFY, INFO, REFER" );
	}
	if( bodytype != MimeContentType::null ) {
		msg->insertHeader( SipHeader::Content_Type, bodytype.type() );
	}
	msg->setBody( body );

	// Calculate content length
	msg->insertHeader( SipHeader::Content_Length, QString::number( msg->messageBody().toUtf8().size() ) );

	// Advertise shamelessly
	msg->insertHeader( SipHeader::User_Agent, getUserAgent() );

	// Use via to tell us where to send it and how
	topvia = msg->getViaList().getTopmostVia();
	switch( topvia.getTransport() ) {
		case SipVia::UDP:
			sip_printf( "SipClient: Sending UDP Response\n" );
			if (symmetricmode) {
				outsocket = &listener;
			} else {
				outsocket = new UDPMessageSocket;
			}
			break;
		case SipVia::TCP:
			sip_printf( "SipClient: Sending TCP Response\n" );
			outsocket = new TCPMessageSocket;
			break;
		case SipVia::TLS:
			sip_printf( "SipClient: TLS in top via, not supported (full TLS support not implemented)\n" );
			break;
		case SipVia::BadTransport:
			sip_printf( "SipClient: Bad transport on incoming Via\n" );
			break;
	}

	// If transport is bad, no use sending
	if( !outsocket ) {
		delete msg;
		return;
	}
	// maddr, received, sentby
	if( topvia.hasMaddrParam() ) {
		sip_printf( "SipClient: Using address from maddr via parameter\n" );
		sendaddr = topvia.getMaddrParam();
	} else if( topvia.hasReceivedParam() ) {
		sip_printf( "SipClient: Using address from received via parameter\n" );
		sendaddr = topvia.getReceivedParam();
	} else {
		sendaddr = topvia.getHostname();
	}

	// Announce where we're sending
	sip_printf( "SipClient: Sending to '%s' port %d\n",
			sendaddr.toUtf8().constData(), topvia.getPortNumber() );
	if( !outsocket->setHostname( sendaddr.toUtf8().data() ) ) {
		delete msg;
		if (outsocket != &listener) {
			delete outsocket;
		}
		return;
	}
	outsocket->connect( topvia.getPortNumber() );

	// Announce what we're sending
	if( traceMessageSending ) {
		sip_printf( "\nSipClient: Sending: %s.%03d\n--------------------------------\n%s\n",
			QTime::currentTime().toString().toUtf8().constData(), QTime::currentTime().msec(),
			msg->message().toUtf8().constData() );
	}

	// Send it
	outsocket->send( msg->message().toUtf8().data(), msg->message().toUtf8().size() );
	if (outsocket != &listener) {
		delete outsocket;
	}
	delete msg;
}

void SipClient::sendTestMessage( QString sendaddr, unsigned int port, QString msg )
{
	MessageSocket *outsocket = 0;
	if (symmetricmode) {
		outsocket = &listener;
	} else {
		outsocket = new UDPMessageSocket;
	}
	outsocket->setHostname( sendaddr.toUtf8().data() );
	outsocket->connect( port );

	// Announce what we're sending
	if( traceMessageSending ) {
		sip_printf( "\nSipClient: Sending: %s.%03d\n--------------------------------\n%s\n",
			QTime::currentTime().toString().toUtf8().constData(), QTime::currentTime().msec(),
			msg.toUtf8().constData() );
	}
	// Send it
	outsocket->send( msg.toUtf8().data(), msg.toUtf8().size() );
	if (outsocket != &listener) {
		delete outsocket;
	}
}

void SipClient::sendAck( SipMessage *origmessage )
{
	SipMessage *msg = new SipMessage;
	msg->setType( SipMessage::Request );
	msg->setMethod( Sip::ACK );
	QString s = origmessage->getHeaderData( SipHeader::CSeq );
	s = s.left( s.indexOf( ' ' ) );
	s = s	+ " " + Sip::getMethodString( msg->getMethod() );
	msg->insertHeader( SipHeader::CSeq, s );
	msg->setRequestUri( SipUri( origmessage->getHeaderData( SipHeader::To ) ) );
	msg->insertHeader( SipHeader::From, origmessage->getHeaderData( SipHeader::From ) );
	msg->insertHeader( SipHeader::To, origmessage->getHeaderData( SipHeader::To ) );
	msg->insertHeader( SipHeader::Call_ID, origmessage->getHeaderData( SipHeader::Call_ID ) );
	// We need the correct branch parameter
	QString branch = origmessage->getViaList().getBottommostVia().getBranchParam();
	sendRequest( msg, true, SipUri::null, branch );
	delete msg;
}

void SipClient::setHideViaMode( HideViaMode newmode )
{
	hidemode = newmode;
}

void SipClient::setCallForward( bool onoff )
{
	fwmode = onoff;
}

void SipClient::setCallForwardUri( const SipUri &u )
{
	forwarduri = u;
}

void SipClient::setCallForwardMessage( const QString &newmessage )
{
	fwbody = newmessage;
}

const QString SipClient::getUserAgent( void )
{
        return "qtopia/4";
	//return "kphone/4.2";
}

void SipClient::setMaxForwards( int newmax )
{
	maxforwards = newmax;
}

void SipClient::setBusy( bool onoff )
{
	busymode = onoff;
}

void SipClient::setBusyMessage( const QString &newmessage )
{
	busybody = newmessage;
}

SipUser *SipClient::getUser( SipUri uri )
{
	if( uri == user->getUri() ) {
		return user;
	} else {
		return NULL;
	}
}

void SipClient::updateIdentity( SipUser *u, QString newproxy )
{
	user = u;
	setupContactUri( user );
	if( newproxy.isEmpty() || newproxy.toLower() == "sip:") {
		setExplicitProxyMode( false );
	} else {
		if( newproxy.left( 4 ).toLower() == "sip:" ) {
			newproxy.remove( 0, 4 );
		}
		setExplicitProxyMode( true );
		setExplicitProxyAddress( newproxy );
	}
}

void SipClient::sendStunRequest( QString uristr )
{
	if( !uristr.isEmpty() ) {
		useStunProxy = true;
		stunProxy = SipUri( uristr );
	}
	if( useStunProxy ) {
		if( !listener.setHostname( stunProxy.getHostname().toUtf8().constData() ) ) { return; }
		listener.connect( stunProxy.getPortNumber() );
		sip_printf( "SipClient: STUN request\n" );
		StunRequestSimple req;
		req.msgHdr.msgType = htons(BindRequestMsg);
		req.msgHdr.msgLength = htons( sizeof(StunRequestSimple)-sizeof(StunMsgHdr) );
		for ( int i=0; i<16; i++ ) {
			req.msgHdr.id.octet[i]=0;
		}
		int id = rand();
		req.msgHdr.id.octet[0] = id;
		req.msgHdr.id.octet[1] = id>>8;
		req.msgHdr.id.octet[2] = id>>16;
		req.msgHdr.id.octet[3] = id>>24;
		listener.send( (char *)&req, sizeof( req ) );
	}
}

QString SipClient::getSipProxySrv( QString dname )
{
	if( sipProxyName == dname ) {
		return sipProxySrv;
	}
	QString srv;
	QString naptr = getNAPTR( dname );
	if( !naptr.isEmpty() ) {
		srv = getSRV( naptr );
	} else {
		srv = getSRV( QString( "_sip._udp." ) + dname );
	}
	if( !srv.isEmpty() ) {
		sipProxyName = dname;
		sipProxySrv = srv;
		return sipProxySrv;
	} else {
		return dname;
	}
}

QString SipClient::getResSearch( QString dname, int type, bool UDP )
{
	unsigned char msg[PACKETSZ],*mptr,*xptr;
	int i,j,l,co;
	unsigned short *usp,ty;
	unsigned int *uip;
	res_response *res;
	char name[PACKETSZ];
	QString tmpName;
	QString domainName = "";
	u_short priority = 0;
	u_short weight = 0;
	u_short port = 5060;
	if(res_init()==-1){
		sip_printf("res_init -error !\n");
	}
	else if((l=res_search( dname.toLatin1().constData(), C_IN, type, msg, sizeof( msg ) ) ) == -1 ) {
		sip_printf( "res_search: NO result !\n" );
	} else if( l <= 0 ){
		sip_printf( "res_search: result is empty !\n" );
	} else {
		sip_printf( "res_search OK (len=%d)\n", l );
		res = (res_response *)msg;
		mptr = msg + sizeof( HEADER );
		co = ntohs( res->header.qdcount );
		for( i=0; i < co; i++ ) {
			j = dn_expand( msg, msg + PACKETSZ, mptr, name, MAXDNAME );
			if( j < 0 ) {
				break;
			} else {
				mptr += j;
				usp = (unsigned short *)mptr;
				mptr += sizeof( short );
				usp = (unsigned short *)mptr;
				mptr += sizeof( short );
			}
		}
		co = ntohs( res->header.ancount );
		for( i = 0 ; i < co; i++ ) {
			j = dn_expand( msg, msg + PACKETSZ, mptr, name, MAXDNAME );
			if( j < 0 ) {
				sip_printf( "\t\tname-error\n" );
				break;
			} else {
				mptr += j;
				usp = (unsigned short *)mptr;
				ty = ntohs( *usp );
				mptr += sizeof( short );
				usp = (unsigned short *)mptr;
				mptr += sizeof(short);
				uip = (unsigned int *)mptr;
				mptr += sizeof(int);
				uip = (unsigned int *)mptr;
				j = ntohs( *uip );
				mptr += sizeof(short);
				xptr = mptr;
				mptr += j;
				if( ty == T_NAPTR ) {
					usp = (unsigned short *)xptr;
					xptr += sizeof(short);
					usp = (unsigned short *)xptr;
					xptr += sizeof(short);
					j = (int)(*xptr);
					xptr += 1;
					while( j > 0 ) {
						xptr+=1;
						j--;
					}
					j = (int)(*xptr);
					xptr += 1;
					while( j > 0 ) {
						xptr += 1;
						j--;
					}
					j=(int)(*xptr);
					xptr+=1;
					while( j > 0 ) {
						xptr += 1;
						j--;
					}
					j = dn_expand( msg, msg + PACKETSZ, xptr, name, MAXDNAME );
					if( j < 0 ) {
						break;
					} else {
						tmpName = QString( name );
						if( UDP ) {
							if( tmpName.contains( "_udp" ) ) {
								domainName = QString( name );
							}
						} else {
							if( tmpName.contains( "_tcp" ) ) {
								domainName = QString( name );
							}
						}
						sip_printf("NAPTR: %s\n",name);
						xptr+=j;
					}
				} else if( ty == T_SRV ) {
					u_short pr;
					u_short we;
					u_short po;
					usp = (unsigned short *)xptr;
					pr = ntohs( *usp );
					xptr += sizeof( short );
					usp = (unsigned short *)xptr;
					we = ntohs( *usp );
					xptr += sizeof( short );
					usp = (unsigned short *)xptr;
					po = ntohs( *usp );
					xptr += sizeof( short );
					j = dn_expand( msg, msg + PACKETSZ, xptr, name, MAXDNAME );
					if( j < 0 ) {
						break;
					} else {
						if( !priority || pr < priority ||
						    (pr == priority && we < weight) ) {
							priority = pr;
							weight = we;
							port = po;
							sip_printf("SRV: %d,%d,%d\n",priority,weight,port);
							domainName = QString( name ) +
								":" + QString::number( port );
							sip_printf("SRV: %s\n",name);
							xptr+=j;
						}
					}
				} else {
				}
			}
		}
		co = ntohs( res->header.nscount );
		co = ntohs( res->header.arcount );
	}
	return domainName;
}

QString SipClient::getNAPTR( QString strUri )
{
	return getResSearch( strUri, T_NAPTR, true );
}

QString SipClient::getSRV( QString naptr )
{
	return getResSearch( naptr, T_SRV, true );
}

void SipClient::callMemberUpdated( void )
{
	callListUpdated();
}

void SipClient::printStatus()
{
	QList<SipCall *>::Iterator it;
	for( it = calls.begin(); it != calls.end(); ++it ) {
		SipCall *curcall = *it;
		// call type
		sip_printf("\nSipClient:printStatus: CallType = ");
		switch ( curcall->getCallType()) {
			case SipCall::StandardCall:
				printf("StandardCall");
			break;
			case SipCall::videoCall:
				printf("videoCall");
			break;
			case SipCall::OptionsCall:
				printf("OptionsCall");
			break;
			case SipCall::RegisterCall:
				printf("RegisterCall");
			break;
			case SipCall::MsgCall:
				printf("MsgCall");
			break;
			case SipCall::BrokenCall:
				printf("BrokenCall");
			break;
			case SipCall::UnknownCall:
				printf("UnknownCall");
			break;
			case SipCall::outSubscribeCall:
				printf("outSubscribeCall");
			break;
			case SipCall::inSubscribeCall:
				printf("inSubscribeCall");
			break;
			case SipCall::inSubscribeCall_2:
				printf("inSubscribeCall_2");
			break;
			default:
				printf("error - undefined call");
			break;
		}
		//call id
		printf("\n  CallId = %s", curcall->getCallId().toUtf8().constData());
		//call status
		printf("\n  CallStatus = ");
		switch ( curcall->getCallStatus()) {
			case SipCall::callUnconnected:
				printf("callUnconnected");
			break;
			case SipCall::callInProgress:
				printf("callInProgress");
			break;
			case SipCall::callDead:
				printf("callDead");
			break;
			default:
				printf("error - undefined callState");
			break;
		}
		//call members
		SipCallMemberList members = curcall->getMemberList();
		SipCallMemberList::Iterator it;
		for (it = members.begin(); it != members.end(); ++it) {
			//call member Uri
			printf("\n  CallMember: reqUri = %s", (*it)->getUri().reqUri().toUtf8().constData());
			//call member status
			printf("\n    CallMemberState = ");

			switch ( (*it)->getState()) {
				case SipCallMember::state_Idle:
					printf(" Idle");
				break;
				case SipCallMember::state_EarlyDialog:
					printf(" EarlyDialog");
				break;
				case SipCallMember::state_Connected:
					printf(" Connected");
				break;
				case SipCallMember::state_Disconnected:
					printf(" Disconnected");
				break;
				case SipCallMember::state_InviteRequested:
					printf(" InviteRequested");
				break;
				case SipCallMember::state_RequestingInvite:
					printf(" RequestingInvite");
				break;
				case SipCallMember::state_RequestingReInvite:
					printf(" RequestingReInvite");
				break;
				case SipCallMember::state_Redirected:
					printf(" Redirected");
				break;
				case SipCallMember::state_Disconnecting:
					printf(" Disconnecting");
				break;
				case SipCallMember::state_CancelPending:
					printf(" CancelPending");
				break;
				default:
					printf("error - undefined callMemberState");
				break;
			}
		}
	}
	printf("\n");
}
