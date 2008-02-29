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

#ifndef SIPCALL_H_INCLUDED
#define SIPCALL_H_INCLUDED

#include <qobject.h>
#include <qlist.h>

#include "sipuri.h"
#include "sipurilist.h"
#include "mimecontenttype.h"

class SipCall;
class SipUser;
class SipClient;
class SipCallId;
class SipMessage;
class SipTransaction;
class TCPMessageSocket;
class QTimer;


/**
 * Object to reference a member of a call.  Contains the remote URI, their
 * current Contact URI, and their current status in the call.  It also contains
 * their current session description, and a copy of our local session
 * description for this call leg.
 *
 * This class also provides an API for modifying call state to this member of
 * the call, such as sending an invitation to join the session, or accepting a
 * call.
 */
class SipCallMember : public QObject
{
	Q_OBJECT

	friend class SipCall;
public:
	/**
	 * Creates a new member for this call with the given URI. Initially,
	 * this sets the Contact URI for the call member to be the same.
	 */
	SipCallMember( SipCall *parent, const SipUri &uri );

	/**
	 * Call member destructor.
	 */
	~SipCallMember( void );

	enum CallMemberType {
		unknown,
		Subscribe,
		Notify,
		Message,
		Invite };

	enum State {
		state_Idle,
		state_EarlyDialog,
		state_Connected,
		state_Disconnected,
		state_InviteRequested,
		state_ReInviteRequested,
		state_RequestingInvite,
		state_RequestingReInvite,
		state_Redirected,
		state_Disconnecting,
		state_CancelPending,
		state_Refer,
		state_Refer_handling };

	enum AuthState {
		authState_Authentication,
		authState_AuthenticationOK,
		authState_AuthenticationRequired,
		authState_AuthenticationTrying,
		authState_AuthenticationTryingWithPassword,
		authState_AuthenticationRequiredWithNewPassword };

	enum AuthType {
		ProxyBasicAuthenticationRequired,
		ProxyDigestAuthenticationRequired };

	/**
	 * Returns the current state of the call member.
	 */
	State getState( void ) const { return state; }

	/**
	 * Returns the most recent session description provided by the member
	 * for sending media to them.
	 */
	QString getSessionDescription( void ) const { return sessiondesc; }

	/**
	 * Returns the MIME Content Type of the session description provided by
	 * the call member. If none was provided, this function returns NULL.
	 */
	MimeContentType getSessionDescriptionType( void ) const { return sessiontype; }

	/**
	 * Returns the most recently sent local session description. Provided
	 * for reference.
	 */
	QString getLocalSessionDescription( void ) const { return localsessiondesc; }

	/**
	 * Returns the MIME Content Type of the most recently sent local
	 * session description.
	 */
	MimeContentType getLocalSessionDescriptionType( void ) const { return localsessiontype; }

	/**
	 * Returns a text description of our current status. Basically, this
	 * is the text from the response line of the last message we received,
	 * or a text description of what we're currently doing or waiting for.
	 * Useful for showing the user what is going on.
	 */
	QString getLocalStatusDescription( void ) const { return statusdesc; }

	/**
	 * Returns the most recent message body we received that was not a
	 * session description.
	 */
	QString getMostRecentMessageBody( void ) const { return recentbody; }

	/**
	 * Returns the MIME type of the most recent message body we received
	 * that was not a session description.
	 */
	MimeContentType getMostRecentMessageBodyType( void ) const { return recentbodytype; }

	/**
	 * SUBSCRIBE
	 */
	void requestSubscribe( int expiresTime = 0, const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null );
	void sendRequestSubscribe( QString username = QString::null, QString password = QString::null );
	void requestAuthSubscribe( void );
	void requestClearSubscribe( void );
	void handlingSubscribeResponse( void );

	/**
	 * NOTIFY
	 */
	void requestNotify( const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null );
	void sendRequestNotify( QString username = QString::null, QString password = QString::null );
	void requestAuthNotify( void );
	void requestClearNotify( const QString &body, const MimeContentType &bodytype );
	void handlingNotifyResponse( void );

	/**
	 * MESSAGE
	 */
	void requestMessage( const QString &body, const MimeContentType &bodytype );
	void sendRequestMessage( QString username = QString::null, QString password = QString::null );
	void requestAuthMessage( void );
	void handlingMessageResponse( void );

	/**
	 * Sends a SIP INVITE request, asking the member to join in the session
	 * described in the given body. The body and MIME type provided will
	 * become the new local session description for this call member.
	 */
	void requestInvite( const QString &body, const MimeContentType &bodytype );
	void sendRequestInvite( QString username = QString::null, QString password = QString::null );
	void requestAuthInvite( void );
	void handlingInviteResponse( void );

