/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include <qsdpservice.h>
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

    if (m_servicesController->isRegistered(m_name)) {
        m_prevOptions = m_servicesController->securityOptions(m_name);
        if (m_prevOptions == -1)    // unknown security
            m_prevOptions = 0;

        m_ui->authCheckBox->setChecked(m_prevOptions & QBluetooth::Authenticated);
        m_ui->encryptCheckBox->setChecked(m_prevOptions & QBluetooth::Encrypted);
    }
}

void ServiceSettingsWindow::finished(int result)
{
    if (m_servicesController->isRegistered(m_name) && result == QDialog::Accepted) {
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
    created using QBluetoothAbstractService), then searches for any
    extra local services (i.e. those registered with the SDP server but not
    created and registered using QBluetoothAbstractService) and adds
    them to the list.

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

    //connect(&m_sdap, SIGNAL(searchComplete(const QSDAPSearchResult &)),
    //        SLOT(results(const QSDAPSearchResult &)));

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
        m_servicesController->translatableDisplayName(name));
    item->setData(Qt::UserRole, QVariant(name));

    switch (m_servicesController->state(name)) {
        case QBluetoothServiceController::Started:
            item->setCheckState(Qt::Checked);
            item->setFlags(CHECKBOX_ENABLED_FLAGS);
            break;
        case QBluetoothServiceController::Stopped:
            item->setCheckState(Qt::Unchecked);
            item->setFlags(CHECKBOX_ENABLED_FLAGS);
            break;
        case QBluetoothServiceController::Starting:
            item->setCheckState(Qt::Checked);
            item->setFlags(CHECKBOX_DISABLED_FLAGS);
            break;
        case QBluetoothServiceController::Stopping:
            item->setCheckState(Qt::Unchecked);
            item->setFlags(CHECKBOX_DISABLED_FLAGS);
            break;
    }

    m_ui->serviceList->addItem(item);

    // map provider name to the row in which it is displayed
    m_displayedProviders[name] = m_ui->serviceList->count()-1;
}

void LocalServicesWindow::addRegisteredServices()
{
    QList<QString> registeredServices =
        m_servicesController->registeredServices();

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

    m_servicesController = new QBluetoothServiceController();
    connect(m_servicesController, SIGNAL(started(const QString &,
                QBluetooth::ServiceError, const QString &)),
            this, SLOT(serviceStarted(const QString &,
                QBluetooth::ServiceError, const QString &)));
    connect(m_servicesController, SIGNAL(stopped(const QString &,
                QBluetooth::ServiceError, const QString &)),
            this, SLOT(serviceStopped(const QString &,
                QBluetooth::ServiceError, const QString &)));
    connect(m_servicesController, SIGNAL(error(const QString &,
                QBluetooth::ServiceError, const QString &)),
            this, SLOT(serviceError(const QString &,
                QBluetooth::ServiceError, const QString &)));

    m_ui->serviceList->clear();

    addRegisteredServices();


    /*
    // display services that are not registered
    m_sdap.browseLocalServices(*m_device);
    m_inprogress = true;

    m_ui->localServices->setRowCount(m_ui->localServices->rowCount()+1);
    QTableWidgetItem *newItem =
        new QTableWidgetItem(tr("BTSettings", "Searching..."));
    newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    m_ui->localServices->setItem(m_ui->localServices->rowCount()-1, 0, newItem);
    */

    m_ui->serviceList->setCurrentRow(0);

    this->setModal( true );
    QtopiaApplication::execDialog( this );
}

    /*
void LocalServicesWindow::results(const QSDAPSearchResult &result)
{
    m_inprogress = false;

    if (result.errorOccurred()) {
        QMessageBox::warning(this, tr("BTSettings", "Error"),
            tr("BTSettings", "<P>Error accessing local services!"));
        return;
    }

    // find unregistered local services and add them to the display
    // don't include services with no names?
    QList<QSDPService> extraServices;
    foreach(QSDPService service, result.services()) {
        if (m_servicesController->findService(service.recordHandle()).isNull()
                && !service.serviceName().isEmpty())
            extraServices.append(service);
    }

    int lastRow = m_ui->localServices->rowCount()-1;
    if (extraServices.isEmpty()) {
        // remove the last row, which had "Searching..." text
        m_ui->localServices->removeRow(lastRow);
    } else {
        int row = lastRow;
        foreach(QSDPService service, extraServices) {
            addService(row, service.serviceName());
            row++;
        }
    }
}
*/

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
    qLog(Bluetooth) << "LocalServicesWindow::itemActivated";

    QVariant nameValue = item->data(Qt::UserRole);
    if (!nameValue.isValid())
        return;

    QString name = nameValue.toString();

    if ( !(item->flags() & Qt::ItemIsEnabled) ) {
        // clicked a disabled item

        QBluetoothServiceController::ServiceState state =
                m_servicesController->state(name);
        qLog(Bluetooth) << "activated disabled item" << name << "in state" << state;

        if (state == QBluetoothServiceController::Starting) {
            QMessageBox::warning(this, tr("Service Busy"),
                QString(tr("<P>Service is starting, please wait...")));

        } else if (state == QBluetoothServiceController::Stopping) {
            QMessageBox::warning(this, tr("Service Busy"),
                QString(tr("<P>Service is shutting down, please wait...")));
        }
        return;
    }

    // disable checking until get response for start/stop
    item->setFlags(CHECKBOX_DISABLED_FLAGS);

    // toggle state
    item->setCheckState( item->checkState() == Qt::Checked?
        Qt::Unchecked : Qt::Checked );

    if (item->checkState() == Qt::Checked)
        m_servicesController->start(name);
    else
        m_servicesController->stop(name);
}

