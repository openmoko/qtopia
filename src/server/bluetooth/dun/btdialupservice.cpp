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
#include <qtopianamespace.h>

#include <qcommdevicesession.h>
#include <qtopiaservices.h>
#include <qtopialog.h>
#include <qvaluespace.h>

class BtDialupServiceProvider : public QBluetoothAbstractService
{
    Q_OBJECT
public:
    BtDialupServiceProvider( BtDialupServiceTask* parent );

    void start();
    void stop();
    void setSecurityOptions( QBluetooth::SecurityOptions options );
    void allowPendingConnection( bool isAllowed );

private slots:
    void newConnection();
    void initiateModemEmulator();
    void emulatorStateChanged();

private:
    void cleanActiveConnections();

private:
    BtDialupServiceTask* m_parent;
    QBluetoothRfcommServer* server;
    quint32 m_sdpRecordHandle;
    QBluetoothLocalDevice* localDev;

    QList<QBluetoothRfcommSerialPort*> pendingConnections;
    QList<QBluetoothRfcommSerialPort*> runningConnections;

    QValueSpaceItem* modemEmulatorVS;

    QCommDeviceSession *m_session;
};

BtDialupServiceProvider::BtDialupServiceProvider( BtDialupServiceTask* parent )
    : QBluetoothAbstractService( QLatin1String("DialupNetworking"), tr("Dial-up Networking Service"), parent ),
        localDev( 0 )
{
    m_parent = parent;
    server = new QBluetoothRfcommServer( this );
    connect( server, SIGNAL(newConnection()), this, SLOT(newConnection()) );
    modemEmulatorVS = new QValueSpaceItem( QLatin1String("/Communications/ModemEmulator/serialPorts"), this );
    connect( modemEmulatorVS, SIGNAL(contentsChanged()), this, SLOT(emulatorStateChanged()) );

    m_session = 0;
}

void BtDialupServiceProvider::start()
{
    QBluetoothLocalDeviceManager manager;
    QBluetoothLocalDevice dev( manager.defaultDevice() );
    if ( !dev.isValid() ) {
        emit started( true, tr("Cannot find local Bluetooth device") );
        return;
    }

    cleanActiveConnections();
    if ( server->isListening() )
        server->close();

    // register the SDP service
    m_sdpRecordHandle = registerRecord(Qtopia::qtopiaDir() + "etc/bluetooth/sdp/dun.xml");
    if (m_sdpRecordHandle == 0) {
        emit started(true,
                tr("Error registering with SDP server") );
        return;
    }

    // For now, hard code in the channel, which has to be the same channel as
    // the one in the XML file passed in the registerRecord() call above
    int channel = 2;

    if ( !server->listen( dev.address(), channel) ) {
        unregisterRecord(m_sdpRecordHandle);
        emit started(true, tr("Cannot listen for incoming connections.") );
        return;
    }

    emit started(false, QLatin1String(""));
    qLog(Bluetooth) << QLatin1String("Dial-up Networking Service started");
}

void BtDialupServiceProvider::stop()
{
    if ( server->isListening() )
        server->close();
    cleanActiveConnections();

    if ( !unregisterRecord(m_sdpRecordHandle) )
        qLog(Bluetooth) << "BtDialupServiceProvider::stop() error unregistering SDP service";

    emit stopped();
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
        QBluetoothRfcommSerialPort* port = new QBluetoothRfcommSerialPort( s, 0, this );
        QString dev = port->device();
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

void BtDialupServiceProvider::initiateModemEmulator()
{
    while ( !pendingConnections.isEmpty() ) {
        QBluetoothRfcommSerialPort* port = pendingConnections.takeFirst();
         // Send a message to the modem emulator to add the serial port.
        QtopiaServiceRequest req( "ModemEmulator", "addSerialPort(QString)" );
        req << port->device();
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
        QString devName = port->device();
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

/*!
  \class BtDialupServiceTask
  \brief The BtDialupServiceTask class provides server side support for the Bluetooth 
  DUN profile.
  \ingroup QtopiaServer::Task::Bluetooth

  This task listens for incoming Bluetooth DUN connections, forwards the request to 
  Qtopia's modem emulator and manages the life time of these connections.
  
  The BtDialupServiceTask class provides the \c {BtDialupServiceTask} task.

  */

/*!
  Constructs the BtDialupServiceTask instance with the given \a parent.
  */
BtDialupServiceTask::BtDialupServiceTask( QObject* parent )
    : QObject( parent )
{
    qLog(Bluetooth) << "Initializing BtDialupService";
    d = new BtDialupServiceProvider( this );
}

/*!
  Destroys the BtDialupServiceTask instance.
  */
BtDialupServiceTask::~BtDialupServiceTask()
{
}

QTOPIA_TASK(BtDialupServiceTask,BtDialupServiceTask);
#include "btdialupservice.moc"

