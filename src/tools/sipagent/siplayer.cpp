/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "siplayer.h"
#include "sipagentmedia.h"
#include "dissipate2/sipcall.h"
#include "dissipate2/sipprotocol.h"
#include "sdpparser.h"
#include "mediasessionclient.h"
#include <qnetworkstate.h>
#include <qnetworkdevice.h>
#include <qtopialog.h>

#include <QNetworkInterface>
#include <qsettings.h>
#include <qtextdocument.h>
#include <quuid.h>

SipLayerCallInfo::SipLayerCallInfo( const QString& id, SipCallMember *member )
    : QObject( member )
{
    _id = id;
    _member = member;
    _call = member->getCall();
    _session = 0;
    _prevState = QPhoneCall::Idle;
    _incomingInvite = false;
}

SipLayerCallInfo::~SipLayerCallInfo()
{
}

void SipLayerCallInfo::setSession( MediaSessionClient *session )
{
    _session = session;
    if ( session )
        session->setParent( this );
}

SipLayerCallInfo *SipLayerCallInfo::fromSession( MediaSessionClient *session )
{
    if ( session )
        return (SipLayerCallInfo *)( session->parent() );
    else
        return 0;
}

SipLayerCallInfo *SipLayerCallInfo::fromCall( SipCall *call )
{
    return fromMember( call->getMemberList().first() );
}

SipLayerCallInfo *SipLayerCallInfo::fromMember( SipCallMember *member )
{
    if ( member )
        return member->findChild<SipLayerCallInfo *>();
    else
        return 0;
}

SipLayer::SipLayer( QObject *parent )
    : QObject( parent )
{
    // Reset all values before we start.
    _client = 0;
    registration = 0;
    atom = 1000;
    isOnline = false;
    isBusy = false;
    prevstate = SipRegister::NotConnected;
    delayedRegTimer = 0;
    activeSession = 0;
    pendingUpdate = false;
    pendingDeregister = false;
    pendingRegister = false;

    // Update libdissipate2's local IP address from Qtopia's network settings.
    QNetworkState *networkState = new QNetworkState( this );
    QObject::connect
        ( networkState, SIGNAL(defaultGatewayChanged(QString,QNetworkInterface)),
          this, SLOT(networkChanged(QString,QNetworkInterface)) );
    if ( networkState->gateway().isEmpty() )
        updateAddress( QNetworkInterface() );
    else
        updateAddress( QNetworkDevice( networkState->gateway() ).address() );

    // Create the SIP client object that will handle all communication.
    _client = new SipClient( this, 0, 5060, false, false, "UDP" );
    connect( _client, SIGNAL(callListUpdated()),
             this, SLOT(callListUpdated()) );
    connect( _client, SIGNAL(incomingSubscribe(SipCallMember*,bool)),
             this, SLOT(incomingSubscribe(SipCallMember*,bool)) );
    connect( _client, SIGNAL(incomingCall(SipCall*,QString)),
             this, SLOT(incomingCall(SipCall*,QString)) );

    // Load the initial settings from the configuration file.
    updateGeneralConfig();
    updateRegistrationConfig();
}

SipLayer::~SipLayer()
{
}

bool SipLayer::online()
{
    return isOnline;
}

void SipLayer::setOnline( bool flag )
{
    isOnline = flag;
    SipCallList calls = client().getCallList();
    SipCallList::Iterator it;
    for ( it = calls.begin(); it != calls.end(); ++it ) {
        if ( (*it)->getCallType() == SipCall::inSubscribeCall ) {
            if ( (*it)->getCallStatus() == SipCall::callInProgress ) {
                SipCallMember *member = (*it)->getMemberList().first();
                if ( member )
                    notify( member );
            }
        }
    }
    emit onlineChanged();
}

bool SipLayer::busy()
{
    return isBusy;
}

void SipLayer::setBusy( bool flag )
{
    isBusy = flag;
    client().setBusy( flag );
    if ( isOnline )
        setOnline( true );      // Force update of subscribed peers.
}

