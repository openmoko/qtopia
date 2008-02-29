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
#include "voipfw_settings.h"
#include <qfile.h>
#include <qtextstream.h>
#include <qtopia/config.h>

VoIPSettingsStorage::VoIPSettingsStorage(QString Filename)
{
	m_filename = Filename;
}

VoIPReturnCode_t VoIPSettingsStorage::getSipIdentity(SipIdentity_t *Identity)
{
	Config cfg(m_filename);
	cfg.setGroup("System");
	Identity->fullName = cfg.readEntry("fullName", "");
	Identity->userPartOfSipUri = cfg.readEntry("userPartOfSipUri", "");
	Identity->hostPartOfSipUri  = cfg.readEntry("hostPartOfSipUri", "");
	Identity->outboundProxy = cfg.readEntry("outboundProxy", "");
	Identity->authenticationUsername = 
				cfg.readEntry("authenticationUsername", ""); 
	Identity->authenticationPassword  = 
				cfg.readEntry("authenticationPassword", "");
	Identity->qValue = cfg.readEntry("qValue", "");	
	Identity->autoRegister = cfg.readBoolEntry("autoRegister", false);		
	return VOIP_SUCCESS;
}

VoIPReturnCode_t VoIPSettingsStorage::setSipIdentity(SipIdentity_t Identity)
{
	Config cfg(m_filename);
	cfg.setGroup("System");

	cfg.writeEntry("fullName", Identity.fullName) ;
	cfg.writeEntry("userPartOfSipUri", Identity.userPartOfSipUri);
	cfg.writeEntry("hostPartOfSipUri", Identity.hostPartOfSipUri);
	cfg.writeEntry("outboundProxy", Identity.outboundProxy);
	cfg.writeEntry("authenticationUsername", 
				Identity.authenticationUsername); 
	cfg.writeEntry("authenticationPassword", 
				Identity.authenticationPassword); 
	cfg.writeEntry("qValue", Identity.qValue);
	cfg.writeEntry("autoRegister", Identity.autoRegister);		
	
	return VOIP_SUCCESS;
}

VoIPReturnCode_t VoIPSettingsStorage::getSipPreferences(SipPreferences_t *SipPreferences)
{
	Config cfg(m_filename);
	cfg.setGroup("System");

	SipPreferences->hideVia = cfg.readNumEntry("hideVia", 1); // Dont HIde
	SipPreferences->maxForwards = cfg.readNumEntry("maxForwards",0); 
	SipPreferences->expTimeForRegistration = 
			cfg.readNumEntry("expTimeForRegistration",900);
	SipPreferences->expTimeForPresenceSubscription = 
			cfg.readNumEntry("expTimeForPresenceSubscription", 900);
	SipPreferences->socketProtocol = cfg.readNumEntry("socketProtocol", 2); //UDP
	SipPreferences->useStunServer = cfg.readBoolEntry("useStunServer"); 	
	SipPreferences->useSymmetricSignaling = 
			cfg.readBoolEntry("useSymmetricSignaling", true); 	
	SipPreferences->useMediaSignaling = 
			cfg.readBoolEntry("useMediaSignaling", true); 	
	SipPreferences->STUNServerAddress = cfg.readEntry("STUNServerAddress");
	SipPreferences->requestPeriodForSTUNServer = 
			cfg.readNumEntry("requestPeriodForSTUNServer", 60);
	SipPreferences->mediaMinPort = cfg.readNumEntry("mediaMinPort", 0);
	SipPreferences->mediaMaxPort  = cfg.readNumEntry("mediaMaxPort", 0);
	SipPreferences->busyMessage = cfg.readEntry("busyMessage", "");	

	return VOIP_SUCCESS;
}

VoIPReturnCode_t VoIPSettingsStorage::setSipPreferences(SipPreferences_t SipPreferences)
{
	Config cfg(m_filename);
	cfg.setGroup("System");

	cfg.writeEntry("hideVia", SipPreferences.hideVia);
	cfg.writeEntry("maxForwards", SipPreferences.maxForwards); 
	cfg.writeEntry("expTimeForRegistration", 
			SipPreferences.expTimeForRegistration);
	cfg.writeEntry("expTimeForPresenceSubscription", 
			SipPreferences.expTimeForPresenceSubscription);
	cfg.writeEntry("socketProtocol", SipPreferences.socketProtocol);
	cfg.writeEntry("useStunServer", SipPreferences.useStunServer); 	
	cfg.writeEntry("useSymmetricSignaling", 
			SipPreferences.useSymmetricSignaling); 	
	cfg.writeEntry("useMediaSignaling", SipPreferences.useMediaSignaling); 	
	cfg.writeEntry("STUNServerAddress", SipPreferences.STUNServerAddress);
	cfg.writeEntry("requestPeriodForSTUNServer", 
			SipPreferences.requestPeriodForSTUNServer);
	cfg.writeEntry("mediaMinPort", SipPreferences.mediaMinPort);
	cfg.writeEntry("mediaMaxPort", SipPreferences.mediaMaxPort);
	cfg.writeEntry("busyMessage", SipPreferences.busyMessage);	
	
	return VOIP_SUCCESS;
}

VoIPReturnCode_t VoIPSettingsStorage::getAudioPreferences(AudioPreferences_t *AudioPreferences)
{
	Config cfg(m_filename);
	cfg.setGroup("System");

	AudioPreferences->OSSDeviceMode = cfg.readNumEntry("OSSDeviceMode", 1); // Read Write
	AudioPreferences->deviceForReadOnlyWriteOnly = 
			cfg.readEntry("deviceForReadOnlyWriteOnly", "/dev/dsp");
	AudioPreferences->deviceForReadOnly = 
			cfg.readEntry("deviceForReadOnly", "");
	AudioPreferences->preferredCodec = cfg.readEntry("preferredCodec", "");
	AudioPreferences->preferredPayloadSize = 
			cfg.readNumEntry("preferredPayloadSize", 2); // 2 stands for size 160 

	return VOIP_SUCCESS;
}

VoIPReturnCode_t VoIPSettingsStorage::setAudioPreferences(AudioPreferences_t AudioPreferences)
{
	Config cfg(m_filename);
	cfg.setGroup("System");

	cfg.writeEntry("OSSDeviceMode", AudioPreferences.OSSDeviceMode);
	cfg.writeEntry("deviceForReadOnlyWriteOnly", 
			AudioPreferences.deviceForReadOnlyWriteOnly);
	cfg.writeEntry("deviceForReadOnly", AudioPreferences.deviceForReadOnly);
	cfg.writeEntry("preferredCodec", AudioPreferences.preferredCodec);
	cfg.writeEntry("preferredPayloadSize", 
			AudioPreferences.preferredPayloadSize);

	return VOIP_SUCCESS;
}

VoIPReturnCode_t VoIPSettingsStorage::setListOfCodecs(QValueList <CodecInformation_t> codecs)
{
	m_codecs = codecs;
	return VOIP_SUCCESS;
}

VoIPReturnCode_t VoIPSettingsStorage::getListOfCodecs(QValueList <CodecInformation_t> *codecs)
{
	*codecs = m_codecs;
	return VOIP_SUCCESS;
}
