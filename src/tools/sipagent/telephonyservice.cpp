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

#include "telephonyservice.h"
#include "siptelephonyservice.h"
#include "siplayer.h"
#include "sipagentmedia.h"
#include <qtopiaapplication.h>

TelephonyService::TelephonyService( QObject *parent )
    : QtopiaAbstractService( "Telephony", parent )
{
    publishAll();
    service = 0;
    layer = 0;
}

TelephonyService::~TelephonyService()
{
}

void TelephonyService::start()
{
    if ( !service ) {
        // Start up the sip-based VoIP telephony service.
        ((QtopiaApplication *)qApp)->registerRunningTask
            ( "SipTelephonyService", this );

        // Start up the MediaSession service implementation.  This is
        // the default implementation, but may be replaced with a better
        // implementation by other applications by re-configuring the
        // MediaSession service within the "services" directory.
        media = new SipAgentMedia( this );

        // Create the telephony service.
        layer = new SipLayer( this );
        service = new SipTelephonyService( "voip", layer, this ); // No tr
        service->initialize();
    }
}

void TelephonyService::stop()
{
    if ( service ) {
        // Shut down the sip-based VoIP telephony service.
        delete service;
        delete layer;
        delete media;
        service = 0;
        layer = 0;
        media = 0;
        ((QtopiaApplication *)qApp)->unregisterRunningTask
            ( "SipTelephonyService" );
    }
}
