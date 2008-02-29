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

#include "qtopia/global.h"
#include <qtopia/qprocess.h>
#include "voipconfig.h"
#include "voipfw_settings.h"

#define VOIP_STACK_CHANNEL "QPE/Application/voipuseragent"
#define VOIP_AGENT_CHANNEL "QPE/VoIPAgent"
#define VOIP_SETTINGS_STORAGE_FILE "VoIPSettingsFile"

VoIPConfig :: VoIPConfig()
{
    VoIPSettings = new VoIPSettingsStorage(VOIP_SETTINGS_STORAGE_FILE);
    channel = new QCopChannel( VOIP_AGENT_CHANNEL, this );
    QObject::connect(channel, SIGNAL(received(const QCString&,const QByteArray&)),
        this, SLOT(voipChannelListener(const QCString&,const QByteArray&)) );
}

VoIPConfig :: ~VoIPConfig()
{
    delete VoIPSettings;
}

VoIPConfig *VoIPConfig :: getInstance()
{
    static VoIPConfig *cfg = 0;
    if ( !cfg )
	cfg = new VoIPConfig();
    return cfg;
}

void VoIPConfig :: getSipIdentity()
{
    SipIdentity_t sipIdentity;
    VoIPSettings->getSipIdentity(&sipIdentity);
    emit updateSipIdentity(sipIdentity);
}

void VoIPConfig :: setSipIdentity(SipIdentity_t& identity)
{
    // Save the values to the configuration file.
    VoIPSettings->setSipIdentity(identity);

    // Notify voipuseragent of the change.
    QCopEnvelope e( VOIP_STACK_CHANNEL, "setSipIdentity(QString,QString,QString,QString,QString,QString,QString,bool)");
	e << identity.fullName
       << identity.userPartOfSipUri
       << identity.hostPartOfSipUri
       << identity.outboundProxy
       << identity.authenticationUsername
       << identity.authenticationPassword
       << identity.qValue
       << (int)identity.autoRegister;
}

void VoIPConfig :: getSipPreferences()
{
    SipPreferences_t sipPreferences;
    VoIPSettings->getSipPreferences(&sipPreferences);
    emit updateSipPreferences(sipPreferences);
}

void VoIPConfig :: setSipPreferences(SipPreferences_t& sipPreferences)
{
    // Save the values to the configuration file.
    VoIPSettings->setSipPreferences(sipPreferences);

    // Notify voipuseragent of the change.
    QCopEnvelope e( VOIP_STACK_CHANNEL, "setSipPreferences(int,int,int,int,int,bool,bool,bool,QString,int,int,int,QString)");
	e << sipPreferences.hideVia
       << sipPreferences.maxForwards
       << sipPreferences.expTimeForRegistration
       << sipPreferences.expTimeForPresenceSubscription
       << sipPreferences.socketProtocol
       << (int)sipPreferences.useStunServer
       << (int)sipPreferences.useSymmetricSignaling
       << (int)sipPreferences.useMediaSignaling
       << sipPreferences.STUNServerAddress
	<< sipPreferences.requestPeriodForSTUNServer
       << sipPreferences.mediaMinPort
       << sipPreferences.mediaMaxPort
       << sipPreferences.busyMessage;
}

void VoIPConfig :: getAudioPreferences()
{
    AudioPreferences_t audioPreferences;
    VoIPSettings->getAudioPreferences(&audioPreferences);
    emit updateAudioPreferences(audioPreferences);
}

void VoIPConfig :: setAudioPreferences(AudioPreferences_t& audioPreferences)
{
    // Save the values to the configuration file.
    VoIPSettings->setAudioPreferences(audioPreferences);

    // Notify voipuseragent of the change.
    QCopEnvelope e( VOIP_STACK_CHANNEL, "setAudioPreferences(int,QString,QString,int,QString,int)");
	e << audioPreferences.OSSDeviceMode
	<< audioPreferences.deviceForReadOnlyWriteOnly
       << audioPreferences.deviceForReadOnly
       << audioPreferences.preferredCodec
       << audioPreferences.preferredPayloadSize;
}

void VoIPConfig :: getListOfCodecs()
{
    QValueList <CodecInformation_t> codecs;
    VoIPSettings->getListOfCodecs(&codecs);
    // TODO: convert into the desired information.
}


void VoIPConfig :: getCurrentRegistrationInfo()
{
    QCopEnvelope e( VOIP_STACK_CHANNEL, "getCurrentRegistrationStatus()" );
}

void VoIPConfig :: getCurrentAvailabilityStatus()
{
    QCopEnvelope e( VOIP_STACK_CHANNEL, "getCurrentAvailabilityStatus()" );  
}  
 
void VoIPConfig :: loginAndRegister()
{
    QCopEnvelope e( VOIP_STACK_CHANNEL, "register()" );
}

void VoIPConfig :: logout()
{
    QCopEnvelope e( VOIP_STACK_CHANNEL, "logout()" );
}

void VoIPConfig :: setStatus(AvailabilityStatus_t status)
{
    QCopEnvelope e( VOIP_STACK_CHANNEL, "setStatus(int)" );
    e<<(int)status;
}

void VoIPConfig::voipChannelListener( const QCString& msg, const QByteArray& data )
{
    QDataStream stream( data, IO_ReadOnly );

    if ( msg == "registrationStatusUpdate(int)" ) {
      int iRegStatus;
      stream>> iRegStatus;
      emit updateRegistrationMessage(iRegStatus);
    }
    else if ( msg == "currentRegistrationStatus(int)" ) {
      int curRegStatus;      
      stream>> curRegStatus;
      emit setCurrentRegistration(curRegStatus);
    }
    else if ( msg == "currentAvailabilityStatus(int)" ) {
      int curAvailStatus;
      stream>> curAvailStatus;
      emit setCurrentAvailabilityStatus(curAvailStatus);
    }
    
}

