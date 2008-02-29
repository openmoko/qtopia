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
#include <voipuseragent.h>
#include <qsettings.h>
#include <qtopia/qpeapplication.h>
#include "phonesocket.h"
#include "voipfw_settings.h"


#define VOIP_SETTINGS_STORAGE_FILE "VoIPSettingsFile"


VoIPUserAgent * VoIPUserAgent::m_VoIPUserAgent = NULL;

VoIPUserAgentInterface* getVoIPUserAgent()
{
	VoIPUserAgentInterface* agent = new VoIPUserAgent ();
	return agent;
}


VoIPUserAgent::VoIPUserAgent() 
    : hphone( 0 ), sipRegistrationObject( 0 ), VoIPSocket( 0 ),
        currentRegStatus( VOIP_PROXY_UNREGISTERED ), delayedRegTimer( 0 )
{
	qDebug("Start of VoIPUserAgent()");

	VoIPAgentChannel= new QCopChannel( VOIP_AGENT_CHANNEL, this );
	
	if (QCopChannel::isRegistered(VOIP_AGENT_CHANNEL) == true)
		qDebug("VOIP_AGENT_CHANNEL is registered. Name is "+VoIPAgentChannel->channel());			
	else 
		qDebug("VOIP_AGENT_CHANNEL is NOT registered. Name is "+VoIPAgentChannel->channel());			
	
	//initialize the stack information parameters
	stackInfo.StackProvidersName = "KPhone Voip User Agent";
	QValueList <CodecInformation_t>codecList;
	codecList.clear();
	CodecInformation_t codec;
	codec.nameOfCodec = "PCMU";
	codec.payloadSizesSupported = 7; // To inidcate that it supports all three sizes(80, 160 and 240)
	codecList.append(codec);
	
	userPrefix = ""; // used for QSettings.  	
	
	//Initialization complete. Now connect the listener slot to the channel
//        QObject::connect
//	    (VoIPAgentChannel, SIGNAL(received(const QCString&,const QByteArray&)),
//	      this, SLOT(VoIPAgentMessage(const QCString&,const QByteArray&)) );

	//Initialization complete. Now connect the listener slot to the channel
        QObject::connect
	    (qApp, SIGNAL(appMessage(const QCString&,const QByteArray&)),
	      this, SLOT(VoIPAgentMessage(const QCString&,const QByteArray&)) );

	//initialize the rgistration status 
	registrationStatus = SipRegister::NotConnected;
	//initialize adaptation call state
	m_CallStatus = VOIP_CALL_IDLE; 	
	m_callIdentifier = "";
	// the adaptation layer call list
	adaptationLayerCallList.clear();

	//QString list of contacts uri ... to be passed down to the Sip stack for keeping track of their presence.
	//contactsUriStringList.clear();
	currentSipServerName = ""; // would be initialized in updateIdentity	
	QCopEnvelope e( VOIP_AGENT_CHANNEL, "bootingUp()");		
	qDebug("End of VoIPUserAgent()");
}

