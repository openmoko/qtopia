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
#ifndef __LOCALSERVICESWINDOW_H__
#define __LOCALSERVICESWINDOW_H__

#include <qbluetoothnamespace.h>
#include <qbluetoothsdpquery.h>

#include <QDialog>
#include <QHash>

class QBluetoothLocalDevice;
class QBluetoothSdpQueryResult;
class BluetoothServicesController;
class QListWidgetItem;
class QAction;
class QBluetoothServiceController;
class ServiceSettingsWindow;

namespace Ui {
    class LocalServices;
}

class LocalServicesWindow : public QDialog
{
    Q_OBJECT
public:
    LocalServicesWindow(QBluetoothLocalDevice *device, QWidget *parent = 0, Qt::WFlags fl=0);
    ~LocalServicesWindow();

    void start();
    void addRegisteredServices();

public slots:
    void serviceStarted(const QString &name, bool error, const QString &desc);
    void serviceStopped(const QString &name);

private slots:
    void editServiceSettings();
    void itemActivated(QListWidgetItem *item);
    void finished();

private:
    void addService(const QString &displayName);
    void addServiceForProvider(const QString &name);

    static Qt::ItemFlags CHECKBOX_ENABLED_FLAGS;
    static Qt::ItemFlags CHECKBOX_DISABLED_FLAGS;

    Ui::LocalServices *m_ui;
    ServiceSettingsWindow *m_settingsDialog;
    QAction *m_settingsAction;

    QBluetoothLocalDevice *m_device;
    QBluetoothServiceController *m_servicesController;
    QHash<QString, int> m_displayedProviders;
};

#endif
