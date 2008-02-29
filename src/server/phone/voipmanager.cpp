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

#include "voipmanager.h"
#include <qvaluespace.h>

#include <QSqlQuery>

class VoIPManagerPrivate {
public:
    VoIPManagerPrivate()
    : netReg(0),
    serviceManager(0),
    status(0) {}

    QNetworkRegistration *netReg;
    QPresence *presence;
    QCommServiceManager *serviceManager;
    QValueSpaceObject *status;
};

/*!
    \class VoIPManager
    \ingroup QtopiaServer
    \brief The VoIPManager class maintains information about the active VoIP telephony service.

    This class provides access to some of the common facilities of the \c{voip} telephony
    service, if it is running within the system, to ease the implementation of VoIP-specific
    user interfaces in the server.  The telephony service itself is started by PhoneServer
    at system start up.

    \sa PhoneServer
*/

/*!
    Returns the VoIPManager instance.
*/
VoIPManager * VoIPManager::instance()
{
    static VoIPManager *manager = 0;
    if(!manager)
        manager = new VoIPManager;
    return manager;
}

/*!
    Returns the current VoIP registration state.

    \sa registrationChanged()
*/
QTelephony::RegistrationState VoIPManager::registrationState() const
{
    if ( d->netReg )
        return d->netReg->registrationState();
    else
        return QTelephony::RegistrationNone;
}

/*!
    \fn void VoIPManager::registrationChanged(QTelephony::RegistrationState state);

    Signal that is emitted when registrationState() changes to \a state.

    \sa registrationState()
*/

/*!
    Returns the presence status of the local user.

    \sa localPresenceChanged()
*/
QPresence::Status VoIPManager::localPresence() const
{
    if ( d->presence )
        return d->presence->localPresence();
    else
        return QPresence::Unavailable;
}

/*!
    \fn void VoIPManager::localPresenceChanged(QPresence::Status status)

    Signal that is emitted when localPresence() changes to \a status.

    \sa localPresence()
*/

/*!
    Returns true if the VoIP user associated with \a uri is available for calling;
    false otherwise.

    \sa monitoredPresenceChanged(), startMonitoring()
*/
bool VoIPManager::isAvailable( const QString &uri )
{
    return d->presence->monitoredUriStatus( uri ) == QPresence::Available;
}

/*!
    \fn void VoIPManager::monitoredPresenceChanged(const QString& uri, bool available)

    Signal that is emitted when the presence of the user identified by \a uri
    changes to \a available.

    \sa isAvailable(), startMonitoring()
*/

VoIPManager::VoIPManager()
{
    d = new VoIPManagerPrivate;
    d->status = new QValueSpaceObject("/Telephony/Status/VoIP", this);
#ifdef QTOPIA_VOIP
    // The "voip" telephony handler may not have started yet.
    // Hook onto QCommServiceManager to watch for it.
    d->serviceManager = new QCommServiceManager( this );
    connect( d->serviceManager, SIGNAL(servicesChanged()),
             this, SLOT(servicesChanged()) );

    // Just in case it has already started.
    servicesChanged();
#endif
}

void VoIPManager::registrationStateChanged()
{
    if ( d->netReg ) {
        emit registrationChanged( d->netReg->registrationState() );
        d->status->setAttribute("Registered", d->netReg->registrationState() == QTelephony::RegistrationHome);
    } else {
        emit registrationChanged( QTelephony::RegistrationNone );
        d->status->setAttribute("Registered", false);
    }
}

void VoIPManager::localPresenceChanged()
{
    if ( d->presence ) {
        emit localPresenceChanged( d->presence->localPresence() );
        d->status->setAttribute("Presence", (d->presence->localPresence() == QPresence::Available)?"Available":"Unavailable");
        d->status->setAttribute("Present", d->presence->localPresence() == QPresence::Available);
    } else {
        emit localPresenceChanged( QPresence::Unavailable );
        d->status->setAttribute("Presence", "Unavailable");
        d->status->setAttribute("Present", false);
    }
}

void VoIPManager::monitoredPresence( const QString &uri, QPresence::Status status )
{
    emit monitoredPresenceChanged( uri, status == QPresence::Available );
}

void VoIPManager::servicesChanged()
{
#ifdef QTOPIA_VOIP
    if ( !d->netReg ) {
        if ( d->serviceManager->interfaces( "voip" )       // No tr
                    .contains( "QNetworkRegistration" ) ) {
            serviceStarted();
        }
    } else {
        if ( !d->serviceManager->interfaces( "voip" )      // No tr
                    .contains( "QNetworkRegistration" ) ) {
            serviceStopped();
        }
    }
#endif
}

void VoIPManager::serviceStarted()
{
#ifdef QTOPIA_VOIP
    // The "voip" handler has started up, so attach to the service.
    d->netReg = new QNetworkRegistration( "voip", this ); // No tr
    if ( !d->netReg->available() ) {
        delete d->netReg;
        d->netReg = 0;
    } else {
        connect( d->netReg, SIGNAL(registrationStateChanged()),
                 this, SLOT(registrationStateChanged()) );
        registrationStateChanged();
    }
    d->presence = new QPresence( "voip", this );
    if ( !d->presence->available() ) {
        delete d->presence;
        d->presence = 0;
    } else {
        connect( d->presence, SIGNAL(localPresenceChanged()),
                this, SLOT(localPresenceChanged()) );
        connect( d->presence, SIGNAL(monitoredPresence(const QString&,QPresence::Status)),
                this, SLOT(monitoredPresence(const QString&,QPresence::Status)) );
    }
    localPresenceChanged();
#endif
}

void VoIPManager::serviceStopped()
{
#ifdef QTOPIA_VOIP
    // The "voip" handler has shut down, so detach from the service.
    delete d->netReg;
    d->netReg = 0;
    registrationStateChanged();
    delete d->presence;
    d->presence = 0;
    localPresenceChanged();
#endif
}

/*!
    Start monitoring all contacts in the user's contact list that have
    associated VoIP identities.  The monitoredPresenceChanged() signal
    will be emitted whenever the presence information for a monitored
    contact changes.

    \sa isAvailable(), monitoredPresenceChanged()
*/
void VoIPManager::startMonitoring()
{
#ifdef QTOPIA_VOIP
    QSqlQuery q;
    q.prepare("SELECT fieldvalue FROM contactcustom WHERE fieldname = :fn");
    q.bindValue(":fn", "VOIP_ID");
    q.exec();
    while(q.next())
        d->presence->startMonitoring( q.value(0).toString() );
#endif
}