void VoIPUserAgent::VoIPAgentMessage( const QCString& msg, const QByteArray& data )
{
	qDebug("***************VoIPUserAgent::VoIPAgentMessage -  message received - " + msg);

       QDataStream stream( data, IO_ReadOnly );
	
	if (msg ==  "startPhoneHandler()" ) {

	    qDebug("VoIPUserAgent::VoIPAgentMessage - starting phone handler");
	    // Qtopia asked us to start the phone handler that connects
	    // back to the phone server to provide VoIP services.
	    startPhoneHandler();

	    // Make sure that this program keeps running from this point on.
	    QPEApplication::setKeepRunning();

	} else if (msg == "register()")
	{
		registerToSipProxy();
	
	}else if (msg == "logout()")
	{
		logoutFromSipProxy();
	
	}else if (msg == "setSipIdentity(QString,QString,QString,QString,QString,QString,QString,bool)")
	{
		SipIdentity_t sipIdentity;
		int tempAutoRegister;
		stream >> sipIdentity.fullName
	       >> sipIdentity.userPartOfSipUri
	       >> sipIdentity.hostPartOfSipUri
	       >> sipIdentity.outboundProxy
	       >> sipIdentity.authenticationUsername
	       >> sipIdentity.authenticationPassword
	       >> sipIdentity.qValue
	       >> tempAutoRegister;
		
		sipIdentity.autoRegister = (bool)tempAutoRegister;
		updateIdentity(sipIdentity);
	
	}else if (msg == "setSipPreferences(int,int,int,int,int,bool,bool,bool,QString,int,int,int,QString)")
	{
		SipPreferences_t sipPreferences;
	       int tempUseStunServer, tempUseSymmetricSignaling, tempUseMediaSignaling;		
		stream >> sipPreferences.hideVia
	       >> sipPreferences.maxForwards
	       >> sipPreferences.expTimeForRegistration
	       >> sipPreferences.expTimeForPresenceSubscription
	       >> sipPreferences.socketProtocol
	       >> tempUseStunServer
	       >> tempUseSymmetricSignaling
	       >> tempUseMediaSignaling	       
	       >> sipPreferences.STUNServerAddress
		>> sipPreferences.requestPeriodForSTUNServer
	       >> sipPreferences.mediaMinPort
	       >> sipPreferences.mediaMaxPort
	       >> sipPreferences.busyMessage;	

	       sipPreferences.useStunServer = (bool)tempUseStunServer;
	       sipPreferences.useSymmetricSignaling = (bool)tempUseSymmetricSignaling;
	       sipPreferences.useMediaSignaling = (bool)tempUseMediaSignaling;

		updateSipPreferences(sipPreferences);
	
	}else if (msg == "setAudioPreferences(int,QString,QString,int,QString,int)")
	{	
		AudioPreferences_t audioPreferences;
	       stream>> audioPreferences.OSSDeviceMode
	       >> audioPreferences.deviceForReadOnlyWriteOnly
	       >> audioPreferences.deviceForReadOnly
	       >> audioPreferences.preferredCodec
	       >> audioPreferences.preferredPayloadSize;
		updateAudioPreferences(audioPreferences);
		
	}else if (msg == "setStatus(int)")
	{
		AvailabilityStatus_t status;
		int tempStatus;
		stream >> tempStatus;
		status = (AvailabilityStatus_t)tempStatus;
		
		changeAvailabilityStatus(status);		
	
	}else if (msg == "dial(QString,QString)")
	{
		
		qDebug("extracting dial parameters") ;
		QString dialString, callIdentifier;
		stream >> dialString
		>> callIdentifier;
		printf(tr("calling VoIPUserAgent::dial"));
		dial(dialString, callIdentifier);

	}else if (msg == "hangup(QString)")
	{
		QString callIdentifier;
		stream >> callIdentifier;
		hangup(callIdentifier);

	}else if (msg == "acceptCall(QString)")
	{

		QString callIdentifier;
		stream >> callIdentifier;
		accept(callIdentifier);
	}else if (msg == "closeCall(QString)")
	{

		QString callIdentifier;
		stream >> callIdentifier;
		closeCall(callIdentifier);
	
	}else if (msg == "getVoIPAgentInfo()")
	{
		getVoIPUserAgentInfo();
	}else if (msg == "feedTone(QString,QString)")
	{
		QString callIdentifier, toneString;
		stream >> callIdentifier
			>> toneString;
		feedTone(callIdentifier,toneString);
		
	}else if (msg == "deinitializeVoIPAgent()")
	{
        qDebug("QCopEnvelope e(VOIP_AGENT_CHANNEL, voIPAgentDeinitialized);");
		QCopEnvelope e(VOIP_AGENT_CHANNEL, "voIPAgentDeinitialized");
        QCopEnvelope e1("QPE/Application/voipuseragent", "kill");
		
	} else if (msg == "getCurrentRegistrationStatus()") {
	    QCopEnvelope e(VOIP_AGENT_CHANNEL, "currentRegistrationStatus(int)");
	    e << currentRegStatus;
	} else if (msg == "getCurrentAvailabilityStatus()") {
	    QCopEnvelope e(VOIP_AGENT_CHANNEL, "currentAvailabilityStatus(int)");
	    e << VoIPGetAvailabilityStatus();
	}
}

