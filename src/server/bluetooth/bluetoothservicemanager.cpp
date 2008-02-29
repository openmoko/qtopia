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

#include "bluetoothservicemanager.h"
#include "qtopiaserverapplication.h"

#include <QtopiaIpcEnvelope>
#include <QtopiaIpcSendEnvelope>
#include <QValueSpaceObject>
#include <QValueSpaceItem>
#include <QTranslatableSettings>
#include <QTimer>
#include <QSet>
#include <QSetIterator>
#include <QList>

#include <qcommservicemanager.h>
#include <qbluetoothservicecontroller.h>
#include <qbluetoothlocaldevicemanager.h>
#include <qbluetoothlocaldevice.h>
#include <qsdp.h>
#include <qsdap.h>
#include <qsdpservice.h>
#include <qbluetoothnamespace.h>
#include <qbluetoothservicecontrol.h>

#include <qtopiaipcadaptor.h>
#include <qtopialog.h>

class BluetoothServiceManagerShutdownHandler : public SystemShutdownHandler
{
    Q_OBJECT
public:
    BluetoothServiceManagerShutdownHandler(BluetoothServiceManager *manager)
        : m_serviceManager(manager),
          m_serviceController(new QBluetoothServiceController(this)),
          m_sdap(new QSDAP())
    {
        m_localDevice = new QBluetoothLocalDevice(this);
    }

    bool systemRestart()
    {
        return doShutdown();
    }

    bool systemShutdown()
    {
        return doShutdown();
    }

    bool doShutdown()
    {
        qLog(Bluetooth) << "Initiating shutdown of BluetoothServiceManager";

        connect(m_serviceController, SIGNAL(stopped(const QString &,
                   QBluetooth::ServiceError, const QString &)),
                SLOT(serviceStopped(const QString &,
                    QBluetooth::ServiceError,
                    const QString &)));

        QList<QString> services = m_serviceController->registeredServices();
        m_registeredServicesCount = services.size();
        foreach (QString service, services) {
            if (m_serviceController->state(service) ==
                    QBluetoothServiceController::Started) {
                m_serviceManager->stopService(service, true);
            }
        }

        // in case the services aren't stopped, don't block the shut down
        // process
        QTimer::singleShot( 5000, this, SLOT(finished()));

        return false;
    }

private slots:
    void serviceStopped(const QString &/*name*/,
                        QBluetooth::ServiceError /*error*/,
                        const QString &/*errorDesc*/)
    {
        m_registeredServicesCount--;
        if (m_registeredServicesCount == 0) {
            // now find all remaining sdp services and stop them too
            connect(m_sdap, SIGNAL(searchComplete(const QSDAPSearchResult &)),
                    SLOT(foundLocalServices(const QSDAPSearchResult &)));
            m_sdap->browseLocalServices(*m_localDevice);
            m_searching = true;
        }
    }

    void foundLocalServices(const QSDAPSearchResult &result)
    {
        QSDP sdp;
        foreach (QSDPService service, result.services()) {
            sdp.unregisterService(*m_localDevice, service);
        }
        finished();
    }

    void finished()
    {
        static bool m_finished = false;
        if (m_finished)
            return;
        m_finished = true;

        if (m_searching)
            m_sdap->cancelSearch();

        qLog(Network) << "Shutdown of BluetoothServiceManager complete" ;
        emit proceed();
    }

private:
    BluetoothServiceManager *m_serviceManager;
    QBluetoothServiceController *m_serviceController;
    int m_registeredServicesCount;
    QSDAP *m_sdap;
    QBluetoothLocalDevice *m_localDevice;
    bool m_searching;
};

/*
    Communicates with those who want to receive messages about services,
    or send messages to change service settings (e.g. BTSettings).
 */
class ServiceListenerController : public QtopiaIpcAdaptor
{
    friend class BluetoothServiceManager;
    Q_OBJECT

public:
    ServiceListenerController(BluetoothServiceManager *parent);
    ~ServiceListenerController();

public slots:
    void startService(const QString &name);
    void stopService(const QString &name);
    void setServiceSecurity(const QString &name, QBluetooth::SecurityOptions);

signals:
    void serviceStarted(const QString &name, QBluetooth::ServiceError error, const QString &errorDesc);
    void serviceStopped(const QString &name, QBluetooth::ServiceError error, const QString &errorDesc);
    void serviceError(const QString &name, QBluetooth::ServiceError error, const QString &errorDesc);

private:
    BluetoothServiceManager *m_parent;
};

