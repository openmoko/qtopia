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

#include "mainwindow.h"

#include "ui_btsettings.h"
#include "ui_device.h"
#include "ui_deviceinfo.h"
#include "ui_visibility.h"

#include "headset.h"
#include "remotedevicesdialog.h"
#include "localserviceswindow.h"

#include <qbluetoothlocaldevice.h>
#include <qbluetoothlocaldevicemanager.h>
#include <qcommdevicecontroller.h>
#include <qtopiacomm/private/remotedevicepropertiesdialog_p.h>
#include <qtopianamespace.h>
#include <qtopialog.h>

#include <QApplication>
#include <QtopiaApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialog>
#include <QSoftMenuBar>
#include <QTimer>

#include <QMenu>
#include <QAction>

class DeviceWindow : public QDialog
{
    Q_OBJECT

public:
    DeviceWindow(QCommDeviceController *controller,
                 QBluetoothLocalDevice *device, QWidget *parent = 0, Qt::WFlags fl=0);
    ~DeviceWindow();

public slots:
    void finished(int result);
    void start();

signals:
    void deviceEnabled(bool enabled);

private:
    Ui::Device *m_ui;
    QBluetoothLocalDevice *m_device;
    bool m_prev;
    QCommDeviceController *m_controller;
};

DeviceWindow::DeviceWindow(QCommDeviceController *controller,
                           QBluetoothLocalDevice *device,
                           QWidget *parent, Qt::WFlags fl) :
        QDialog(parent, fl)
{
    m_device = device;

    m_controller = controller;
    m_ui = new Ui::Device();
    m_ui->setupUi(this);

    connect(this, SIGNAL(finished(int)), SLOT(finished(int)));

    setWindowTitle(tr("Device"));
    setObjectName("device");
}

DeviceWindow::~DeviceWindow()
{
    delete m_ui;
}

void DeviceWindow::finished(int result)
{
    bool cur = m_ui->deviceOnButton->isChecked();

    if (result == QDialog::Accepted && cur != m_prev) {
        if (cur) {
            // For now assume it is brought up
            m_controller->bringUp();
            emit deviceEnabled(cur);
        } else {
            // For now assume it is brought down
            m_controller->bringDown();
            emit deviceEnabled(cur);
        }
    }
}

void DeviceWindow::start()
{
    m_prev = m_controller->powerState() == QCommDeviceController::On;
    m_ui->deviceOnButton->setChecked(m_prev);
    m_ui->deviceOffButton->setChecked(!m_prev);

    this->setModal( true );
#ifdef QTOPIA_DESKTOP
    this->exec();
#else
    QtopiaApplication::execDialog( this );
#endif
}

class VisibilityWindow : public QDialog
{
    Q_OBJECT

public:
    VisibilityWindow(QBluetoothLocalDevice *device, QWidget *parent = 0, Qt::WFlags fl=0);
    ~VisibilityWindow();

public slots:
    void finished(int result);
    void start();

private:
    Ui::Visibility *m_ui;
    QBluetoothLocalDevice *m_device;
    bool m_prev;
};

VisibilityWindow::VisibilityWindow(QBluetoothLocalDevice *device, QWidget *parent, Qt::WFlags fl) :     QDialog(parent, fl)
{
    m_device = device;

    m_ui = new Ui::Visibility();
    m_ui->setupUi(this);

    connect(this, SIGNAL(finished(int)), SLOT(finished(int)));

    setWindowTitle(QObject::tr("Visibility"));
    setObjectName("visibility");
}

VisibilityWindow::~VisibilityWindow()
{
    delete m_ui;
}

void VisibilityWindow::finished(int result)
{
    bool cur = m_ui->visibilityOnButton->isChecked();

    if (result == QDialog::Accepted && cur != m_prev) {
        if (cur) {
            if (!m_device->setDiscoverable()) {
                qLog(Bluetooth) << "Error making device discoverable:"
                    << m_device->error();
                QMessageBox::warning(this, tr("Device Error"),
                    QString(tr("<P>Unable to turn on device visibility")));
            }
        } else {
            if (!m_device->setConnectable()) {
                qLog(Bluetooth) << "Error turning off device:"
                    << m_device->error();
                QMessageBox::warning(this, tr("Device Error"),
                    QString(tr("<P>Unable to turn off device visibility")));
            }
        }
    }
}

