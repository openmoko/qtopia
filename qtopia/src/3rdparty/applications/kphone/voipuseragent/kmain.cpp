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
 
#include <qapplication.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include "kcallwidget.h"
#include "kphone.h"
#include "kphoneview.h"
#include "siputil.h"
#include "sipregister.h"
#include "ksipregistrations.h"
#include "sdp.h"
#include "sipclient.h"

#include <voipuseragent.h>

// Stack interaction
QStringList getListofContact();



// int VoIPInit(QApplication *a, int argc, char **argv)
// Description:
// This Programming Interface call is called to initialize the VoIP Stack
// The framework shall listen to the states and
// communicate up the result of the registration
// Parameters:
// IN QString&username - user name
// IN QString&password - password
// Version 0.01
int VoIPUserAgent::VoIPInit(/*QStringList listOfContacts*/)
{
	int listenport = 5060;
	QString port = "";
	QString user = "";
	QString autodial = "";
	QString autodtmf = "";
	bool looseRoute = false;
	bool strictRoute = false;

	//int argc=2;
	//char argv[][10]={
	// "./kphone",
	// "-qws"
	//};

/*	// list logic
	int found = false;
	QString tmpstr;
	AdaptationLayerContact *adaptationLayerContactItem;
	QStringList::Iterator it;

	//store it contactsUriStringList	contactsUriStringList = listOfContacts;
  
	for(it = listOfContacts.begin(); it != listOfContacts.end(); ++it) 
	{
		tmpstr = (*it).stripWhiteSpace();
		for ( adaptationLayerContactItem = adaptationLayerContactList.first(); adaptationLayerContactItem!= 0; adaptationLayerContactItem= adaptationLayerContactList.next() ) 
		{
			if(adaptationLayerContactItem->uriStr == tmpstr)
			found = true;
		}
		// if not found then create and append it to the contact List
		if(!found)
		{
/ * RJ - ToDo: add this logic
			Contact *tmp = new Contact(tmpstr);
			if(tmp)
			adaptationLayerContactList.append( tmp);
			contactsUriStringList.append(tmpstr);
			found = false;
* /
		}
		else
		{
			found =  false;
		}

	}
*/


	KPhone *phone = new KPhone( listenport, user, looseRoute, strictRoute );
  	hphone = phone;
   return 0;
}

// int VoIPUserAgent::VoIPDeInit(void)
// Description:
// This Programming Interface call is called deinitialize the kphone and adaptation layer
// Parameters:
// void
int VoIPUserAgent::VoIPDeInit()
{
	QString tmp;

  // TBD - Please don't delete this
  // Drop all the calls if any active

	//delete hphone;
  return 0;

  }

// int VoIPUserAgent::VoIPRegister(const QString &username, const QString &password)
// Description:
// This Programming Interface call is called to register to the VoIP Server
// The framework shall listen to the states and communicate up the result of the registration
// Parameters:
// IN QString&username - user name
// IN QString&password - password

int VoIPUserAgent::VoIPRegister(const QString &username, const QString &password)
{
  sipRegistrationObject = hphone->getSipRegistrations();

  if(sipRegistrationObject)
  {
    printf("Valid registration object with me");
    qWarning("----------------VoIPUserAgent::VoIPRegister----------Valid registration object with me");
    sipRegistrationObject->RegisterVoIPSip(username, password);
  }
  return 0;
}


int VoIPUserAgent::VoIPlogout(void)
{
  sipRegistrationObject = hphone->getSipRegistrations();

  if(sipRegistrationObject)
  {
	sipRegistrationObject->unregAllRegistration();
    	printf("Valid registration object with me");
    //sipRegistrationObject->RegisterVoIPSip(username, password);
  }
  return 0;
}

// int VoIPCallHangup(KCallWidget *hC)
// Description:
// This Programming Interface call is supposed to be called to hangup active call or reject the incoming call.
// Parameters:
// OUT KCallWidget *hC
// IN QString number 
int VoIPUserAgent::VoIPDial(KCallWidget *&hC, QString number )
{

  printf("\n Adaption Layer::VoIPDial -inside \n");
  // For audio Call
  if(hphone)
    hC = hphone->getView()->VoIPCallDial( number, SipCall::StandardCall );

  printf("\n Adaption Layer::VoIPDial -out of it \n");
  return 0;
}