ServiceListenerController::ServiceListenerController(BluetoothServiceManager *parent)
    : QtopiaIpcAdaptor("QPE/BluetoothServiceListeners", parent),
      m_parent(parent)
{
    publishAll(SignalsAndSlots);
}

ServiceListenerController::~ServiceListenerController()
{
}

void ServiceListenerController::startService(const QString &name)
{
    m_parent->startService(name);
}

void ServiceListenerController::stopService(const QString &name)
{
    m_parent->stopService(name, false);
}

void ServiceListenerController::setServiceSecurity(const QString &name,
    QBluetooth::SecurityOptions options)
{
    qLog(Bluetooth) << "ServiceListenerController::setServiceSecurity" << options;
    m_parent->setServiceSecurity(name, options);
}

/*
    Wraps QBluetoothServiceControl messages and passes them onto
    BluetoothServiceManager with the service name added.
 */
class ServiceControlHandler : public QObject
{
    Q_OBJECT

public:
    ServiceControlHandler(QBluetoothServiceControl *control, BluetoothServiceManager *manager);
    ~ServiceControlHandler();
public slots:
    //void registerService(const QString &displayname);
    void started(QBluetooth::ServiceError, const QString &);
    void stopped(QBluetooth::ServiceError, const QString &);
    void error(QBluetooth::ServiceError, const QString &);
private slots:
    void disconnected();
private:
    QBluetoothServiceControl *m_control;
    BluetoothServiceManager *m_manager;
};

ServiceControlHandler::ServiceControlHandler(QBluetoothServiceControl *control, BluetoothServiceManager *manager)
    : QObject(control),
      m_control(control),
      m_manager(manager)
{
    connect(control, SIGNAL(started(QBluetooth::ServiceError, const QString &)),
        SLOT(started(QBluetooth::ServiceError, const QString &))),
    connect(control, SIGNAL(stopped(QBluetooth::ServiceError, const QString &)),
        SLOT(stopped(QBluetooth::ServiceError, const QString &))),
    connect(control, SIGNAL(error(QBluetooth::ServiceError, const QString &)),
        SLOT(error(QBluetooth::ServiceError, const QString &)));

    connect(control, SIGNAL(disconnected()), SLOT(disconnected()));
}

ServiceControlHandler::~ServiceControlHandler()
{
}

void ServiceControlHandler::started(QBluetooth::ServiceError error, const QString &errorDesc)
{
    m_manager->serviceStarted(m_control->service(), error, errorDesc);
}

void ServiceControlHandler::stopped(QBluetooth::ServiceError error, const QString &errorDesc)
{
    m_manager->serviceStopped(m_control->service(), error, errorDesc);
}

void ServiceControlHandler::error(QBluetooth::ServiceError error, const QString &errorDesc)
{
    m_manager->serviceError(m_control->service(), error, errorDesc);
}

void ServiceControlHandler::disconnected()
{
    qLog(Bluetooth) << "ServiceControlHandler::disconnected, unregistering"
        << m_control->service();
    m_manager->unregisterService(m_control->service());
}


const QString BluetoothServiceManager::SETTINGS_VALUE_SPACE_PATH = "Communications/Bluetooth/Services";
const QString BluetoothServiceManager::SETTINGS_CONFIG_NAME = "BluetoothServices";

int BluetoothServiceManager::RFCOMM_CHANNELS_START = 1;
int BluetoothServiceManager::RFCOMM_CHANNELS_END = 31;

const QList<QString> BluetoothServiceManager::persistentSettings =
        BluetoothServiceManager::persistentServiceSettings();


