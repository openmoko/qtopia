/*
 * Copyright (c) 2000 Billy Biggs <bbiggs@div8.net>
 * Copyright (c) 2004 Wirlab <kphone@wirlab.net>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 */

#ifndef SIPCLIENT_H_INCLUDED
#define SIPCLIENT_H_INCLUDED

#include <qobject.h>
#include <qstring.h>
#include <qlist.h>

#include "sipuri.h"
#include "udpmessagesocket.h"
#include "tcpmessagesocket.h"
#include "sipcall.h"
#include "sipuser.h"

class SipStatus;
class SipMessage;
class SipCallMember;
class MimeContentType;

typedef unsigned char  UInt8;
typedef unsigned short UInt16;
typedef unsigned int   UInt32;
typedef struct { unsigned char octet[16]; }  UInt128;

// define types for a stun message
const UInt16 BindRequestMsg          = 0x0001;
const UInt16 BindResponseMsg         = 0x0101;
const UInt16 BindErrorMsg            = 0x0111;
const UInt16 SharedSecretRequestMsg  = 0x0002;
const UInt16 SharedSecretResponseMsg = 0x0102;

/// define a structure to hold a stun address
const UInt8  IPv4Family = 0x01;
const UInt8  IPv6Family = 0x02;

// define  stun attribute
const UInt16 MappedAddress = 0x0001;

typedef struct
{
	UInt8 pad;
	UInt8 family;
	UInt16 port;
} StunAddrHdr;

typedef struct
{
	UInt16 type;
	UInt16 length;
} StunAtrHdr;

typedef struct
{
	UInt16 msgType;
	UInt16 msgLength;
	UInt128 id;
} StunMsgHdr;

typedef struct
{
	StunAddrHdr addrHdr;
	union {
		UInt32  v4addr;
		UInt128 v6addr;
	} addr;
} StunAddress;

typedef struct
{
	UInt16 type;
	UInt16 length;
	StunAddress address;
} StunAtrAddress;

typedef struct
{
	UInt16 type;
	UInt16 length;
	StunAddrHdr addrHdr;
	UInt32  v4addr;
} StunAtrAddress4;

typedef struct
{
	StunMsgHdr msgHdr;
} StunRequestSimple;


class SipClient : public QObject
{
	Q_OBJECT

	friend class SipUser;
	friend class SipCall;
public:
	SipClient( QObject *parent = 0, const char *name = 0, unsigned int newListenport = 0,
		bool newLooseRoute = true, bool newStrictRoute = true, QString socketStr = QString::null );
	~SipClient( void );
	void doSelect( bool block );
	void sendStunRequest( const QString uristr = QString::null );
	const SipUri &getContactUri( void ) { return contacturi; }
	SipUser *getUser( void ) const { return user; }
	void setUser( SipUser *user );
	void setCallForwardUri( const SipUri &u );
	const SipUri &getCallForwardUri( void ) { return forwarduri; }
	void setCallForward( bool onoff );
	bool getCallForward( void ) const { return fwmode; }
	void setCallForwardMessage( const QString &newmessage );
	QString getCallForwardMessage( void ) const { return fwbody; }
	void setBusy( bool onoff );
	bool getBusy( void ) const { return busymode; }
	void setBusyMessage( const QString &newmessage );
	QString getBusyMessage( void ) const { return busybody; }
	bool getProxyMode( void ) { return useProxyDial; }
	void setExplicitProxyMode( bool eproxy );
	bool getExplicitProxyMode( void ) { return useExplicitProxy; }
	void setExplicitProxyAddress( const QString &newproxy );
	const QString &getExplicitProxyUri( void ) { return proxy; }
	QString getExplicitProxyAddress( void );
	void setMaxForwards( int newmax );
	int getMaxForwards( void ) const { return maxforwards; }

	/**
	 * Via hiding mode.
	 */
	enum HideViaMode {
		DontHideVia,
		HideHop,
		HideRoute };
	void setHideViaMode( HideViaMode newmode );
	HideViaMode getHideViaMode( void ) { return hidemode; }

