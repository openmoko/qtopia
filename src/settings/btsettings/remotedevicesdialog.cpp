/****************************************************************************
**
** Copyright ( C ) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#include "remotedevicesdialog.h"

#include <qtopia/comm/qbluetoothaddress.h>
#include <qtopia/comm/qbluetoothlocaldevice.h>
#include <qtopia/comm/qbluetoothremotedevice.h>
#include <qtopia/comm/qbluetoothnamespace.h>
#include <qdocumentselector.h>
#include <qtopialog.h>

#include <qtopiaservices.h>

#include <QtopiaApplication>
#include <QMessageBox>
#include <QTimer>
#include <QAction>
#include <QFile>
#include <QDataStream>
#include <QBuffer>
#include <QVBoxLayout>


RemoteDevicesWindow::RemoteDevicesWindow( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ),
      m_deviceSelector( new QBluetoothDeviceSelector( this ) )
{
    setFocusPolicy( Qt::NoFocus );

    // don't close the device selector when a device is activated
    disconnect( m_deviceSelector, SIGNAL(deviceActivated(const QBluetoothAddress &)),
                m_deviceSelector, SLOT(accept()) );

    // create actions
    m_fileSelector = new QDialog( this );
    m_fileSelector->setObjectName( "fileSelectorDialog" );
    m_fileSelector->setModal( true );
    m_fileSelector->setWindowTitle( tr( "Select File to Send..." ) );

    QVBoxLayout *qvbl = new QVBoxLayout( m_fileSelector );
    m_docSelector = new QDocumentSelector( m_fileSelector );
    m_docSelector->setObjectName( "fileselector" );
    qvbl->addWidget( m_docSelector );

    connect( m_docSelector, SIGNAL( documentSelected( const QContent& ) ),
             SLOT( sendFileRequest( const QContent& ) ) );

#ifdef QTOPIA_KEYPAD_NAVIGATION
    QAction *newAction;

#ifdef QTOPIA_PHONE
    newAction = new QAction( QIcon( ":icon/business" ),
            tr( "Send Business Card..." ), this );
    connect( newAction, SIGNAL(triggered()), SLOT(sendVCard()) );
    m_deviceSelector->addDeviceAction( newAction );
#endif

    newAction = new QAction( QIcon( ":icon/txt" ), tr( "Send File..." ),
            this );
    connect( newAction, SIGNAL(triggered()), SLOT(sendFile()) );
    m_deviceSelector->addDeviceAction( newAction );
#endif

    setWindowTitle( QObject::tr( "Remote Devices" ) );
    setObjectName( "remote-devices" );
}

RemoteDevicesWindow::~RemoteDevicesWindow()
{
}


void RemoteDevicesWindow::start()
{
/*
    QtopiaApplication::showDialog( m_deviceSelector );
    m_deviceSelector->raise();
    m_deviceSelector->activateWindow();
    */
    m_deviceSelector->showMaximized();
    QtopiaApplication::execDialog(m_deviceSelector);
}

void RemoteDevicesWindow::sendFile()
{
    qLog(Bluetooth) << "RemoteDevicesWindow::sendFile()";

    if ( !m_deviceSelector->selectedDevice().valid() ) {
        qLog(Bluetooth) << "Can't send file, no selected device";
        return;
    }

#ifdef QTOPIA_DESKTOP
    fileSelectorDialog->exec();
#else
    QtopiaApplication::execDialog( m_fileSelector );
#endif
}

void RemoteDevicesWindow::sendFileRequest( const QContent& dl )
{
    qLog(Bluetooth) << "RemoteDevicesWindow::sendFileRequest()";

    QBluetoothAddress device = m_deviceSelector->selectedDevice();
    if ( !device.valid() ) {
        qLog(Bluetooth) << "Can't send file, no selected device";
        return;
    }

    m_fileSelector->accept();

    QtopiaServiceRequest req("BluetoothPush", "pushFile(QBluetoothAddress,QContentId)");
    req << device << dl.id();
    req.send();
}

void RemoteDevicesWindow::sendVCard()
{
    QBluetoothAddress device = m_deviceSelector->selectedDevice();
    if ( !device.valid() )
        return;

    QtopiaServiceRequest req("BluetoothPush", "pushBusinessCard(QBluetoothAddress)");
    req << device;
    req.send();
}
