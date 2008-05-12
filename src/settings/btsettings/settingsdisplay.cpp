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
#include "settingsdisplay.h"
#include "localservicesdialog.h"
#include "ui_settings.h"

#include <qbluetoothaddress.h>

#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qformlayout.h>
#include <QPhoneProfileManager>

#include <QDialog>
#include <QAction>
#include <QList>
#include <QMenu>
#include <QSoftMenuBar>
#include <QMessageBox>
#include <QTimer>


SettingsDisplay::SettingsDisplay(QBluetoothLocalDevice *local, QCommDeviceController *controller, QWidget *parent)
    : QWidget(parent),
      m_local(local),
      m_deviceController(controller),
      m_localServicesDialog(0)
{
    m_ui = new Ui::Settings();
    m_ui->setupUi(this);

    m_phoneProfileMgr = new QPhoneProfileManager(this);
    connect(m_phoneProfileMgr, SIGNAL(planeModeChanged(bool)),
            SLOT(planeModeChanged(bool)));

    initDisplayedValues();
    powerStateChanged(m_deviceController->powerState());  // set initial state

    m_detailsAction = new QAction(tr("Other details"), this);
    addAction(m_detailsAction);
    m_servicesAction = new QAction(tr("My services..."), this);
    addAction(m_servicesAction);

    QTimer::singleShot(0, this, SLOT(init()));
}

SettingsDisplay::~SettingsDisplay()
{
}

void SettingsDisplay::toggleLocalPowerState(bool enable)
{
    qLog(Bluetooth) << "SettingsDisplay::toggleLocalPowerState()" << enable;

    if (enable) {
        m_ui->powerCheckBox->setEnabled(false);
        m_deviceController->bringUp();

    } else {
        if (m_deviceController->sessionsActive()) {
            int result = QMessageBox::question(
                    this,
                    QObject::tr("Turn off Bluetooth?"),
                    QObject::tr("<P>There are applications using the bluetooth device.  Are you sure you want to turn it off?"),
                    QMessageBox::Yes|QMessageBox::No);

            if (result == QMessageBox::No) {
                qLog(Bluetooth) << "User doesn't want to shut down the device...";
                m_ui->powerCheckBox->setChecked(true);
                return;
            }
        }

        m_ui->powerCheckBox->setEnabled(false);
        setInteractive(false);
        m_deviceController->bringDown();
    }
}

void SettingsDisplay::toggleLocalVisibility(bool visible)
{
    qLog(Bluetooth) << "SettingsDisplay::toggleLocalVisibility()" << visible;

    m_ui->visibilityCheckBox->setEnabled(false);
    if (visible) {
        m_local->setDiscoverable(getTimeout());
    } else {
        m_ui->timeoutSpinBox->setEnabled(false);
        m_local->setConnectable();
    }
}

void SettingsDisplay::nameEditingFinished()
{
    QString newName = m_ui->nameEdit->text();
    if (newName.trimmed().isEmpty()) {
        // revert to stored local name
        m_ui->nameEdit->setText(m_local->name());
        return;
    }

    m_local->setName(newName);
}

void SettingsDisplay::timeoutEditingFinished()
{
    int timeout = getTimeout();
    if (timeout == m_lastTimeout)
        return;

    qLog(Bluetooth) << "SettingsDisplay: set new discoverable timeout:" << timeout;
    if (m_local->setDiscoverable(timeout))
        m_lastTimeout = timeout;
}

int SettingsDisplay::getTimeout()
{
    // timeouts are shown in minutes, setDiscoverable() needs them in seconds
    return m_ui->timeoutSpinBox->value() * 60;
}


void SettingsDisplay::powerStateChanged(QCommDeviceController::PowerState state)
{
    qLog(Bluetooth) << "SettingsDisplay::powerStateChanged()" << state;

    // call setChecked() in case call didn't work and checkbox state needs
    // to be reverted
    bool enabled = (state != QCommDeviceController::Off);
    m_ui->powerCheckBox->setChecked(enabled);

    if (!m_phoneProfileMgr->planeMode()) {
        setInteractive(enabled);
        m_ui->powerCheckBox->setEnabled(true);
        m_ui->powerCheckBox->setFocus();
    }
}

void SettingsDisplay::deviceStateChanged(QBluetoothLocalDevice::State state)
{
    qLog(Bluetooth) << "SettingsDisplay::deviceStateChanged()" << state;

    // call setChecked() in case call didn't work and checkbox state needs
    // to be reverted
    bool discoverable = (state == QBluetoothLocalDevice::Discoverable);
    m_ui->visibilityCheckBox->setChecked(discoverable);

    if (!m_phoneProfileMgr->planeMode()) {
        m_ui->timeoutSpinBox->setEnabled(discoverable);
        m_ui->visibilityCheckBox->setEnabled(true);
        m_ui->visibilityCheckBox->setFocus();
    }	
}