// int VoIPUserAgent::VoIPCallHangup(KCallWidget *hC)
// Description:
// This Programming Interface call is supposed to be called to hangup active call or reject the incoming call.
// Parameters:
// IN KCallWidget *hC
int VoIPUserAgent::VoIPCallHangup(KCallWidget *hC)
{

 printf("\n Adaption Layer::VoIPCallHangup -inside \n");
 if(hC)
  hC->hangupCall();
 printf("\n Adaption Layer::VoIPCallHangup -out \n");

 return 0;

}


// int VoIPUserAgent::VoIPCallClose(KCallWidget *hC)
// Description:
// This Programming Interface call is supposed to be called to destroy the call object
// This is called to do the cleanup after disconnect
// Parameters:
// IN KCallWidget *hC
int VoIPUserAgent::VoIPCallClose(KCallWidget *hC)
{

 printf("\n Adaption Layer::VoIPCallHangup -inside \n");
 if(hC)
  hC->forceDisconnect();
 printf("\n Adaption Layer::VoIPCallHangup -out \n");

 // TBD -- To be Done
 // Important
 // Need to check how the SIPCallMEmbers and SIPCalls are getting
 // Deleted from the KPHONE STACK
  return 0;
}


// Programming Interface  VoIPUserAgent::VoIPCallAccept(QStringList listContacts )
// Description:
// This Programming Interface is supposed to be called when an incoming call is accepted by
// user at GUI level
// Parameters:
// IN KCallWidget *hC

int  VoIPUserAgent::VoIPCallAccept(KCallWidget *hC)
{
	if( hC->curstate == KCallWidget::Called ) 
    // Accept
		hC->acceptCall();
  return 0;
}

// Programming Interface  VoIPUserAgent::UpdateContactList(QStringList listContacts )
// Description:
// This Programming Interface  is supposed to be called every time we have a VoIP contact added or
// deleted in GUI this list should be updated list shuold be passed on to the
// stack using this Programming Interface call.
// Parameters:
//  IN QStringList updatedlistContacts

int VoIPUserAgent::UpdateContactList(QStringList updatedlistContacts)
{
  if(hphone)
   	hphone->getView()->updateVoIPContactList( updatedlistContacts);
  return 0;
}


// Programming Interface  VoIPUserAgent::VoIPChangeAvailabilityStatus(int AvailStatus)
// Description:
// This Programming Interface call shall be called to change the status of the
// availaiility to Offline/Online
// Parameters:
// IN int Availbility Status

int  VoIPUserAgent::VoIPChangeAvailabilityStatus(int AvailStatus)
{

  if(hphone)
    hphone->getView()->ChangeAvailabilityStatus( AvailStatus);
    
  return 0;

}



// Programming Interface  VoIPUserAgent::VoIPGetAvailabilityStatus()
// Description:
// This Programming Interface call shall be called to get the status of the
// availaiility: either Offline or Online
// Parameters:
// IN int Availbility Status

int  VoIPUserAgent::VoIPGetAvailabilityStatus()
{

  if(hphone)
    return hphone->getView()->GetAvailabilityStatus();
    
  return 0;

}



