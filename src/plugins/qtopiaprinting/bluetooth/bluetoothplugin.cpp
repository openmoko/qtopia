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

#include "bluetoothplugin.h"

#include <QCommDeviceSession>
#include <QtopiaApplication>
#include <QtopiaIpcEnvelope>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothlocaldevicemanager.h>
#include <qbluetoothremotedevicedialog.h>
#include <QPrintEngine>
#include <QDebug>

#include "qbluetoothobexagent.h"

using namespace QBluetooth;

class BluetoothPrinterPluginPrivate : public QObject
{
    Q_OBJECT
public:
    BluetoothPrinterPluginPrivate() : m_session(0) {
        m_localDevice = 0;
        m_agent = 0;
    }

    ~BluetoothPrinterPluginPrivate() {
        delete m_localDevice;
        m_localDevice = 0;
    }

    bool isAvailable() {
        QBluetoothLocalDeviceManager localDevManager( this );
        return !localDevManager.defaultDevice().isNull();
    }

    void send( const QString &fileName, const QString &mimeType = QString() ) {
        init();

        if (m_localDevice) {
            m_session = QCommDeviceSession::session(m_localDevice->deviceName().toLatin1());
        }

        if (!m_session) {
            // printing failed
            QtopiaIpcEnvelope e ( "QPE/Service/Print", "done(bool)" );
            e << true;
            return;
        }

        QBluetoothAddress addr = QBluetoothRemoteDeviceDialog::getRemoteDevice(
                0, m_deviceSelectorProfiles, m_deviceDialogFilter );
        if ( addr.isValid() ) {
            m_agent = new QBluetoothObexAgent( QBluetoothRemoteDevice( addr ) );
            m_agent->setAutoDelete( true );
            m_agent->send( fileName, mimeType );
            connect( m_agent, SIGNAL(done(bool)),
                    this, SLOT(done(bool)) );
        } else {
            // printing failed
            QtopiaIpcEnvelope e ( "QPE/Service/Print", "done(bool)" );
            e << true;
        }
    }

    void sendHtml( const QString &html ) {
        init();

        if (m_localDevice) {
            m_session = QCommDeviceSession::session(m_localDevice->deviceName().toLatin1());
        }

        if (!m_session) {
            // printing failed
            QtopiaIpcEnvelope e ( "QPE/Service/Print", "done(bool)" );
            e << true;
            return;
        }

        QBluetoothAddress addr = QBluetoothRemoteDeviceDialog::getRemoteDevice(
                0, m_deviceSelectorProfiles, m_deviceDialogFilter );
        if ( addr.isValid() ) {
            m_agent = new QBluetoothObexAgent( QBluetoothRemoteDevice( addr ) );
            m_agent->setAutoDelete( true );
            m_agent->sendHtml( html );
            connect( m_agent, SIGNAL(done(bool)),
                    this, SLOT(done(bool)) );
        } else {
            // printing failed
            QtopiaIpcEnvelope e ( "QPE/Service/Print", "done(bool)" );
            e << true;
        }
    }

    bool abort() {
        if (m_session) {
            m_session->endSession();
            delete m_session;
            m_session = 0;
        }

        if ( m_agent ) {
            m_agent->abort();
            return true;
        } else
            return false;
    }

    QBluetoothLocalDevice *m_localDevice;
    QBluetoothRemoteDeviceDialogFilter m_deviceDialogFilter;
    QSet<SDPProfile> m_deviceSelectorProfiles;
    QBluetoothObexAgent *m_agent;
    QCommDeviceSession *m_session;

public slots:
    void done( bool error ) {
        if (m_session) {
            m_session->endSession();
            delete m_session;
            m_session = 0;
        }

        QtopiaIpcEnvelope e( "QPE/Service/Print", "done(bool)" );
        e << error;
    }

private:
    void init() {
        if ( !m_localDevice ) {
            // search for local devices

            QBluetoothLocalDeviceManager localDevManager( this );
            QString devName = localDevManager.defaultDevice();

            if ( !devName.isNull() )
                m_localDevice = new QBluetoothLocalDevice( devName );
        }

        // show only imaging devices with object push profile
        QSet<QBluetooth::DeviceMajor> devMajors;
        devMajors.insert( Imaging );
        m_deviceDialogFilter.setAcceptedDeviceMajors( devMajors );
        m_deviceSelectorProfiles.insert( ObjectPushProfile );
    }
};

//---------------------------------------------

BluetoothPrinterPlugin::BluetoothPrinterPlugin( QObject *parent )
    : QtopiaPrinterPlugin( parent )
{
    d = new BluetoothPrinterPluginPrivate();
}

BluetoothPrinterPlugin::~BluetoothPrinterPlugin()
{
    delete d;
}

void BluetoothPrinterPlugin::print( QMap<QString, QVariant> options )
{
    // currently simple push transfer model is implemented
    // other values needed to support job based transfer.
    QString fileName = options.value( QString::number( QPrintEngine::PPK_OutputFileName ) ).toString();

    d->send( fileName );
}

void BluetoothPrinterPlugin::printFile( const QString &fileName, const QString &mimeType )
{
    d->send( fileName, mimeType );
}

void BluetoothPrinterPlugin::printHtml( const QString &html )
{
    d->sendHtml( html );
}

bool BluetoothPrinterPlugin::abort()
{
    return d->abort();
}

bool BluetoothPrinterPlugin::isAvailable()
{
    return d->isAvailable();
}

QString BluetoothPrinterPlugin::name()
{
    return tr( "Bluetooth" );
}

QTOPIA_EXPORT_PLUGIN( BluetoothPrinterPlugin );

#include "bluetoothplugin.moc"