bool SipLayer::isRegistered()
{
    if ( !registration )
        return false;
    return ( registration->getRegisterState() == SipRegister::Connected );
}

SipRegister::RegisterState SipLayer::registrationState()
{
    return prevstate;
}

void SipLayer::updateGeneralConfig()
{
    QSettings config( "Trolltech", "SIPAgent" );
    config.beginGroup( "Client" );

    // Select UDP or TCP for the connection protocol.
    QString mode = config.value( "SocketMode", QString( "UDP" ) ).toString();
    if ( mode == "UDP" )
        client().setSocketMode( SipClient::UDP );
    else
        client().setSocketMode( SipClient::TCP );

    // Set other obscure client options.
    client().setSymmetricMode( config.value( "SymmetricMode", true ).toBool() );
    QString hideMode = config.value
        ( "HideViaMode", QString( "DontHideVia" ) ).toString();
    if ( hideMode == "HideHop" )
        client().setHideViaMode( SipClient::HideHop );
    else if ( hideMode == "HideRoute" )
        client().setHideViaMode( SipClient::HideRoute );
    else
        client().setHideViaMode( SipClient::DontHideVia );

    config.endGroup();      // Client

    // Configure status information.
    config.beginGroup( "Status" );

    // Set the call forwarding options.
    client().setCallForward( config.value( "CallForward", false ).toBool() );
    client().setCallForwardUri
        ( config.value( "CallForwardAddress", QString() ).toString() );
    client().setCallForwardMessage
        ( config.value( "CallForwardMessage", QString() ).toString() );
    client().setMaxForwards
        ( config.value( "MaxForwards", 0 ).toInt() );

    // Set the busy options.
    client().setBusyMessage
        ( config.value( "BusyMessage", QString() ).toString() );

    config.endGroup();      // Status

    // Configure presence options.
    config.beginGroup( "Presence" );

    // Set the presence subscribe expiry time.
    subscribeExpires = config.value( "SubscribeExpires", 600 ).toInt();

    config.endGroup();      // Presence
}

void SipLayer::updateRegistrationConfig()
{
    if ( pendingDeregister ) {
        // Probably switching to a new configuration, so we need
        // to wait until libdissipate2 has completely unregistered.
        pendingUpdate = true;
    } else {
        updateRegistrationConfigInner();
    }
}

bool SipLayer::updateRegistrationConfigInner()
{

    QSettings config( "Trolltech", "SIPAgent" );
    config.beginGroup( "Registration" );

    // Get the expiry time.
    int expires = config.value( "Expires", 900 ).toInt();
    if ( expires <= 0 )
        expires = -1;

    // Get the user and server identities.
    QString useruri = config.value( "UserUri", QString() ).toString();
    QString proxy = config.value( "Proxy", QString() ).toString();
    SipUri nameuri( useruri );
    SipUri proxyuri( proxy );

    // Create a new registration object if necessary.
    SipUser *user = 0;
    if ( useruri != prevuser || proxy != prevserver ) {
        if ( registration ) {
            // We had a previous registration.
            if ( useruri.isEmpty() ) {
                // We are changing to no registration.
                delete registration;
                registration = 0;
            } else if ( useruri != prevuser ) {
                // Registration URI changed from the previous value.
                delete registration;
                user = new SipUser( _client, nameuri );
                registration = new SipRegister( user, proxyuri, expires, "" );
                prevstate = SipRegister::NotConnected;
                connect( registration, SIGNAL(statusUpdated()),
                         this, SLOT(registrationStatusUpdated()) );
            }
        } else if ( !useruri.isEmpty() ) {
            // No previous registration - creating a new one.
            user = new SipUser( _client, nameuri );
            registration = new SipRegister( user, proxyuri, expires, "" );
            prevstate = SipRegister::NotConnected;
            connect( registration, SIGNAL(statusUpdated()),
                     this, SLOT(registrationStatusUpdated()) );
        }
        if ( user && registration )
            _client->updateIdentity( user, registration->getOutboundProxy() );
        prevuser = useruri;
        prevserver = proxy;
    } else {
        user = client().getUser();
    }

    // Set the proxy authentication details.
    if ( user ) {
        // Default to the User ID on "Identity Tab" if Proxy User ID is not set
        QString proxyUserName = config.value( "ProxyUserName",
                                              QString("") ).toString();
        if (proxyUserName.isEmpty())
            proxyUserName = user->getMyUri()->getUsername();
        user->getMyUri()->setProxyUsername(proxyUserName);
        QString password =
            config.value( "ProxyPassword", QString( "" ) ).toString();
        if ( password.startsWith( ":" ) ) {
            // The password has been base64-encoded.
            password = QString::fromUtf8
                ( QByteArray::fromBase64( password.mid(1).toLatin1() ) );
        }
        user->getMyUri()->setPassword( password );
    }

    // Does the user want to auto-register?
    bool autoRegister = config.value( "AutoRegister", false ).toBool();

    // Perform registration if we have valid server information.
    if ( registration ) {
        if ( autoRegister ) {
            registration->requestRegister();
            registerToProxy();
        } else {
            registration->setAutoRegister( false );
        }
    }

    config.endGroup();      // Registration

    return autoRegister;
}

