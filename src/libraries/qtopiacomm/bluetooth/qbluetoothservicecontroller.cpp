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

#include "qbluetoothservicecontroller.h"
#include <qtopialog.h>

#include <QList>
#include <QtopiaIpcAdaptor>
#include <QValueSpaceItem>

class QBluetoothServiceController_Private : public QtopiaIpcAdaptor
{
    friend class QBluetoothServiceController;
    Q_OBJECT

public:
    QBluetoothServiceController_Private(QBluetoothServiceController *parent);
    ~QBluetoothServiceController_Private();

    void start(const QString &name);
    void stop(const QString &name);
    QBluetoothServiceController::ServiceState state(const QString &name) const;

    void setSecurityOptions(const QString &name,
                            QBluetooth::SecurityOptions options);
    QBluetooth::SecurityOptions securityOptions(const QString &name);

    QString translatableDisplayName(const QString &name);
    QList<QString> registeredServices();
    bool isRegistered(const QString &name) const;

public slots:
    void serviceStarted(const QString &name, QBluetooth::ServiceError error, const QString &errorDesc);
    void serviceStopped(const QString &name, QBluetooth::ServiceError error, const QString &errorDesc);

    void serviceError(const QString &name,
                      QBluetooth::ServiceError error,
                      const QString &errorDesc);
signals:
    void startService(const QString &name);
    void stopService(const QString &name);
    void setServiceSecurity(const QString &name, QBluetooth::SecurityOptions);

private:
    QVariant serviceValue(const QString &name, const QString &attr) const;

    static const QString VALUE_SPACE_PATH;
    QBluetoothServiceController *m_parent;
};


/*
    This receives IPC messages from the BluetoothServiceManager when a service
    is registered, modified, etc. It's enabled the "other end" of the messages
    to/from ServiceListenerController in src/server/bluetoothservicemanager.cpp.
 */

const QString QBluetoothServiceController_Private::VALUE_SPACE_PATH = "Communications/Bluetooth/Services";

QBluetoothServiceController_Private::QBluetoothServiceController_Private(QBluetoothServiceController *parent)
    : QtopiaIpcAdaptor("QPE/BluetoothServiceListeners", parent),
      m_parent(parent)
{
    publishAll(SignalsAndSlots);
}

QBluetoothServiceController_Private::~QBluetoothServiceController_Private()
{
}

void QBluetoothServiceController_Private::start(const QString &name)
{
    emit startService(name);
}

void QBluetoothServiceController_Private::stop(const QString &name)
{
    emit stopService(name);
}

QBluetoothServiceController::ServiceState QBluetoothServiceController_Private::state(const QString &name) const
{
    bool started = serviceValue(name, "Enabled").toBool();

    if (serviceValue(name, "ChangingState").toBool()) {
        return ( started ? QBluetoothServiceController::Stopping :
                    QBluetoothServiceController::Starting );
    } else {
        return ( started ? QBluetoothServiceController::Started :
                    QBluetoothServiceController::Stopped );
    }
}

void QBluetoothServiceController_Private::setSecurityOptions(const QString &name, QBluetooth::SecurityOptions options)
{
    emit setServiceSecurity(name, options);
}

QBluetooth::SecurityOptions QBluetoothServiceController_Private::securityOptions(const QString &name)
{
    QVariant options = serviceValue(name, "Security");
    if (options.isValid())
        return static_cast<QBluetooth::SecurityOptions>(options.toInt());

    qLog(Obex) << "QBluetoothServiceController::securityOptions() no valid security options for service:" << name;
    return 0;
}

QString QBluetoothServiceController_Private::translatableDisplayName(const QString &name)
{
    return serviceValue(name, "DisplayName").toString();
}

QList<QString> QBluetoothServiceController_Private::registeredServices()
{
    QList<QString> services;
    foreach (QString name, QValueSpaceItem(VALUE_SPACE_PATH).subPaths()) {
        if (isRegistered(name))
            services.append(name);
    }
    return services;
}

bool QBluetoothServiceController_Private::isRegistered(const QString &name) const
{
    QVariant registered = serviceValue(name, "Registered");
    return (registered.isValid() && registered.toBool());
}

void QBluetoothServiceController_Private::serviceStarted(const QString &name, QBluetooth::ServiceError error, const QString &errorDesc)
{
    emit m_parent->started(name, error, errorDesc);
}