void VoIPUserAgent::RegistrationStatusCallback(int rState)
{
   //printf("\n Adaption Layer::RegistrationStatusCallback Inside \n");
	RegistrationStatus_t status = VOIP_PROXY_UNREGISTERED;
	bool flagSendStatus = true;

    qWarning("Registration state changed" );
	QStringList contactsUriStringList; // to be used in case the contact uri list with teh stack needs to be updated
	if (registrationStatus != rState)
	{	
		
		switch(rState) {
		case SipRegister::NotConnected:
			status = VOIP_PROXY_UNREGISTERED;
			//send empty list to the stack after registration
			contactsUriStringList.clear();
			UpdateContactList(contactsUriStringList);
			break;
			
		case SipRegister::TryingServer:
		case SipRegister::TryingServerWithPassword:
			status = VOIP_PROXY_CONTACTING_PROXY;
			break;
		case SipRegister::AuthenticationRequired:
			flagSendStatus = false;
			break;

		case SipRegister::AuthenticationRequiredWithNewPassword:
			status = VOIP_PROXY_AUTHENITCATION_FAILED_NEED_NEW_PASSWORD;
			break;
		case SipRegister::Connected:
			//   VoIPDial(hCallW, num);
	      	qDebug("Registration state: Connected \n" );
			status = VOIP_PROXY_REGISTERED;
			//send teh uri string list of contacts to teh stack after initialization			
			//get the uriStringList from adaptationLayerContactList
				
			contactsUriStringList.clear();
			getUriStringList(contactsUriStringList );
			//initialize the contact subscription with the above list
			UpdateContactList(contactsUriStringList);
			// clear the temporary list as it is passed by value
			contactsUriStringList.clear();
			break;
		case SipRegister::Disconnecting:
			flagSendStatus = false;      
			break;
		case SipRegister::Reconnecting:	      
			flagSendStatus = false;  
			break;
		}

		if (flagSendStatus  == true)
		{
			qWarning("-----------VoIPUserAgent::RegistrationStatusCallback-----------  %d ",(int)status);            
			sendRegistrationChange( (int)status );
		}
		registrationStatus = rState;

	}
	
}

