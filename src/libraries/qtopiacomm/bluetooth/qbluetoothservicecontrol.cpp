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

#include <qtopiacomm/qbluetoothservicecontrol.h>

/*!
    \internal
    \class QBluetoothServiceControl
    \brief The QBluetoothServiceControl class allows a Bluetooth service to be controlled through an IPC interface using the IPC mechanisms in the Qtopia framework.

    This class is used by the Bluetooth Service Manager in the server to
    control and manage system Bluetooth services, and should not be used by
    external sources. If an external source needs to control a Bluetooth
    service it should use the QBluetoothServiceController class.
 */

const QString QBluetoothServiceControl::KEY_DISPLAY_NAME = "DisplayName";

QBluetoothServiceControl::QBluetoothServiceControl(const QString &serviceName, QObject *parent, QCommInterface::Mode mode)
    : QCommInterface("QBluetoothServiceControl", serviceName, parent, mode)
{
    proxyAll(staticMetaObject);
}

QBluetoothServiceControl::~QBluetoothServiceControl()
{
}

QString QBluetoothServiceControl::translatableDisplayName() const
{
    return value(KEY_DISPLAY_NAME).toString();
}

void QBluetoothServiceControl::start(int channel)
{
    invoke(SLOT(start(int)), qVariantFromValue(channel));
}

void QBluetoothServiceControl::stop()
{
    invoke(SLOT(stop()));
}

void QBluetoothServiceControl::setSecurityOptions(QBluetooth::SecurityOptions options)
{
    invoke(SLOT(setSecurityOptions(QBluetooth::SecurityOptions)), qVariantFromValue(options));
}