	/**
	 * Sends a SIP BYE request to disconnect the session.
	 */
	void requestDisconnect( void );

	/**
	 * Disconnects the session with a request to transfer to another party.
	 */
	void requestTransfer( const SipUri &transferto, const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null );

	/**
	 * Sends a SIP OPTIONS request, asking the member what they support.
	 * The body and MIME type provided serve no known purpose at this time.
	 * The response to the OPTIONS request will become the new remote
	 * session description, so this should not be called on an active call.
	 * It is provided here for consistency.
	 */
	void requestOptions( const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null );

	/**
	 * Accepts the invitation to join the session sent by the call member.
	 * The body and MIME type provided will become the new local session
	 * description for this call member.
	 */
	void acceptInvite( const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null );

	/**
	 * Declines the invitation to join the session sent by the call member.
	 * The body and MIME type provided are for possibly giving a reason as
	 * to why the call was declined.
	 */
	void declineInvite( const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null );

	/**
	 * Returns the URI for this call member.
	 */
	const SipUri &getUri( void ) const { return memberuri; }

	/**
	 * Returns the current Contact URI for this call member.
	 */
	const SipUri &getContactUri( void ) const { return contacturi; }

	/**
	 * Sets the Contact URI for this call member.
	 */
	void setContactUri( const SipUri &newcontact );

	/**
	 * Updates the URI for this call member.
	 */
	void setUri( const SipUri &newuri );

	/**
	 * Returns the list of URIs where we were redirected.
	 */
	const SipUriList &getRedirectList( void ) { return redirectlist; }

	/**
	 * Cancel Transaction.
	 */
	void cancelTransaction( void );

	/**
	 * Returns the subject for this call members call.
	 */
	QString getSubject( void );
	void contactUpdate( bool active, QString presence = "" );
	void timerStart( int time );
	void notAcceptableHere( void );
	SipCall *getCall( void ) { return call; }

	void setCallMemberType( CallMemberType type ) { callMemberType = type; }
	CallMemberType getCallMemberType( void ) { return callMemberType; }
	AuthType getAuthenticationType( void ) { return authtype; }
	AuthState getAuthState( void ) { return authstate; }
	void setIdle( void ) { op = opIdle; }
	void setState( State newstate ) { state = newstate; }

signals:
	/**
	 * This signal is sent whenever the status of this call member has
	 * changed.
	 */
	void statusUpdated( SipCallMember *member );

private slots:
	void localStatusUpdated( void );
	void remoteStatusUpdated( void );
	void call_timeout( void );

private:

	enum Operation {
		opIdle,
		opRequest,
		opClearRequest };
	Operation op;

	// For SipCall
	void incomingTransaction( SipTransaction *newtrans );
	SipUri memberuri;
	SipUri contacturi;

	CallMemberType callMemberType;
	State state;
	AuthState authstate;
	AuthType authtype;

	SipCall *call;
	SipTransaction *local;
	SipTransaction *remote;
	SipUriList redirectlist;
	QString sessiondesc;
	MimeContentType sessiontype;
	QString localsessiondesc;
	MimeContentType localsessiontype;
	QString statusdesc;
	QString recentbody;
	MimeContentType recentbodytype;
	QString proxyauthstr;
	QString proxyauthresponse;
	int localExpiresTime;
	QTimer *timer;
};

typedef QList<SipCallMember *> SipCallMemberList;

/**
 * @short A class for referencing a speficic SIP call.
 * @author Billy Biggs <bbiggs@div8.net>
 *
 * A class for referencing a specific SIP call.
 *
 */
class SipCall : public QObject
{
	Q_OBJECT

	friend class SipClient;
	friend class SipTransaction;
	friend class SipCallMember;
public:

	enum CallType {
		StandardCall,
		videoCall,
		OptionsCall,
		RegisterCall,
		MsgCall,
		BrokenCall,
		UnknownCall,
		outSubscribeCall,
		inSubscribeCall,
		inSubscribeCall_2 };

	/**
	 * Creates a new call for a given SipUser.
	 */
	SipCall( SipUser *local, const QString &id = QString::null, CallType ctype = UnknownCall );

	/**
	 * SipCall destructor. Will not attempt to send any BYEs (yet).
	 */
	~SipCall( void );

	enum CallStatus {
		callDisconneting,
		callUnconnected,
		callInProgress,
		callDead };


	void setCallStatus( CallStatus status ) { callstatus = status; }
	CallStatus getCallStatus( void ) { return callstatus; }
	QString getPresenceStatus( void ) const { return presenceStatus; }
	void setPresenceStatus( QString status );
	CallType getCallType( void ) const { return calltype; }
	void setCallType( CallType newtype );