void VoIPUserAgent::startPhoneHandler()
{
    // Bail out if the handler is already running.
    if ( VoIPSocket )
	return;

    // Create the handler socket.
    VoIPSocket = new PhoneSocket( this );
    connect( VoIPSocket, SIGNAL(received(const QStringList&)),
	     this, SLOT(socketReceived(const QStringList&)) );
    connect( VoIPSocket, SIGNAL(closed()), this, SLOT(socketClosed()) );

    // Send the initialisation message.
    VoIPSocket->send( "INIT", "voip" );

    // Load the configuration values and initialize the VoIP stack.
    VoIPSettingsStorage settings( VOIP_SETTINGS_STORAGE_FILE );
    Configure_t configData;
    settings.getSipIdentity( &configData.sipIdentity );
    settings.getSipPreferences( &configData.sipPreferences );
    settings.getAudioPreferences( &configData.audioPreferences );
    initializeVoIPUserAgent( configData );

    // TODO: send the initial call state for the currently active calls.
}

void VoIPUserAgent::socketReceived( const QStringList& cmd )
{
    QString c = cmd[0];
    if ( c == "DIAL" ) {

	// Dial a new call.  The type and "restrict" flag are currently ignored.
	if ( cmd.count() >= 4 )
	    dial(cmd[2], cmd[1]);

    } else if ( c == "HANGUP" ) {

	// Hang up a call.  The scope is currently ignored.
	if ( cmd.count() >= 2 )
	    hangup(cmd[1]);

    } else if ( c == "ACCEPT" ) {

	// Accept an incoming call.
	if ( cmd.count() >= 2 )
	    accept(cmd[1]);

    } else if ( c == "HOLD" ) {

	// Put a call on hold.  Not implemented yet.
	// TODO

        // for now hangup
        if ( cmd.count() >= 2 )
            hangup(cmd[1]);

    } else if ( c == "ACTIVATE" ) {

	// Activate a held call.  Not implemented yet.
	// TODO

    } else if ( c == "JOIN" ) {

	// Join held and active calls together.  Not implemented yet.
	// TODO

    } else if ( c == "DTMF" ) {

	// Send DTMF tones on a call.
	if ( cmd.count() >= 3 )
	    feedTone(cmd[1], cmd[2]);

    } else if ( c == "TRANSFER" ) {

	// Transfer a call to another number.  Not implemented yet.
	// TODO

    } else if ( c == "REGISTER" ) {

	// Register to the network.
	registerToSipProxy();

    } else if ( c == "DEREGISTER" ) {

	// De-register from the network.
	logoutFromSipProxy();

    } else if ( c == "STARTMONITOR" ) {

	// Start monitoring the presence of a user.
	if ( cmd.count() >= 2 ) {
	    addContactToContactsList(cmd[1]);
	}

    } else if ( c == "STOPMONITOR" ) {

	// Stop monitoring the presence of a user.
	if ( cmd.count() >= 2 ) {
	    deleteContactFromContactsList(cmd[1]);
	}

    } else if ( c == "GETPRESENCE" ) {

	// Get the current presence status of the local user.
	if ( VoIPGetAvailabilityStatus() )
	    VoIPSocket->send( "PRESENCE", "available" );
	else
	    VoIPSocket->send( "PRESENCE", "unavailable" );

    } else if ( c == "SETPRESENCE" ) {

	// Set the current presence status of the local user.
	if ( cmd.count() > 1 && cmd[1] == "available" )
	    changeAvailabilityStatus(AVAILABLE);
	else
	    changeAvailabilityStatus(UNAVAILABLE);

    } else if ( c == "GET" || c == "SET" ) {

	// Not used at present - ignore.

    } else if ( c == "TEST" ) {

	// The handler has asked us if we support some particular commands.
	QStringList supportedCommands;
	supportedCommands += "INIT";	    // Us to Qtopia.
	supportedCommands += "STATUS";
	supportedCommands += "NAME";
	supportedCommands += "GROUP";
	supportedCommands += "REGISTRATION";
	supportedCommands += "OPERATOR";
	supportedCommands += "PRESENCE";
	supportedCommands += "MONITOR";
	supportedCommands += "VALUE";
	supportedCommands += "NOTIFY";
	supportedCommands += "DIAL";	    // Qtopia to us.
	supportedCommands += "HANGUP";
	supportedCommands += "ACCEPT";
	supportedCommands += "HOLD";
	supportedCommands += "ACTIVATE";
	supportedCommands += "JOIN";
	supportedCommands += "HOLD";
	supportedCommands += "DTMF";
	supportedCommands += "TRANSFER";
	supportedCommands += "REGISTER";
	supportedCommands += "DEREGISTER";
	supportedCommands += "STARTMONITOR";
	supportedCommands += "STOPMONITOR";
	supportedCommands += "GETPRESENCE";
	supportedCommands += "SETPRESENCE";
	supportedCommands += "GET";
	supportedCommands += "SET";
	supportedCommands += "FAIL";	    // Both directions.
	supportedCommands += "TEST";
	supportedCommands += "SUPPORTS";
	QStringList response;
	QStringList::ConstIterator iter;
	response += "SUPPORTS";
	iter = cmd.begin();
	++iter;
	while ( iter != cmd.end() ) {
	    if ( supportedCommands.contains( *iter ) )
		response += *iter;
	    else
		response += "!" + *iter;
	    ++iter;
	}
	VoIPSocket->send( response );

    } else if ( c == "FAIL" ) {

	// A command that we sent to the handler has failed for some reason.
	QString joined = cmd.join( " " );
	qDebug( "VoIPUserAgent::socketReceived: %s", joined.latin1() );

    } else {

	// We don't know what to do with this kind of command.
	QStringList error;
	error += "FAIL";
	error += "unknown-command";
	error += cmd;
	VoIPSocket->send( error );

    }
}

