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
#include "btsettings.h"
#include "btsettings_p.h"
#include "settingsdisplay.h"
#include "mydevicesdisplay.h"

#include <qbluetoothlocaldevice.h>

#include <QTabWidget>
#include <QVBoxLayout>
#include <QMenu>
#include <QSoftMenuBar>
#include <QLabel>
#include <QSettings>
#include <QTimer>
#include <QScrollArea>

BTSettingsMainWindow::BTSettingsMainWindow(QWidget *parent, Qt::WFlags fl)
    : QMainWindow(parent, fl)
{
    QBluetoothLocalDevice *localDevice = new QBluetoothLocalDevice(this);
    if (!localDevice->isValid()) {
        QLabel *label = new QLabel(tr("(Bluetooth not available.)"));
        label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        label->setWordWrap(true);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setCentralWidget(label);
        return;
    }

    QScrollArea* scroll = new QScrollArea();
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidgetResizable(true);
    scroll->setFrameStyle(QFrame::NoFrame);

    m_menu = QSoftMenuBar::menuFor(this);
    m_tabs = new QTabWidget();

    SettingsDisplay *settings = new SettingsDisplay(localDevice);
    scroll->setWidget(settings);
    m_tabs->addTab(scroll, tr("Settings"));

    MyDevicesDisplay *myDevices = new MyDevicesDisplay(localDevice);
    m_tabs->addTab(myDevices, tr("Paired Devices"));
    QTimer::singleShot(0, myDevices, SLOT(initDisplay()));

    m_tabs->setCurrentIndex(0);

    // see whether we need to fiddle with the context menu at all.
    connect(m_tabs, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));

    QCommDeviceController *controller =
            new QCommDeviceController(localDevice->deviceName().toLatin1(), this);
    connect(controller, SIGNAL(powerStateChanged(QCommDeviceController::PowerState)),
            SLOT(setTabsEnabled(QCommDeviceController::PowerState)));
    setTabsEnabled(controller->powerState());

    // set the context menu
    tabChanged(m_tabs->currentIndex());

    setCentralWidget(m_tabs);
    setWindowTitle(tr("Bluetooth"));
}

BTSettingsMainWindow::~BTSettingsMainWindow()
{
}

void BTSettingsMainWindow::tabChanged(int /*index*/)
{
    QWidget *w = m_tabs->currentWidget();
    m_menu->clear();
    m_menu->addActions(w->actions());
}

void BTSettingsMainWindow::setTabsEnabled(QCommDeviceController::PowerState state)
{
    // turn off all tabs except the first one if device is turned off.
    bool bluetoothOn = (state != QCommDeviceController::Off);
    for (int i=1; i<m_tabs->count(); i++)
        m_tabs->setTabEnabled(i, bluetoothOn);
}


//====================================================================

void BtSettings::setAudioProfileChannel(const QBluetoothAddress &addr, QBluetooth::SDPProfile profile, int channel)
{
    QSettings settings("Trolltech", "BluetoothKnownHeadsets");
    settings.setValue(addr.toString() + "/" + QString::number(int(profile)),
                      channel);
}

int BtSettings::audioProfileChannel(const QBluetoothAddress &addr, QBluetooth::SDPProfile profile)
{
    QSettings settings("Trolltech", "BluetoothKnownHeadsets");
    return settings.value(addr.toString() + "/" + QString::number(int(profile)), -1).toInt();
}
