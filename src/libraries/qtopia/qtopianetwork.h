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
#ifndef QTOPIA_NETWORKING_H
#define QTOPIA_NETWORKING_H

#include <qtopiaglobal.h>
#include <QString>
#include <QPointer>
#include <QVariant>

class QStringList;
class QtopiaNetworkInterface;
class QtopiaNetworkProperties;

class QTOPIA_EXPORT QtopiaNetwork
{
public:
    enum TypeFlag
    {
        LAN             = 0x00000001,    //network of type LAN
        WirelessLAN     = 0x00000002,    //network of type WLAN


        Dialup          = 0x00000004,    //analog dialup
        GPRS            = 0x00000008,    //GPRS/UMTS/EDGA dialup
#ifdef QTOPIA_CELL
        PhoneModem      = 0x00000010,    //network device builtin into the device (Phone Edition only)
#endif
        NamedModem      = 0x00000020,    //network device is named e.g. /dev/ttyS0
        PCMCIA          = 0x00000040,    //network device is a PCMCIA card


        Bluetooth       = 0x00001000,    //general Bluetooth marker
        BluetoothDUN    = 0X00002000,    //Dial-up Networking profile (DNP) client for Bluetooth
        //BluetoothPAN    = 0x00008000,    //Personal Area Network profile (PAN) client for Bluetooth
        //BluetoothPAND   = 0x00010000,    //Personal Area Network profile (PAN) client for Bluetooth

        Hidden          = 0x10000000,    //hidden network interface
        Any             = 0x00000000     //unknown type
    };

    Q_DECLARE_FLAGS(Type, TypeFlag)

    static bool online();
    static void startInterface( const QString& handle, const QVariant& options = QVariant() );
    static void stopInterface( const QString& handle, bool deleteIface = false);
    static void setDefaultGateway( const QString& handle );
    static void unsetDefaultGateway( const QString& handle );
    static void extendInterfaceLifetime( const QString& handle, bool isExtended );
    static void privilegedInterfaceStop( const QString& handle );
    static void shutdown();
    static void lockdown( bool isLocked );
    static Type toType( const QString& config );

    static QString settingsDir();
    static QStringList availableNetworkConfigs(QtopiaNetwork::Type type = QtopiaNetwork::Any,
            const QString& dir = QString());

    static QPointer<QtopiaNetworkInterface> loadPlugin( const QString& handle );
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QtopiaNetwork::Type);

#endif //QTOPIA_NETWORKING_H
