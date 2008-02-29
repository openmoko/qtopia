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
#include "remotedevicesdialog.h"

#include <qbluetoothremotedevicedialog.h>
#include <qbluetoothaddress.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothremotedevice.h>
#include <qbluetoothnamespace.h>
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


class MyDeviceDialog : public QBluetoothRemoteDeviceDialog
{
    Q_OBJECT
public:
    MyDeviceDialog(QWidget *parent);

public slots:
    virtual void accept();
};

MyDeviceDialog::MyDeviceDialog(QWidget *parent)
    : QBluetoothRemoteDeviceDialog(parent)
{
}

void MyDeviceDialog::accept()
{
    // The default accept() implementation closes the dialog.
    // Don't do that for this dialog, because it's not behaving as a
    // "selector" dialog in Bluetooth Settings - it's more like a
    // device "viewer".
}



RemoteDevicesWindow::RemoteDevicesWindow(QWidget *parent, Qt::WFlags fl)
    : QDialog(parent, fl),
      m_deviceDialog(new MyDeviceDialog(this))
{
    setFocusPolicy(Qt::NoFocus);

#ifdef QTOPIA_KEYPAD_NAVIGATION
    QAction *newAction;

#ifdef QTOPIA_PHONE
    newAction = new QAction(QIcon(":icon/business"),
            tr("Send Business Card..."), this);
    connect(newAction, SIGNAL(triggered()), SLOT(sendVCard()));
    m_deviceDialog->addAction(newAction);
#endif

    newAction = new QAction(QIcon(":icon/txt"), tr("Send File..."),
            this);
    connect(newAction, SIGNAL(triggered()), SLOT(sendFile()));
    m_deviceDialog->addAction(newAction);
#endif

    setWindowTitle(QObject::tr("Remote Devices"));
    setObjectName("remote-devices");
}

RemoteDevicesWindow::~RemoteDevicesWindow()
{
}


void RemoteDevicesWindow::start()
{
    m_deviceDialog->showMaximized();
#ifdef QTOPIA_DESKTOP
    m_deviceDialog->exec();
#else
    QtopiaApplication::execDialog(m_deviceDialog);
#endif
}

void RemoteDevicesWindow::sendFile()
{
    qLog(Bluetooth) << "RemoteDevicesWindow::sendFile() entry";

    QBluetoothAddress device = m_deviceDialog->selectedDevice();
    if (!device.isValid()) {
        qLog(Bluetooth) << "Can't send file, no selected device";
        return;
    }

    // display a dialog that allows the user to select a document
    QDocumentSelectorDialog dialog;
    dialog.setFilter(QContentFilter(QContent::Document));
#ifdef QTOPIA_DESKTOP
    if (dialog.exec() == QDialog::Accepted) {
#else
    if (QtopiaApplication::execDialog(&dialog) == QDialog::Accepted) {
#endif
        QContent content = dialog.selectedDocument();
        qLog(Bluetooth) << "Sending file:" << content;
        QtopiaServiceRequest req("BluetoothPush", "pushFile(QBluetoothAddress,QContentId)");
        req << device << content.id();
        req.send();
    }
}

void RemoteDevicesWindow::sendVCard()
{
    QBluetoothAddress device = m_deviceDialog->selectedDevice();
    if (!device.isValid())
        return;

    QtopiaServiceRequest req("BluetoothPush", "pushPersonalBusinessCard(QBluetoothAddress)");
    req << device;
    req.send();
}

#include "remotedevicesdialog.moc"
