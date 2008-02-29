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

#include "externalaccess.h"
#include <qcommservicemanager.h>
#include <qnetworkregistration.h>
#include <qtopiaservices.h>
#include <qtopialog.h>
#include <QSettings>
#include <QTimer>

/*!
    \class ExternalAccess
    \brief The ExternalAccess class starts the modem emulator on a serial port to allow external access via a serial cable.
    \ingroup QtopiaServer::Task

    Many modern phones have an external serial interface, which is used to
    connect to a computer for the purposes of data calls, FAX transmissions,
    or synchronization with desktop applications.  The \l{Modem Emulator}
    assists with supporting such interfaces.

    To enable the modem emulator to use an external serial interface,
    the \c Phone.conf file should be placed under the \c etc/default/Trolltech
    directory on the vendor system, with contents such as the following:

    \code
    [SerialDevices]
    ExternalAccessDevice=/dev/ttyS1:115200
    \endcode

    where \c{/dev/ttyS1} should be replaced with the name of the serial device
    to listen to, and \c{115200} should be replaced with the desired baud rate.

    If this option is configured, the modem emulator will be launched
    automatically once the phone server has been initialized.  If this
    option is not configured, then the modem emulator will only be launched
    when other components in the system request it (for example,
    Bluetooth hands-free kits).

    \sa {Modem Emulator}
*/

/*!
    Create the external access task and attach it to \a parent.
*/
ExternalAccess::ExternalAccess( QObject *parent )
    : QObject( parent )
{
    started = false;

    // Track when the modem service comes up.
    manager = new QCommServiceManager( this );
    connect( manager, SIGNAL(servicesChanged()),
             this, SLOT(servicesChanged()) );

    // If we get 10 seconds in, then start it anyway.
    QTimer::singleShot( 10000, this, SLOT(start()) );

    // Do an initial service check just in case the services are
    // already alive at this point in system startup.
    servicesChanged();
}

/*!
    Destroy the external access task.
*/
ExternalAccess::~ExternalAccess()
{
}

void ExternalAccess::servicesChanged()
{
    // If already started, then ignore this.
    if ( started )
        return;

    // Start external access immediately if the modem has come up, as it is
    // the most important service for the modem emulator to use.
    if ( manager->services().contains( "modem" ) )  // No tr
        start();

#ifndef QTOPIA_MODEM
    // If we don't have a modem, then start it when we get any
    // telephony service (usually it is the "voip" service).
    if ( manager->supports<QNetworkRegistration>().size() > 0 )
        start();
#endif
}

void ExternalAccess::start()
{
    // We only need to do this once.
    if ( started )
        return;
    started = true;

    // We won't need the service manager any more, so delete it.
    // This will stop the service signals, making the system more efficient.
    if ( manager ) {
        delete manager;
        manager = 0;
    }

    // Look for the external access device and pass it to the modem emulator.
    QString device;
    QSettings config( "Trolltech", "Phone" );       // No tr
    config.beginGroup( "SerialDevices" );           // No tr
    device = config.value
        ( "ExternalAccessDevice", QString() ).toString(); // No tr
    if ( !device.isEmpty() ) {
        // There is an external access device specified, so start it up.
        qLog(Modem) << "Starting modem emulator on" << device;
        QtopiaServiceRequest req
            ( "ModemEmulator", "addSerialPort(QString)" );  // No tr
        req << device;
        req.send();
    }
}

QTOPIA_TASK(ExternalAccess, ExternalAccess);
QTOPIA_TASK_PROVIDES(ExternalAccess, ExternalAccess);
