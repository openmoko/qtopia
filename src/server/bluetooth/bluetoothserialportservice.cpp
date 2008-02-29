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

#include "bluetoothserialportservice.h"

#include <qbluetoothabstractservice.h>
#include <qbluetoothlocaldevicemanager.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothrfcommserver.h>
#include <qbluetoothrfcommserialport.h>
#include <qcommdevicesession.h>
#include <qtopiaservices.h>
#include <qtopialog.h>
#include <qvaluespace.h>

#include <QStringList>
#include <QTimer>

class QBluetoothSerialPortServicePrivate
{
public:
    QBluetoothSerialPortServicePrivate()
    {
        server = new QBluetoothRfcommServer();
        modemEmulatorVS = new QValueSpaceItem( QLatin1String("/Communications/ModemEmulator/serialPorts") );
        m_session = 0;
        securityOptions = 0;
    }

    ~QBluetoothSerialPortServicePrivate()
    {
        delete server;
        delete modemEmulatorVS;
        if ( m_session )
            delete m_session;
    }

    void cleanActiveConnections()
    {
        while( !pendingConnections.isEmpty() ) {
            delete pendingConnections.takeFirst();
        }
        while( !runningConnections.isEmpty() ) {
            delete runningConnections.takeFirst();
        }

        if ( m_session ) {
            m_session->endSession();
            delete m_session;
            m_session = 0;
        }
    }

    QBluetoothRfcommServer* server;
    quint32 sdpRecordHandle;
    QCommDeviceSession *m_session;
    QBluetooth::SecurityOptions securityOptions;

    QList<QBluetoothRfcommSerialPort*> pendingConnections;
    QList<QBluetoothRfcommSerialPort*> runningConnections;

    QValueSpaceItem* modemEmulatorVS;
    QString sdpRecordFileName;
    int rfcommChannel;
};

/*!
    \class QBluetoothSerialPortService
    \brief
    The QBluetoothSerialPortService class is a Bluetooth service implementation
    for the Bluetooth Serial Port Profile.
    \ingroup QtopiaServer::Task::Bluetooth

    The Bluetooth Serial Port Profile emulates a serial port over a Bluetooth
    connection. This is used on mobile phones to give complete raw access to
    the modem device. For instance, it can be used to dial and receive calls or
    to send an SMS.

    This profile is the prerequisite for a few other Bluetooth services such
    as the Dial-up Networking or Fax profile, too.

    An incoming Bluetooth connection is automatically forwarded to the modem
    emulator.
  
    This class is part of the Qtopia server and cannot be used by other QtopiaApplications.
*/

/*!
    Creates a Bluetooth serial port service and registers the service using
    \a sdpRecordFileName. This is frequently used when the Serial Port Profile
    is a prerequisite for another Bluetooth profile that needs access to the
    modem device, such as the Dial-up Networking Profile:

    \code
    // Use a file that contains the SDP record data in XML format for creating
    // a Dial-Up Networking service.
    QString fileName = "dun.xml";
    int channel = 2;    // the channel stored in the XML file

    QBluetoothSerialPortService* srv = new QBluetoothSerialPortService( "DialupNetworking",
            tr("Dial-up Networking Service"), fileName, rfcommChannel );
    \endcode

    The \a serviceID is the unique identifier for this Bluetooth service. The
    user visible (translatable) name for this service is \a serviceName.
    The \a rfcommChannel is the channel on which the service will be published.
    \a parent is the standard parent parameter defined by QObject.

    A new service is registered in the Qtopia Bluetooth framework when the
    constructor is called.
*/
QBluetoothSerialPortService::QBluetoothSerialPortService( const QString& serviceID, const QString& serviceName, const QString &sdpRecordFileName, int rfcommChannel, QObject* parent )
    : QBluetoothAbstractService( serviceID, serviceName, parent )
{
    d = new QBluetoothSerialPortServicePrivate();
    d->sdpRecordFileName = sdpRecordFileName;
    d->rfcommChannel = rfcommChannel;

    connect( d->server, SIGNAL(newConnection()), this, SLOT(newConnection()) );
    connect( d->modemEmulatorVS, SIGNAL(contentsChanged()), this, SLOT(emulatorStateChanged()) );
}

