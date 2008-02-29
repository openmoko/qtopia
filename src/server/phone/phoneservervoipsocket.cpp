/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include "phoneservervoipsocket.h"
#include <qphonesocket.h>
#include <qtopialog.h>
#include <qtopiaservices.h>
#include <qvaluespace.h>
#include <QTimer>

PhoneServerVoIPCall::PhoneServerVoIPCall( PhoneServerVoIPCallProvider *provider, const QString& callType, const QString& identifier )
    : QPhoneCallImpl( provider, identifier, callType )
{
}

PhoneServerVoIPCall::~PhoneServerVoIPCall()
{
    // Nothing to do here.
}

void PhoneServerVoIPCall::dial( const QDialOptions& options )
{
    if ( options.callerId() == QDialOptions::SendCallerId )
        send( "DIAL", options.number(), "voip" );
    else
        send( "DIAL", options.number(), "voip", "restrict" );
    setActions( QPhoneCallImpl::Tone ); // DTMF tones will queue up.
    setState( QPhoneCall::Dialing );
}

void PhoneServerVoIPCall::hangup( QPhoneCall::Scope scope )
{
    if ( scope == QPhoneCall::Group )
        send( "HANGUP", "group" );
    else
        send( "HANGUP", "callonly" );
    setState( QPhoneCall::HangupLocal );
}

void PhoneServerVoIPCall::accept()
{
    send( "ACCEPT" );
    setActions( QPhoneCallImpl::Hold | QPhoneCallImpl::Tone );
    setState( QPhoneCall::Connected );
}

void PhoneServerVoIPCall::hold()
{
    send( "HOLD" );
}

void PhoneServerVoIPCall::activate( QPhoneCall::Scope scope )
{
    if ( scope == QPhoneCall::Group )
        send( "ACTIVATE", "group" );
    else
        send( "ACTIVATE", "callonly" );
}

void PhoneServerVoIPCall::join( bool detachSubscriber )
{
    if ( detachSubscriber )
        send( "JOIN", "detach" );
    else
        send( "JOIN" );
}

void PhoneServerVoIPCall::tone( const QString& tones )
{
    send( "DTMF", tones );
}

void PhoneServerVoIPCall::transfer( const QString& number )
{
    send( "TRANSFER", number );
}

QPhoneSocket *PhoneServerVoIPCall::handler() const
{
    PhoneServerVoIPCallProvider *p = qobject_cast<PhoneServerVoIPCallProvider *>(&(provider()));
    if ( p )
        return p->service()->handler();
    else
        return 0;
}

void PhoneServerVoIPCall::send( const QString& cmd )
{
    QPhoneSocket *socket = handler();
    if ( socket )
        socket->send( cmd, identifier() );
}

void PhoneServerVoIPCall::send( const QString& cmd, const QString& arg1 )
{
    QPhoneSocket *socket = handler();
    if ( socket )
        socket->send( cmd, identifier(), arg1 );
}

void PhoneServerVoIPCall::send( const QString& cmd, const QString& arg1, const QString& arg2 )
{
    QPhoneSocket *socket = handler();
    if ( socket )
        socket->send( cmd, identifier(), arg1, arg2 );
}

void PhoneServerVoIPCall::send( const QString& cmd, const QString& arg1, const QString& arg2, const QString& arg3 )
{
    QPhoneSocket *socket = handler();
    if ( socket ) {
        QStringList list;
        list << cmd;
        list << identifier();
        list << arg1;
        list << arg2;
        list << arg3;
        socket->send( list );
    }
}

PhoneServerVoIPCallProvider::PhoneServerVoIPCallProvider( PhoneServerVoIPService *service )
    : QPhoneCallProvider( service->service(), service )
{
    _service = service;
    setCallTypes( QStringList( "VoIP" ) );
    connect( service, SIGNAL(handlerReceived(QStringList)),
             this, SLOT(received(QStringList)) );
}