void SipLayer::registerToProxy()
{
    if ( pendingDeregister ) {
        // Probably switching to a new configuration, so we need
        // to wait until libdissipate2 has completely unregistered.
        pendingRegister = true;
    } else {
        registerToProxyInner();
    }
}

void SipLayer::registerToProxyInner()
{
    if ( registration &&
         registration->getRegisterState() != SipRegister::Connected ) {
        registration->updateRegister();
        setOnline( true );
    }
}

void SipLayer::deregisterFromProxy()
{
    if ( registration &&
         registration->getRegisterState() == SipRegister::Connected ) {
        pendingDeregister = true;
        setBusy( true );
        registration->requestClearRegistration();
    }
}

void SipLayer::dial( const QString& id, const QString& _uri, bool /*callerid*/ )
{
    // Make sure that the uri starts with "sip:".
    QString uri = _uri;
    if ( !uri.startsWith( "sip:" ) &&
         uri.indexOf( QChar('@') ) != -1 && uri.indexOf( QChar(':') ) == -1 ) {
        uri = "sip:" + uri;
    }

    // Create a new call object and its associated member.
    SipCall *sipcall = new SipCall
        ( client().getUser(), QString::null, SipCall::StandardCall );
    sipcall->setSubject( uri );
    SipCallMember *member = new SipCallMember( sipcall, SipUri( uri ) );

    // Tag the call object with the identifier and get our call structure.
    SipLayerCallInfo *call = tagCall( sipcall, id );

    // Hook up interesting signals.
    connect( member, SIGNAL(statusUpdated(SipCallMember*)),
             this, SLOT(handleMemberAuth(SipCallMember*)) );
    connect( member, SIGNAL(statusUpdated(SipCallMember*)),
             this, SLOT(callStatusUpdated(SipCallMember*)) );

    // Start a locally-initiated media session to send the invite.
    // sendLocalParameters() will do the invite once the local media
    // parameters have been determined.
    MediaSessionClient *session = new MediaSessionClient();
    connect( session, SIGNAL(sendLocalParameters(QString)),
             this, SLOT(sendLocalParameters(QString)) );
    connect( session, SIGNAL(onHoldChanged()), this, SLOT(onHoldChanged()) );
    connect( session, SIGNAL(remoteOnHoldChanged()),
             this, SLOT(remoteOnHoldChanged()) );
    call->setSession( session );
    session->setupLocal();

    // Update the call status, which will announce the call to the system.
    call->setPrevState( QPhoneCall::Dialing );
    emit stateChanged( id, QPhoneCall::Dialing, uri );
}