void VoIPUserAgent::socketClosed()
{
    qDebug( "VoIPUserAgent::socketClosed: connection to phone server severed");
    delete VoIPSocket;
    VoIPSocket = 0;
}

// Callback
  
// void changePresenceStatusCallback(QString uristr, int presenceState )
// Description:
// This callback Interface call shall be called to indicate change in the
// the availaiility  status of the uri to Offline/Online
// Parameters:
// IN int Availbility Status

void VoIPUserAgent::changePresenceStatusCallback(QString uristr, int presenceState )
{
    qDebug("Adaption Layer::changePresenceStatusCallback Out" );
    qDebug("sip uri  - " + uristr + "		status - %d", presenceState);

    if ( VoIPSocket ) {
	VoIPSocket->send( "MONITOR", uristr,
			  ( presenceState ? "available" : "unavailable" ) );
    } else {
	qDebug("Adaption Layer::changePresenceStatusCallback: not connected" );
    }
}

// Send status changes to the phone server.
void VoIPUserAgent::sendStatusChange( const QString& id, int status, const QString& callerId )
{
    if ( !VoIPSocket ) {
	qDebug("Adaption Layer::sendStatusChange: not connected" );
	return;
    }

    QString statusString;
    switch ( (CallStatus_t)status ) {
	case VOIP_CALL_INCOMING:	statusString = "incoming"; break;
	case VOIP_CALL_DIALING:		statusString = "dialing"; break;
	case VOIP_CALL_CONNECTED:	statusString = "connected"; break;
	case VOIP_CALL_HOLD:		statusString = "hold"; break;
	case VOIP_CALL_HANGUP_LOCAL:	statusString = "hanguplocal"; break;
	case VOIP_CALL_HANGUP_REMOTE:	statusString = "hangupremote"; break;
	case VOIP_CALL_MISSED:		statusString = "missed"; break;
	case VOIP_CALL_NETWORK_FAILURE:	statusString = "networkfailure"; break;
	default:			statusString = "otherfailure"; break;
    }

    QStringList cmd;
    cmd << "STATUS";
    cmd << id;
    cmd << statusString;
    cmd << "voip";
    if ( callerId.length() > 0 )
	cmd << callerId;
    VoIPSocket->send( cmd );
}

void VoIPUserAgent::sendCallingName( const QString& id, const QString& name )
{
    if ( !VoIPSocket ) {
	qDebug("Adaption Layer::sendCallingName: not connected" );
	return;
    }
    VoIPSocket->send( "NAME", id, name );
}

void VoIPUserAgent::sendRegistrationChangeNow( int status )
{
    currentRegStatus = status;

    // Notify the voip settings program of the change.
    QCopEnvelope e(VOIP_AGENT_CHANNEL, "registrationStatusUpdate(int)");
    e << status;

    if ( !VoIPSocket ) {
	qDebug("Adaption Layer::sendRegistrationChangeNow: not connected" );
	return;
    }

    QString statusString;
    switch ( (RegistrationStatus_t)status ) {
	case VOIP_PROXY_UNREGISTERED:
	case VOIP_PROXY_CONNECTION_FAILED:
	case VOIP_PROXY_REGISTRATION_TIMEDOUT:
	case VOIP_PROXY_CONNECTION_LOST:
	default:
	    statusString = "unregistered";
	    break;

	case VOIP_PROXY_REGISTERED:
	    statusString = "home";
	    break;

	case VOIP_PROXY_CONTACTING_PROXY:
	    statusString = "searching";
	    break;

	case VOIP_PROXY_AUTHENITCATION_FAILED:
	case VOIP_PROXY_AUTHENITCATION_FAILED_NEED_NEW_PASSWORD:
	    statusString = "denied";
	    break;
    }

    VoIPSocket->send( "REGISTRATION", statusString );
}

