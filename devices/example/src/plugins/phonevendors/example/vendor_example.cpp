/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "vendor_example_p.h"
#include <qmodemindicators.h>
#include <qatresultparser.h>
#include <qatutils.h>
#include <qvaluespace.h>
#include <QTimer>

ExampleCallProvider::ExampleCallProvider( QModemService *service )
    : QModemCallProvider( service )
{
}

ExampleCallProvider::~ExampleCallProvider()
{
}

QModemCallProvider::AtdBehavior ExampleCallProvider::atdBehavior() const
{ 
    return AtdOkIsConnect;
}

QString ExampleCallProvider::putOnHoldCommand() const
{
    qWarning("**********ExampleCallProvider::putOnHoldCommand");
    return "AT+CHLD=2";
}

ExampleModemService::ExampleModemService
        ( const QString& service, QSerialIODeviceMultiplexer *mux,
          QObject *parent )
    : QModemService( service, mux, parent )
{
    // We need to do some extra stuff at reset time.
    connect( this, SIGNAL(resetModem()), this, SLOT(reset()) );

    // Register for signal strength notification
    primaryAtChat()->registerNotificationType
        ( "+CSQ:", this, SLOT(signalStrength(QString)), true );

}

ExampleModemService::~ExampleModemService()
{
}

void ExampleModemService::initialize()
{
    if ( !callProvider() )
        setCallProvider( new ExampleCallProvider( this ) );

    QModemService::initialize();
}

void ExampleModemService::reset()
{
    // Make sure that "AT*ECAM" is re-enabled after a reset.
    chat( "AT*ECAM=1" );

    // Turn on unsolicited signal strength indicators.
    chat( "AT*ECIND=2,1,1" );
}

void ExampleModemService::signalStrength( const QString& msg )
{
    int value = msg.mid(12).toInt();
    indicators()->setSignalQuality( value, 5 );
    qWarning("**********ExampleModemService::signalStrength()");
}