void VoIPUserAgent::CallStatusCallback(KCallWidget *callW)
{
	SipCallMember * member = callW->getCallMember();

	CallStatus_t fwNewCallStatus = VOIP_CALL_HANGUP_LOCAL; // The status to be sent over the VOIP_AGENT_CHANNEL
	bool flagSendStatus = false;	
	CallStatus_t fwCurrentCallStatus = getCallStatus(callW);

	switch(fwCurrentCallStatus)
	{
	case VOIP_CALL_IDLE:
		qDebug("VoIPUserAgent::CallStatusCallback - fwCurrentCallStatus  -VOIP_CALL_IDLE" );
		break;
	case VOIP_CALL_INCOMING:
		qDebug("VoIPUserAgent::CallStatusCallback - fwCurrentCallStatus  -VOIP_CALL_INCOMING" );
		break;
	case VOIP_CALL_DIALING:
		qDebug("VoIPUserAgent::CallStatusCallback - fwCurrentCallStatus  -VOIP_CALL_DIALING" );
		break;
	case VOIP_CALL_CONNECTED:
		qDebug("VoIPUserAgent::CallStatusCallback - fwCurrentCallStatus  -VOIP_CALL_CONNECTED" );
		break;
	case VOIP_CALL_HOLD:
		qDebug("VoIPUserAgent::CallStatusCallback - fwCurrentCallStatus  -VOIP_CALL_HOLD" );
		break;
	case VOIP_CALL_HANGUP_LOCAL:
		qDebug("VoIPUserAgent::CallStatusCallback - fwCurrentCallStatus  -VOIP_CALL_HANGUP_LOCAL" );
		break;	
	case VOIP_CALL_HANGUP_REMOTE:
		qDebug("VoIPUserAgent::CallStatusCallback - fwCurrentCallStatus  -VOIP_CALL_HANGUP_REMOTE" );
		break;
	case VOIP_CALL_MISSED:
		qDebug("VoIPUserAgent::CallStatusCallback - fwCurrentCallStatus  -VOIP_CALL_MISSED" );
		break;
	case VOIP_CALL_NETWORK_FAILURE:
		qDebug("VoIPUserAgent::CallStatusCallback - fwCurrentCallStatus  -VOIP_CALL_NETWORK_FAILURE" );
		break;
	case VOIP_CALL_OTHER_FAILURE:
		qDebug("VoIPUserAgent::CallStatusCallback - fwCurrentCallStatus  -VOIP_CALL_OTHER_FAILURE" );
		break;
	default :
		qDebug("VoIPUserAgent::CallStatusCallback - fwCurrentCallStatus  -UNKNOWN_STATE" );
		break;
	}


	switch (member->getState() )
	{
		case SipCallMember::state_Idle:
			qDebug("VoIPUserAgent::CallStatusCallback - state_Idle ");
			//if (fwCurrentCallStatus != VOIP_CALL_IDLE) {  // this state does not make sense right now.
			//	fwNewCallStatus= VOIP_CALL_IDLE;//idle		
			//	flagSendStatus = true;
			//	}
			break;
			
		case SipCallMember::state_EarlyDialog:
			qDebug("VoIPUserAgent::CallStatusCallback -state_EarlyDialog ");
			// ToDo - how to map this site
			//if (fwCurrentCallStatus != VOIP_CALL_CONNECTED) {
			//	fwNewCallStatus = VOIP_CALL_CONNECTED;//this state is 'ringing' for outgoing call
			//	flagSendStatus = true;		
			//	}
			break;
			
		case SipCallMember::state_Connected:
			qDebug("VoIPUserAgent::CallStatusCallback -state_Connected ");
			if(fwCurrentCallStatus == VOIP_CALL_HANGUP_LOCAL) {
				fwNewCallStatus = VOIP_CALL_HANGUP_LOCAL; // This is a patch for an error condition in dissipate
															// On calcelling a ringing outgoing call, we get a connected state!
				flagSendStatus = true;

			}else if (fwCurrentCallStatus != VOIP_CALL_CONNECTED) {
				fwNewCallStatus = VOIP_CALL_CONNECTED;//this state is 'connected' for incoming as well as 'outgoing' calls
				flagSendStatus = true;		
				}

			break;
			
		case SipCallMember::state_Disconnected:
			qDebug("VoIPUserAgent::CallStatusCallback -state_Disconnected ");
			if (fwCurrentCallStatus == VOIP_CALL_CONNECTED) {			
				fwNewCallStatus = VOIP_CALL_HANGUP_REMOTE;
				flagSendStatus = true;
			}else if (fwCurrentCallStatus == VOIP_CALL_DIALING) {	
				fwNewCallStatus = VOIP_CALL_HANGUP_REMOTE; // This status should be passed up only after state_Disconnected 
														 //on this call is receiveds				
				flagSendStatus = true;
			}else if (fwCurrentCallStatus == VOIP_CALL_HANGUP_LOCAL) {
				fwNewCallStatus = VOIP_CALL_HANGUP_LOCAL; // This status should be passed up only after state_Disconnected 
														 //on this call is received
				flagSendStatus = true;
			}else if (fwCurrentCallStatus == VOIP_CALL_INCOMING) {	
				fwNewCallStatus = VOIP_CALL_MISSED; // This status should be passed up only after state_Disconnected 
														 //on this call is receiveds				
				flagSendStatus = true;
			}			
			break;
			
		case SipCallMember::state_InviteRequested:
			qDebug("VoIPUserAgent::CallStatusCallback - state_InviteRequested");
			break;
			
		case SipCallMember::state_ReInviteRequested:
			qDebug("VoIPUserAgent::CallStatusCallback - state_ReInviteRequested");
			break;
			
		case SipCallMember::state_RequestingInvite:
			qDebug("VoIPUserAgent::CallStatusCallback -state_RequestingInvite ");
			//if (fwCurrentCallStatus != VOIP_CALL_DIALING) {				// no need to pas this state up
			//	fwNewCallStatus = VOIP_CALL_DIALING;//dialing / ringing
			//	flagSendStatus = true;
			//}
			break;
			
		case SipCallMember::state_RequestingReInvite:
			qDebug("VoIPUserAgent::CallStatusCallback - state_RequestingReInvite");
			break;
			
		case SipCallMember::state_Redirected:
			qDebug("VoIPUserAgent::CallStatusCallback - state_Redirected");
			//fwNewCallStatus = SIP_CALL_TRANSFERRED;//transferred		
			//flagSendStatus = true;
			break;
			
		case SipCallMember::state_Disconnecting:
			qDebug("VoIPUserAgent::CallStatusCallback - state_Disconnecting");
			break;
			
		case SipCallMember::state_CancelPending:
			qDebug("VoIPUserAgent::CallStatusCallback - state_CancelPending");
			//fwNewCallStatus = SIP_CALL_PENDING_CANCELLED;//cancelled the pending ??
			//flagSendStatus = true;
			break;
			
		case SipCallMember::state_Refer:
			qDebug("VoIPUserAgent::CallStatusCallback - state_Refer");
			//fwNewCallStatus = SIP_CALL_TRANSFERRING;//transferring ??
			//lagSendStatus = true;
			break;
			
		case SipCallMember::state_Refer_handling:
			qDebug("VoIPUserAgent::CallStatusCallback - state_Refer_handling");
			break;
			
		default: 
			break;			
			
	}
   
	//send the message up to the framework
	if (flagSendStatus == true)
	{
		setCallStatus(member,fwNewCallStatus);
		QString fwCallIdentifier =  getCallidentifier(member);
		qDebug("!!!!!!!!!!!!!!!!!!! VoIPUserAgent::CallStatusCallback - fwNewCallStatus =  "+ (int)fwNewCallStatus );
		sendStatusChange( fwCallIdentifier, (int)fwNewCallStatus, QString::null );
	 }

}
   
