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

#include <qsdp.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothaddress.h>
#include <qsdpservice.h>
#include <qtopiacomm/private/qbluetoothnamespace_p.h>
#include <qtopianamespace.h>
#include <qtopialog.h>

#include <QString>
#include <QStringList>
#include <QProcess>

class QSDPPrivate
{
public:
    QProcess *m_process;
    QString m_error;
};

/*!
    \class QSDP
    \brief The QSDP class allows interaction with the local SDP server.

The QSDP class provides access to the local SDP server.  It allows registration and
de-registration of Bluetooth services.

    \ingroup qtopiabluetooth
    \sa QSDAP, QSDPService
 */

/*!
    Constructs a new QSDP object.
 */
QSDP::QSDP()
{
    m_data = new QSDPPrivate();
    m_data->m_process = new QProcess();
}

/*!
    Deconstructs a QSDP object.
*/
QSDP::~QSDP()
{
    if (m_data) {
        delete m_data->m_process;
        delete m_data;
    }
}

/*
    Tries to register a service with the local sdpd
    Please note that currently the BlueZ framework does not
    allow services to differ between local interfaces.  While
    there is some support for this inside the API, I do not
    believe it is functional as of bluez-2.19.  Thus for devices
    with multiple interfaces, registerService will register
    the service for all local devices, or at least it will appear
    so during a local scan.
*/

/*!
    Registers a profile with the local SDP server.  If \a channel
    is not provided, a unique channel is picked.  The local device
    for which this service is registered is given by
    the \a device parameter.  The profile to register with the
    server is given by the \a profile parameter.

    This method returns true if the service was registered
    successfully, false otherwise.
*/
bool QSDP::registerService(const QBluetoothLocalDevice &device,
                           QBluetooth::SDPProfile profile, int channel)
{
    QString bluezName = map_sdp_profile_to_bluez_name(profile);
    if (bluezName.isNull()) {
        m_data->m_error = "Unknown protocol";
        return false;
    }

    QStringList args;
    args << "-i" << device.deviceName() << "add";
    if (channel != -1)
        args << "--channel" << QString::number(channel);
    args << bluezName;

    QString sdptool = find_sdptool();
    qLog(Bluetooth) << "Starting sdptool in" << sdptool;

    m_data->m_process->start(sdptool, args);
    m_data->m_process->waitForFinished(-1);

    if ((m_data->m_process->exitStatus() != QProcess::QProcess::NormalExit) ||
         (m_data->m_process->exitCode() != 0)) {
        qLog(Bluetooth) << "Running sdptool failed";
        return false;
    }

    return true;
}

/*!
    Unregisters a service from the local SDP server.  The device
    to unregister the service from is given by \a device.  The
    service to unregister is given by \a service. Note that
    the service needs to be a valid Bluetooth service that is
    currently registered on the local SDP server, e.g.
    obtained by using \c QSDAP::browseLocal().

    This method returns true if the de-registration was
    completed successfully, false otherwise.
*/
bool QSDP::unregisterService(const QBluetoothLocalDevice &device,
                             const QSDPService &service)
{
    QStringList args;
    args << "-i" << device.deviceName() << "del";

    QString number = QString::number(service.recordHandle(), 16);
    QString arg("0x");
    arg.append(number);
    args << arg;

    QString sdptool = find_sdptool();
    qLog(Bluetooth) << "Starting sdptool in" << sdptool;

    m_data->m_process->start(sdptool, args);
    m_data->m_process->waitForFinished(-1);

    if ((m_data->m_process->exitStatus() != QProcess::QProcess::NormalExit) ||
         (m_data->m_process->exitCode() != 0))
    {
        qLog(Bluetooth) << "Running sdptool failed";
        return false;
    }

    return true;
}

/*!
    This method returns the last error that occurred.
*/
QString QSDP::error()
{
    return QString(m_data->m_process->readAllStandardError());
}
