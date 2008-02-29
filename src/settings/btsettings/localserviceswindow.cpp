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
#include "localserviceswindow.h"
#include "ui_localservices.h"
#include "ui_localservicesettings.h"

#include <QApplication>
#include <QtopiaApplication>
#include <QTimer>
#include <QMessageBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSoftMenuBar>
#include <QMenu>
#include <QHash>

#include <qbluetoothsdprecord.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothservicecontroller.h>
#include <qtopialog.h>

/*!
    This dialog shows the settings (e.g. security options) for an individual
    service, and allows the user to change these settings.
 */
class ServiceSettingsWindow : public QDialog
{
    Q_OBJECT

public:
    ServiceSettingsWindow(QWidget *parent = 0, Qt::WFlags fl=0);
    ~ServiceSettingsWindow();

    void setService(const QString &name);

private slots:
    void finished(int result);

private:
    Ui::ServiceSettings *m_ui;
    QString m_name;
    QBluetoothServiceController *m_servicesController;
    QBluetooth::SecurityOptions m_prevOptions;
};

ServiceSettingsWindow::ServiceSettingsWindow(QWidget *parent, Qt::WFlags fl)
    : QDialog(parent, fl),
      m_servicesController(new QBluetoothServiceController(this))
{
    m_ui = new Ui::ServiceSettings();
    m_ui->setupUi(this);
    connect(this, SIGNAL(finished(int)), SLOT(finished(int)));

    setWindowTitle(tr("Security options"));
    setObjectName("serviceSettings");
}

ServiceSettingsWindow::~ServiceSettingsWindow()
{
    delete m_ui;
}

void ServiceSettingsWindow::setService(const QString &name)
{
    m_name = name;

    m_prevOptions = m_servicesController->securityOptions(m_name);
    if (m_prevOptions == -1)    // unknown security
        m_prevOptions = 0;

    m_ui->authCheckBox->setChecked(m_prevOptions & QBluetooth::Authenticated);
    m_ui->encryptCheckBox->setChecked(m_prevOptions & QBluetooth::Encrypted);
}

void ServiceSettingsWindow::finished(int result)
{
    if (result == QDialog::Accepted) {
        QBluetooth::SecurityOptions newSecurityOptions(0);

        if (m_ui->authCheckBox->isChecked())
            newSecurityOptions |= QBluetooth::Authenticated;
        if (m_ui->encryptCheckBox->isChecked())
            newSecurityOptions |= QBluetooth::Encrypted;

        // save changed settings
        if (newSecurityOptions != m_prevOptions)
            m_servicesController->setSecurityOptions(m_name, newSecurityOptions);
    }
}


/*!
    \class LocalServicesWindow

    This displays a list of local services, and allows the user to perform
    operations enabled registered local services.

    When this dialog starts up, it displays all registered services (i.e. those
    created using QBluetoothAbstractService).

    Clicking enabled a registered service takes the user to the
    ServiceSettingsWindow dialog. Each registered service also has a checkbox
    next to it to enable/disable the service.
 */

Qt::ItemFlags LocalServicesWindow::CHECKBOX_ENABLED_FLAGS =
        Qt::ItemIsSelectable | Qt::ItemIsEnabled;
Qt::ItemFlags LocalServicesWindow::CHECKBOX_DISABLED_FLAGS =
        Qt::ItemIsSelectable;

LocalServicesWindow::LocalServicesWindow(QBluetoothLocalDevice *device,
                                         QWidget *parent, Qt::WFlags)
    : QDialog(parent),
      m_settingsDialog(0),
      m_device(device),
      m_servicesController(0)
{
    m_ui = new Ui::LocalServices();
    m_ui->setupUi(this);
    m_ui->serviceList->setFocusPolicy(Qt::StrongFocus);

    connect(m_ui->serviceList, SIGNAL(itemActivated(QListWidgetItem *)),
            SLOT(itemActivated(QListWidgetItem *)));

    connect(this, SIGNAL(finished(int)), SLOT(finished()));

#ifdef QTOPIA_PHONE
    QMenu *contextMenu = QSoftMenuBar::menuFor(this);
    m_settingsAction = contextMenu->addAction(tr("Edit settings..."));
    connect(m_settingsAction, SIGNAL(triggered()),
            SLOT(editServiceSettings()));
#endif

    setWindowTitle(tr("Local Services"));
    setObjectName("local-services");
}

LocalServicesWindow::~LocalServicesWindow()
{
    delete m_ui;
}

void LocalServicesWindow::addService(const QString &displayName)
{
    m_ui->serviceList->addItem(new QListWidgetItem(displayName));
}