void QBluetoothServiceController_Private::serviceStopped(const QString &name, QBluetooth::ServiceError error, const QString &errorDesc)
{
    emit m_parent->stopped(name, error, errorDesc);
}

void QBluetoothServiceController_Private::serviceError(const QString &name,
                                QBluetooth::ServiceError error,
                                const QString &errorDesc)
{
    emit m_parent->error(name, error, errorDesc);
}

QVariant QBluetoothServiceController_Private::serviceValue(const QString &name, const QString &attr) const
{
    return QValueSpaceItem(VALUE_SPACE_PATH + "/" + name).value(attr);
}


/*!
    \class QBluetoothServiceController
    \brief The QBluetoothServiceController class provides access to information about Bluetooth services within the Qtopia Bluetooth services framework.

    The QBluetoothServiceController class provides access to information about
    Bluetooth services within the Qtopia Bluetooth services framework (i.e.
    services that are subclasses of QBluetoothAbstractService). It allows the
    settings for these services to be set and retrieved, and emits signals when
    any settings are changed.

    \ingroup qtopiabluetooth
 */

/*!
    \enum QBluetoothServiceController::ServiceState

    Defines the service state of the service.

    \value Stopped The service is stopped.
    \value Started The service is started.
    \value Stopping The service is transitioning from the Started to Stopped state.
    \value Starting The service is transitioning from the Stopped to Started state.
*/

/*!
    Constructs a QBluetoothServiceController with the parent object \a parent.
 */
QBluetoothServiceController::QBluetoothServiceController(QObject *parent)
    : QObject(parent),
      m_private(new QBluetoothServiceController_Private(this))
{
}

/*!
    Deconstructs a QBluetoothServiceController.
 */
QBluetoothServiceController::~QBluetoothServiceController()
{
}

/*!
    Starts the service with name \a name.
 */
void QBluetoothServiceController::start(const QString &name)
{
    m_private->start(name);
}

/*!
    Stops the service with name \a name.
 */
void QBluetoothServiceController::stop(const QString &name)
{
    m_private->stop(name);
}

/*!
    Returns the state of the service \a name.
 */
QBluetoothServiceController::ServiceState QBluetoothServiceController::state(const QString &name) const
{
    return m_private->state(name);
}

/*!
    Sets the security options for the service with name \a name to the given
    \a options.
 */
void QBluetoothServiceController::setSecurityOptions(const QString &name, QBluetooth::SecurityOptions options)
{
    m_private->setSecurityOptions(name, options);
}

/*!
    Returns the security options for the service with name \a name.
 */
QBluetooth::SecurityOptions QBluetoothServiceController::securityOptions(const QString &name) const
{
    return m_private->securityOptions(name);
}

/*!
    Returns the translatable display name for the service with name \a name.
 */
QString QBluetoothServiceController::translatableDisplayName(const QString &name) const
{
    return m_private->translatableDisplayName(name);
}

/*!
    Returns a list of the names of all known services within the Qtopia
    Bluetooth framework.
 */
QList<QString> QBluetoothServiceController::registeredServices() const
{
    return m_private->registeredServices();
}

/*!
    Returns whether a service with the name \a name exists within the Qtopia
    Bluetooth framework.
 */
bool QBluetoothServiceController::isRegistered(const QString &name) const
{
    return m_private->isRegistered(name);
}

/*!
    \fn void QBluetoothServiceController::started(const QString &name, QBluetooth::ServiceError error, const QString &errorDesc)

    This signal is emitted when the service named \a name has started or
    failed while attempting to start. If there was a failure, the error details
    are provided with the \a error identifier and \a errorDesc description.
 */

/*!
    \fn void QBluetoothServiceController::stopped(const QString &name, QBluetooth::ServiceError error, const QString &errorDesc)

    This signal is emitted when the service named \a name has stopped or
    failed while attempting to stop. If there was a failure, the error details
    are provided with the \a error identifier and \a errorDesc description.
 */

/*!
    \fn void QBluetoothServiceController::error(const QString &name, QBluetooth::ServiceError error, const QString &errorDesc)

    This signal is emitted when the service named \a name has encountered a
    fatal error while running. The error details are provided with the \a error
    identifier and \a errorDesc description.
 */

#include "qbluetoothservicecontroller.moc"
