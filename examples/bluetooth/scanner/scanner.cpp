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

#include <QLabel>
#include <QTimer>
#include <QStringList>
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialog>
#include <QAction>
#include <QMenu>

#include <QBluetoothLocalDevice>
#include <QBluetoothRemoteDevice>

#include <QtopiaApplication>
#include <QSoftMenuBar>

#include <stdio.h>

#include "scanner.h"

class RemoteInfoDialog : public QDialog
{
public:
    RemoteInfoDialog(const QBluetoothRemoteDevice &dev, QWidget *parent = 0);

private:
    QLineEdit *newEntry(const QString &value);
};

QLineEdit *RemoteInfoDialog::newEntry(const QString &value)
{
    QLineEdit *entry = new QLineEdit(this);
    entry->setReadOnly(true);
    entry->setText(value);

    return entry;
}

RemoteInfoDialog::RemoteInfoDialog(const QBluetoothRemoteDevice &dev, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QString("[%1]").arg(dev.address().toString()));

    QVBoxLayout *top = new QVBoxLayout(this);
    QFormLayout *attributes = new QFormLayout;

    attributes->addRow(tr("Name:"), newEntry(dev.name()));
    attributes->addRow(tr("Version:"), newEntry(dev.version()));
    attributes->addRow(tr("Revision:"), newEntry(dev.revision()));
    attributes->addRow(tr("Manufacturer:"), newEntry(dev.manufacturer()));
    attributes->addRow(tr("Company:"), newEntry(dev.company()));
    attributes->addRow(tr("Major Class:"), newEntry(dev.deviceMajorAsString()));
    attributes->addRow(tr("Minor Class:"), newEntry(dev.deviceMinorAsString()));

    top->addLayout(attributes);

    QLabel *label = new QLabel("Service Classes:", this);
    top->addWidget(label);

    QListWidget *list = new QListWidget(this);
    QStringList serviceClasses = dev.serviceClassesAsString();
    foreach (QString serviceClass, serviceClasses) {
        list->addItem(serviceClass);
    }

    top->addWidget(list);
}

Scanner::Scanner(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    btDevice = new QBluetoothLocalDevice(this);

    if (!btDevice->isValid()) {
        QVBoxLayout *l = new QVBoxLayout;
        QLabel *label = new QLabel(tr("(Bluetooth not available.)"), this);
        label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        label->setWordWrap(true);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        l->addWidget(label);
        setLayout(l);
        return;
    }

    connect(btDevice, SIGNAL(discoveryStarted()), this, SLOT(discoveryStarted()));
    connect(btDevice, SIGNAL(discoveryCompleted()), this, SLOT(discoveryComplete()));
    connect(btDevice, SIGNAL(remoteDeviceFound(const QBluetoothRemoteDevice &)),
                this, SLOT(remoteDeviceFound(const QBluetoothRemoteDevice &)));

    deviceList = new QListWidget(this);
    connect(deviceList, SIGNAL(itemActivated(QListWidgetItem *)),
                this, SLOT(itemActivated(QListWidgetItem *)));

    startScan = new QAction(tr("Discover Devices"), this);
    connect(startScan, SIGNAL(triggered()), btDevice, SLOT(discoverRemoteDevices()));
    QSoftMenuBar::menuFor(this)->addAction(startScan);

    setCentralWidget(deviceList);
    setWindowTitle(tr("Bluetooth Discovery"));
}

Scanner::~Scanner()
{

}

void Scanner::discoveryStarted()
{
    deviceInfo.clear();
    deviceList->clear();
    startScan->setText(tr("Scanning..."));
    startScan->setEnabled(false);
}

void Scanner::discoveryComplete()
{
    startScan->setText(tr("Discover Devices"));
    startScan->setEnabled(true);
}

void Scanner::remoteDeviceFound(const QBluetoothRemoteDevice &dev)
{
    deviceInfo.insert(dev.address().toString(), dev);
    deviceList->addItem(dev.address().toString());
}

void Scanner::itemActivated(QListWidgetItem *item)
{
    QString addr = item->text();

    QMap<QString, QBluetoothRemoteDevice>::const_iterator i =
            deviceInfo.find(addr);

    if (i == deviceInfo.constEnd()) {
        qWarning() << "Device" << addr << "not found!";
        return;
    }

    btDevice->cancelDiscovery();

    RemoteInfoDialog dlg(i.value(), this);
    QtopiaApplication::execDialog(&dlg);
}

#include "scanner.moc"
