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

#include "atinterface.h"
#include "atsessionmanager.h"
#include "atfrontend.h"
#include "atcommands.h"
#include <qapplication.h>
#include <qtopialog.h>

AtInterface::AtInterface
        ( bool testMode, const QString& startupOptions, QObject *parent )
    : QObject( parent )
{
    qLog(ModemEmulator) << "atinterface starting up";

    manager = new AtSessionManager( this );
    connect( manager, SIGNAL(newSession(AtFrontEnd*)),
             this, SLOT(newSession(AtFrontEnd*)) );

    // Bind to port 12350 if we are simply testing the AT interface.
    if ( testMode ) {
        if ( !manager->addTcpPort( 12350, true, startupOptions ) ) {
            qWarning( "Could not bind to AT interface test port" );
            QApplication::quit();
            return;
        }
    }
}

AtInterface::~AtInterface()
{
    qLog(ModemEmulator) << "atinterface shutting down";
}

void AtInterface::newSession( AtFrontEnd *session )
{
    // Wrap the session in an AtCommands object.  It will be
    // automatically cleaned up when the session is destroyed.
    new AtCommands( session, manager );
}
