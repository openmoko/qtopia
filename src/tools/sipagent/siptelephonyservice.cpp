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

#include "siptelephonyservice.h"
#include "siplayer.h"

SipTelephonyService::SipTelephonyService
        ( const QString& service, SipLayer *layer, QObject *parent )
    : QTelephonyService( service, parent )
{
    _layer = layer;
}

SipTelephonyService::~SipTelephonyService()
{
}

void SipTelephonyService::initialize()
{
    if ( !supports<QPresence>() )
        addInterface( new SipPresence( this ) );

    if ( !supports<QNetworkRegistration>() )
        addInterface( new SipNetworkRegistration( this ) );

    if ( !supports<QTelephonyConfiguration>() )
        addInterface( new SipConfiguration( this ) );

    if ( !callProvider() )
        setCallProvider( new SipCallProvider( this ) );

    QTelephonyService::initialize();
}

SipPresence::SipPresence( SipTelephonyService *service )
    : QPresence( service->service(), service, QCommInterface::Server )
{
    this->service = service;
    connect( service->layer(), SIGNAL(monitorChanged(QString,bool)),
             this, SLOT(monitorChanged(QString,bool)) );
}

SipPresence::~SipPresence()
{
}

bool SipPresence::startMonitoring( const QString& uri )
{
    if ( QPresence::startMonitoring( uri ) ) {
        service->layer()->startMonitor( uri );
        return true;
    } else {
        return false;
    }
}

bool SipPresence::stopMonitoring( const QString& uri )
{
    if ( QPresence::stopMonitoring( uri ) ) {
        service->layer()->stopMonitor( uri );
        return true;
    } else {
        return false;
    }
}

void SipPresence::setLocalPresence( QPresence::Status status )
{
    // Update the network with the new presence state.
    if ( status == QPresence::Available )
        service->layer()->setBusy( false );
    else
        service->layer()->setBusy( true );

    // Notify client applications of the change.
    setValue( "localPresence", (int)status );
    emit localPresenceChanged();
}

void SipPresence::monitorChanged( const QString& uri, bool available )
{
    // Notify client applications of the change in monitoring status.
    updateMonitoredPresence( uri, ( available ? Available : Unavailable ) );
}

SipNetworkRegistration::SipNetworkRegistration( SipTelephonyService *service )
    : QNetworkRegistrationServer( service->service(), service )
{
    this->service = service;
    connect( service->layer(),
             SIGNAL(registrationChanged(SipRegister::RegisterState)),
             this, SLOT(registrationChanged(SipRegister::RegisterState)) );
}

SipNetworkRegistration::~SipNetworkRegistration()
{
}

void SipNetworkRegistration::setCurrentOperator
        ( QTelephony::OperatorMode mode, const QString&, const QString& )
{
    // The mode is the only parameter that makes sense for VoIP.
    // We use it to register to or deregister from the network.
    if ( mode == QTelephony::OperatorModeDeregister ) {
        service->layer()->deregisterFromProxy();
    } else {
        service->layer()->registerToProxy();
    }
}

void SipNetworkRegistration::requestAvailableOperators()
{
    // "Available network operators" doesn't make sense for VoIP,
    // so return an empty list.
    QList<QNetworkRegistration::AvailableOperator> list;
    emit availableOperators( list );
}

void SipNetworkRegistration::registrationChanged
        ( SipRegister::RegisterState state )
{
    QTelephony::RegistrationState value = QTelephony::RegistrationNone;
    switch ( state ) {
        case SipRegister::Connected:
            value = QTelephony::RegistrationHome; break;

        case SipRegister::NotConnected:
            value = QTelephony::RegistrationNone; break;

        case SipRegister::TryingServerWithPassword:
        case SipRegister::TryingServer:
            value = QTelephony::RegistrationSearching; break;

        case SipRegister::AuthenticationRequiredWithNewPassword:
            value = QTelephony::RegistrationDenied; break;

        case SipRegister::Disconnecting:
        case SipRegister::AuthenticationRequired:
        case SipRegister::Reconnecting:
            // These values don't need to be reported higher up.
            return;
    }
    updateRegistrationState( value );
}

SipConfiguration::SipConfiguration( SipTelephonyService *service )
    : QTelephonyConfiguration( service->service(), service,
                               QCommInterface::Server )
{
    this->service = service;
}

SipConfiguration::~SipConfiguration()
{
}