BluetoothServiceManager::BluetoothServiceManager(QObject *parent)
    : QObject(parent),
      m_commServiceManager(new QCommServiceManager(this)),
      m_listenerMessenger(new ServiceListenerController(this)),
      m_servicesValueSpace(new QValueSpaceObject(
            SETTINGS_VALUE_SPACE_PATH, this)),
      m_configSettings(new QTranslatableSettings(
            "Trolltech", SETTINGS_CONFIG_NAME, this)),
      m_sdap(new QSDAP()),
      m_serviceCommObjects(new QHash<QString, QBluetoothServiceControl *>())
{
    loadConfig();
    initAvailableChannels();
    initReservedChannels();

    connect(m_commServiceManager, SIGNAL(serviceAdded(const QString &)),
        SLOT(commServiceAdded(const QString &)));
    connect(m_commServiceManager, SIGNAL(serviceRemoved(const QString &)),
        SLOT(commServiceRemoved(const QString &)));

    // respond if the local device is added/removed
    QBluetoothLocalDeviceManager *manager = new QBluetoothLocalDeviceManager(this);
    connect(manager, SIGNAL(deviceAdded(const QString &)),
        SLOT(localDeviceAdded(const QString &)));
    connect(manager, SIGNAL(deviceRemoved(const QString &)),
        SLOT(localDeviceRemoved(const QString &)));

    // stop all services when qtopia shuts down
    QtopiaServerApplication::addAggregateObject(this,
        new BluetoothServiceManagerShutdownHandler(this));

    qLog(Bluetooth) << "Created BluetoothServiceManager";
}

BluetoothServiceManager::~BluetoothServiceManager()
{
    if (m_sdap)
        delete m_sdap;

    const QList<QString> keys = m_serviceCommObjects->keys();
    foreach( QString k, keys ) {
        delete m_serviceCommObjects->take( k );
    }
    delete m_serviceCommObjects;

}

QHash<QString, QVariant> BluetoothServiceManager::defaultServiceSettings()
{
    // These are all the settings that will be in the value space for a service.

    // The "Registered" setting is basically to tell whether a service has been
    // created yet, because a service's details might have been loaded from the
    // config (i.e. the service was created in a previous qtopia session) but
    // it mightn't been created/registered yet in *this* session.

    QHash<QString, QVariant> settings;
    settings["DisplayName"] = QVariant(QString()); // the translatable name
    settings["Registered"] = QVariant(false);
    settings["Enabled"] = QVariant(false);
    settings["ChangingState"] = QVariant(false);
    settings["Autostart"] = QVariant(true); // whether to start when registered
    settings["Security"] = QVariant(QBluetooth::Authenticated | QBluetooth::Encrypted);
    settings["Channel"] = QVariant(-1);
    return settings;
}

QList<QString> BluetoothServiceManager::persistentServiceSettings()
{
    QList<QString> settings;
    settings.append("DisplayName");
    settings.append("Autostart");
    settings.append("Security");
    settings.append("Channel");
    return settings;
}

void BluetoothServiceManager::commServiceAdded(const QString &name)
{
    QCommServiceManager manager;
    if ( !(manager.interfaces(name).contains("QBluetoothServiceControl")) )
        return;

    qLog(Bluetooth) << "BluetoothServiceManager adding service" << name;

    // comm object for talking to this service
    QBluetoothServiceControl *control = new QBluetoothServiceControl(name);
    new ServiceControlHandler(control, this);
    m_serviceCommObjects->insert(name, control);

    registerService(name, control->translatableDisplayName());
}

void BluetoothServiceManager::commServiceRemoved(const QString &name)
{
    qLog(Bluetooth) << "BluetoothServiceManager::commServiceRemoved" << name;

    QBluetoothServiceControl *control = m_serviceCommObjects->take(name);
    if (control)
        delete control;

    qLog(Bluetooth) << "BluetoothServiceManager::commServiceRemoved DONE" << name;
}

void BluetoothServiceManager::localDeviceAdded(const QString &deviceName)
{
    qLog(Bluetooth) << "BluetoothServiceManager::localDeviceAdded()" << deviceName;

    // When the local device is added, (re)start all services.

    // check that it isn't a second added device, cos then the services would
    // already running
    QBluetoothLocalDeviceManager manager;
    if (manager.devices().size() == 1) {
        // wait a bit for local device to start up (otherwise it will be invalid)
        QTimer::singleShot(200, this, SLOT(startAllServices()));
    }
}

void BluetoothServiceManager::localDeviceRemoved(const QString &deviceName)
{
    qLog(Bluetooth) << "BluetoothServiceManager:localDeviceRemoved()" << deviceName;

    // When the local device is removed, shut down all services.

    // check there are no available devices left
    QBluetoothLocalDeviceManager manager;
    if (manager.devices().size() == 0) {
        stopAllServices();

        // maybe these channels will work next time
        m_badRfcommChannels.clear();
    }
}