void SipLayer::accept( const QString& id )
{
    SipLayerCallInfo *call = callFromIdentifier( id );
    if ( !call )
        return;
    SipCallMember *member = call->member();

    // Bail out if the call is not able to be accepted.
    if ( member->getState() != SipCallMember::state_RequestingInvite )
        return;

    // Check that the codec is supported.
    QString remotesdp = member->getSessionDescription();
    SdpParser parser( remotesdp );
    QStringList codecs = parser.codecs( "audio" );
    if ( ! SipAgentMedia::supportsCodecs( codecs ) ) {
        // Reject the call outright, and report the failure.
        call->setPrevState( QPhoneCall::OtherFailure );
        member->notAcceptableHere();
        emit stateChanged( id, QPhoneCall::OtherFailure, member->getSubject() );
        return;
    }

    // Start negotiating a media session for this call.
    MediaSessionClient *session = new MediaSessionClient();
    call->setSession( session );
    call->setIncomingInvite( true );
    connect( session, SIGNAL(sendLocalParameters(QString)),
             this, SLOT(sendLocalParameters(QString)) );
    connect( session, SIGNAL(onHoldChanged()), this, SLOT(onHoldChanged()) );
    connect( session, SIGNAL(remoteOnHoldChanged()),
             this, SLOT(remoteOnHoldChanged()) );
    session->setupRemote( remotesdp );
}

void SipLayer::hangup( const QString& id )
{
    SipLayerCallInfo *call = callFromIdentifier( id );
    if ( !call )
        return;
    SipCallMember *member = call->member();

    // Shut down the media session, if one is present.
    hangupMedia( call );

    // Record that we hung up the call locally.
    call->setPrevState( QPhoneCall::HangupLocal );

    // Perform the hangup.
    if ( member->getState() == SipCallMember::state_RequestingInvite ) {

        // Hangup for incoming call means that we decline to be invited.
        member->declineInvite();

    } else if ( call->call()->getCallStatus() != SipCall::callDead ) {

        if ( member->getState() == SipCallMember::state_Connected ) {
            member->requestDisconnect();
        } else {
            member->cancelTransaction();
            emit stateChanged
                ( id, QPhoneCall::HangupLocal, member->getSubject() );
        }

    }
}

void SipLayer::tone( const QString& id, const QString& tones )
{
    // Get the call details for the identifier.
    SipLayerCallInfo *call = callFromIdentifier( id );
    if ( !call )
        return;
    MediaSessionClient *session = call->session();
    if ( !session )
        return;

    // Send the tones.
    session->dtmf( tones );
}

void SipLayer::hold( const QString& id )
{
    // Get the call details for the identifier.
    SipLayerCallInfo *call = callFromIdentifier( id );
    if ( !call )
        return;
    SipCallMember *member = call->member();
    MediaSessionClient *session = call->session();
    if ( !session )
        return;

    // If the call is not in the connected state, then bail out.
    if ( member->getState() != SipCallMember::state_Connected )
        return;

    // If the session is already on hold, then nothing further to do.
    if ( session->localOnHold() )
        return;

    // Clear the active session.
    if ( activeSession == session )
        activeSession = 0;

    // Put the session on hold.
    session->setLocalOnHold( true );
}

void SipLayer::activate( const QString& id )
{
    // Get the call details for the identifier.
    SipLayerCallInfo *call = callFromIdentifier( id );
    if ( !call )
        return;
    SipCallMember *member = call->member();
    MediaSessionClient *session = call->session();
    if ( !session )
        return;

    // If the call is not in the connected state, then bail out.
    if ( member->getState() != SipCallMember::state_Connected )
        return;

    // If the session is not on hold, then nothing further to do.
    if ( !session->localOnHold() )
        return;

    // Put the active session on hold.
    if ( activeSession )
        activeSession->setLocalOnHold( true );

    // Take the session off hold and make it the active session.
    activeSession = session;
    session->setLocalOnHold( false );
}

void SipLayer::startMonitor( const QString& uri )
{
    if ( !monitoring.contains( uri ) ) {
        monitoring += uri;
        if ( isRegistered() )
            setMonitor( uri );
    }
}

