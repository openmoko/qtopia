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

#include <QBluetoothLocalDevice>
#include <QBluetoothLocalDeviceManager>
#include <QBluetoothAddress>

#include <QVBoxLayout>
#include <QTabWidget>
#include <QListWidget>

#include "devinfo.h"

DevInfo::DevInfo(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    QTabWidget *tab = new QTabWidget(this);

    QBluetoothLocalDeviceManager mgr;

    // Obtain list of all devices on the system
    QStringList devices = mgr.devices();

    foreach (QString deviceName, devices) {

        QBluetoothLocalDevice dev(deviceName);

        if (!dev.isValid()) {
            continue;
        }

        QListWidget *list = new QListWidget;

        list->addItem("[General]");
        list->addItem("Name:");
        list->addItem(dev.deviceName());
        list->addItem("Address:");
        list->addItem(dev.address().toString());
        list->addItem("Manufacturer:");
        list->addItem(dev.manufacturer());
        list->addItem("Version:");
        list->addItem(dev.version());
        list->addItem("Company:");
        list->addItem(dev.company());
        list->addItem("Revision:");
        list->addItem(dev.revision());

        list->addItem("");

        list->addItem("[State]");
        if (dev.connectable()) {
            list->addItem("Connectable:");
            list->addItem("true");
        } else {
            list->addItem("Connectable:");
            list->addItem("false");
        }

        list->addItem("Discoverable:");
        if (dev.discoverable()) {
            list->addItem("true");
            list->addItem("Timeout:");
            list->addItem(QString::number(dev.discoverableTimeout() / 60));
        } else {
            list->addItem("false");
        }

        list->addItem("");

        list->addItem("[Connections]");
        QList<QBluetoothAddress> connections = dev.connections();
        foreach (QBluetoothAddress addr, connections) {
            list->addItem(addr.toString());
        }

        list->addItem("");

        list->addItem("[PairedDevices]");
        QList<QBluetoothAddress> paired = dev.pairedDevices();
        foreach (QBluetoothAddress addr, paired) {
            list->addItem(addr.toString());
        }

        QWidget *base = new QWidget;
        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addWidget(list);
        base->setLayout(mainLayout);
        tab->addTab(base, deviceName);
    }

    setCentralWidget(tab);
}

DevInfo::~DevInfo()
{

}
