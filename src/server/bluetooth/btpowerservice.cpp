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

#include "btpowerservice.h"
#include <qbluetoothlocaldevicemanager.h>
#include <qtopialog.h>
#include "qtopiaserverapplication.h"
#include <QByteArray>
#include <QString>
#include <QTimer>
#include <QObject>
#include <QPhoneProfileManager>

#include <qtopiacomm/qbluetoothaddress.h>

class BtPowerServicePrivate
{
public:
    BtPowerServicePrivate(const QByteArray &devId);
    ~BtPowerServicePrivate();

    QBluetoothLocalDevice *m_device;
    QPhoneProfileManager *m_phoneProfileMgr;
    bool upRequest;
};

BtPowerServicePrivate::BtPowerServicePrivate(const QByteArray &devId)
{
    m_device = new QBluetoothLocalDevice(devId);
    qLog(Bluetooth) << "BtPowerServicePrivate: Created local device:"
        << devId << m_device->address().toString();

    m_phoneProfileMgr = new QPhoneProfileManager;
}

BtPowerServicePrivate::~BtPowerServicePrivate()
{
    delete m_device;
    delete m_phoneProfileMgr;
}

/*!
    \class BtPowerService
    \ingroup QtopiaServer
    \brief The BtPowerService class provides the Qtopia Bluetooth Power service.

    The \i BtPower service enables applications to notify the server
    of Bluetooth device useage, such that the server can intelligently
    manage the bluetooth device for maximum power efficiency.

    The \i BtPower service is typically supplied by the Qtopia server,
    but the system integrator might change the application that
    implements this service.
 */

/*!
    \internal
 */
BtPowerService::BtPowerService(const QByteArray &serverPath,
                               const QByteArray &devId, QObject *parent)
    : QAbstractCommDeviceManager(serverPath, devId, parent)
{
    m_data = new BtPowerServicePrivate(devId);

    qLog(Bluetooth) << "Bluetooth Power Service created";

    connect(m_data->m_device, SIGNAL(stateChanged(QBluetoothLocalDevice::State)),
            this, SLOT(stateChanged(QBluetoothLocalDevice::State)));
    connect(m_data->m_device, SIGNAL(error(QBluetoothLocalDevice::Error,const QString&)),
            this, SLOT(error(QBluetoothLocalDevice::Error,const QString&)));

    connect(m_data->m_phoneProfileMgr, SIGNAL(planeModeChanged(bool)),
            this, SLOT(planeModeChanged(bool)));
}

/*!
    \internal
 */
BtPowerService::~BtPowerService()
{
    if (m_data)
        delete m_data;
}

/*!
    \internal
 */
void BtPowerService::initialize()
{
    // ensure the service is down if plane mode is on
    if (m_data->m_phoneProfileMgr->planeMode())
        bringDown();
}

/*!
    Brings up the device.
*/
void BtPowerService::bringUp()
{
    bool res = m_data->m_device->setConnectable();

    m_data->upRequest = true;

    if (!res)
        emit upStatus(true, tr("Could not bring up bluetooth device"));
}

/*!
    Brings down the device.
*/
void BtPowerService::bringDown()
{
    bool res = m_data->m_device->turnOff();

    m_data->upRequest = false;

    if (!res)
        emit downStatus(true, tr("Could not bring down bluetooth device"));
}

/*!
    Returns whether the device is up.
*/
bool BtPowerService::isUp() const
{
    return m_data->m_device->isUp();
}

/*!
    \internal
*/
void BtPowerService::stateChanged(QBluetoothLocalDevice::State state)
{
    if ( (state == QBluetoothLocalDevice::Connectable) ||
         (state == QBluetoothLocalDevice::Discoverable)) {
        emit upStatus(false, QString());
    }
    else {
        emit downStatus(false, QString());
    }
}