void VoIPUserAgent::sendRegistrationChange( int status )
{
    if ( status == VOIP_PROXY_CONTACTING_PROXY &&
         currentRegStatus == VOIP_PROXY_REGISTERED ) {

	// We have just transitioned from "home" to "searching".
	// This may have happened because the SIP stack is doing a
	// "keep alive" to the server.  We delay telling the rest
	// of the system about "searching" until we are reasonably
	// sure that it is actually searching and not just polling.
	// If it goes back to "home" quickly, there is no need
	// for the rest of the system to be informed.
	if ( !delayedRegTimer ) {
	    delayedRegTimer = new QTimer( this );
	    connect( delayedRegTimer, SIGNAL(timeout()),
		     this, SLOT(delayedRegTimerTimeout()) );
	}
	delayedRegTimer->start( 500, TRUE );

    } else {

	// Send the registration status change immediately.
	if ( delayedRegTimer ) {
	    delete delayedRegTimer;
	    delayedRegTimer = 0;
	}
	sendRegistrationChangeNow( status );
    }
}

void VoIPUserAgent::delayedRegTimerTimeout()
{
    // We received a timeout while the state was "searching".
    // So we really do seem to be searching, and not just polling.
    sendRegistrationChangeNow( VOIP_PROXY_CONTACTING_PROXY );
}

void VoIPUserAgent::getVoIPUserAgentInfo()
{
	//StackInfo_t *StackInfo
	//ToDo - Prepare and send a qCop message for VoIPUserAgentInfo
	const QByteArray data;
	QDataStream stream( data, IO_WriteOnly );
	stream << stackInfo.StackProvidersName
		<< stackInfo.codecs;
	
//	QCopChannel::send(VOIP_AGENT_CHANNEL, "voipUserAgentInfo(QString,QValueList<CodecInformation_t>", data);
	QCopEnvelope e(VOIP_AGENT_CHANNEL, "voipUserAgentInfo(QString,QValueList<CodecInformation_t>");
	e<< stackInfo.StackProvidersName
	 << stackInfo.codecs;    
	
	

}

void VoIPUserAgent::initializeVoIPUserAgent(Configure_t configurationData)
{


	qDebug(" VoIPUserAgent::initializeVoIPUserAgent - calling update identity");
	updateIdentity(configurationData.sipIdentity);
	qDebug(" VoIPUserAgent::initializeVoIPUserAgent - calling update sip preferences");
	updateSipPreferences(configurationData.sipPreferences);
	qDebug(" VoIPUserAgent::initializeVoIPUserAgent - calling update audiopreferences");
	updateAudioPreferences(configurationData.audioPreferences);

	// intialize the kphoen stack
	qDebug(" VoIPUserAgent::initializeVoIPUserAgent - calling voipInit");	
	VoIPInit();

	//extract the QStringList from contacts list sent by the framework
	SipContactList::Iterator it;
	monitoredUris.clear();
	for(it =configurationData.sipContactList.begin(); it!=configurationData.sipContactList.end(); ++it )
	{	
	    if ( !monitoredUris.contains( (*it).sipUri ) )
		monitoredUris.append( (*it).sipUri );
	#if REMOVE_ME
		//add to the string list
		//QString sipUriStr = (*it).sipUri;
		//contactsUriStringList.append(sipUriStr);

		// add to the adaptation layer object
		AdaptationLayerContact *adaptationLayerContact =  new AdaptationLayerContact (
		(*it).sipUri, CONTACT_OFFLINE, (*it).sipContactId);

		qDebug("contact id - "+ (*it).sipContactId +"sip uri"+ (*it).sipUri);
		
		adaptationLayerContactList.append(adaptationLayerContact);
	#endif
	
	}
	UpdateContactList( monitoredUris );
	qDebug(" VoIPUserAgent::initializeVoIPUserAgent - voipInit returned");	
	//Now send a message saying init successful on VOIP_AGENT_CHANNEL

	QCopEnvelope e(VOIP_AGENT_CHANNEL, "voipUserAgentInitialized()");    

}


