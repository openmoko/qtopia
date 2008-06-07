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

#include "phoneserver.h"
#include <qtopiaservices.h>
#include <qtopiaipcenvelope.h>
#ifdef QTOPIA_MODEM
#include "phoneserverdummymodem.h"
#include <qmodemservice.h>
#endif
#include <QValueSpaceObject>

/*!
    \class PhoneServer
    \brief The PhoneServer class represents the central dispatch server for phone requests.

    \ingroup QtopiaServer

    Typical phone hardware only allows one process to access the
    AT command stream at any one time.  Because of this, Qtopia Phone
    multiplexes multiple process' requests through the phone server,
    which is the only process that may access the actual hardware.

    The Qtopia phone server is responsible for starting all telephony
    services, including those for GSM, VoIP, and other network types.

    At start up, the Qtopia phone server sends a \c{start()} message to
    all applications that are registered as implementing the
    \l{TelephonyService}{Telephony} service.  This is the usual method
    for starting VoIP and third-party telephony services.

    If Qtopia is configured with the \c{QTOPIA_MODEM} flag, it will also
    start the default built-in AT command handler for the \c modem service using
    QModemService::createVendorSpecific().  If Qtopia is not configured
    with this flag, then the \c modem service is either not required, or will
    be provided by a third-party telephony service implementation.

    This class is part of the Qtopia server and cannot be used by other Qtopia applications.
    \sa QTelephonyService, QModemService
*/

/*!
    \internal
    Returns the number of telephony services available.
*/
static bool executeTelephony( const QString& message )
{
    QStringList channels = QtopiaService::channels( "Telephony" );  // No tr
    int count = 0;
    foreach ( QString channel, channels ) {
        QtopiaIpcEnvelope e( channel, message );
        count++;
    }
    return count;
}

/*!
    Constructs a new PhoneServer attached to \a parent.
*/
PhoneServer::PhoneServer( QObject* parent )
    : QObject(parent)
{
    // Launch the third-party telephony agents.
    int serviceCount = executeTelephony( "Telephony::start()" );       // No tr

    status = new QValueSpaceObject("/Telephony", this);

    // Create the AT-based modem service.  If QTOPIA_PHONE_DUMMY is set,
    // we create a dummy handler for testing purposes.
#ifdef QTOPIA_MODEM
#ifdef QT_ILLUME_LAUNCHER
    QTelephonyService *service = QModemService::createVendorSpecific
            ( "modem", QString(), this );
#else
    char *env = getenv( "QTOPIA_PHONE_DUMMY" );
    QTelephonyService *service;
    if ( env && *env == '1' ) {
        service = new QTelephonyServiceDummy( "modem", this );
    } else {
        service = QModemService::createVendorSpecific
            ( "modem", QString(), this );
    }
#endif
    service->initialize();
    serviceCount++;
#endif

    status->setAttribute("AvailableServiceCount", serviceCount);
}

/*!
    Destructs the PhoneServer.
*/
PhoneServer::~PhoneServer()
{
    // Shut down the third-party telephony agents.
    executeTelephony( "Telephony::stop()" );        // No tr
    status->removeAttribute("AvailableServiceCount");
}

QTOPIA_TASK(PhoneServer, PhoneServer);
QTOPIA_TASK_PROVIDES(PhoneServer, PhoneServer);