void SipLayer::stopMonitor( const QString& uri )
{
    if ( !monitoring.contains( uri ) ) {
        monitoring.removeAll( uri );
        if ( isRegistered() )
            clearMonitor( uri );
    }
}

void SipLayer::setMonitor( const QString& uri )
{
    SipCall *call;
    SipCallMember *member;

    call = new SipCall( client().getUser(), QString::null,
                        SipCall::outSubscribeCall );
    call->setSubject( uri );

    member = new SipCallMember( call, SipUri( uri ) );
    connect( member, SIGNAL(statusUpdated(SipCallMember*)),
             this, SLOT(handleMemberAuth(SipCallMember*)) );
    member->requestSubscribe( subscribeExpires );
}

void SipLayer::clearMonitor( const QString& uri, bool resubscribe )
{
    // Search for the monitor and remove it.
    SipCallList calls = client().getCallList();
    SipCallList::Iterator it;
    for ( it = calls.begin(); it != calls.end(); ++it ) {
        if ( (*it)->getCallType() == SipCall::outSubscribeCall ) {
            if ( (*it)->getSubject() == uri ) {
                // Deleting the SipCall object will arrange for it
                // to be removed from the client's call list.
                if ( resubscribe )
                    (*it)->getMember( SipUri( uri ) )->requestClearSubscribe();
                delete (*it);
                break;
            }
        }
    }

    // Report the monitor as unavailable once it has been cleared.
    if ( !resubscribe )
        emit monitorChanged( uri, false );
}

void SipLayer::setAllMonitors()
{
    QStringList::ConstIterator it;
    for ( it = monitoring.begin(); it != monitoring.end(); ++it )
        setMonitor( *it );
}

void SipLayer::clearAllMonitors()
{
    QStringList::ConstIterator it;
    for ( it = monitoring.begin(); it != monitoring.end(); ++it )
        clearMonitor( *it );
}

void SipLayer::callListUpdated()
{
    // Check for presence changes on all of the callers we are connected to.
    SipCallList calls = client().getCallList();
    SipCallList::Iterator iter;
    for ( iter = calls.begin(); iter != calls.end(); ++iter ) {
        if ( (*iter)->getCallType() == SipCall::outSubscribeCall ) {
            if ( (*iter)->getCallStatus() != SipCall::callDead ) {
                bool available;
                if ( (*iter)->getCallStatus() == SipCall::callInProgress ) {
                    available =
                      ( (*iter)->getPresenceStatus().toLower() != "busy" );
                } else {
                    available = false;
                }
                emit monitorChanged( (*iter)->getSubject(), available );
            }
        }
    }
}

void SipLayer::registrationStatusUpdated()
{
    if ( registration ) {
        SipRegister::RegisterState state;
        state = registration->getRegisterState();
        if ( ( state == SipRegister::TryingServer ||
               state == SipRegister::TryingServerWithPassword ) &&
             prevstate == SipRegister::Connected ) {

            // We have just transitioned from "home" to "searching".
            // This may have happened because the SIP stack is doing a
            // "keep alive" to the server.  We delay telling the rest
            // of the system about "searching" until we are reasonably
            // sure that it is actually searching and not just polling.
            // If it goes back to "home" quickly, there is no need
            // for the rest of the system to be informed.
            if ( !delayedRegTimer ) {
                delayedRegTimer = new QTimer( this );
                delayedRegTimer->setSingleShot( true );
                connect( delayedRegTimer, SIGNAL(timeout()),
                         this, SLOT(delayedRegTimerTimeout()) );
            }
            delayedRegTimer->start( 500 );

        } else if ( state == SipRegister::AuthenticationRequired ) {

            // The server has asked for our authentication credentials.
            QString username, password;
            username = registration->getRegisterCall()->getProxyUsername();
            password = registration->getRegisterCall()->getPassword();
            registration->requestRegister( username, password );

        } else {

            // Send the registration status change immediately.
            if ( delayedRegTimer )
                delayedRegTimer->stop();
            emit registrationChanged( state );

            // We need to update the monitored contacts if we just
            // transitioned to or from the connected state.
            if ( state != prevstate ) {
                if ( state == SipRegister::Connected ) {
                    setAllMonitors();
                } else if ( ( state == SipRegister::NotConnected &&
                              prevstate == SipRegister::Connected ) ) {
                    clearAllMonitors();
                }
            }

            // Record the previous state.
            prevstate = state;

            // If we just unregistered, then update the configuration.
            if ( state == SipRegister::NotConnected ) {
                pendingDeregister = false;
                if ( pendingUpdate ) {
                    bool needRegister = pendingRegister;
                    pendingUpdate = false;
                    pendingRegister = false;
                    if ( !updateRegistrationConfigInner() && needRegister ) {
                        // Update may not have registered if auto-register
                        // was not set in the configuration.
                        registerToProxyInner();
                    }
                } else if ( pendingRegister ) {
                    pendingRegister = false;
                    registerToProxyInner();
                }
            }
        }
    }
}