void VoIPUserAgent::registerToSipProxy()
{
	QString username, password;
	//ToDo - read username and password from settigns
	QString s= "/kphone/" + userPrefix + "Registration/";
	username = QSettings().readEntry(s + "/UserName", "");
	password = QSettings().readEntry(s + "/Password", "");
	qDebug("VoIPUserAgent::registerToSipProxy - Registering with username -  "+ username 	+ " &  password -" + password );
	qWarning("VoIPUserAgent::registerToSipProxy - Registering with username -  "+ username 	+ " &  password -" + password );    

	VoIPRegister(username,password);
}

void VoIPUserAgent::changeAvailabilityStatus(AvailabilityStatus_t status)
{
	VoIPChangeAvailabilityStatus((int)status);
}

QString VoIPUserAgent::adaptationParseUri(QString numberToCall)
{
	if(numberToCall.contains("@"))
	{ 	//this is a sip id
		if(numberToCall.contains("sip:" == false))
			return "sip:" + numberToCall;
		else
			return numberToCall;
	}
	else
	{ 	//this is a standard telephony number
		return ("sip:" + numberToCall + "@" + currentSipServerName);
	}  
  
}


void VoIPUserAgent::dial(QString numberToCall, QString callIdentifier)
{
	// Make sure that the uri is properly prefixed with "sip:".
	if ( !numberToCall.startsWith( "sip:" ) &&
	     numberToCall.find( QChar('@') ) != -1 &&
	     numberToCall.find( QChar(':') ) == -1 ) {
	    numberToCall = "sip:" + numberToCall;
	}

	m_callIdentifier = callIdentifier;
	// format the number in he format required.
	//ToDo - a parser to replace this part	
	//QString tempRemoteUri = "sip:" + numberToCall +"@fwd.pulver.com";
	QString tempRemoteUri = adaptationParseUri(numberToCall);
	qDebug("#####################Remote usri being called - "+ tempRemoteUri);
	printf(tr("calling VoIPUserAgent::VoIPDial"));


	KCallWidget *callW;// = NULL;//= new KCallWidget();
	VoIPDial(callW,tempRemoteUri );

	m_CallStatus = VOIP_CALL_DIALING;
	AdaptationLayerCallInfo *callinfo = new AdaptationLayerCallInfo(callW, callIdentifier,VOIP_CALL_DIALING);
	adaptationLayerCallList.append(callinfo);
	
}

void VoIPUserAgent::accept(QString callIdentifier)
{
	qDebug("VoIPUserAgent::accept - callIdentifier - " + callIdentifier);
	KCallWidget *hC = getCallWidget(callIdentifier);
	qDebug("#########VoIPUserAgent::accept -CallWidget - %ld ", (long)hC );
	VoIPCallAccept(hC);
	// ?? do we need to set the call status here or would it be handled in callStatusUpdate callback?
}

void VoIPUserAgent::hangup(QString callIdentifier)
{
	KCallWidget *hC = getCallWidget(callIdentifier);
	SipCallMember *member = hC ->getCallMember();
	setCallStatus(member,VOIP_CALL_HANGUP_LOCAL); 
	// This will be informed as a call status update over the VOIP_AGENT_CHANNEL only after the 'call disconnect' happens
	VoIPCallHangup(hC);

}
void VoIPUserAgent::closeCall(QString callIdentifier)
{
	KCallWidget *hC = getCallWidget(callIdentifier);
  if(hC)
    return;

}

void VoIPUserAgent::feedTone(QString callIdentifier, QString toneString)
{
    KCallWidget *hC = getCallWidget(callIdentifier);
    if (hC) {
        qDebug("VoIPUserAgent::feedTone - for the digit/character " + toneString);
        hC->feedTone(toneString);
    }
}