/*!
  Destructs this object.
*/
QBluetoothSerialPortService::~QBluetoothSerialPortService()
{

    delete d;
}

/*!
  \reimp
*/
void QBluetoothSerialPortService::start()
{
    if ( d->server->isListening() ) {
        emit started( true, tr("%1 already running").arg(displayName()) );
        return;
    }

    d->cleanActiveConnections();

    d->sdpRecordHandle = registerRecord( d->sdpRecordFileName );
    if (  d->sdpRecordHandle == 0 ) {
        emit started( true, tr( "Error registering with SDP server" ) );
        return;
    }

    if ( !d->server->listen( QBluetoothAddress::any, d->rfcommChannel) ) {
        unregisterRecord( d->sdpRecordHandle );
        emit started(true, tr("Cannot listen for incoming connections.") );
        return;
    }

    //TODO
    //d->server->setSecurityOptions( d->securityOptions );

    emit started(false, QLatin1String(""));
    qLog(Bluetooth) << displayName() << "started on channel" << d->rfcommChannel;
}

/*!
  \reimp
*/
void QBluetoothSerialPortService::stop()
{
    if ( d->server->isListening() )
        d->server->close();
    d->cleanActiveConnections();

    if ( !unregisterRecord( d->sdpRecordHandle ) ) {
        qLog(Bluetooth) << "Error unregistering from SDP server:" << name();
    }

    emit stopped();
    qLog(Bluetooth) << displayName() << "stopped";
}


/*!
  \reimp
*/
void QBluetoothSerialPortService::setSecurityOptions( QBluetooth::SecurityOptions options )
{
    d->securityOptions = options;
    if ( d->server->isListening() )
        d->server->setSecurityOptions( options );
}


/*!
  \internal
*/
void QBluetoothSerialPortService::newConnection()
{
    qLog(Bluetooth) << "Incoming Bluetooth connection for" << displayName();
    if ( !d->server->hasPendingConnections() )
        return;

    while ( d->server->hasPendingConnections() ) {
        QBluetoothRfcommSocket* s =
                static_cast<QBluetoothRfcommSocket *>(d->server->nextPendingConnection());
        QBluetoothRfcommSerialPort* port = new QBluetoothRfcommSerialPort( s, 0, this );
        QString dev = port->device();
        if ( !dev.isEmpty() ) {
            d->pendingConnections.append ( port );
        } else {
            qLog(Bluetooth) << displayName() << ": Cannot create rfcomm device. Ignoring incoming connection.";
            delete port;
        }
        s->close();
        delete s;
    }

    QTimer::singleShot( 100, this, SLOT(initiateModemEmulator()) );
}

/*!
  \internal
*/
void QBluetoothSerialPortService::initiateModemEmulator()
{
    while ( !d->pendingConnections.isEmpty() ) {
        QBluetoothRfcommSerialPort* port = d->pendingConnections.takeFirst();
         // Send a message to the modem emulator to add the serial port.
        QtopiaServiceRequest req( "ModemEmulator", "addSerialPort(QString)" );
        req << port->device();
        req.send();

        d->runningConnections.append( port );

        if (d->runningConnections.size() == 1) {
            if (!d->m_session) {
                QBluetoothLocalDevice local;
                d->m_session = new QCommDeviceSession(local.deviceName().toLatin1(), this);
            }

            d->m_session->startSession();
        }
    }
}

/*!
  \internal
*/
void QBluetoothSerialPortService::emulatorStateChanged()
{
    QStringList serialPorts = d->modemEmulatorVS->value().toStringList();
    QMutableListIterator<QBluetoothRfcommSerialPort*> iter(d->runningConnections);
    QBluetoothRfcommSerialPort* port = 0;
    while ( iter.hasNext() ) {
        port = iter.next();
        QString devName = port->device();
        if ( devName.isEmpty() || !serialPorts.contains(devName) ) {
            qLog(Bluetooth) << displayName() << ": session on" << devName  <<" terminated";
            iter.remove();
            delete port;
        }
    }

    if (d->runningConnections.size() == 0) {
        if (d->m_session)
            d->m_session->endSession();
    }
}