void VisibilityWindow::start()
{
    m_prev = m_device->discoverable();
    m_ui->visibilityOnButton->setChecked(m_prev);
    m_ui->visibilityOffButton->setChecked(!m_prev);

    this->setModal( true );
#ifdef QTOPIA_DESKTOP
    this->exec();
#else
    QtopiaApplication::execDialog( this );
#endif
}

class DeviceInfoWindow : public QDialog
{
    Q_OBJECT

public:
    DeviceInfoWindow(QBluetoothLocalDevice *device, QWidget *parent = 0, Qt::WFlags fl=0);
    ~DeviceInfoWindow();

public slots:
    void finished(int result);
    void start();

private:
    Ui::DeviceInfo *m_ui;
    QBluetoothLocalDevice *m_device;
    QString m_prev;
};

DeviceInfoWindow::DeviceInfoWindow(QBluetoothLocalDevice *device, QWidget *parent, Qt::WFlags fl) :     QDialog(parent, fl)
{
    m_device = device;

    m_ui = new Ui::DeviceInfo();
    m_ui->setupUi(this);

    connect(this, SIGNAL(finished(int)), SLOT(finished(int)));

    m_ui->addressEdit->setText(m_device->address().toString());
    m_ui->versionEdit->setText(m_device->version());
    m_ui->mfgEdit->setText(m_device->manufacturer());
    m_ui->companyEdit->setText(m_device->company());
        // must have IEEE oui.txt for company to work - HCI daemon expects
        // oui.txt to be at /usr/share/misc/oui.txt

    setWindowTitle(QObject::tr("Info"));
    setObjectName("device-info");
}

DeviceInfoWindow::~DeviceInfoWindow()
{
    delete m_ui;
}

void DeviceInfoWindow::finished(int result)
{
    if (result == QDialog::Accepted) {
        QString cur = m_ui->nameEdit->text();
        if (cur != m_prev && !cur.trimmed().isEmpty()) {
            m_device->setName(cur);
        }
    }
}

void DeviceInfoWindow::start()
{
    m_prev = m_device->name();

    QVariant nameFont = Qtopia::findDisplayFont(m_prev);
    if (nameFont.isNull()) {
        // cannot display this name with any of the installed fonts
        m_ui->nameEdit->setText("");
    } else {
        m_ui->nameEdit->setText(m_prev);
        m_ui->nameEdit->setFont(nameFont.value<QFont>());
    }

    this->setModal( true );
#ifdef QTOPIA_DESKTOP
    this->exec();
#else
    QtopiaApplication::execDialog( this );
#endif
}



class BTSettings_Private {
public:
    BTSettings_Private();
    ~BTSettings_Private();

    QBluetoothLocalDevice *m_device;
    QCommDeviceController *m_controller;

    Ui::BTSettings *m_btsettings;
    QWidget *m_mainWidget;

    DeviceWindow *m_deviceWindow;
    VisibilityWindow *m_visibilityWindow;
    DeviceInfoWindow *m_deviceInfoWindow;
    LocalServicesWindow *m_localServicesWindow;
};

BTSettings_Private::BTSettings_Private()
{
    m_device = 0;
}

BTSettings_Private::~BTSettings_Private()
{
    if (m_device)
        delete m_device;
}