void VoIPUserAgent::updateIdentity(SipIdentity_t sipIdentity)
{
    {
	QSettings settings;
	QString s= "/kphone/" + userPrefix + "Registration/";

	QString uriStr = "\"" + sipIdentity.fullName + "\" " +
		"<sip:" + sipIdentity.userPartOfSipUri + "@" + sipIdentity.hostPartOfSipUri + ">" ;

	settings.writeEntry(s + "/SipUri", uriStr);
	settings.writeEntry(s + "/UserName", sipIdentity.authenticationUsername);
	settings.writeEntry(s + "/Password", sipIdentity.authenticationPassword);
	if (sipIdentity.autoRegister)
		settings.writeEntry(s + "/AutoRegister", "Yes");
	else 
		settings.writeEntry(s + "/AutoRegister", "No");   
	settings.writeEntry(s + "/SipServer" , sipIdentity.outboundProxy);
	settings.writeEntry(s + "/qValue", sipIdentity.qValue);

	currentSipServerName = sipIdentity.hostPartOfSipUri ; 	// would be used to append to the standard telephony number.
    }

    // QSettings is placed within it's own scope so as it is destructed by this point. Otherwise
    // settings are not correctly read in update() below.

    if (hphone)
        sipRegistrationObject = hphone->getSipRegistrations();
    if (sipRegistrationObject)
        sipRegistrationObject->update();
}


void VoIPUserAgent::updateSipPreferences(SipPreferences_t sipPreferences)
{
  	QSettings settings;
	
	settings.writeEntry( "/kphone/sip/maxforwards", sipPreferences.maxForwards);
	settings.writeEntry( "/kphone/sip/busymsg", sipPreferences.busyMessage);
	
	QString hvstr;
	switch( sipPreferences.hideVia ) {
	default:
	case NoHide: hvstr = "NoHide"; break;
	case HideHop: hvstr = "HideHop"; break;
	case HideRoute: hvstr = "HideRoute"; break;
	}
	settings.writeEntry( "/kphone/sip/hideviamode", hvstr );
	
	if( sipPreferences.expTimeForRegistration) {
		settings.writeEntry( "/kphone/local/RegistrationExpiresTime", sipPreferences.expTimeForRegistration );
	}
	
	QString str;
	
	int expTime = sipPreferences.expTimeForPresenceSubscription;
	if( expTime ) {
		if( expTime < 60 ) {
			expTime = 60;
		}
		str.setNum(expTime);
		settings.writeEntry( "/kphone/local/SubscribeExpiresTime", str );
	}
	
	switch( sipPreferences.socketProtocol ) {
	case TCP:
		settings.writeEntry( "/kphone/General/SocketMode", "TCP" );
		break;
	default:
		settings.writeEntry( "/kphone/General/SocketMode", "UDP" );
		break;
	}
	
	settings.writeEntry( "/kphone/STUN/StunServer", sipPreferences.STUNServerAddress);
	
	str.setNum(sipPreferences.requestPeriodForSTUNServer);
	settings.writeEntry( "/kphone/STUN/RequestPeriod", str );
	
	if (sipPreferences.useStunServer ) {
		settings.writeEntry( "/kphone/STUN/UseStun", "Yes" );
	}
	else {
		settings.writeEntry( "/kphone/STUN/UseStun", "No" );
	}
	
	if (sipPreferences.useSymmetricSignaling) {
    		settings.writeEntry("/kphone/Symmetric/Signalling", "Yes");
	}
	else {
		settings.writeEntry("/kphone/Symmetric/Signalling", "No");
	}
	
	if (sipPreferences.useMediaSignaling) {
		settings.writeEntry("/kphone/Symmetric/Media", "Yes");
	}
	else {
		settings.writeEntry("/kphone/Symmetric/Media", "No");
	}
	
	str.setNum( sipPreferences.mediaMinPort );
	settings.writeEntry("/kphone/Media/MinPort", str );
	str.setNum( sipPreferences.mediaMaxPort );
	settings.writeEntry("/kphone/Media/MaxPort", str);

	
	
}


void VoIPUserAgent::updateAudioPreferences(AudioPreferences_t audioPreferences)
{
	QSettings settings;
	
	settings.writeEntry( "/kphone/audio/Codec", audioPreferences.preferredCodec);
	settings.writeEntry( "/kphone/audio/deviceforROWO",  audioPreferences.deviceForReadOnlyWriteOnly);
	settings.writeEntry( "/kphone/audio/deviceforRO",  audioPreferences.deviceForReadOnly);
	settings.writeEntry( "/kphone/dsp/SizeOfPayload",  audioPreferences.preferredPayloadSize);
	
  
}
void VoIPUserAgent::addContactToContactsList(const QString& contactUri)
{
    if ( !monitoredUris.contains( contactUri ) ) {
	monitoredUris.append( contactUri );
	if ( currentRegStatus == VOIP_PROXY_REGISTERED )
	    UpdateContactList( monitoredUris );
    }
}