	/**
	 * This returns the local URI that we are known by for this call.
	 * If there is none, returns the URI of our client parent.
	 */
	const SipUri &localAddress( void ) { return localuri; }

	/**
	 * Returns the associated call ID.
	 */
	const QString &getCallId( void ) { return callid; }

	/**
	 * Tries to find a @ref SipCallMember for the given URI.
	 */
	SipCallMember *getMember( const SipUri &uri );

	/**
	 * Returns the subject for this call.
	 */
	QString getSubject( void ) const { return subject; }

	/**Authentication
	 * Set the subject for this call. Will be used on all outgoing INVITEs
	 * sent afterwards.
	 */
	void setSubject( const QString &newsubject );


	SipUri getContactUri( void );

	/**
	 * Returns the contact string for this call.
	 */
	QString getContactStr( void ) const { return contactstr; }

	/**
	 * Set the contact string for this call.
	 */
	void setContactStr( const QString &newcontactstr ) { contactstr = newcontactstr; }

	/**
	 * Sends a SIP request under this call to the given call member.
	 * Returns a @ref SipTransaction for tracking.
	 */
	SipTransaction *newRequest( SipCallMember *member, Sip::Method meth,
			const QString &body = QString::null,
			const MimeContentType &bodytype = MimeContentType::null,
			const SipUri &transferto = SipUri::null,
			const QString &proxyauthentication = QString::null,
			int expiresTime = -1 );

	/**
	 * Sends a SIP register under this call to the given call member.
	 * Returns a @ref SipTransaction for tracking.
	 */
	SipTransaction *newRegister( const SipUri &registerserver, int expiresTime,
			const QString &authentication = QString::null,
			const QString &proxyauthentication = QString::null,
			const QString &qvalue = QString::null,
			const QString &body = QString::null,
			const MimeContentType &bodytype = MimeContentType::null );

	/**
	 * Returns an iterator for the list of all current members of thisAuthentication
	 * call.
	 */
	SipCallMemberList getMemberList( void ) const { return members; }

	/**
	 * Returns ProxyUsername
	 */
	QString getProxyUsername( void );

	/**
	 * Returns Hostname
	 */
	QString getHostname( void );

	/**
	 * Returns uri of SipProxy
	 */
	QString getSipProxy( void );

	/**
	 * Returns uri of SipProxy
	 */
	QString getSipUri( void );

	/**
	 * Sets ProxyUsername.
	 */
	void setProxyUsername( QString newUsername );

	/**
	 * Returns password or QString:null if password not given for this call.
	 */
	QString getPassword( void );

	/**
	 * Sets the password for this call.
	 */
	void setPassword( QString newPassword );

	/**
	 * Returns pointer to localuri.
	 */
	SipUri *getPointerToLocaluri( void ) { return &localuri; }

	void hideCallWidget( void );
	void setSdpMessageMask( QString body ) { bodyMask = body; }
	QString getSdpMessageMask( void ) { return bodyMask; }
	void updateSubscribes( void );

signals:
	/**
	 * Triggered whenever the call status changes.
	 */
	void callStatusUpdated( void );

	/**
	 * Triggered whenever the call subject changes.
	 */
	void subjectChanged( void );

private:
	// Our client
	SipClient *parent;
	// Localuri defines who we are for this call. This is generated from
	// the given SipUser, but after that we have no more association with
	// the SipUser object
	SipUri localuri;

	// Our lists
	SipCallMemberList members;
	QList<SipTransaction *> transactions;

	// CSeq numbers, local and remote
	unsigned int lastremoteseq;
	unsigned int lastseq;

	// Call state
	CallType calltype;
	CallStatus callstatus;
	QString callid;
	QString subject;
	QString presenceStatus;
	bool hasrecordroute;
	SipUriList recordroute;
	bool hasroute;
	SipUriList route;
	SipUri getLocaluri( void ) const { return localuri; }

	// Audit call state
	void auditCall( void );

	// For readability of the code
	SipCallMember *incomingRequest( SipMessage *message );
	void incomingResponse( SipMessage *message );

	// For SipTransaction
	bool sendRequest( SipMessage *reqmsg, bool contact = true, const SipUri &proxy = SipUri::null, const QString &branch = QString::null );
	void sendResponse( SipMessage *reqmsg, bool contact = true );
	void sendRaw( SipMessage *msg );

	// For SipClient
	SipCallMember *incomingMessage( SipMessage *message );
	QList<SipTransaction *> &getTransactionList( void ) { return transactions; }

	// For SipCallMember
	void addMember( SipCallMember *newmember );

	QString bodyMask;
	QString contactstr;

	bool looseRoute;

};

typedef QList<SipCall *> SipCallList;

#endif // SIPCALL_H_INCLUDED