// void VoIPUserAgent::incomingCallindicationCallback(SipCallMember *member)
// Description: This Callback indicates the arrival of the MEDIA CALL
// Parameters:
//
// KCallWidget *incomingCallWidget
// We need to extract caller ID from this and send it up.
// Qtopia framework shall be required to do the contact matching and present it to user.
// If user accepts it, it shall result into VoIPCallAccept
// If user rejects it, it shall result into VoIPCallHangup

void VoIPUserAgent::incomingCallindicationCallback(KCallWidget *incomingCallWidget)
{

	// Do send callback up
	QString callIdentifier = "";
	callIdentifier = Global::generateUuid().toString();
	AdaptationLayerCallInfo *callinfo = new AdaptationLayerCallInfo(incomingCallWidget, callIdentifier,VOIP_CALL_INCOMING);
	adaptationLayerCallList.append(callinfo);
	SipUri incomingUri= incomingCallWidget->getCallMember()->getUri();
	QString incomingUriString = "sip:" + incomingUri.getUsername() + "@" +  incomingUri.getHostname();	
	qWarning("VoIPUserAgent::incomingCallindicationCallback -incomingUriString -  "+ incomingUriString);
	
	sendStatusChange( callIdentifier, (int)VOIP_CALL_INCOMING, incomingUriString );
	QString fullName = incomingUri.getFullname();
	if ( !fullName.isEmpty() )
	    sendCallingName( callIdentifier, fullName );
}

// void incomingContactSubscribeindicationCallback(SipCallMember *member)
// Description: This Callback indicates the arrival of the CONTACT SUBSCRIPTION CALL
// Parameters:
//
// KCallWidget *incomingCallWidget
// We need to extract caller ID from this and send it up.
// Qtopia framework shall be required to do the contact matching and present it to user.
// If user accepts it, it shall result into VoIPCallAccept
// If user rejects it, it shall result into VoIPCallHangup
void VoIPUserAgent::incomingContactSubscribeCallback(SipUri sipUri)
{

// Do send callback up
   qDebug(" Adaption Layer::incomingContactSubscribeindicationCall  back  incoming Contact Subscription Request");
	const QByteArray data;
    	QDataStream stream( data, IO_WriteOnly );
	stream << sipUri.reqUri()
		<< sipUri.getFullname();
//	QCopChannel::send ( VOIP_AGENT_CHANNEL, "incomingSubscribe(QString,QString)",data);
	QCopEnvelope e( VOIP_AGENT_CHANNEL, "incomingSubscribe(QString,QString)");
	e << sipUri.reqUri()
		<< sipUri.getFullname();    
}