void VoIPUserAgent::deleteContactFromContactsList(const QString& contactUri)
{
    if ( monitoredUris.contains( contactUri ) ) {
	monitoredUris.remove( contactUri );
	if ( currentRegStatus == VOIP_PROXY_REGISTERED )
	    UpdateContactList( monitoredUris );
    }
}

void VoIPUserAgent::logoutFromSipProxy()
{
    // hang up existing calls
    for (unsigned int callI = 0; callI < adaptationLayerCallList.count(); ++callI) {
        AdaptationLayerCallInfo* callinfo = adaptationLayerCallList.at(callI);
        KCallWidget *hC = callinfo->getCallWidgetIdentifier();
        if (hC) {
            SipCallMember *member = hC->getCallMember();
            if (getCallStatus(member) == VOIP_CALL_DIALING || getCallStatus(member) == VOIP_CALL_CONNECTED) {
                setCallStatus(member,VOIP_CALL_HANGUP_LOCAL);
                // This will be informed as a call status update over the VOIP_AGENT_CHANNEL only after the 'call disconnect' happens
                VoIPCallHangup(hC);
            }
        }
    }

    VoIPlogout();
}

void VoIPUserAgent::deInitializeStack()
{
//ToDo:
}



//internal functions -  would be involved in dealing with the call list after some time.

KCallWidget* VoIPUserAgent::getCallWidget(QString callIdentifier)
{

	AdaptationLayerCallInfo *callinfo = adaptationLayerCallList.first ();
	while (callinfo )
	{
		if (callinfo->getCallIdentifier() == callIdentifier)
			return callinfo->getCallWidgetIdentifier();
		callinfo = adaptationLayerCallList.next();
	}
	return NULL;

}

QString VoIPUserAgent::getCallIdentifier(KCallWidget* callW)
{
	AdaptationLayerCallInfo *callinfo = adaptationLayerCallList.first ();
	while (callinfo )
	{
		if (callinfo->getCallWidgetIdentifier()== callW)
			return callinfo->getCallIdentifier() ;
		callinfo = adaptationLayerCallList.next();
	}
	return NULL;
}
CallStatus_t VoIPUserAgent::getCallStatus(SipCallMember *member)
{
	AdaptationLayerCallInfo *callinfo = adaptationLayerCallList.first ();
	while (callinfo )
	{
		if (callinfo->getCallWidgetIdentifier()->getCallMember()== member)
			return callinfo->getCallStatus() ;
		callinfo = adaptationLayerCallList.next();
	}
	return VOIP_CALL_IDLE; //ToDo: verify of this is fine


}
CallStatus_t VoIPUserAgent::getCallStatus(KCallWidget *callW) 
{
	AdaptationLayerCallInfo *callinfo = adaptationLayerCallList.first ();
	while (callinfo )
	{
		if (callinfo->getCallWidgetIdentifier()== callW)
			return callinfo->getCallStatus() ;
		callinfo = adaptationLayerCallList.next();
	}
	return VOIP_CALL_IDLE; //ToDo: verify of this is fine

}
void VoIPUserAgent::setCallStatus(SipCallMember *member, CallStatus_t  fwNewCallStatus)
{
	AdaptationLayerCallInfo *callinfo = adaptationLayerCallList.first ();
	while (callinfo )
	{
		if (callinfo->getCallWidgetIdentifier()->getCallMember()== member)
		{
			callinfo->setCallStatus(fwNewCallStatus) ;
			return ;
		}
		callinfo = adaptationLayerCallList.next();
	}

}

QString VoIPUserAgent::getCallidentifier(SipCallMember *member)
{
	AdaptationLayerCallInfo *callinfo = adaptationLayerCallList.first ();
	while (callinfo )
	{
		if (callinfo->getCallWidgetIdentifier()->getCallMember()== member)
			return callinfo->getCallIdentifier() ;
		callinfo = adaptationLayerCallList.next();
	}
	return NULL;

}

void VoIPUserAgent::getUriStringList(	QStringList  & contactsUriStringList )
{	
    contactsUriStringList = monitoredUris;
}

AdaptationLayerCallInfo::AdaptationLayerCallInfo
	(KCallWidget * newCallW, QString newCallIdentifier, CallStatus_t newCallStatus)
{
	callW = newCallW;
	callIdentifier = newCallIdentifier; 
	callStatus = newCallStatus; 	

}
AdaptationLayerCallInfo::~AdaptationLayerCallInfo(void)
{

}


