/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "btdialupservice.h"

#include <QTimer>

#include <qbluetoothabstractservice.h>
#include <qbluetoothlocaldevicemanager.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothrfcommserver.h>
#include <qbluetoothrfcommserialport.h>
#include <qbluetoothremotedevice.h>
#include <qcommdevicesession.h>
#include <qtopiaservices.h>
#include <qtopialog.h>
#include <qvaluespace.h>

#include <qcommservicemanager.h>
#include <qnetworkregistration.h>

class BtDialupServiceProvider : public QBluetoothAbstractService
{
    Q_OBJECT
public:
    BtDialupServiceProvider( BtDialupService* parent );

    void start(int channel);
    void delayedStart();
    void stop();
    void setSecurityOptions( QBluetooth::SecurityOptions options );
    void allowPendingConnection( bool isAllowed );

    QString translatableDisplayName() const;

protected:
    void timerEvent( QTimerEvent* event );

private slots:
    void newConnection();
    void initiateModemEmulator();
    void emulatorStateChanged();
    void registrationChanged();
    void delayedShutdown();

private:
    void cleanActiveConnections();
    void shutdownService();

private:
    BtDialupService* m_parent;
    QBluetoothRfcommServer* server;
    QBluetoothLocalDevice* localDev;

    QList<QBluetoothRfcommSerialPort*> pendingConnections;
    QList<QBluetoothRfcommSerialPort*> runningConnections;

    QValueSpaceItem* modemEmulatorVS;
    QCommServiceManager* commManager;
    QNetworkRegistration* netReg;
    int channel;
    bool serviceRunning;

    int delayedShutdownTimerId;

    QCommDeviceSession *m_session;
};

BtDialupServiceProvider::BtDialupServiceProvider( BtDialupService* parent )
    : QBluetoothAbstractService( QLatin1String("DialupNetworking"), parent ),
        localDev( 0 ), commManager( 0 ), netReg( 0 ), channel( -1 ), serviceRunning( false ),
        delayedShutdownTimerId( 0 )
{
    m_parent = parent;
    server = new QBluetoothRfcommServer( this );
    connect( server, SIGNAL(newConnection()), this, SLOT(newConnection()) );
    modemEmulatorVS = new QValueSpaceItem( QLatin1String("/Communications/ModemEmulator/serialPorts"), this );
    connect( modemEmulatorVS, SIGNAL(contentsChanged()), this, SLOT(emulatorStateChanged()) );
    initialize();

    m_session = 0;
}

void BtDialupServiceProvider::timerEvent( QTimerEvent* event )
{
    if ( event->timerId() == delayedShutdownTimerId ) {
        if ( delayedShutdownTimerId )
            killTimer( delayedShutdownTimerId );
        delayedShutdownTimerId = 0;
        if ( !netReg )
            shutdownService();
        else if ( netReg->registrationState() != QTelephony::RegistrationHome
                    && netReg->registrationState() != QTelephony::RegistrationRoaming )
            shutdownService();
        else
            qLog(Bluetooth) << "Aborting delayed shutdown of DUN due to updated network registration";
   }
}


//Sometimes the network registration becomes available, disappears and becomes available again
//To prevent the stop of the service during such a transitional period we add a small
//grace period to the shutdown process. The service will not abort for another 15s in the hope
//that the registration returns. If the network registration is still insufficient after 15s
//abort the service and announce it to the BluetoothServiceManager.
void BtDialupServiceProvider::delayedShutdown()
{
    if ( delayedShutdownTimerId )  //delayed shutdown already running
        return;

    qLog(Bluetooth) << "Delaying shutdown of DUN due to missing network registration";
    delayedShutdownTimerId = startTimer( 15000 );
}

void BtDialupServiceProvider::start(int ch)
{
    if ( !commManager ) {
        commManager = new QCommServiceManager( this );
        connect( commManager, SIGNAL(servicesChanged()), this, SLOT(registrationChanged()));
    }
    channel = ch;
    registrationChanged(); //first update
}

//ensure that the service isn't available/running while we have an
//invalid network registration
void BtDialupServiceProvider::registrationChanged()
{
    if ( commManager->supports<QNetworkRegistration>().contains(QLatin1String("modem")) ) {
        if ( !netReg ) {
            netReg = new QNetworkRegistration( QLatin1String("modem"), this );
            connect( netReg, SIGNAL(registrationStateChanged()),
                     this, SLOT(registrationChanged()) );
        }
    } else if ( netReg ) {
        //we just lost the modem -> our modem network registration
        //is useless now -> shutdown the service
        qLog(Bluetooth) << "Bluetooth dial-up network service lost network registration";
        delete netReg;
        netReg = 0;
        shutdownService();
        return;
    } else { //no modem network registration service available
        shutdownService();
        return;
    }

    QTelephony::RegistrationState state = netReg->registrationState();
    if ( state == QTelephony::RegistrationHome
            || state == QTelephony::RegistrationRoaming ) {
        if ( channel >= 0 && !serviceRunning ) {
            delayedStart();
        }
        return;
    } else if (serviceRunning) {
        //we dropped out of network registration -> stop service
        delayedShutdown();
    }
}

