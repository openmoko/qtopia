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
#ifndef __SETTINGSDISPLAY_H__
#define __SETTINGSDISPLAY_H__

#include <QWidget>
#include <QBluetoothLocalDevice>
#include <QCommDeviceController>

namespace Ui {
    class Settings;
};

class LocalServicesDialog;
class QBluetoothLocalDevice;
class QAction;

class SettingsDisplay : public QWidget
{
    Q_OBJECT
public:
    SettingsDisplay(QBluetoothLocalDevice *local, QWidget *parent = 0);
    ~SettingsDisplay();

private slots:
    void toggleLocalPowerState(bool enable);
    void toggleLocalVisibility(bool visible);
    void nameEditingFinished();
    void timeoutEditingFinished();

    void powerStateChanged(QCommDeviceController::PowerState state);
    void deviceStateChanged(QBluetoothLocalDevice::State state);

    void showDetailsDialog();


private:
    int getTimeout();
    void setInteractive(bool interactive);
    void initDisplayedValues();
    void initActions();

    Ui::Settings *m_ui;
    QBluetoothLocalDevice *m_local;
    QCommDeviceController *m_deviceController;

    LocalServicesDialog *m_localServicesDialog;

    QAction *m_servicesAction;
    QAction *m_detailsAction;

    int m_lastTimeout;
};

#endif