/*!
    \internal
*/
void BtPowerService::error(QBluetoothLocalDevice::Error, const QString& /*msg*/)
{
    if (m_data->upRequest) {
        emit upStatus(true, tr("Could not bring up bluetooth device"));
    }
    else {
        emit downStatus(true, tr("Could not bring down bluetooth device"));
    }
}

/*!
    \internal
*/
void BtPowerService::planeModeChanged(bool enabled)
{
    // switch the device off if plane mode is switched on, and vice-versa
    if (enabled) {
        bringDown();
    } else {
        bringUp();
    }
}

/*!
    \internal
*/
bool BtPowerService::shouldBringDown(QUnixSocket *) const
{
    if (!sessionsActive())
        return true;

    QMessageBox *box = new QMessageBox(QObject::tr("Turn off Bluetooth?"),
                                       QObject::tr("<P>There are applications using the bluetooth device.  Are you sure you want to turn it off?"),
                                       QMessageBox::Question,
                                       QMessageBox::Yes|QMessageBox::Default,
                                       QMessageBox::No|QMessageBox::Escape,
                                       QMessageBox::NoButton);
#ifdef QTOPIA_PHONE
    int result = QtopiaApplication::execDialog(box);
#else
    int result = box->exec();
#endif

    if (result == QMessageBox::No) {
        qLog(Bluetooth) << "User doesn't want to shut down the device..";
        return false;
    }
    else {
        qLog(Bluetooth) << "User wants to shut down the device..";
        return true;
    }
}

/*!
  \class BtPowerServiceTask
  \ingroup QtopiaServer::Task
  \brief The BtPowerServiceTask class provides the BtPowerService.
 */

/*!
    \internal
*/
BtPowerServiceTask::BtPowerServiceTask(QObject *parent)
    : QObject(parent), m_btPower(0)
{
    QBluetoothLocalDeviceManager *mgr = new QBluetoothLocalDeviceManager(this);

    // get notifications when a local device is added or removed
    connect(mgr, SIGNAL(deviceAdded(const QString &)),
            SLOT(deviceAdded(const QString &)));
    connect(mgr, SIGNAL(deviceRemoved(const QString &)),
            SLOT(deviceRemoved(const QString &)));

    startService();
}

/*!
    \internal
*/
BtPowerServiceTask::~BtPowerServiceTask()
{
    if (m_btPower) {
        m_btPower->stop();
        delete m_btPower;
        m_btPower = 0;
    }
}

/*!
    \internal
*/
void BtPowerServiceTask::deviceAdded(const QString &devName)
{
    qLog(Bluetooth) << "BtPowerServiceTask::deviceAdded" << devName;

    if (!m_btPower)
        QTimer::singleShot(200, this, SLOT(startService()));
}

/*!
    \internal
*/
void BtPowerServiceTask::deviceRemoved(const QString &devName)
{
    qLog(Bluetooth) << "BtPowerServiceTask::deviceRemoved" << devName;

    // stop the power service if its device has been removed
    if (m_btPower && m_btPower->deviceId() == devName && m_btPower->isStarted()) {
        m_btPower->stop();
        delete m_btPower;
        m_btPower = 0;
    }
}

/*!
    \internal
*/
void BtPowerServiceTask::startService()
{
    qLog(Bluetooth) << "BtPowerServiceTask::startService";

    if (!m_btPower) {
        QBluetoothLocalDeviceManager manager;
        QString devName = manager.defaultDevice();
        if (devName.isNull()) {
            qLog(Bluetooth) << "BtPowerServiceTask: cannot start BtPowerService, no local device available";
            return;
        }

        qLog(Bluetooth) << "BtPowerServiceTask: creating btpowerservice...";
        QByteArray path( (Qtopia::tempDir()+"bt_power_"+devName).toLocal8Bit() );

        m_btPower = new BtPowerService(path, devName.toLatin1(), this);
        m_btPower->initialize();
        m_btPower->start();
    }
}

QTOPIA_TASK(BtPowerService, BtPowerServiceTask);
