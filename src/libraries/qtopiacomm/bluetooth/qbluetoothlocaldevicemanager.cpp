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

#include <qbluetoothlocaldevicemanager.h>
#include <qtopialog.h>

#include <qtdbus/qdbusargument.h>
#include <qtdbus/qdbusconnection.h>
#include <qtdbus/qdbusinterface.h>
#include <qtdbus/qdbusreply.h>
#include <qtdbus/qdbusmessage.h>

class QBluetoothLocalDeviceManager_Private : public QObject
{
    Q_OBJECT
public:
    QBluetoothLocalDeviceManager_Private(QBluetoothLocalDeviceManager *parent);

public slots:
    void deviceAdded(const QString &device);
    void deviceRemoved(const QString &device);

public:
    QBluetoothLocalDeviceManager *m_parent;
    QDBusInterface *m_iface;
};

QBluetoothLocalDeviceManager_Private::QBluetoothLocalDeviceManager_Private(
        QBluetoothLocalDeviceManager *parent) : QObject(parent), m_parent(parent)
{
    QDBusConnection dbc = QDBusConnection::systemBus();
    if (!dbc.isConnected()) {
        qWarning() << "Unable to connect do D-BUS:" << dbc.lastError();
        return;
    }

    m_iface = new QDBusInterface("org.bluez", "/org/bluez",
                                 "org.bluez.Manager", dbc);
    if (!m_iface->isValid()) {
        qWarning() << "Could not find org.bluez interface";
        return;
    }

    dbc.connect("org.bluez", "/org/bluez", "org.bluez.Manager", "AdapterAdded",
                this, SIGNAL(deviceAdded(const QString &)));
    dbc.connect("org.bluez", "/org/bluez", "org.bluez.Manager", "AdapterRemoved",
                this, SIGNAL(deviceRemoved(const QString &)));
}

void QBluetoothLocalDeviceManager_Private::deviceAdded(const QString &device)
{
    emit m_parent->deviceAdded(device.mid(11));
}

void QBluetoothLocalDeviceManager_Private::deviceRemoved(const QString &device)
{
    emit m_parent->deviceRemoved(device.mid(11));
}


/*!
  \class QBluetoothLocalDeviceManager
  \brief The QBluetoothLocalDeviceManager class provides access to local Bluetooth devices.


*/

/*!
    Constructs the device manager for local Bluetooth devices.  The
    \a parent parameter specifies the parent.
  */
QBluetoothLocalDeviceManager::QBluetoothLocalDeviceManager(QObject *parent) :
    QObject(parent), m_data(0)
{
    m_data = new QBluetoothLocalDeviceManager_Private(this);
}

/*!
    Destructor.
*/
QBluetoothLocalDeviceManager::~QBluetoothLocalDeviceManager()
{
    if (m_data)
        delete m_data;
}

/*!
    Returns a list of all Bluetooth devices found on the system.  The list
    typically consists of hci0, hci1, ..., hciN, depending on the number of
    devices.
 */
QStringList QBluetoothLocalDeviceManager::devices()
{
    QStringList ret;

    if (!m_data->m_iface->isValid())
        return ret;

    QDBusReply<QStringList> reply = m_data->m_iface->call("ListAdapters");

    if (!reply.isValid()) {
        return ret;
    }

    foreach (QString device, reply.value()) {
        ret.push_back(device.mid(11));
    }

    return ret;
}

/*!
    Returns a device name of the default Bluetooth adapter.  This is
    typically 'hci0'.  Returns a null string if no device is installed.
 */
QString QBluetoothLocalDeviceManager::defaultDevice()
{
    if (!m_data->m_iface->isValid())
        return QString();

    QDBusReply<QString> reply = m_data->m_iface->call("DefaultAdapter");

    if (!reply.isValid()) {
        return QString();
    }

    return reply.value().mid(11);
}

/*!
    \fn void QBluetoothLocalDeviceManager::deviceAdded(const QString &device)

    This signal is emitted whenever a new device has been added to the system.
    The \a device parameter contains the representation of the device which can be
    passed to the QBluetoothLocalDevice constructor.

    \sa QBluetoothLocalDevice
 */

/*!
    \fn void QBluetoothLocalDeviceManager::deviceRemoved(const QString &device)

    This signal is emitted whenever a device has been removed from the system.
    The \a device parameter contains the representation of the device which
    was removed.
 */

#include "qbluetoothlocaldevicemanager.moc"