void BtDialupServiceProvider::delayedStart()
{
    serviceRunning = true;
    QBluetoothLocalDeviceManager manager;
    QBluetoothLocalDevice dev( manager.defaultDevice() );
    if ( !dev.isValid() ) {
        emit started( QBluetooth::NoSuchAdapter,
                tr("Cannot find local Bluetooth device") );
        return;
    }

    cleanActiveConnections();
    if ( server->isListening() )
        server->close();

    if ( !server->listen( dev.address(), channel) ) {
        emit started(QBluetooth::NotRunning, tr("Cannot listen for incoming connections.") );
        return;
    }

    if ( !sdpRegister( dev.address(), QBluetooth::DialupNetworkingProfile,
                channel ) )
    {
        server->close();
        emit started(QBluetooth::SDPServerError,
                tr("Error registering with SDP server") );
        return;
    }
    emit started(QBluetooth::NoError, QLatin1String(""));
    qLog(Bluetooth) << QLatin1String("Dial-up Networking Service started");
}

void BtDialupServiceProvider::stop()
{
    if ( channel >= 0 ) //prevent pending (delayed) start
        channel = -1;

    if ( !serviceRunning )
        return;

    if ( server->isListening() )
        server->close();
    cleanActiveConnections();

    channel = -1;
    serviceRunning = false;

    if ( !sdpUnregister() ) {
        emit stopped( QBluetooth::SDPServerError,
                tr("Error unregistering from SDP server") );
        return;
    }

    emit stopped(QBluetooth::NoError, QString());
    qLog(Bluetooth) << QLatin1String("Dial-up Networking Service stopped");
}

//This function can stop the service at any time by emitting an error signal.
//The stop() function is only called during regular service stops requested by the
//Bluetooth service manager.
//If the service is not running this function does nothing.
void BtDialupServiceProvider::shutdownService()
{
    if ( !serviceRunning )
        return;

    qLog(Bluetooth) << "Shutdown of DUN due to missing network registration";
    if ( server->isListening() )
        server->close();
    cleanActiveConnections();
    serviceRunning = false;
    channel = -1;
    sdpUnregister();
    emit error( QBluetooth::NotRunning, tr("Lost network contact") );
}

void BtDialupServiceProvider::cleanActiveConnections()
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

void BtDialupServiceProvider::setSecurityOptions( QBluetooth::SecurityOptions options )
{
    server->setSecurityOptions( options );
}

void BtDialupServiceProvider::newConnection()
{
    qLog(Bluetooth) << "Incoming Bluetooth Dialup connection";
    if ( !server->hasPendingConnections() )
        return;

    bool refuseConnections = false;
    if ( !netReg
            || (netReg->registrationState() != QTelephony::RegistrationHome
                && netReg->registrationState() != QTelephony::RegistrationRoaming )
       )
        refuseConnections = true; //can happen during delayed shutdown

    while ( server->hasPendingConnections() ) {
        QBluetoothRfcommSocket* s =
                static_cast<QBluetoothRfcommSocket *>(server->nextPendingConnection());
        if ( !refuseConnections ) {
            QBluetoothRfcommSerialPort* port = new QBluetoothRfcommSerialPort( this );
            QString dev = port->createTty( s );
            if ( !dev.isEmpty() ) {
                pendingConnections.append ( port );
            } else {
                qLog(Bluetooth) << "BtDialupService: Cannot create rfcomm device. Ignoring incoming connection.";
                delete port;
            }
        }
        s->close();
        delete s;
    }

    QTimer::singleShot( 100, this, SLOT(initiateModemEmulator()) );
}


QString BtDialupServiceProvider::translatableDisplayName() const
{
    return tr("Dial-up Networking Service");
}

void BtDialupServiceProvider::initiateModemEmulator()
{
    while ( !pendingConnections.isEmpty() ) {
        QBluetoothRfcommSerialPort* port = pendingConnections.takeFirst();
         // Send a message to the modem emulator to add the serial port.
        QtopiaServiceRequest req( "ModemEmulator", "addSerialPort(QString)" );
        req << port->boundDevice();
        req.send();

        runningConnections.append( port );

        if (runningConnections.size() == 1) {
            if (!m_session) {
                QBluetoothLocalDevice local;
                m_session = new QCommDeviceSession(local.deviceName().toLatin1(), this);
            }

            m_session->startSession();
        }
    }
}

void BtDialupServiceProvider::emulatorStateChanged()
{
    QStringList serialPorts = modemEmulatorVS->value().toStringList();
    QMutableListIterator<QBluetoothRfcommSerialPort*> iter(runningConnections);
    QBluetoothRfcommSerialPort* port = 0;
    while ( iter.hasNext() ) {
        port = iter.next();
        QString devName = port->boundDevice();
        if ( devName.isEmpty() || !serialPorts.contains(devName) ) {
            qLog(Bluetooth) << "Bluetooth Dialup session terminated.";
            iter.remove();
            delete port;
        }
    }

    if (runningConnections.size() == 0) {
        if (m_session)
            m_session->endSession();
    }
}

//TODO qdoc
BtDialupService::BtDialupService( QObject* parent )
    : QObject( parent )
{
    qLog(Bluetooth) << "Initializing BtDialupService";
    d = new BtDialupServiceProvider( this );
}

BtDialupService::~BtDialupService()
{
}

QTOPIA_TASK(BtDialupService,BtDialupService);
QTOPIA_TASK_PROVIDES(BtDialupService,BtDialupService);
#include "btdialupservice.moc"