	/**
	 * set rport parameter and be symmetic
	 */
	void setSymmetricMode( bool newmode );

	/**
	 * True if we are symmetric
	 */
	bool getSymmetricMode( void ) { return symmetricmode; }

	SipCallList getCallList( void ) const { return calls; }
	static const QString getUserAgent( void );
	QString getAuthenticationUsername( void ) const { return authentication_username; }
	QString getAuthenticationPassword( void ) const { return authentication_password; }
	QString getSipProxy( void ) const { return proxy; }

	/**
	 * Return NULL if uri don't match
	 */
	SipUser *getUser( SipUri uri );

	void updateIdentity( SipUser *user, QString newproxy);
	enum Socket { UDP, TCP };
	bool isTcpSocket( void ) const { return SocketMode == TCP; }
	void setSocketMode( Socket socket ) { SocketMode = socket; }
	QString getSipProxySrv( QString dname );
	bool isLooseRoute( void ) const { return looseRoute; }
	bool isStrictRoute( void ) const { return strictRoute; }
	void sendTestMessage( QString sendaddr, unsigned int port, QString msg );
	void setTest( bool on ) { testOn = on; }
	void setUseProxyDial( bool on ) { useProxyDial = on; }
	QString getNAPTR( QString strUri );
	QString getSRV( QString naptr );
	void printStatus();
	TCPMessageSocketList getTcpSocketList( void ) const { return tcpSockets; }


private slots:
	void callMemberUpdated( void );
        void incomingTcpConnection();
	void incomingMessage( int socketfd );

signals:
	void incomingCall( SipCall *, QString );
	void hideCallWidget( SipCall * );
	void callListUpdated( void );
	void incomingInstantMessage( SipMessage * );
	void incomingNotify( SipMessage * );
	void incomingSubscribe( SipCallMember *, bool );
	void incomingTestMessage();
	void updateSubscribes( void );

private:
	SipCallList calls;

	QString authentication_username;
	QString authentication_password;
	Socket SocketMode;
	int clilen, newsockfd;
	struct sockaddr_in cli_addr;

	// Our proxy, if applicable.
	QString proxy;
	unsigned int proxyport;
	bool useProxyDial;
	bool useExplicitProxy;
	SipUri sipProxy;
	QString sipProxySrv;
	QString sipProxyName;
	SipUri contacturi;

	// Call forwarding
	bool fwmode;
	SipUri forwarduri;
	QString fwbody;

	// Busy
	bool busymode;
	QString busybody;

	// Max-forwards
	int maxforwards;

	// Via hide mode
	HideViaMode hidemode;

	// Symmetric signalling mode
	bool symmetricmode;

	SipUser *user;
	UDPMessageSocket listener;
	TCPMessageSocket TCP_listener;

	// Log stuff to a file
	int loggerfd;

	// Audit pending messages (retransmissions).
	void auditPending( void );

	void setupContactUri(  SipUser *user = 0 );
	bool setupSocketStuff( unsigned int newListenport, QString socketStr );
	void parseMessage( QString fullmessage );
	void sendQuickResponse( SipMessage *origmessage, const SipStatus &status,
		const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null );

	void sendAck( SipMessage *origmessage );

	// These methods are for SipCall
	void addCall( SipCall *call );
	void deleteCall( SipCall *call );
	bool sendRequest( SipMessage *msg, bool contact = true,
		const SipUri &regProxy = SipUri::null, const QString &branch = QString::null );
	void sendResponse( SipMessage *msg, bool contact = true );
	void sendRaw( SipMessage *msg );
	void callTypeUpdated( void );
	QString getResSearch( QString dname, int type, bool UDP );
	QString messageCID;
	QString subscribeCID;
	bool useStunProxy;
	SipUri stunProxy;
	QString messageCSeq;
	QString subscribeCSeq;
	bool looseRoute;
	bool strictRoute;

	bool testOn;

	TCPMessageSocket *tcpSocket;
	TCPMessageSocketList tcpSockets;
};

#endif // SIPCLIENT_H_INCLUDED