void BluetoothServiceManager::startAllServices()
{
    qLog(Bluetooth) << "BtPowerServiceTask::startAllServices";

    QBluetoothLocalDevice device;
    if (device.isValid()) {
        qLog(Bluetooth) << "BluetoothServiceManager: (re)starting known services";

        QList<QString> registeredServices = m_configSettings->childGroups();
        for (int i=0; i<registeredServices.size(); i++) {
            if (serviceValue(registeredServices[i], "Autostart").toBool())
                startService(registeredServices[i]);
        }
    }
}

void BluetoothServiceManager::stopAllServices()
{
    qLog(Bluetooth) << "BluetoothServiceManager: stopping known services";

    QList<QString> registeredServices = m_configSettings->childGroups();
    for (int i=0; i<registeredServices.size(); i++) {
        if (serviceValue(registeredServices[i], "Enabled").toBool())
            stopService(registeredServices[i], true);
    }
}

void BluetoothServiceManager::registerService(const QString &name, const QString &translatableDisplayName)
{
    // request from a service provider to register a service.

    qLog(Bluetooth) << "BluetoothServiceManager registering" << name << translatableDisplayName;

    initServiceSettings(name, translatableDisplayName);
    setServiceValue(name, "Registered", QVariant(true));

    if (serviceValue(name, "Autostart").toBool())
        startService(name);
}

void BluetoothServiceManager::unregisterService(const QString &name)
{
    // request from a service provider to unregister a service.

    if (isRegistered(name)) {
        qLog(Bluetooth) << "BluetoothServiceManager unregistering" << name;

        setServiceValue(name, "Registered", QVariant(false));
    }
}

void BluetoothServiceManager::serviceStarted(const QString &name,  QBluetooth::ServiceError error, const QString &errorDesc)
{
    // message from a service provider that a service has been started.

    if (!isRegistered(name))
        return;

    // a channel should have been assigned when the service got a "start"
    // message but this must be changed when considering more than just RFCOMM
    // channels!
    QString errorDescMod = errorDesc;
    int channel = m_usedRfcommChannels.key(name);
    if (channel == 0) {
        error = QBluetooth::UnknownService;
        errorDescMod = tr("Unknown service, no RFCOMM channel assigned to this service");
    }

    if (error) {
        // couldn't start service, remove from used channels
        m_usedRfcommChannels.remove(channel);

    } else {
        qLog(Bluetooth) << "BluetoothServiceManager serviceStarted" << name;

        setServiceValue(name, "Enabled", QVariant(true));
        setServiceValue(name, "Channel", QVariant(channel));
    }

    // service has finished starting
    setServiceValue(name, "ChangingState", QVariant(false));

    // if it's SDP error and we haven't tried to fix it once already, try to
    // fix it by starting the service again on a different channel
    if (error == QBluetooth::SDPServerError &&
            !m_servicesWithSdpError.contains(name)) {
        qLog(Bluetooth) << "BluetoothServiceManager: Service" << name
            << "got SDP error on start(), try to restart it on different channel";

        // trying to fix error now, don't try again next time if there's another
        // SDP error (just emit the error)
        m_servicesWithSdpError.insert(name);

        // reset the reserved channel for this service
        setServiceValue(name, "Channel", -1);
        m_badRfcommChannels.insert(channel);    // assume can't use it any more

        // Try again. This will start the service again, on a different channel
        startService(name);

        return;
    }

    // clear any noted SDP error for this service
    m_servicesWithSdpError.remove(name);

    // pass on message to e.g. BTSettings
    emit m_listenerMessenger->serviceStarted(name, error, errorDescMod);
}

void BluetoothServiceManager::serviceStopped(const QString &name, QBluetooth::ServiceError error, const QString &errorDesc)
{
    // message from a service provider that a service has been stopped.

    if (!isRegistered(name))
        return;

    if (!error) {
        qLog(Bluetooth) << "BluetoothServiceManager serviceStopped" << name;
        handleServiceTerminated(name);
    }

    // service has finished stopping
    setServiceValue(name, "ChangingState", QVariant(false));

    // pass on message to e.g. BTSettings
    emit m_listenerMessenger->serviceStopped(name, error, errorDesc);
}