void SettingsDisplay::setInteractive(bool interactive)
{
    // enable/disable everything except the "Turn Bluetooth on" checkbox
    m_ui->optionsGroupBox->setEnabled(interactive);

    // show/hide actions in the context menu
    QList<QAction *> actions = this->actions();
    for (int i=0; i<actions.size(); i++)
        actions.at(i)->setVisible(interactive);
}

// similar to details display for remote devices (in remotedeviceinfodialog.cpp)
void SettingsDisplay::showDetailsDialog()
{
    QDialog dlg;
    QFormLayout form(&dlg);

    form.addRow(tr("Address:"), new QLabel(m_local->address().toString()));
    form.addRow(tr("Version:"), new QLabel(m_local->version()));
    form.addRow(tr("Vendor:"), new QLabel(m_local->manufacturer()));
    form.addRow(tr("Company:"), new QLabel(m_local->company()));

    for (int i=0; i<form.count(); i++) {
        QLabel *label = qobject_cast<QLabel*>(form.itemAt(i)->widget());
        if (label) {
            label->setWordWrap(true);
            if (label->text() == "(null)")  // returned from hcid
                label->setText("");
        }
    }

    dlg.setWindowTitle(tr("Other details"));
    QtopiaApplication::execDialog(&dlg);
}

void SettingsDisplay::planeModeChanged(bool enabled)
{
    m_ui->powerCheckBox->setEnabled(!enabled);
    setInteractive(!enabled);
    if (!enabled)
        m_ui->powerCheckBox->setFocus();
}

void SettingsDisplay::initDisplayedValues()
{
    m_ui->powerCheckBox->setChecked(m_deviceController->powerState() != QCommDeviceController::Off);

    bool discoverable = m_local->discoverable();
    m_ui->visibilityCheckBox->setChecked(discoverable);

    m_lastTimeout = m_local->discoverableTimeout();
    m_ui->timeoutSpinBox->setValue(m_lastTimeout == 0? 0 : (m_lastTimeout / 60));
    m_ui->timeoutSpinBox->setEnabled(discoverable);

    m_ui->nameEdit->setText(m_local->name());

    if (m_phoneProfileMgr->planeMode())
        planeModeChanged(m_phoneProfileMgr->planeMode());
}

void SettingsDisplay::showMyServices()
{
    if (!m_localServicesDialog)
        m_localServicesDialog = new LocalServicesDialog(this);
    m_localServicesDialog->start();
}


void SettingsDisplay::init()
{
    // set widgets tab order
    QWidget::setTabOrder(m_ui->powerCheckBox, m_ui->visibilityCheckBox);
    QWidget::setTabOrder(m_ui->visibilityCheckBox, m_ui->timeoutSpinBox);
    QWidget::setTabOrder(m_ui->timeoutSpinBox, m_ui->nameEdit);

    // action signals
    connect(m_detailsAction, SIGNAL(triggered()),
            SLOT(showDetailsDialog()));
    connect(m_servicesAction, SIGNAL(triggered()),
            SLOT(showMyServices()));

    // for bluetooth on/off
    connect(m_ui->powerCheckBox, SIGNAL(clicked(bool)),
            SLOT(toggleLocalPowerState(bool)));
    connect(m_deviceController, SIGNAL(powerStateChanged(QCommDeviceController::PowerState)),
            SLOT(powerStateChanged(QCommDeviceController::PowerState)));

    // for device visibility
    connect(m_ui->visibilityCheckBox, SIGNAL(clicked(bool)),
            SLOT(toggleLocalVisibility(bool)));
    connect(m_local, SIGNAL(stateChanged(QBluetoothLocalDevice::State)),
            SLOT(deviceStateChanged(QBluetoothLocalDevice::State)));

    // for visibility timeout
    connect(m_ui->timeoutSpinBox, SIGNAL(editingFinished()),
            SLOT(timeoutEditingFinished()));

    // set the name field
    connect(m_ui->nameEdit, SIGNAL(editingFinished()),
            SLOT(nameEditingFinished()));

    m_ui->timeoutSpinBox->installEventFilter(this);
}

bool SettingsDisplay::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::LeaveEditFocus) {
        // in Qt 4.3, editingFinished() isn't emitted when 
        // leaving edit focus, only on focus out
        timeoutEditingFinished();
    }
    return false;
}

#include "settingsdisplay.moc"
