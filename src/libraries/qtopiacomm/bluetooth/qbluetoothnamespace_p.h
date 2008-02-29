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

#ifndef __QBLUETOOTHGLOBAL_P_H__
#define __QBLUETOOTHGLOBAL_P_H__

#include <qbluetoothnamespace.h>

class QString;
class QStringList;

QString map_sdp_profile_to_bluez_name(QBluetooth::SDPProfile profile);

QString convertDeviceMajorToString(QBluetooth::DeviceMajor dev_major);
QString convertDeviceMinorToString(QBluetooth::DeviceMajor major, qint8 minor);
QStringList convertServiceClassesToString(QBluetooth::ServiceClasses classes);
QBluetooth::DeviceMajor major_to_device_major(quint8 major);

// NOTE This header is from GPLed library, but only defines interfaces
// According to local GPL experts this should be fine
#include <bluetooth/bluetooth.h>
#include <QString>

void str2bdaddr(const QString &addr, bdaddr_t *bdaddr);
QString bdaddr2str(const bdaddr_t *bdaddr);
bool _q_getSecurityOptions(int sockfd, QBluetooth::SecurityOptions &options);
bool _q_setSecurityOptions(int sockfd, QBluetooth::SecurityOptions options);

QString find_sdptool();

#endif
