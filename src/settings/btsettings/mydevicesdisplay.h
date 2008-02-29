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
#ifndef __MYDEVICESDISPLAY_H__
#define __MYDEVICESDISPLAY_H__

#include <qbluetoothaddress.h>
#include <qbluetoothnamespace.h>

#include <QWidget>
#include <QList>

class QBluetoothRemoteDeviceSelector;
class QBluetoothLocalDevice;
class QAction;
class QWaitWidget;
class PairingAgent;
class QBluetoothRemoteDeviceDialog;
class QBluetoothSdpQuery;
class QBluetoothSdpQueryResult;
class RemoteDeviceInfoDialog;
class DeviceDialogFilter;
class QLabel;

class MyDevicesDisplay : public QWidget
{
    Q_OBJECT
public:
    MyDevicesDisplay(QBluetoothLocalDevice *local, QWidget *parent = 0);
    ~MyDevicesDisplay();

public slots:
    void initDisplay();

private slots:
    void refreshNextDevice();
    void setAlias();
    void deviceSelectionChanged();
    void deviceActivated(const QBluetoothAddress &addr);

    void newPairing();
    void selectedPairingTarget();
    void pairingAgentDone(bool error);
    void pairingCreated(const QBluetoothAddress &addr);

    void deleteDevice();
    void pairingRemoved(const QBluetoothAddress &addr);

    void foundServices(const QBluetoothSdpQueryResult &);

private:
    void initActions();
    void resetDisplay();
    void createPairing(const QBluetoothAddress &addr);
    void doneAddDevice(bool error, const QString &errorString);

    QBluetoothLocalDevice *m_local;
    QBluetoothRemoteDeviceSelector *m_browser;
    QLabel *m_devicesPlaceholderLabel;
    QList<QBluetoothAddress> m_initialDevices;
    int m_refreshIndex;

    PairingAgent *m_pairingAgent;
    QWaitWidget *m_pairingWaitWidget;

    QBluetoothRemoteDeviceDialog *m_pairingDeviceDialog;
    DeviceDialogFilter *m_pairingDeviceDialogFilter;
    QBluetoothSdpQuery *m_sdpQuery;

    RemoteDeviceInfoDialog *m_deviceInfoDialog;

    QList<QAction *> m_deviceActions;
};

#endif