void SipConfiguration::update( const QString& name, const QString& )
{
    // Process messages from the "sipsettings" program for config updates.
    if ( name == "registration" )
        service->layer()->updateRegistrationConfig();
    else if ( name == "general" )
        service->layer()->updateGeneralConfig();
}

void SipConfiguration::request( const QString& name )
{
    // Not supported - just return an empty value.
    emit notification( name, QString() );
}

SipCallObject::SipCallObject
        ( SipCallProvider *provider, const QString& identifier,
          const QString& callType )
    : QPhoneCallImpl( provider, identifier, callType )
{
    service = provider->service();
    connect( service->layer(),
             SIGNAL(stateChanged(QString,QPhoneCall::State,QString)),
             this, SLOT(stateChanged(QString,QPhoneCall::State,QString)) );
    connect( service->layer(),
             SIGNAL(callingName(QString,QString)),
             this, SLOT(callingName(QString,QString)) );
    connect( service->layer(),
             SIGNAL(remoteHold(QString,QString)),
             this, SLOT(remoteHold(QString,QString)) );
}

SipCallObject::~SipCallObject()
{
}

void SipCallObject::dial( const QDialOptions& options )
{
    service->layer()->dial( identifier(), options.number(),
                            options.callerId() == QDialOptions::SendCallerId );
}

void SipCallObject::hangup( QPhoneCall::Scope /*scope*/ )
{
    service->layer()->hangup( identifier() );
}

void SipCallObject::accept()
{
    service->layer()->accept( identifier() );
}

void SipCallObject::hold()
{
    service->layer()->hold( identifier() );
}

void SipCallObject::activate( QPhoneCall::Scope /*scope*/ )
{
    service->layer()->activate( identifier() );
}

void SipCallObject::join( bool /*detachSubscriber*/ )
{
    // Join is not supported by sip.
    emit requestFailed( QPhoneCall::JoinFailed );
}

void SipCallObject::tone( const QString& tones )
{
    service->layer()->tone( identifier(), tones );
}

void SipCallObject::transfer( const QString& /*number*/ )
{
    // Transfer is not supported by sip.
    emit requestFailed( QPhoneCall::TransferFailed );
}

static QPhoneCallImpl::Actions actionsForState( QPhoneCall::State state )
{
    switch ( state ) {

        case QPhoneCall::Dialing:
        case QPhoneCall::Alerting:
            return QPhoneCallImpl::Tone;

        case QPhoneCall::Incoming:
            return QPhoneCallImpl::Accept;

        case QPhoneCall::Connected:
            return QPhoneCallImpl::Hold | QPhoneCallImpl::Tone;

        case QPhoneCall::Hold:
            return QPhoneCallImpl::ActivateCall | QPhoneCallImpl::ActivateGroup;

        default: return QPhoneCallImpl::None;
    }
}

void SipCallObject::stateChanged( const QString& id, QPhoneCall::State state,
                                  const QString& uri )
{
    if ( id == identifier() ) {
        if ( !uri.isEmpty() )
            setNumber( uri );
        setActions( actionsForState( state ) );
        setState( state );
    }
}

void SipCallObject::callingName( const QString& id, const QString& name )
{
    if ( id == identifier() )
        emit notification( QPhoneCall::CallingName, name );
}

void SipCallObject::remoteHold( const QString& id, const QString& value )
{
    if ( id == identifier() )
        emit notification( QPhoneCall::RemoteHold, value );
}

SipCallProvider::SipCallProvider( SipTelephonyService *service )
    : QPhoneCallProvider( service->service(), service )
{
    _service = service;
    connect( service->layer(),
             SIGNAL(stateChanged(QString,QPhoneCall::State,QString)),
             this, SLOT(stateChanged(QString,QPhoneCall::State,QString)) );
    setCallTypes( QStringList( "VoIP" ) );
}

SipCallProvider::~SipCallProvider()
{
}

QPhoneCallImpl *SipCallProvider::create
        ( const QString& identifier, const QString& callType )
{
    return new SipCallObject( this, identifier, callType );
}

void SipCallProvider::stateChanged
        ( const QString& id, QPhoneCall::State state, const QString& uri )
{
    if ( !findCall( id ) && state == QPhoneCall::Incoming ) {
        // This is a new incoming call that we haven't seen before.
        // Create a new call object for it and announce it to the system.
        QPhoneCallImpl *call = create( id, "VoIP" );    // No tr
        if ( !uri.isEmpty() )
            call->setNumber( uri );
        call->setActions( actionsForState( state ) );
        call->setState( state );
    }
}
