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

#include "phoneserver.h"
#include <qtopiaservices.h>
#include <qtopiaipcenvelope.h>
#ifdef QTOPIA_MODEM
#include "phoneserverdummymodem.h"
#include <qmodemservice.h>
#endif
#ifdef QTOPIA_VOIP
#include "phoneservervoipsocket.h"
#endif

/*!
    \class PhoneServer
    \brief The PhoneServer class represents the central dispatch server for phone requests.

    \ingroup QtopiaServer

    Typical phone hardware only allows one process to access the
    AT command stream at any one time.  Because of this, Qtopia Phone
    multiplexes multiple process' requests through the phone server,
    which is the only process that may access the actual hardware.
*/

static void executeTelephony( const QString& message )
{
    QStringList channels = QtopiaService::channels( "Telephony" );  // No tr
    foreach ( QString channel, channels ) {
        QtopiaIpcEnvelope e( channel, message );
    }
}

/*!
    Constructs a new PhoneServer attached to \a parent.
*/
PhoneServer::PhoneServer( QObject* parent )
    : QObject(parent)
{
    // Launch the third-party telephony agents.
    executeTelephony( "Telephony::start()" );       // No tr

    // Create the AT-based modem service.  If QTOPIA_PHONE_DUMMY is set,
    // we create a dummy handler for testing purposes.
#ifdef QTOPIA_MODEM
    char *env = getenv( "QTOPIA_PHONE_DUMMY" );
    QTelephonyService *service;
    if ( env && *env == '1' ) {
        service = new QTelephonyServiceDummy( "modem", this );
    } else {
        service = QModemService::createVendorSpecific
            ( "modem", QString(), this );
    }
    service->initialize();
#endif

#ifdef QTOPIA_VOIP
    // Create the socket-based voip handler if there is one in the system.
    // We don't need this if the voip system is a real telephony agent
    // based on QTelephonyService.  In that case, it will be started by
    // the call to executeTelephony() above.
    if ( PhoneServerVoIPService::handlerPresent() ) {
        PhoneServerVoIPService *service
            = new PhoneServerVoIPService( "voip", this );
        service->initialize();
    }
#endif
}

/*!
    Destructs the PhoneServer.
*/
PhoneServer::~PhoneServer()
{
    // Shut down the third-party telephony agents.
    executeTelephony( "Telephony::stop()" );        // No tr
}

QTOPIA_TASK(PhoneServer, PhoneServer);
QTOPIA_TASK_PROVIDES(PhoneServer, PhoneServer);