void SipLayer::delayedRegTimerTimeout()
{
    // We received a timeout while the state was "searching".
    // So we really do seem to be searching, and not just polling.
    prevstate = SipRegister::TryingServer;
    emit registrationChanged( SipRegister::TryingServer );
}

void SipLayer::handleMemberAuth( SipCallMember *member )
{
    // Bail out if this is not an authentication request.
    SipCallMember::AuthState authState = member->getAuthState();
    if ( authState != SipCallMember::authState_AuthenticationRequired &&
         authState !=
            SipCallMember::authState_AuthenticationRequiredWithNewPassword ) {
        return;
    }

    // Fetch the username and password to use for authentication.
    // TODO: handle the "new password" case in some useful fashion.
    QString username = member->getCall()->getProxyUsername();
    QString password = member->getCall()->getPassword();

    // Determine the kind of authentication that must be performed.
    switch ( member->getCallMemberType() ) {

        case SipCallMember::Subscribe:
            member->sendRequestSubscribe( username, password );
            break;

        case SipCallMember::Notify:
            member->sendRequestNotify( username, password );
            break;

        case SipCallMember::Message:
            member->sendRequestMessage( username, password );
            break;

        case SipCallMember::Invite:
            member->sendRequestInvite( username, password );
            break;

        default: break;
    }
}

void SipLayer::incomingSubscribe( SipCallMember *member, bool sendSubscribe )
{
    // Hook up interesting signals.
    connect( member, SIGNAL(statusUpdated(SipCallMember*)),
             this, SLOT(handleMemberAuth(SipCallMember*)) );

    // If we are currently online, then notify the other party.
    if ( online() )
        notify( member );

    // Bail out if we don't need to send a reverse subscribe,
    // or if we are not currently registered.
    if ( !sendSubscribe || !isRegistered() )
        return;

    // Search for a matching outgoing subscribe.
    SipCallList calls = client().getCallList();
    SipCallList::Iterator it;
    for ( it = calls.begin(); it != calls.end(); ++it ) {
        if ( (*it)->getCallType() == SipCall::outSubscribeCall &&
             (*it)->getCallStatus() != SipCall::callDead ) {
            if ( (*it)->getMember( member->getUri() ) ) {
                // Clear the subscription and then re-subscribe, which
                // will have the effect of answering the incoming subscribe.
                QString uri = (*it)->getSubject();
                clearMonitor( uri, true );
                setMonitor( uri );
                break;
            }
        }
    }
}

void SipLayer::incomingCall( SipCall *call, const QString& body )
{
    // Create a globally-unique identifier for the new call.
    QString identifier = QUuid::createUuid().toString();
    SipLayerCallInfo *info = tagCall( call, identifier );

    // Set up the call the way we want it.
    call->setSdpMessageMask( body );
    SipCallMember *member = info->member();
    call->setSubject( member->getUri().uri() );
    connect( member, SIGNAL(statusUpdated(SipCallMember*)),
             this, SLOT(callStatusUpdated(SipCallMember*)) );

    // Force an initial status check to announce the incoming call.
    callStatusUpdated( member );

    // If we have a full name for the incoming call, then advertise it.
    QString name = member->getUri().getFullname();
    if ( !name.isEmpty() )
        emit callingName( identifier, name );
}

