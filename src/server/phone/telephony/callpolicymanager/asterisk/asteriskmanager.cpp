/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#include "asteriskmanager.h"
#include <qvaluespace.h>
#include <qservicechecker.h>

class AsteriskManagerPrivate {
public:
    AsteriskManagerPrivate()
    : netReg(0),
    serviceManager(0),
    status(0) {}

    QNetworkRegistration *netReg;
    QCommServiceManager *serviceManager;
    QValueSpaceObject *status;
};

/*!
    \class AsteriskManager
    \ingroup QtopiaServer
    \brief The AsteriskManager class maintains information about the active Asterisk telephony service.

    This class provides access to some of the common facilities of the \c{asterisk} telephony
    service, if it is running within the system, to ease the implementation of Asterisk-specific
    user interfaces in the server.  The telephony service itself is started by PhoneServer
    at system start up.
    
    This class is part of the Qtopia server and cannot be used by other Qtopia applications.
    \sa PhoneServer
*/

/*!
    Create a new Asterisk telephony service manager and attach it to \a parent.
*/
AsteriskManager::AsteriskManager( QObject *parent )
    : QAbstractCallPolicyManager( parent )
{
    d = new AsteriskManagerPrivate;
    d->status = new QValueSpaceObject("/Telephony/Status/Asterisk", this);
#ifdef QTOPIA_VOIP
    // The "asterisk" telephony handler may not have started yet.
    // Hook onto QCommServiceManager to watch for it.
    d->serviceManager = new QCommServiceManager( this );
    connect( d->serviceManager, SIGNAL(servicesChanged()),
             this, SLOT(servicesChanged()) );

    // Just in case it has already started.
    servicesChanged();
#endif
}

/*!
    Destroys this Asterisk telephony service manager.
*/
AsteriskManager::~AsteriskManager()
{
}

/*!
    \reimp
*/
QString AsteriskManager::callType() const
{
    return "Asterisk";      // No tr
}

/*!
    \reimp
*/
QString AsteriskManager::trCallType() const
{
    return tr("Asterisk");
}

/*!
    \reimp
*/
QString AsteriskManager::callTypeIcon() const
{
    return "iaxsettings/Asterisk";  // No tr
}

/*!
    \reimp
*/
QTelephony::RegistrationState AsteriskManager::registrationState() const
{
    if ( d->netReg )
        return d->netReg->registrationState();
    else
        return QTelephony::RegistrationNone;
}

/*!
    \reimp
*/
QAbstractCallPolicyManager::CallHandling AsteriskManager::handling
        (const QString& number, QString &error)
{
    // Cannot handle URI's that contain '@' or ':'.
    if (number.contains(QChar('@')) || number.contains(QChar(':')))
        return CannotHandle;

    // If no network registration, then cannot handle at this time.
    if (registrationState() != QTelephony::RegistrationHome) {
        error += tr("Asterisk is not registered to the home network (%1).<br>").arg(registrationState());
        return CannotHandle;
    }

    // Assume that this is a number that we can dial.
    return CanHandle;
}

/*!
    \reimp
*/
bool AsteriskManager::isAvailable(const QString& number)
{
    // No way to check presence, so assume that it is always ok.
    Q_UNUSED(number);
    return true;
}

/*!
    \reimp
*/
QString AsteriskManager::registrationMessage() const
{
    // Don't display anything if we do not have an Asterisk service active yet.
    if ( !d->netReg )
        return QString();

    // Determine if the Asterisk service is configured.
    // If it isn't, then there is no point talking about it.
    QServiceChecker checker( "asterisk" );      // No tr.
    if ( !checker.isValid() )
        return QString();

    QString msg;
    QTelephony::RegistrationState v = registrationState();

    switch (v) {
    case QTelephony::RegistrationNone:
        msg = tr("No Asterisk network");
        break;
    case QTelephony::RegistrationHome:
    case QTelephony::RegistrationUnknown:
    case QTelephony::RegistrationRoaming:
        break;
    case QTelephony::RegistrationSearching:
        msg = tr("Searching for Asterisk network");
        break;
    case QTelephony::RegistrationDenied:
        msg = tr("Asterisk Authentication Failed");
        break;
    }

    return msg;
}

/*!
    \reimp
*/
QString AsteriskManager::registrationIcon() const
{
    // Icons not used at present for Asterisk.
    return QString();
}

void AsteriskManager::registrationStateChanged()
{
    if ( d->netReg ) {
        emit registrationChanged( d->netReg->registrationState() );
        d->status->setAttribute("Registered", d->netReg->registrationState() == QTelephony::RegistrationHome);
    } else {
        emit registrationChanged( QTelephony::RegistrationNone );
        d->status->setAttribute("Registered", false);
    }
}

void AsteriskManager::servicesChanged()
{
    if ( !d->netReg ) {
        if ( d->serviceManager->interfaces( "asterisk" )       // No tr
                    .contains( "QNetworkRegistration" ) ) {
            serviceStarted();
        }
    } else {
        if ( !d->serviceManager->interfaces( "asterisk" )      // No tr
                    .contains( "QNetworkRegistration" ) ) {
            serviceStopped();
        }
    }
}

void AsteriskManager::serviceStarted()
{
    // The "asterisk" handler has started up, so attach to the service.
    d->netReg = new QNetworkRegistration( "asterisk", this ); // No tr
    if ( !d->netReg->available() ) {
        delete d->netReg;
        d->netReg = 0;
    } else {
        connect( d->netReg, SIGNAL(registrationStateChanged()),
                 this, SLOT(registrationStateChanged()) );
        registrationStateChanged();
    }
}

void AsteriskManager::serviceStopped()
{
    // The "asterisk" handler has shut down, so detach from the service.
    delete d->netReg;
    d->netReg = 0;
    registrationStateChanged();
}

QTOPIA_TASK(Asterisk, AsteriskManager);
QTOPIA_TASK_PROVIDES(Asterisk, AsteriskManager);
QTOPIA_TASK_PROVIDES(Asterisk, QAbstractCallPolicyManager);
