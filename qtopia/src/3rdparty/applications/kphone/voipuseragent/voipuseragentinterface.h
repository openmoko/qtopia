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
#ifndef VOIPUSERAGENTINTERFACE_H
#define VOIPUSERAGENTINTERFACE_H

#include <qobject.h>
#include <qstring.h>
#include <qvaluelist.h>
//#include <voipuseragentstructures.h>




class VoIPUserAgentInterface :public QObject
{

public:

/*	//virtual static VoIPUserAgentInterface* getInstance() = 0;
	virtual ReturnCode_t getVoIPUserAgentInfo(StackInfo_t *StackInfo)= 0;
	virtual ReturnCode_t initializeVoIPUserAgent(Configure_t configurationData)= 0;
	virtual ReturnCode_t registerToSipProxy()= 0;
	virtual ReturnCode_t changeAvailabilityStatus(AvailabilityStatus_t status)= 0;
	virtual ReturnCode_t dial(QString numberToCall, QString callIdentifier)= 0;
	virtual ReturnCode_t accept(QString callIdentifier)= 0;
	virtual ReturnCode_t hangup(QString callIdentifier)= 0;
	virtual ReturnCode_t closeCall(QString callIdentifier)= 0;
	virtual ReturnCode_t feedTone(QString callIdentifier, ToneType_t tone)= 0;
	virtual ReturnCode_t updateIdentity(SipIdentity_t userIdentity)= 0;
	virtual ReturnCode_t updateSipPreferences(SipPreferences_t sipPreferences)= 0;
	virtual ReturnCode_t updateAudioPreferences(AudioPreferences_t audioPreferences)= 0;
	virtual ReturnCode_t addContactToContactsList(SipContact_t contact)= 0;
	virtual ReturnCode_t deleteContactFromContactsList(SipContact_t contact)= 0;
	virtual ReturnCode_t updateContactInContactsList(SipContact_t contact)= 0;
	virtual ReturnCode_t logoutFromSipProxy()= 0;
	virtual ReturnCode_t deInitializeStack() = 0;
  
signals:

	void callStatusUpdate(QString identifier, CallStatus_t status);
	void registrationStatusUpdate(RegistrationStatus_t status);	
	void contactAvailabilityStatusUpdate(QString identifier, AvailabilityStatus_t status);	
	void incomingCall(QString contactIdentifier,QString sipUri);	
	void incomingSubscribeMessage(QString sipUri);	
	*/
	
};

VoIPUserAgentInterface* getVoIPUserAgent();

//VoIPUserAgentInterface* getVoIPUserAgent();

#endif