void SipLayer::callStatusUpdated( SipCallMember *member )
{
    MediaSessionClient *session;
    SipLayerCallInfo *call = SipLayerCallInfo::fromMember( member );
    if ( !call )
        return;
    session = call->session();
    QPhoneCall::State state = QPhoneCall::Idle;
    switch( member->getState() ) {
        case SipCallMember::state_Connected:
            if ( call->prevState() == QPhoneCall::HangupLocal ) {
                // We cancelled an outgoing call, but it is still connected
                // because libdissipate2 hasn't shut it down completely yet.
                state = QPhoneCall::HangupLocal;
            } else if ( call->prevState() == QPhoneCall::Dialing ||
                        call->prevState() == QPhoneCall::Alerting ||
                        call->prevState() == QPhoneCall::Hold ) {
                // Just transitioned from Dialing to Connected, or from
                // Hold to Connected.
                // Notify the session about the new remote parameters.
                if ( session ) {
                    session->changeRemoteParameters
                        ( member->getSessionDescription() );
                }
                state = QPhoneCall::Connected;
            } else if ( session && session->onHold() )
                state = QPhoneCall::Hold;
            else
                state = QPhoneCall::Connected;
            call->setPrevState( state );
            break;

        case SipCallMember::state_Disconnected:
            state = call->prevState();
            if ( state == QPhoneCall::Dialing ||
                 state == QPhoneCall::Alerting )
                state = QPhoneCall::HangupLocal;
            else if ( state == QPhoneCall::Incoming )
                state = QPhoneCall::Missed;
            else if ( state == QPhoneCall::Connected ||
                      state == QPhoneCall::Hold ) {
                // If we were connected or on hold and we get here,
                // then it was because of a remote hangup.  Local
                // hangups set the previous state in hangup().
                state = QPhoneCall::HangupRemote;
                hangupMedia( call );
            }
            call->setPrevState( state );
            break;

        case SipCallMember::state_InviteRequested:
            //state = QPhoneCall::Dialing;
            //call->setPrevState( state );
            break;

        case SipCallMember::state_RequestingInvite:
            state = QPhoneCall::Incoming;
            call->setPrevState( state );
            break;

        case SipCallMember::state_RequestingReInvite:
            // The other end is changing the session parameters.
            // Probably going on hold or coming off hold.
            if ( session ) {
                call->setIncomingInvite( true );
                session->changeRemoteParameters
                    ( member->getSessionDescription() );
            }
            break;

        case SipCallMember::state_Idle:
        case SipCallMember::state_EarlyDialog:
        case SipCallMember::state_Redirected:
        case SipCallMember::state_ReInviteRequested:
        case SipCallMember::state_Disconnecting:
        case SipCallMember::state_CancelPending:
        case SipCallMember::state_Refer:
        case SipCallMember::state_Refer_handling:
            break;
    }
    if ( state != QPhoneCall::Idle ) {
        emit stateChanged( call->id(), state, member->getSubject() );
    }
}

void SipLayer::sendLocalParameters( const QString& sdp )
{
    qLog(Sip) << "SipLayer::sendLocalParameters(" << sdp << ")";
    MediaSessionClient *session = (MediaSessionClient *)sender();
    SipLayerCallInfo *call = SipLayerCallInfo::fromSession( session );
    if ( !call )
        return;
    qLog(Sip) << "have call";
    SipCallMember *member = call->member();
    if ( call->incomingInvite() ) {
        qLog(Sip) << "acceptInvite";
        member->acceptInvite( sdp, MimeContentType( "application/sdp" ) );
        call->setIncomingInvite( false );
    } else {
        qLog(Sip) << "requestInvite";
        member->requestInvite( sdp, MimeContentType( "application/sdp" ) );
    }
}

