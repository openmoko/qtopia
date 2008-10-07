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

#include <qmodemgprsnetworkregistration.h>
#include <qmodemservice.h>
#include <qnetworkregistration.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <qatutils.h>
#include <qtopialog.h>
#include <qtimer.h>

/*!
    \class QModemGprsNetworkRegistration
    \mainclass
    \brief The QModemGprsNetworkRegistration class provides information about GPRS network registration on AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CGREG} command from 3GPP TS 27.007.

    QModemGprsNetworkRegistration implements the QGprsNetworkRegistration telephony
    interface.  Client applications should use QGprsNetworkRegistration instead
    of this class to access the modem's GPRS network registration state.

    \sa QGprsNetworkRegistration
*/

class QModemGprsNetworkRegistrationPrivate
{
public:
    QModemService *service;
    QTimer *cgregTimer;
};

/*!
    Create an AT-based GPRS network registration handler for \a service.
*/
QModemGprsNetworkRegistration::QModemGprsNetworkRegistration
        ( QModemService *service )
    : QGprsNetworkRegistrationServer( service->service(), service )
{
    d = new QModemGprsNetworkRegistrationPrivate();
    d->service = service;

    service->primaryAtChat()->registerNotificationType
        ( "+CGREG:", this, SLOT(cgregNotify(QString)), true );
    connect( service, SIGNAL(resetModem()), this, SLOT(resetModem()) );

    // Set up the one-shot timer that can be used to filter out outages
    // (i.e. the "bouncy rubber calypso" problem with the Openmoko GTA01/GTA02)
    d->cgregTimer = new QTimer( this );
    d->cgregTimer->setSingleShot( true );
    connect ( d->cgregTimer, SIGNAL(timeout ()), this, SLOT(cgregTimeOut()) );
}

/*!
    Destroy this AT-based GPRS network registration handler.
*/
QModemGprsNetworkRegistration::~QModemGprsNetworkRegistration()
{
    delete d;
}

void QModemGprsNetworkRegistration::resetModem()
{
    // Turn on unsolicited AT+CGREG notifications.
    d->service->primaryAtChat()->chat( "AT+CGREG=2" );

    // Query the current state.
    d->service->primaryAtChat()->chat
        ( "AT+CGREG?", this, SLOT(cgregQuery(bool,QAtResult)) );
}

void QModemGprsNetworkRegistration::cgregQuery( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    if ( parser.next( "+CGREG:" ) ) {
        parser.readNumeric();  // Skip unsolicited result flag which is unused.
        uint stat = parser.readNumeric();
        QString lac = parser.readString();
        QString ci = parser.readString();
        if ( !lac.isEmpty() && !ci.isEmpty() ) {
            // We have location information after the state value.
            updateRegistrationState( (QTelephony::RegistrationState)stat,
                                     lac.toInt( 0, 16 ), ci.toInt( 0, 16 ) );
        } else {
            // We don't have any location information.
            updateRegistrationState( (QTelephony::RegistrationState)stat );
        }
    }
}

void QModemGprsNetworkRegistration::cgregNotify( const QString& msg )
{
    uint posn = 7;
    uint stat = QAtUtils::parseNumber( msg, posn );
    QString lac = QAtUtils::nextString( msg, posn );
    QString ci = QAtUtils::nextString( msg, posn );

    // Don't be too hasty to call out a loss of network registration.
    // Instead, start a one-shot timer -- and cancel the timer if
    // if we get a positive registration before it expires.
    if ( stat == 0 ) {
        if ( !d->cgregTimer->isActive() ) {
            d->cgregTimer->start( 10000 );  // 10 second one-shot timer
            qLog(Modem) << "Started GPRS Loss-of-Registration timer.";
       }
     } else {
        if ( d->cgregTimer->isActive() ) {
            d->cgregTimer->stop();
            qLog(Modem) << "Cancelled GPRS Loss-of-Registration timer.";
       }
        // If we have a positive registration here, go ahead and update
        // the registration status.
        if ( stat != 0 ) {
            if ( !lac.isEmpty() && !ci.isEmpty() ) {
                // We have location information after the state value.
                updateRegistrationState( (QTelephony::RegistrationState)stat,
                                          lac.toInt( 0, 16 ), ci.toInt( 0, 16 ) );
            } else {
                // We don't have any location information.
                updateRegistrationState( (QTelephony::RegistrationState)stat );
            }
        }
     }
}

void QModemGprsNetworkRegistration::cgregTimeOut()
{
    // Timeout on a de-register event; must have been legitimate; report it now.
    qLog(Modem) << "GPRS Loss-of-Registration timer expired; reporting.";
    updateRegistrationState( (QTelephony::RegistrationState)0 );
}
