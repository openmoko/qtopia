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

class BtDialupServiceProvider : public QBluetoothAbstractService
{
    Q_OBJECT
public:
    BtDialupServiceProvider( BtDialupService* parent );

    void start(int channel);
    void stop();
    void setSecurityOptions( QBluetooth::SecurityOptions options );
    void allowPendingConnection( bool isAllowed );

    QString translatableDisplayName() const;

private slots:
    void newConnection();
    void initiateModemEmulator();
    void emulatorStateChanged();

private:
    void cleanActiveConnections();

private:
    BtDialupService* m_parent;
    QBluetoothRfcommServer* server;
    QBluetoothLocalDevice* localDev;

    QList<QBluetoothRfcommSerialPort*> pendingConnections;
    QList<QBluetoothRfcommSerialPort*> runningConnections;

    QValueSpaceItem* modemEmulatorVS;

    QCommDeviceSession *m_session;
};

BtDialupServiceProvider::BtDialupServiceProvider( BtDialupService* parent )
    : QBluetoothAbstractService( QLatin1String("DialupNetworking"), parent ),
        localDev( 0 )
{
    m_parent = parent;
    server = new QBluetoothRfcommServer( this );
    connect( server, SIGNAL(newConnection()), this, SLOT(newConnection()) );
    modemEmulatorVS = new QValueSpaceItem( QLatin1String("/Communications/ModemEmulator/serialPorts"), this );
    connect( modemEmulatorVS, SIGNAL(contentsChanged()), this, SLOT(emulatorStateChanged()) );
    initialize();

    m_session = 0;
}

void BtDialupServiceProvider::start(int ch)
{
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

    if ( !server->listen( dev.address(), ch) ) {
        emit started(QBluetooth::NotRunning, tr("Cannot listen for incoming connections.") );
        return;
    }

    if ( !sdpRegister( dev.address(), QBluetooth::DialupNetworkingProfile, ch ) )
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
    if ( !sdpUnregister() ) {
        emit stopped( QBluetooth::SDPServerError,
                tr("Error unregistering from SDP server") );
    }

    if ( server->isListening() )
        server->close();
    cleanActiveConnections();

    emit stopped(QBluetooth::NoError, QString());
    qLog(Bluetooth) << QLatin1String("Dial-up Networking Service stopped");
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

    while ( server->hasPendingConnections() ) {
        QBluetoothRfcommSocket* s =
                static_cast<QBluetoothRfcommSocket *>(server->nextPendingConnection());
        QBluetoothRfcommSerialPort* port = new QBluetoothRfcommSerialPort( this );
        QString dev = port->createTty( s );
        if ( !dev.isEmpty() ) {
            pendingConnections.append ( port );
        } else {
            qLog(Bluetooth) << "BtDialupService: Cannot create rfcomm device. Ignoring incoming connection.";
            delete port;
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
            qLog(Bluetooth) << "Bluetooth Dialup session on" << devName  <<" terminated.";
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