void SipLayer::onHoldChanged()
{
    MediaSessionClient *session = (MediaSessionClient *)sender();
    SipLayerCallInfo *call = SipLayerCallInfo::fromSession( session );
    if ( !call )
        return;
    SipCallMember *member = call->member();
    if ( member->getState() == SipCallMember::state_Connected ) {
        emit stateChanged( call->id(),
                           ( session->onHold()
                                ? QPhoneCall::Hold
                                : QPhoneCall::Connected ),
                           member->getSubject() );
    }
}

void SipLayer::remoteOnHoldChanged()
{
    MediaSessionClient *session = (MediaSessionClient *)sender();
    SipLayerCallInfo *call = SipLayerCallInfo::fromSession( session );
    if ( !call )
        return;
    SipCallMember *member = call->member();
    if ( member->getState() == SipCallMember::state_Connected ) {
        emit remoteHold( call->id(),
                         ( session->remoteOnHold() ? "hold" : "unhold" ) );
    }
}

// Notify the other end of a call that our presence information has changed.
void SipLayer::notify( SipCallMember *member )
{
    QString msg;
    msg = "<?xml version=\"1.0\">\n"
          "<!DOCTYPE presence PUBLIC \"-//IETF//DTD RFCxxxx XPIDF 1.0//EN\" "
                "\"xpidf.dtd\">\n"
          "<presence>\n"
          "<presentity uri=\"";
    msg += Qt::escape( member->getUri().reqUri() );
    msg += ";method=SUBSCRIBE\"/>\n";
    msg += "<atom id=\"" + QString::number( atom++ ) + "\">\n";
    msg += "<address uri=\"";
    msg += client().getContactUri().reqUri();
    msg += ";user=ip\" priority=\"0,800000\">\n";
    msg += "<status status=\"open\"/>\n";
    if ( isOnline ) {
        if ( isBusy )
            msg += "<msnsubstatus substatus=\"busy\"/>\n";
        else
            msg += "<msnsubstatus substatus=\"online\"/>\n";
    } else {
        msg += "<msnsubstatus substatus=\"\"/>\n";
    }
    msg += "</address>\n</atom>\n</presence>\n";
    if ( isOnline ) {
        member->requestNotify
            ( msg, MimeContentType( "application/xpidf+xml" ) );
    } else {
        member->requestClearNotify
            ( msg, MimeContentType( "application/xpidf+xml" ) );
    }
}

// We use the contact string field in SipCall to store our call identifier.
SipLayerCallInfo *SipLayer::tagCall( SipCall *call, const QString& id )
{
    call->setContactStr( id );
    return new SipLayerCallInfo( id, call->getMemberList().first() );
}

SipLayerCallInfo *SipLayer::callFromIdentifier( const QString& id )
{
    SipCallList calls = client().getCallList();
    SipCallList::Iterator it;
    for ( it = calls.begin(); it != calls.end(); ++it ) {
        if ( (*it)->getContactStr() == id )
            return SipLayerCallInfo::fromCall( (*it) );
    }
    return 0;
}

void SipLayer::hangupMedia( SipLayerCallInfo *call )
{
    MediaSessionClient *session = call->session();
    if ( activeSession == session )
        activeSession = 0;
    if ( session ) {
        delete session;
        call->setSession( 0 );
    }
}

void SipLayer::networkChanged( QString ,
        const QNetworkInterface& newAddress
        )
{
    updateAddress( newAddress );
}

void SipLayer::updateAddress( const QNetworkInterface& address )
{
    if ( address.isValid() ) {
        // Use Qtopia's preferred IP address for the local machine.
        QList<QNetworkAddressEntry> list = address.addressEntries();
        if ( list.count() ) {
            Sip::setLocalAddress( list[0].ip().toString() );
            return;
        }
    }
    // We don't have an address from Qtopia, so force libdissipate2
    // to auto-discover an address from the kernel network interfaces.
    Sip::setLocalAddress( QString::null );
    Sip::getLocalAddress();
}
