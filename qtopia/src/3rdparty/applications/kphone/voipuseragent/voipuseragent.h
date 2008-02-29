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
#ifndef VOIPUSERAGENT_H
#define  VOIPUSERAGENT_H

#include "voipuseragentinterface.h"
#include "voipuseragentstructures.h"

#include <qobject.h>
#include <qapplication.h>
#include <qtranslator.h>
#include <qtextcodec.h>
//#include <qptrlist.h>
#include "kcallwidget.h"
#include "kphone.h"
#include "kphoneview.h"
#include "siputil.h"
#include "sipregister.h"
#include "ksipregistrations.h"
#include "sdp.h"
#include "sipclient.h"
#include <qmap.h>

//#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/global.h>
//#endif


enum hideVia_t{
NoHide = 1,
HideHop,
HideRoute
};

enum protocolType_t {
TCP=1,
UDP
};




class AdaptationLayerCallInfo : public QObject
{
	Q_OBJECT
public:
	AdaptationLayerCallInfo(	KCallWidget *,QString ,CallStatus_t );
	~AdaptationLayerCallInfo(void);
	

	KCallWidget *getCallWidgetIdentifier(void) const { return callW;}
	QString getCallIdentifier(void) const{ return callIdentifier; }
	CallStatus_t  getCallStatus(void) const{ return callStatus; }

	void setCallWidgetIdentifier(KCallWidget *newCallW) { callW = newCallW;}
	void setCallIdentifier(QString newCallIdentifier) { callIdentifier = newCallIdentifier; }
	void setCallStatus(CallStatus_t newCallStatus){ callStatus = newCallStatus; }	

private:
	KCallWidget *callW;
	QString callIdentifier;
	CallStatus_t callStatus;	

};

// Contact related data structures 
enum ContactStatus {
CONTACT_OFFLINE=0,
CONTACT_ONLINE
};

/*
class AdaptationLayerCallInfo : public QObject 
{ 
public:
	inline KCallWidget *getCallWidgetIdentifier() { return callW;) 
	QString getCallIdentifier() { return callIdentifier; }
	int getcallStatus(){ return callStatus; }

	void setCallWidgetIdentifier(KCallWidget *newCallW) { callW = newCallW;) 
	void setCallIdentifier(QString newCallIdentifier) { callIdentifier = newCallIdentifier; }
	void setcallStatus(int newCallStatus){ callStatus = newCallStatus; }	

private:
	KCallWidget *CallW;
	QString callIdentifier;
	int callStatus;	
};
*/
class VoIPUserAgent;

class PhoneSocket;

class VoIPUserAgent : public VoIPUserAgentInterface 
{
	Q_OBJECT
public:


	VoIPUserAgent();
	~VoIPUserAgent() { }

	KPhone *hphone;
	KSipRegistrations *sipRegistrationObject;
	KCallWidget *hCallW, *incomingCallW, *currentCall;
	// KPhone kphonehandle;
	//Qsettings settings
	// CallList CallIdentifierList;
	// CurrentCallIdentifier 
	
	void getVoIPUserAgentInfo();
	void initializeVoIPUserAgent(Configure_t configurationData);
	void registerToSipProxy();
	void changeAvailabilityStatus(AvailabilityStatus_t status);
	void dial(QString numberToCall, QString callIdentifier);
	void accept(QString callIdentifier);
	void hangup(QString callIdentifier);
	void closeCall(QString callIdentifier);
	void feedTone(QString callIdentifier, QString toneString);
	void updateIdentity(SipIdentity_t sipIdentity);
	void updateSipPreferences(SipPreferences_t sipPreferences);
	void updateAudioPreferences(AudioPreferences_t audioPreferences);
	void addContactToContactsList(const QString& contactUri);
	void deleteContactFromContactsList(const QString& contactUri);
	void logoutFromSipProxy();
	void deInitializeStack();
	void startPhoneHandler();

private slots:
	void VoIPAgentMessage( const QCString& msg, const QByteArray& data );
	void socketReceived( const QStringList& cmd );
	void socketClosed();
	void delayedRegTimerTimeout();
	
private:
	static VoIPUserAgent *m_VoIPUserAgent;
   	QCopChannel *VoIPAgentChannel;
	PhoneSocket *VoIPSocket;

private:

	//Programming Interface	
	int  VoIPInit(/*QStringList listOfContacts*/);
	int  VoIPRegister(const QString &username, const QString &password);
	int  VoIPlogout( void );
	int  VoIPDial(KCallWidget *&hC, QString number );
	int  VoIPCallHangup(KCallWidget *hC);
	int  VoIPCallAccept(KCallWidget *hC);
	int  VoIPCallClose(KCallWidget *hC);
	int  VoIPGetAvailabilityStatus();
	int  VoIPChangeAvailabilityStatus(int AvailStatus);
	int  UpdateContactList(QStringList updatedlistContacts);
	int  VoIPDeInit();
public:
	//callbacks
	void incomingCallindicationCallback(KCallWidget *);
	void incomingContactSubscribeCallback(SipUri sipUri);
	void CallStatusCallback(KCallWidget *callW);
	void RegistrationStatusCallback(int rState);                                                  
	void changePresenceStatusCallback(QString uristr, int presenceState );
	
private:
	//internal functions
	KCallWidget* getCallWidget(QString);
	QString getCallIdentifier(KCallWidget* );
	CallStatus_t getCallStatus(SipCallMember *);
	CallStatus_t getCallStatus(KCallWidget *);
	void setCallStatus(SipCallMember *, CallStatus_t  );
	QString getCallidentifier(SipCallMember *member);
	QString adaptationParseUri(QString numberToCall);
	void getUriStringList(QStringList  & contactsUriStringList );
	void sendStatusChange( const QString& id, int status, const QString& callerId );
	void sendCallingName( const QString& id, const QString& name );
	void sendRegistrationChangeNow( int status );
	void sendRegistrationChange( int status );
	
private:
	QString userPrefix;
	//QStringList cList; // this is maintained separately and is used to update the callList available with the SipStack
	QStringList monitoredUris;  // list of sip uris that we are monitoring.
	
	
	StackInfo_t stackInfo;
	int registrationStatus;
	CallStatus_t m_CallStatus; // would finally go in the adatation layer call list 
	QString m_callIdentifier; // would finally go in the adatation layer call list 
	QList  <AdaptationLayerCallInfo> adaptationLayerCallList;

	QString currentSipServerName; 	// would be used to append to the standard telephony number. 
	int currentRegStatus;
	QTimer *delayedRegTimer;
	
};



#endif 
