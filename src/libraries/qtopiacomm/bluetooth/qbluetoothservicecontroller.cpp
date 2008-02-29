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

#include <QtopiaIpcAdaptor>
#include <QValueSpaceItem>
#include <qtopialog.h>

/*
    \internal
    \class QBluetoothServiceControllerPrivate

    This receives IPC messages from the BluetoothServiceManager when a service
    is registered, modified, etc. It's enabled the "other end" of the messages
    to and from ServiceUserMessenger in src/server/bluetoothservicemanager.cpp.
 */

class QBluetoothServiceControllerPrivate : public QtopiaIpcAdaptor
{
    friend class QBluetoothServiceController;
    Q_OBJECT

public:
    QBluetoothServiceControllerPrivate(QBluetoothServiceController *parent);
    ~QBluetoothServiceControllerPrivate();

    void start(const QString &name);
    void stop(const QString &name);
    QBluetoothServiceController::ServiceState state(const QString &name) const;

    void setSecurityOptions(const QString &name,
                            QBluetooth::SecurityOptions options);
    QBluetooth::SecurityOptions securityOptions(const QString &name);

    QString displayName(const QString &name);
    QStringList services();

public slots:
    void serviceStarted(const QString &name, bool error, const QString &desc);
    void serviceStopped(const QString &name);

signals:
    void startService(const QString &name);
    void stopService(const QString &name);
    void setServiceSecurity(const QString &name, QBluetooth::SecurityOptions);

private:
    QVariant serviceValue(const QString &name, const QString &attr) const;

    static const QString VALUE_SPACE_PATH;
    QBluetoothServiceController *m_parent;
};



const QString QBluetoothServiceControllerPrivate::VALUE_SPACE_PATH = "Communications/Bluetooth/Services";

QBluetoothServiceControllerPrivate::QBluetoothServiceControllerPrivate(QBluetoothServiceController *parent)
    : QtopiaIpcAdaptor("QPE/BluetoothServiceListeners", parent),
      m_parent(parent)
{
    publishAll(SignalsAndSlots);
}

QBluetoothServiceControllerPrivate::~QBluetoothServiceControllerPrivate()
{
}

void QBluetoothServiceControllerPrivate::start(const QString &name)
{
    emit startService(name);
}

void QBluetoothServiceControllerPrivate::stop(const QString &name)
{
    emit stopService(name);
}

QBluetoothServiceController::ServiceState QBluetoothServiceControllerPrivate::state(const QString &name) const
{
    return QBluetoothServiceController::ServiceState(
                    serviceValue(name, "State").toInt());
}

void QBluetoothServiceControllerPrivate::setSecurityOptions(const QString &name, QBluetooth::SecurityOptions options)
{
    emit setServiceSecurity(name, options);
}

QBluetooth::SecurityOptions QBluetoothServiceControllerPrivate::securityOptions(const QString &name)
{
    QVariant options = serviceValue(name, "Security");
    if (options.isValid())
        return static_cast<QBluetooth::SecurityOptions>(options.toInt());

    qLog(Obex) << "QBluetoothServiceController::securityOptions() no valid security options for service:" << name;
    return 0;
}

QString QBluetoothServiceControllerPrivate::displayName(const QString &name)
{
    return serviceValue(name, "DisplayName").toString();
}

QStringList QBluetoothServiceControllerPrivate::services()
{
    return QValueSpaceItem(VALUE_SPACE_PATH).subPaths();
}

void QBluetoothServiceControllerPrivate::serviceStarted(const QString &name, bool error, const QString &desc)
{
    emit m_parent->started(name, error, desc);
}

void QBluetoothServiceControllerPrivate::serviceStopped(const QString &name)
{
    emit m_parent->stopped(name);
}

QVariant QBluetoothServiceControllerPrivate::serviceValue(const QString &name, const QString &attr) const
{
    return QValueSpaceItem(VALUE_SPACE_PATH + "/" + name).value(attr);
}


/*!
    \class QBluetoothServiceController
    \brief The QBluetoothServiceController class provides a means to control and access information about Bluetooth services in Qtopia.

    To create a Bluetooth service that can be accessed by instances of
    QBluetoothServiceController, simply subclass QBluetoothAbstractService.

    \sa QBluetoothAbstractService
    \ingroup qtopiabluetooth
 */

/*!
    \enum QBluetoothServiceController::ServiceState

    Defines the service state of the service.

    \value NotRunning The service is not running.
    \value Starting The service is starting.
    \value Running The service is running.
*/

/*!
    Constructs a QBluetoothServiceController with the parent object \a parent.
 */
QBluetoothServiceController::QBluetoothServiceController(QObject *parent)
    : QObject(parent),
      m_data(new QBluetoothServiceControllerPrivate(this))
{
}

/*!
    Destroys a QBluetoothServiceController.
 */
QBluetoothServiceController::~QBluetoothServiceController()
{
}

/*!
    Starts the service named \a name.
 */
void QBluetoothServiceController::start(const QString &name)
{
    m_data->start(name);
}

/*!
    Stops the service named \a name.
 */
void QBluetoothServiceController::stop(const QString &name)
{
    m_data->stop(name);
}

/*!
    Returns the state of the service named \a name.
 */
QBluetoothServiceController::ServiceState QBluetoothServiceController::state(const QString &name) const
{
    return m_data->state(name);
}

/*!
    Sets the security options for the service with name \a name to the given
    \a options.
 */
void QBluetoothServiceController::setSecurityOptions(const QString &name, QBluetooth::SecurityOptions options)
{
    m_data->setSecurityOptions(name, options);
}

/*!
    Returns the security options for the service named \a name.
 */
QBluetooth::SecurityOptions QBluetoothServiceController::securityOptions(const QString &name) const
{
    return m_data->securityOptions(name);
}

/*!
    Returns the user-friendly, internationalized display name for the service
    named \a name.
 */
QString QBluetoothServiceController::displayName(const QString &name) const
{
    return m_data->displayName(name);
}

/*!
    Returns a list of the names of all known Bluetooth services within Qtopia.
 */
QStringList QBluetoothServiceController::services() const
{
    return m_data->services();
}


/*!
    \fn void QBluetoothServiceController::started(const QString &name, bool error, const QString &description)

    This signal is emitted when the service named \a name has started or
    failed while attempting to start. If there was a failure, \a error is \c true
    and \a description provides the human-readable error description.
 */

/*!
    \fn void QBluetoothServiceController::stopped(const QString &name)

    This signal is emitted when the service named \a name has stopped.
*/


#include "qbluetoothservicecontroller.moc"