void BluetoothServiceManager::serviceError(
                                const QString &name,
                                QBluetooth::ServiceError error,
                                const QString &errorDesc)
{
    // message from a service provider that a service error has occured.

    if (!isRegistered(name))
        return;

    qLog(Bluetooth) << "BluetoothServiceManager::serviceError" << name
        << errorDesc;

    handleServiceTerminated(name);

    // pass on message to e.g. BTSettings
    emit m_listenerMessenger->serviceError(name, error, errorDesc);
}

void BluetoothServiceManager::handleServiceTerminated(const QString &name)
{
    setServiceValue(name, "Enabled", QVariant(false));
    int channel = serviceValue(name, "Channel").toInt();
    m_usedRfcommChannels.remove(channel);
}

void BluetoothServiceManager::startService(const QString &name)
{
    // request (e.g. from BTSettings) to start a service.

    qLog(Bluetooth) << "called BluetoothServiceManager::startService" << name;

    if (!isRegistered(name)) {
        ServiceListenerController controller(this);
        emit controller.serviceStarted(name, QBluetooth::UnknownService,
            tr("Unknown service, service must first be registered"));
        return;
    }

    int channel = findAvailableRfcommChannel(name);
    if (channel == -1) {
        qLog(Bluetooth) << "BluetoothServiceManager: no available channels"
            << "left, cannot start service" << name;

        // tell listeners about error
        ServiceListenerController controller(this);
        emit controller.serviceStarted(name, QBluetooth::NoAvailablePort,
            tr("No service channels available"));

    } else {
        qLog(Bluetooth) << "BluetoothServiceManager starting" << name
            << "on channel" << channel;

        Q_ASSERT(m_serviceCommObjects->contains(name));

        // this channel should not be assigned to other services now
        m_usedRfcommChannels.insert(channel, name);

        // set the security options before starting the service, to ensure
        // service knows what security should be used to start with
        setServiceSecurity(name, QBluetooth::SecurityOptions(
                                   serviceValue(name, "Security").toInt()));

        // tell service to start
        m_serviceCommObjects->value(name)->start(channel);

        // note that service is busy starting up
        setServiceValue(name, "ChangingState", QVariant(true));

        // Since the service has been started, set to autostart it next time.
        // This allows for autostarts across reboots
        setServiceValue(name, "Autostart", QVariant(true));
    }
}

void BluetoothServiceManager::stopService(const QString &name, bool systemCall)
{
    // request (e.g. from BTSettings) to stop a service.

    qLog(Bluetooth) << "called BluetoothServiceManager::stopService" << name;

    if (!isRegistered(name)) {
        ServiceListenerController controller(this);
        emit controller.serviceStopped(name, QBluetooth::UnknownService,
            tr("Unknown service, service must first be registered"));
        return;
    }

    qLog(Bluetooth) << "BluetoothServiceManager stopping" << name;

    // pass on message to service provider.
    Q_ASSERT(m_serviceCommObjects->contains(name));
    m_serviceCommObjects->value(name)->stop();

    // note that service is busy stopping
    setServiceValue(name, "ChangingState", QVariant(true));

    // If the user app requested that the service be stopped, don't autostart
    // it next time
    // This allows for controlling auto-starting of services across reboots
    // (and also across powering on/off the local bluetooth device)
    if (!systemCall)
        setServiceValue(name, "Autostart", QVariant(false));
}

void BluetoothServiceManager::setServiceSecurity(const QString &name,
    QBluetooth::SecurityOptions options)
{
    // request (e.g. from BTSettings) to set a service's security options.

    if (!isRegistered(name)) {
        qLog(Bluetooth) << "BluetoothServiceManager can't set security"
            << "for unknown (unregistered) service:" << name;
        return;
    }

    setServiceValue(name, "Security", QVariant(options));

    // pass on message to service provider.
    if (m_serviceCommObjects->contains(name)) {
        m_serviceCommObjects->value(name)->setSecurityOptions(options);
    }
}

void BluetoothServiceManager::initAvailableChannels()
{
    // browse for local services to note channels already in use
    connect(m_sdap, SIGNAL(searchComplete(const QSDAPSearchResult &)),
            SLOT(foundLocalServices(const QSDAPSearchResult &)));
    QBluetoothLocalDevice device;
    if (device.isValid())
        m_sdap->browseLocalServices(device);
}