void LocalServicesWindow::serviceStarted(const QString &name,
    QBluetooth::ServiceError error, const QString &errorDesc)
{
    qLog(Bluetooth) << "LocalServicesWindow::serviceStarted" << name << error << errorDesc;

    QListWidgetItem *checkBox = m_ui->serviceList->item(
            m_displayedProviders[name]);

    if (!checkBox)
        return;

    checkBox->setFlags(CHECKBOX_ENABLED_FLAGS);   // re-enable checkbox

    if (error && error != QBluetooth::AlreadyRunning) { // ignore AlreadyRunning
        checkBox->setCheckState(Qt::Unchecked); // revert checkbox state
        QMessageBox::warning(this, tr("Service Error"),
            QString(tr("<P>Unable to start service:") + QString("\r\n"))
            + errorDesc);
    } else {
        checkBox->setCheckState(Qt::Checked);
    }
}

void LocalServicesWindow::serviceStopped(const QString &name,
    QBluetooth::ServiceError error, const QString &errorDesc)
{
    qLog(Bluetooth) << "LocalServicesWindow::serviceStopped" << name << error << errorDesc;

    QListWidgetItem *checkBox = m_ui->serviceList->item(
            m_displayedProviders[name]);

    if (!checkBox)
        return;

    checkBox->setFlags(CHECKBOX_ENABLED_FLAGS);   // re-enable checkbox

    if (error == QBluetooth::UnknownError) {
        // only revert for bizarre errors i.e. UnknownError, otherwise should
        // just pretend stop worked

        checkBox->setCheckState(Qt::Checked);   // revert checkbox state
        QMessageBox::warning(this, tr("Service Error"),
            QString(tr("<P>Unable to stop service:") + QString("\r\n"))
            + errorDesc);
    } else {
        checkBox->setCheckState(Qt::Unchecked);
    }
}

void LocalServicesWindow::serviceError(const QString &name,
    QBluetooth::ServiceError error, const QString &errorDesc)
{
    Q_UNUSED(name);
    Q_UNUSED(error);
    QMessageBox::warning(this, tr("Service Error"),
        QString(tr("<P>Service error:") + QString("\r\n")) + errorDesc);
}

void LocalServicesWindow::finished()
{
    if (m_inprogress) {
        m_inprogress = false;
        m_sdap.cancelSearch();
        qLog(Bluetooth) << "LocalServicesWindow::Cancelling search";
    }

    // ensure m_servicesController does not send messages outside
    // the local services window
    if (m_servicesController) {
        delete m_servicesController;
        m_servicesController = 0;
    }
}

#include "localserviceswindow.moc"