BTSettingsMainWindow::BTSettingsMainWindow(QWidget *parent, Qt::WFlags fl)
    : QMainWindow(parent, fl)
{
    m_data = new BTSettings_Private();
    m_data->m_mainWidget = new QWidget(this);

    // Take care of the case where we have no Bluetooth adapters
    QBluetoothLocalDeviceManager manager;
    QString defaultDeviceName = manager.defaultDevice();
    if (defaultDeviceName.isNull()) {
        QVBoxLayout *layout = new QVBoxLayout(m_data->m_mainWidget);
        QLabel *label = new QLabel(tr("<P>No bluetooth adapters found"));
        label->setWordWrap(true);
        layout->addWidget(label);
        setCentralWidget(m_data->m_mainWidget);
        return;
    }

    m_data->m_device = new QBluetoothLocalDevice(defaultDeviceName);
    m_data->m_controller =
            new QCommDeviceController(m_data->m_device->deviceName().toLatin1(), this);

    // Setup main window
    m_data->m_btsettings = new Ui::BTSettings();
    m_data->m_btsettings->setupUi(m_data->m_mainWidget);
    connect(m_data->m_btsettings->actionSelection, SIGNAL(itemActivated(QListWidgetItem *)),
            SLOT(itemSelected(QListWidgetItem *)));

    m_data->m_deviceWindow = new DeviceWindow(m_data->m_controller, m_data->m_device, this);
    connect(m_data->m_deviceWindow, SIGNAL(deviceEnabled(bool)), SLOT(deviceEnabled(bool)));

    m_data->m_visibilityWindow = new VisibilityWindow(m_data->m_device, this);
    m_data->m_deviceInfoWindow = new DeviceInfoWindow(m_data->m_device, this);
    m_data->m_localServicesWindow = new LocalServicesWindow(m_data->m_device, this);

    setCentralWidget(m_data->m_mainWidget);

    // without this, the screen spills off-screen on the left side if
    // qtopia is in RTL language mode
    m_data->m_mainWidget->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

    // UpTimed and UpOneItem are still treated as off since they are transient states
    deviceEnabled(m_data->m_controller->powerState() == QCommDeviceController::On);

#ifdef QTOPIA_KEYPAD_NAVIGATION
    QSoftMenuBar::menuFor( this );
#endif
    setWindowTitle(tr("Bluetooth"));
    m_data->m_btsettings->actionSelection->setCurrentRow(0);
}

BTSettingsMainWindow::~BTSettingsMainWindow()
{
    delete m_data;
}

void BTSettingsMainWindow::deviceEnabled(bool enable)
{
    m_data->m_btsettings->actionSelection->item(0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

    Qt::ItemFlags flags;

    if (enable) {
        flags = Qt::ItemIsSelectable|Qt::ItemIsEnabled;
    }
    else {
        flags = 0;
    }

    // start at 1 to skip first "Device" item (for on/off)
    for (int i=1; i<m_data->m_btsettings->actionSelection->count(); i++)
        m_data->m_btsettings->actionSelection->item(i)->setFlags(flags);
}

void BTSettingsMainWindow::itemSelected(QListWidgetItem *item)
{
    // don't activate item if it's not enabled (odd to have to do this check?)
    if ( !(item->flags() & Qt::ItemIsEnabled) )
        return;

    if (item->text() == tr("Device")) {
        m_data->m_deviceWindow->start();
    }
    else if (item->text() == tr("Visibility")) {
        m_data->m_visibilityWindow->start();
    }
    else if (item->text() == tr("Device Info")) {
        m_data->m_deviceInfoWindow->start();
    }
    else if (item->text() == tr("Local Services")) {
        m_data->m_localServicesWindow->start();
    }
    else if (item->text() == tr("Remote Devices")) {
        RemoteDevicesWindow *dlg = new RemoteDevicesWindow(this);
        dlg->start();
        delete dlg;
    }
    else if (item->text() == tr("My Headset")) {
        HeadsetDialog *dlg = new HeadsetDialog(QBluetooth::HeadsetProfile, this);
        dlg->start();
        delete dlg;
    }
    else if (item->text() == tr("My Handsfree")) {
        HeadsetDialog *dlg = new HeadsetDialog(QBluetooth::HandsFreeProfile, this);
        dlg->start();
        delete dlg;
    }
}

#include "mainwindow.moc"
