/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#include <QTimer>
#include <QStringList>
#include <QLabel>
#include <QMessageBox>
#include <QListWidget>
#include <QString>

#include <QBluetoothLocalDevice>
#include <QBluetoothSdpQuery>
#include <QBluetoothSdpRecord>
#include <QBluetoothAddress>
#include <QBluetoothSdpUuid>
#include <QBluetoothRemoteDeviceDialog>

#include <QtopiaApplication>
#include <QAction>
#include <QMenu>
#include <QSoftMenuBar>

#include "query.h"

Query::Query(QWidget *parent, Qt::WFlags f)
    : QMainWindow(parent, f)
{
    btDevice = new QBluetoothLocalDevice;

    setWindowTitle(tr("Bluetooth Query"));

    if (!btDevice->isValid()) {
        QLabel *label = new QLabel(tr("Bluetooth not available."), this);
        label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        label->setWordWrap(true);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setCentralWidget(label);
        return;
    }

    QTimer::singleShot(0, this, SLOT(startQuery()));

    sdap = new QBluetoothSdpQuery(this);
    connect(sdap, SIGNAL(searchComplete(const QBluetoothSdpQueryResult &)),
        this, SLOT(searchComplete(const QBluetoothSdpQueryResult &)));

    #warning "Canceling query broken due WaitWidget removal"

    startQueryAction = new QAction(tr("Query device..."), this);
    connect(startQueryAction, SIGNAL(triggered()), this, SLOT(startQuery()));
    QSoftMenuBar::menuFor(this)->addAction(startQueryAction);

    serviceList = new QListWidget(this);
    setCentralWidget(serviceList);
}

Query::~Query()
{
    
}

void Query::startQuery()
{
    canceled = false;

    startQueryAction->setVisible(false);
    serviceList->clear();
    QBluetoothAddress addr = QBluetoothRemoteDeviceDialog::getRemoteDevice(this);

    if (!addr.isValid()) {
        QMessageBox::warning(this, tr("Query Error"),
                QString(tr("<P>No device selected")));
        startQueryAction->setVisible(true);
        return;
    }

    // Search using the L2CAP UUID to even find services
    // not in public browse group
    quint16 id = 0x0100;
    sdap->searchServices(addr, *btDevice, QBluetoothSdpUuid(id));
}

void Query::searchComplete(const QBluetoothSdpQueryResult &result)
{
    startQueryAction->setVisible(true);

    if (canceled) {
        return;
    }

    if (!result.isValid()) {
        QMessageBox::warning(this, tr("Query Error"),
                QString(tr("<P>Error has occurred:"))
                        + result.error());
        return;
    }


    foreach (QBluetoothSdpRecord record, result.services()) {
        serviceList->addItem(record.serviceName());
    }
}

void Query::searchCancelled()
{
    startQueryAction->setVisible(true);
}

void Query::cancelQuery()
{
    canceled = true;
}