void BluetoothServiceManager::initReservedChannels()
{
    // reserve the channels that each service used last time -- better that
    // services always start on the same channels if possible
    QList<QString> registeredServices = m_configSettings->childGroups();
    foreach (QString service, registeredServices)
        m_reservedRfcommChannels.insert(serviceValue(service, "Channel").toInt());
}

int BluetoothServiceManager::findAvailableRfcommChannel(const QString &name) const
{
    int channel = -1;

    // return the saved channel if there is one and it's available
    // (the saved channel should also be in the "reserved" channels)
    QVariant savedChannel = serviceValue(name, "Channel");
    if (savedChannel.isValid()) {
        channel = savedChannel.toInt();
        if (channel != -1 && !m_usedRfcommChannels.contains(channel))
            return channel;
    }

    // find an available channel
    for (int i=RFCOMM_CHANNELS_START; i<RFCOMM_CHANNELS_END; i++) {
        if (!m_usedRfcommChannels.contains(i) && !m_reservedRfcommChannels.contains(i) && !m_badRfcommChannels.contains(i))
            return i;
    }

    // as a last resort, return a channel that's normally reserved but is not
    // actually being used at the moment
    // (but this shouldn't happen since there are RFCOMM 30 channels available...)
    for (int i=0; i<m_reservedRfcommChannels.size(); i++) {
        if (!m_usedRfcommChannels.contains(i))
            return i;
    }

    return -1;
}


void BluetoothServiceManager::foundLocalServices(const QSDAPSearchResult &result)
{
    QBluetoothLocalDevice device;
    QSDP sdp;

    foreach (QSDPService service, result.services()) {

        // clear all services on start-up, so that all the channels are freed up
        sdp.unregisterService(device, service);

        // or, if you prefer to not clear all services on start-up, just
        // mark these channels as "in use"
        /*
        int channel = QSDPService::rfcommChannel(service);
        if (channel != -1)
            m_usedRfcommChannels.insert(channel);
        */
    }

    delete m_sdap;
    m_sdap = 0;
}

/*!
    Add the details for a new service to the value space.
 */
void BluetoothServiceManager::initServiceSettings(const QString &name,
    const QString &translatableName)
{
    // set only settings with no current value (e.g. if security setting was
    // saved in config, don't override it)
    QHash<QString, QVariant> defaultSettings = defaultServiceSettings();
    foreach(QString setting, defaultSettings.keys()) {
        if (serviceValue(name, setting).isNull())
            setServiceValue(name, setting, defaultSettings[setting]);
    }

    // overwrite default (null) name setting
    setServiceValue(name, "DisplayName", translatableName);
}

bool BluetoothServiceManager::isRegistered(const QString &name) const
{
    QVariant registered = serviceValue(name, "Registered");
    return (registered.isValid() && registered.toBool());
}

void BluetoothServiceManager::setServiceValue(const QString &name, const QString &attr, const QVariant &value)
{
    m_servicesValueSpace->setAttribute(name + "/" + attr, value);

    // some settings need to be saved into config for persistence
    if (persistentSettings.contains(attr))
        m_configSettings->setValue(name + "/" + attr, value);
}

QVariant BluetoothServiceManager::serviceValue(
    const QString &name, const QString &attribute) const
{
    return QValueSpaceItem(m_servicesValueSpace->objectPath() + "/"
        + name).value(attribute);
}

void BluetoothServiceManager::loadConfig()
{
    QList<QString> registeredServices = m_configSettings->childGroups();
    foreach (QString service, registeredServices) {
        // first check this is a valid service (should have a display name)
        QString displayname = m_configSettings->value(service + "/"
            + "DisplayName").toString();
        if (!displayname.isNull()) {
            // Load saved settings.

            // get the saved settings, call endGroup() cos setServiceValue()
            // assumes no group has been started
            m_configSettings->beginGroup(service);
            QStringList childKeys = m_configSettings->childKeys();
            m_configSettings->endGroup();

            for (int i=0; i<childKeys.size(); i++) {
                setServiceValue(service, childKeys[i],
                        m_configSettings->value(service + "/" + childKeys[i]));
            }

            // load default values for other settings
            initServiceSettings(service, displayname);
        }
    }
}

QTOPIA_TASK(BluetoothServiceManager, BluetoothServiceManager);
QTOPIA_TASK_PROVIDES(BluetoothServiceManager, SystemShutdownHandler);

#include "bluetoothservicemanager.moc"