PhoneServerVoIPCallProvider::PhoneServerVoIPCallProvider::~PhoneServerVoIPCallProvider()
{
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

QPhoneCallImpl *PhoneServerVoIPCallProvider::create
        ( const QString& identifier, const QString& callType )
{
    return new PhoneServerVoIPCall( this, callType, identifier );
}

void PhoneServerVoIPCallProvider::received( const QStringList& cmd )
{
    if ( cmd[0] == "STATUS" ) {

        // Change the status of a call.
        if ( cmd.count() >= 3 ) {

            // Decode the status string into a state value.
            QPhoneCall::State state;
            if ( cmd[2] == "incoming" )
                state = QPhoneCall::Incoming;
            else if ( cmd[2] == "dialing" )
                state = QPhoneCall::Dialing;
            else if ( cmd[2] == "alerting" )
                state = QPhoneCall::Alerting;
            else if ( cmd[2] == "connected" )
                state = QPhoneCall::Connected;
            else if ( cmd[2] == "hold" )
                state = QPhoneCall::Hold;
            else if ( cmd[2] == "hanguplocal" )
                state = QPhoneCall::HangupLocal;
            else if ( cmd[2] == "hangupremote" )
                state = QPhoneCall::HangupRemote;
            else if ( cmd[2] == "missed" )
                state = QPhoneCall::Missed;
            else if ( cmd[2] == "networkfailure" )
                state = QPhoneCall::NetworkFailure;
            else if ( cmd[2] == "servicehangup" )
                state = QPhoneCall::ServiceHangup;
            else
                state = QPhoneCall::OtherFailure;

            // We ignore the type - it is assumed to be "voip" for now.

            // Extract the caller id, if present.
            QString callerId;
            if ( cmd.count() >= 5 )
                callerId = cmd[4];

            // Find the call object.
            PhoneServerVoIPCall *callp = qobject_cast<PhoneServerVoIPCall *>( findCall( cmd[1] ) );
            if ( !callp ) {
                if ( state == QPhoneCall::Incoming ||
                     state == QPhoneCall::Dialing ||
                     state == QPhoneCall::Alerting ||
                     state == QPhoneCall::Connected ||
                     state == QPhoneCall::Hold ) {
                    // The handler is informing us about a call that
                    // we haven't seen before.  Usually incoming calls.
                    callp = new PhoneServerVoIPCall( this, "VoIP", cmd[1] );  // No tr
                    if ( callerId.length() > 0 )
                        callp->setNumber( callerId );
                    callp->setActions( actionsForState( state ) );
                    callp->setState( state );
                }
            }
            if ( callp ) {
                if ( callerId.length() > 0 )
                    callp->setNumber( callerId );
                callp->setActions( actionsForState( state ) );
                callp->setState( state );
            } else {
                qLog(Modem)
                    << "PhoneServerVoIPCallProvider::received: could not find call"
                    << cmd[1];
            }
        }

    } else if ( cmd[0] == "NAME" ) {

        // Set the CallingName property for a call.
        if ( cmd.count() >= 3 ) {
            PhoneServerVoIPCall *callp = qobject_cast<PhoneServerVoIPCall *>( findCall( cmd[1] ) );
            if ( callp ) {
                callp->emitNotification( QPhoneCall::CallingName, cmd[2] );
            }
        }

    } else if ( cmd[0] == "REMOTEHOLD" ) {

        // Set the RemoteHold property for a call.
        if ( cmd.count() >= 3 ) {
            PhoneServerVoIPCall *callp = qobject_cast<PhoneServerVoIPCall *>( findCall( cmd[1] ) );
            if ( callp ) {
                callp->emitNotification( QPhoneCall::RemoteHold, cmd[2] );
            }
        }
    }
}

PhoneServerVoIPConfiguration::PhoneServerVoIPConfiguration( PhoneServerVoIPService *service )
    : QTelephonyConfiguration( service->service(), service,
                               QCommInterface::Server )
{
    this->service = service;
    connect( service, SIGNAL(handlerReceived(QStringList)),
             this, SLOT(received(QStringList)) );
}

PhoneServerVoIPConfiguration::~PhoneServerVoIPConfiguration()
{
}

void PhoneServerVoIPConfiguration::update( const QString& name, const QString& value )
{
    QPhoneSocket *handler = service->handler();
    if ( !handler )
        return;
    handler->send( "SET", name, value );
}

void PhoneServerVoIPConfiguration::request( const QString& name )
{
    QPhoneSocket *handler = service->handler();
    if ( !handler )
        return;
    handler->send( "GET", name );
}

void PhoneServerVoIPConfiguration::received( const QStringList& cmd )
{
    if ( cmd[0] == "VALUE" ) {
        // The handler has sent us a configuration value - pass it on.
        if ( cmd.size() == 2 ) {
            emit notification( cmd[1], QString() );
        } else if ( cmd.size() > 2 ) {
            emit notification( cmd[1], cmd[2] );
        }
    }
}

PhoneServerVoIPNetworkRegistration::PhoneServerVoIPNetworkRegistration( PhoneServerVoIPService *service )
    : QNetworkRegistrationServer( service->service(), service )
{
    this->service = service;
    this->registrationState = QTelephony::RegistrationNone;
    connect( service, SIGNAL(handlerReceived(QStringList)),
             this, SLOT(received(QStringList)) );
}

PhoneServerVoIPNetworkRegistration::~PhoneServerVoIPNetworkRegistration()
{
}

void PhoneServerVoIPNetworkRegistration::setCurrentOperator
            ( QTelephony::OperatorMode mode, const QString& /*id*/,
              const QString& /*technology*/ )
{
    // The mode is the only parameter that makes sense for VoIP.
    // We use it to register to or deregister from the network.
    QPhoneSocket *handler = service->handler();
    if ( mode == QTelephony::OperatorModeDeregister ) {
        if ( handler )
            handler->send( "DEREGISTER" );
    } else {
        if ( handler )
            handler->send( "REGISTER" );
    }
}

void PhoneServerVoIPNetworkRegistration::requestAvailableOperators()
{
    // "Available network operators" doesn't make sense for VoIP,
    // so return an empty list.
    QList<QNetworkRegistration::AvailableOperator> list;
    emit availableOperators( list );
}

void PhoneServerVoIPNetworkRegistration::received( const QStringList& cmd )
{
    if ( cmd[0] == "REGISTRATION" ) {

        // Change in network registration status.
        if ( cmd.count() < 2 )
            return;
        if ( cmd[1] == "unregistered" )
            registrationState = QTelephony::RegistrationNone;
        else if ( cmd[1] == "searching" )
            registrationState = QTelephony::RegistrationSearching;
        else if ( cmd[1] == "denied" )
            registrationState = QTelephony::RegistrationDenied;
        else
            registrationState = QTelephony::RegistrationHome;
        updateOperatorInfo();

    } else if ( cmd[0] == "OPERATOR" ) {

        // Name of the network operator.
        if ( cmd.count() >= 2 ) {
            operatorName = cmd[1];
            updateOperatorInfo();
        }

    } else if ( cmd[0] == "INIT" ) {

        // Check that this is really a voip handler, for debug purposes.
        if ( !cmd.contains( "voip" ) )
            qLog(Modem) << "PhoneServerVoIPNetworkRegistration::received: handler does not support 'voip'";
        else
            updateInitialized( true );
    }
}

void PhoneServerVoIPNetworkRegistration::updateOperatorInfo()
{
    updateRegistrationState( registrationState );
    if ( registrationState == QTelephony::RegistrationHome ) {
        updateCurrentOperator( QTelephony::OperatorModeAutomatic,
                               operatorName, operatorName, "VoIP" );      // No tr
    } else {
        // Clear the operator name because we may have just transitioned
        // away from registered to deregistered and the operator name
        // is not valid in the deregistered state.
        operatorName = QString();
        updateCurrentOperator( QTelephony::OperatorModeDeregister,
                               operatorName, operatorName, "VoIP" );      // No tr
    }
}

PhoneServerVoIPPresence::PhoneServerVoIPPresence( PhoneServerVoIPService *service )
        : QPresence( service->service(), service, QCommInterface::Server )
{
    this->service = service;
    connect( service, SIGNAL(handlerReceived(QStringList)),
             this, SLOT(received(QStringList)) );
}

PhoneServerVoIPPresence::~PhoneServerVoIPPresence()
{
}

bool PhoneServerVoIPPresence::startMonitoring( const QString& uri )
{
    if ( !QPresence::startMonitoring( uri ) )
        return false;
    QPhoneSocket *handler = service->handler();
    if ( handler )
        handler->send( "STARTMONITOR", uri );
    return true;
}

bool PhoneServerVoIPPresence::stopMonitoring( const QString& uri )
{
    if ( !QPresence::stopMonitoring( uri ) )
        return false;
    QPhoneSocket *handler = service->handler();
    if ( handler )
        handler->send( "STOPMONITOR", uri );
    return true;
}

void PhoneServerVoIPPresence::setLocalPresence( QPresence::Status status )
{
    QPhoneSocket *handler = service->handler();
    if ( handler ) {
        handler->send( "SETPRESENCE", ( status == Available ?
                                        "available" : "unavailable" ) );
    }
    setPresence( status );
}

void PhoneServerVoIPPresence::setPresence( QPresence::Status status )
{
    setValue( "localPresence", (int)status );
    emit localPresenceChanged();
}

void PhoneServerVoIPPresence::received( const QStringList& cmd )
{
    QString c = cmd[0];
    QPresence::Status status;
    if ( c == "PRESENCE" ) {

        // Response to a "GETPRESENCE" query: update the local presence.
        if ( cmd.count() >= 2 && cmd[1] == "available" )
            status = Available;
        else
            status = Unavailable;
        setPresence( status );

    } else if ( c == "REGISTRATION" ) {

        // The registration state has changed, so request the current
        // presence state from the handler as it may no longer reflect
        // what we have cached within "localPresence".
        QPhoneSocket *handler = service->handler();
        if ( handler )
            handler->send( "GETPRESENCE" );

    } else if ( c == "MONITOR" ) {

        // Change in the presence status of a monitored user.
        if ( cmd.count() >= 3 ) {
            if ( cmd[2] == "available" )
                status = Available;
            else
                status = Unavailable;
            updateMonitoredPresence( cmd[1], status );
        }

    }
}

PhoneServerVoIPService::PhoneServerVoIPService( const QString& service, QObject *parent )
    : QTelephonyService( service, parent )
{
    server = new QPhoneSocket( -1, this );
    _handler = 0;
    connect( server, SIGNAL(incoming(QPhoneSocket *)),
             this, SLOT(incoming(QPhoneSocket *)) );

    // Full list of commands that we support.
    ourCommands += "INIT";                  // Handler to us.
    ourCommands += "STATUS";
    ourCommands += "NAME";
    ourCommands += "REMOTEHOLD";
    ourCommands += "GROUP";
    ourCommands += "REGISTRATION";
    ourCommands += "OPERATOR";
    ourCommands += "PRESENCE";
    ourCommands += "MONITOR";
    ourCommands += "VALUE";
    supportedCommands += ourCommands;
    supportedCommands += "DIAL";            // Us to handler.
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
    supportedCommands += "FAIL";            // Both directions.
    supportedCommands += "TEST";
    supportedCommands += "SUPPORTS";

    // Launch the voip handler.
    QTimer::singleShot( 2000, this, SLOT(launchHandler()) );
}

PhoneServerVoIPService::~PhoneServerVoIPService()
{
}

void PhoneServerVoIPService::initialize()
{
    if ( !supports<QPresence>() )
        addInterface( new PhoneServerVoIPPresence( this ) );

    if ( !supports<QNetworkRegistration>() )
        addInterface( new PhoneServerVoIPNetworkRegistration( this ) );

    if ( !supports<QTelephonyConfiguration>() )
        addInterface( new PhoneServerVoIPConfiguration( this ) );

    if ( !callProvider() )
        setCallProvider( new PhoneServerVoIPCallProvider( this ) );

    QTelephonyService::initialize();
}

// Determine if there is an socket-based VoIP handler in the system.
bool PhoneServerVoIPService::handlerPresent()
{
    QString channel = QtopiaService::channel( "VoIP" );
    return !channel.isEmpty();
}

void PhoneServerVoIPService::testCommand( const QStringList& cmd )
{
    if ( cmd[0] == "TEST" ) {

        // The handler has asked us if we support some particular commands.
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
        _handler->send( response );

    } else if ( !ourCommands.contains( cmd[0] ) ) {

        // We don't know what to do with this kind of command.
        QStringList error;
        error += "FAIL";
        error += "unknown-command";
        error += cmd;
        _handler->send( error );
    }
}

void PhoneServerVoIPService::incoming( QPhoneSocket *socket )
{
    if ( !_handler ) {
        _handler = socket;
        connect( _handler, SIGNAL(received(const QStringList&)),
                 this, SIGNAL(handlerReceived(const QStringList&)) );
        connect( _handler, SIGNAL(received(const QStringList&)),
                 this, SLOT(testCommand(const QStringList&)) );
        connect( _handler, SIGNAL(closed()), this, SLOT(closed()) );
        qLog(Modem) << "PhoneServerVoIPService::incoming: voip handler has connected";
        emit handlerStarted();
    } else {
        qLog(Modem) << "PhoneServerVoIPService::incoming: attempt to register multiple voip handlers";
        delete socket;
    }
}

void PhoneServerVoIPService::closed()
{
    qLog(Modem) << "PhoneServerVoIPService::closed: connection to voip handler has closed";
    delete _handler;
    _handler = 0;
    emit handlerClosed();
}

void PhoneServerVoIPService::launchHandler()
{
    qLog(Modem) << "PhoneServerVoIPService::launchHandler: sending request to start VoIP handler";
    QtopiaServiceRequest req( "VoIP", "startPhoneHandler()" );
    req.send();
}
