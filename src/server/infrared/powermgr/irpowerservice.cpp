/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#include "irpowerservice.h"
#include <qirlocaldevice.h>
#include <qtopialog.h>
#include "qtopiaserverapplication.h"
#include <qvaluespace.h>
#include <QByteArray>
#include <QString>
#include <QTimer>

class IrPowerService_Private
{
public:
    IrPowerService_Private(const QByteArray &devId);
    ~IrPowerService_Private();

    QIrLocalDevice *m_device;
};

IrPowerService_Private::IrPowerService_Private(const QByteArray &devId)
{
    m_device = new QIrLocalDevice(devId);
}

IrPowerService_Private::~IrPowerService_Private()
{
    delete m_device;
}

/*!
    \class IrPowerService
    \ingroup QtopiaServer
    \brief The IrPowerService class provides the Qtopia Infrared Power service.

    The \i IrPower service enables applications to notify the server
    of Infrared useage, such that the server can intelligently manage
    the infrared port for maximum power efficiency.

    The \i IrPower service is typically supplied by the Qtopia server,
    but the system integrator might change the application that
    implements this service.

    This class is part of the Qtopia server and cannot be used by other Qtopia applications.
    \sa QCommDeviceController, QCommDeviceSession
 */

/*!
    Creates the IrPowerService.  The \a serverPath contains the UNIX server socket
    path.  The \a devId contains the device id of the infrared device, e.g. irda0.
    The \a parent parameter contains the QObject parent.
*/
IrPowerService::IrPowerService(const QByteArray &serverPath,
                               const QByteArray &devId,
                               QObject *parent) :
    QAbstractCommDeviceManager(serverPath, devId, parent)
{
    m_data = new IrPowerService_Private(devId);

    qLog(Infrared) << "Infrared Power Service running";
}

/*!
    Destructor.
 */
IrPowerService::~IrPowerService()
{
    if (m_data)
        delete m_data;
}

/*!
    \reimp
*/
void IrPowerService::bringUp()
{
    bool res = m_data->m_device->bringUp();

    if (res)
        emit upStatus(false, QString());
    else
        emit upStatus(true, tr("Could not bring up infrared device"));
}

/*!
    \reimp
*/
void IrPowerService::bringDown()
{
    bool res = m_data->m_device->bringDown();

    if (res)
        emit downStatus(false, QString());
    else
        emit downStatus(true, tr("Could not bring down infrared device"));
}

/*!
    \reimp
*/
bool IrPowerService::isUp() const
{
    return m_data->m_device->isUp();
}

/*!
    \reimp
*/
bool IrPowerService::shouldBringDown(QUnixSocket *) const
{
    if (!sessionsActive())
        return true;

    QMessageBox *box = new QMessageBox(QObject::tr("Turn off Infrared?"),
                                       QObject::tr("<P>There are applications using the infrared device.  Are you sure you want to turn it off?"),
                                       QMessageBox::Question,
                                       QMessageBox::Yes|QMessageBox::Default,
                                       QMessageBox::No|QMessageBox::Escape,
                                       QMessageBox::NoButton);
    int result = QtopiaApplication::execDialog(box);

    if (result == QMessageBox::No) {
    qLog(Infrared) << "User doesn't want to shut down the device..";
    return false;
    }
    else {
        qLog(Infrared) << "User wants to shut down the device..";
        return true;
    }
}


/*!
  \class IrPowerServiceTask
  \ingroup QtopiaServer::Task
  \brief The IrPowerServiceTask class provides the IrPowerService as a server task.

    The IrPowerServiceTask manages the lifetime of an IrPowerService object.
  
    This class is part of the Qtopia server and cannot be used by other Qtopia applications.
 */

/*!
    Creates the task.  The \a parent paramter holds the QObject parent.
*/
IrPowerServiceTask::IrPowerServiceTask(QObject *parent)
: QObject(parent), m_irPower(0)
{
    QStringList irDevices = QIrLocalDevice::devices();
    m_valueSpace = new QValueSpaceObject("/Communications/Infrared/");

    if(irDevices.size() > 0) {
        QByteArray path( (Qtopia::tempDir()+"ir_power_"+irDevices[0]).toLocal8Bit() );

        m_irPower = new IrPowerService(path, irDevices[0].toLatin1(), this);
        m_irPower->start();
        m_valueSpace->setAttribute("Available", true);
    }
    else {
        m_valueSpace->setAttribute("Available", false);
    }
}

/*!
    Destructor.
*/
IrPowerServiceTask::~IrPowerServiceTask()
{
    if (m_irPower) {
        m_irPower->stop();
        delete m_irPower;
        m_irPower = 0;
    }
    delete m_valueSpace;
}

QTOPIA_TASK(IrPowerService, IrPowerServiceTask);