void LocalServicesWindow::addServiceForProvider(const QString &name)
{
    QListWidgetItem *item = new QListWidgetItem(
        m_servicesController->displayName(name));
    item->setData(Qt::UserRole, QVariant(name));

    switch (m_servicesController->state(name)) {
        case QBluetoothServiceController::Running:
            item->setCheckState(Qt::Checked);
            item->setFlags(CHECKBOX_ENABLED_FLAGS);
            break;
        case QBluetoothServiceController::NotRunning:
            item->setCheckState(Qt::Unchecked);
            item->setFlags(CHECKBOX_ENABLED_FLAGS);
            break;
        case QBluetoothServiceController::Starting:
            item->setCheckState(Qt::Checked);
            item->setFlags(CHECKBOX_DISABLED_FLAGS);
            break;
    }

    m_ui->serviceList->addItem(item);

    // map provider name to the row in which it is displayed
    m_displayedProviders[name] = m_ui->serviceList->count()-1;
}

void LocalServicesWindow::addRegisteredServices()
{
    QList<QString> registeredServices = m_servicesController->services();

    foreach(QString name, registeredServices) {
        addServiceForProvider(name);
    }
}

void LocalServicesWindow::start()
{
    if (m_servicesController) {
        delete m_servicesController;
        m_servicesController = 0;
    }

    m_servicesController = new QBluetoothServiceController(this);
    connect(m_servicesController, SIGNAL(started(const QString &, bool, const QString &)),
            this, SLOT(serviceStarted(const QString &, bool, const QString &)));
    connect(m_servicesController, SIGNAL(stopped(const QString &)),
            this, SLOT(serviceStopped(const QString &)));

    m_ui->serviceList->clear();

    addRegisteredServices();

    m_ui->serviceList->setCurrentRow(0);

    this->setModal( true );
    QtopiaApplication::execDialog( this );
}

void LocalServicesWindow::editServiceSettings()
{
    QListWidgetItem *currentItem = m_ui->serviceList->currentItem();
    if (!currentItem)
        return;

    QVariant name = currentItem->data(Qt::UserRole);
    if (name.isValid()) {
        if (!m_settingsDialog)
            m_settingsDialog = new ServiceSettingsWindow(this);
        m_settingsDialog->setService(name.toString());
        QtopiaApplication::execDialog(m_settingsDialog);
    }
}


void LocalServicesWindow::itemActivated(QListWidgetItem *item)
{
    QVariant nameValue = item->data(Qt::UserRole);
    if (!nameValue.isValid())
        return;

    QString name = nameValue.toString();

    if ( !(item->flags() & Qt::ItemIsEnabled) ) {
        // clicked a disabled item

        if (m_servicesController->state(name) == QBluetoothServiceController::Starting) {
            QMessageBox::warning(this, tr("Service Busy"),
                QString(tr("<P>Service is starting, please wait...")));
        }
        return;
    }

    // disable checking until get response for start/stop
    item->setFlags(CHECKBOX_DISABLED_FLAGS);

    // toggle state
    item->setCheckState( item->checkState() == Qt::Checked ?
            Qt::Unchecked : Qt::Checked );

    if (item->checkState() == Qt::Checked)
        m_servicesController->start(name);
    else
        m_servicesController->stop(name);
}

void LocalServicesWindow::serviceStarted(const QString &name, bool error, const QString &desc)
{
    QListWidgetItem *checkBox = m_ui->serviceList->item(
            m_displayedProviders[name]);

    if (!checkBox)
        return;

    if (error) {
        checkBox->setCheckState(Qt::Unchecked); // revert checkbox state
        QMessageBox::warning(this, tr("Service Error"),
            QString(tr("<P>Unable to start service:") + QString("\r\n"))
            + desc);
    } else {
        checkBox->setCheckState(Qt::Checked);
    }

    checkBox->setFlags(CHECKBOX_ENABLED_FLAGS);   // re-enable checkbox
}

void LocalServicesWindow::serviceStopped(const QString &name)
{
    QListWidgetItem *checkBox = m_ui->serviceList->item(
            m_displayedProviders[name]);
    if (!checkBox)
        return;

    checkBox->setCheckState(Qt::Unchecked);
    checkBox->setFlags(CHECKBOX_ENABLED_FLAGS);   // re-enable checkbox
}

void LocalServicesWindow::finished()
{
    // ensure m_servicesController does not send messages outside
    // the local services window
    if (m_servicesController) {
        delete m_servicesController;
        m_servicesController = 0;
    }
}

#include "localserviceswindow.moc"
