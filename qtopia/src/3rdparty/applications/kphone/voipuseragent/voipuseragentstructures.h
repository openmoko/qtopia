/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

///////////////////////////////////////////////////////////////////////////
/*
typedef struct StructUserIdentity_t {
QString FullName;
SipUri_t sipUri; 
QString sipProxy;
QString authenticationUserName;
QString password;
YesNo_t isAuthenticationRequired;
AuthType_t authType;
}UserIdentity_t;
*/

#ifndef VOIPUSERAGENTSTRUCTURES_H
#define VOIPUSERAGENTSTRUCTURES_H

#define VOIP_AGENT_CHANNEL "QPE/VoIPAgent"

#include <qobject.h>
#include <qstring.h>
#include <qvaluelist.h>

class QDataStream;

typedef struct  
{
	QString fullName;
	QString userPartOfSipUri;
	QString hostPartOfSipUri;
	QString outboundProxy;
	QString authenticationUsername; 
	QString authenticationPassword; 	
	QString qValue;		//string of X.Y , where X is 0 or 1, Y is 0 to 9 
	bool  autoRegister;		
	
}SipIdentity_t;

typedef struct 
{

	int hideVia;		// 1- Dont Hide, 2- Request Next Hop, 3 - Request Full Hop
	int maxForwards; 
	int expTimeForRegistration;			// in seconds
	int expTimeForPresenceSubscription;	// in seconds
	int socketProtocol; 	// 1 -  TCP,  2 - UDP
	bool useStunServer; 	
	bool useSymmetricSignaling; 	
	bool useMediaSignaling; 	
	QString STUNServerAddress;
	int requestPeriodForSTUNServer;
	int mediaMinPort;
	int mediaMaxPort;
	QString busyMessage;	

}SipPreferences_t;

typedef struct 
{
	int OSSDeviceMode;	// 1= ReadWrite, 2 = ReadOnly/WriteOnly
	QString deviceForReadOnlyWriteOnly;
	QString deviceForReadOnly;
	QString preferredCodec;
	int preferredPayloadSize;

}AudioPreferences_t;



//////////////////////////////////////////////////
enum ReturnCode_t {
	STACK_SUCCESS=0,
	STACK_ERROR
};
///////////////////////////////////////////////////////////////////////////
enum FrameworkReturnCode_t{
	FRAMEWORK_SUCCESS=0,
	FRAMEWORK_ERROR
};
///////////////////////////////////////////////////////////////////////////

typedef class CodecInformation 
{
public:
	QString nameOfCodec;
	int payloadSizesSupported;	// {0,8} - Consider three bits representiong 3 different sizes (1st bit - 80, 2nd bit - 160 and 3rd - 240)
								// The particular bit will be 1 if that size if is supported
}CodecInformation_t;

QDataStream &operator>>( QDataStream&, CodecInformation_t& );
QDataStream &operator<<( QDataStream&, const CodecInformation_t& );

typedef struct 
{
QString StackProvidersName;
QValueList <CodecInformation_t>codecs;
}StackInfo_t;


///////////////////////////////////////////////////////////////////////////
enum AvailabilityStatus_t{
UNAVAILABLE=0,
AVAILABLE
};
///////////////////////////////////////////////////////////////////////////
typedef struct 
{
QString Uri;
}SipUri_t;
///////////////////////////////////////////////////////////////////////////
enum Codecs_t { 
PCMU, 
GSM_CODEC, 
PCMA, 
ILIBC, 
ILIBC_20, 
ILIBC_30 };
///////////////////////////////////////////////////////////////////////////
typedef class  SipContact
{	
public:
	SipContact(QString uid, QString uri) { sipContactId = uid;sipUri = uri;}
	SipContact() { }
	~SipContact() { }
	QString sipContactId;
	QString sipUri;
}SipContact_t;

QDataStream &operator>>( QDataStream&, SipContact_t& );
QDataStream &operator<<( QDataStream&, const SipContact_t& );

typedef QValueList <SipContact_t> SipContactList;
///////////////////////////////////////////////////////////////////////////
typedef struct 
{
SipIdentity_t sipIdentity;
SipPreferences_t sipPreferences;
AudioPreferences_t audioPreferences;
QValueList <SipContact_t> sipContactList;
}Configure_t;
///////////////////////////////////////////////////////////////////////////
enum ToneType_t {
BUSY_TONE=0,
RINGING_TONE };
///////////////////////////////////////////////////////////////////////////


enum State
{
	idle,		
	dialing,	    
	connected,	    
	diconnected,
	cancelled,
	transferring,
	transferred
};

// CallStatus_t needs to be in sync with the PhoneCall::State
enum CallStatus_t{
VOIP_CALL_IDLE=0,
VOIP_CALL_INCOMING,
VOIP_CALL_DIALING,
VOIP_CALL_CONNECTED,
VOIP_CALL_HOLD,
VOIP_CALL_HANGUP_LOCAL,
VOIP_CALL_HANGUP_REMOTE,
VOIP_CALL_MISSED,
VOIP_CALL_NETWORK_FAILURE,
VOIP_CALL_OTHER_FAILURE, 
};
///////////////////////////////////////////////////////////////////////////
enum RegistrationStatus_t{
VOIP_PROXY_UNREGISTERED=0,
VOIP_PROXY_CONTACTING_PROXY,
VOIP_PROXY_CONNECTION_FAILED,
VOIP_PROXY_AUTHENITCATION_FAILED,
VOIP_PROXY_AUTHENITCATION_FAILED_NEED_NEW_PASSWORD,
VOIP_PROXY_REGISTRATION_TIMEDOUT,
VOIP_PROXY_REGISTERED,
VOIP_PROXY_CONNECTION_LOST
};

#endif

