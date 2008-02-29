/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef __QBLUETOOTHGLOBAL_P_H__
#define __QBLUETOOTHGLOBAL_P_H__

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qbluetoothnamespace.h>

#ifdef QTOPIA_BLUETOOTH
#include <QIcon>
#endif

class QString;
class QStringList;
class QBluetoothRemoteDevice;

QString convertDeviceMajorToString(QBluetooth::DeviceMajor dev_major);
QString convertDeviceMinorToString(QBluetooth::DeviceMajor major, qint8 minor);
QStringList convertServiceClassesToString(QBluetooth::ServiceClasses classes);
QBluetooth::DeviceMajor major_to_device_major(quint8 major);

//TODO: Need a better solution for this.  I think eventually
//We will need a QBluetoothGui library/namespace.
#ifdef QTOPIA_BLUETOOTH
QBLUETOOTH_EXPORT QIcon find_device_icon(const QBluetoothRemoteDevice &remote);
QBLUETOOTH_EXPORT QIcon find_device_icon(QBluetooth::DeviceMajor major, quint8 minor, QBluetooth::ServiceClasses serviceClasses);
#endif

// NOTE This header is from GPLed library, but only defines interfaces
// According to local GPL experts this should be fine
#include <bluetooth/bluetooth.h>
#include <QString>

void str2bdaddr(const QString &addr, bdaddr_t *bdaddr);
QString bdaddr2str(const bdaddr_t *bdaddr);
bool _q_getSecurityOptions(int sockfd, QBluetooth::SecurityOptions &options);
bool _q_setSecurityOptions(int sockfd, QBluetooth::SecurityOptions options);
bool _q_getL2CapSecurityOptions(int sockfd, QBluetooth::SecurityOptions &options);
bool _q_setL2CapSecurityOptions(int sockfd, QBluetooth::SecurityOptions options);

#endif
