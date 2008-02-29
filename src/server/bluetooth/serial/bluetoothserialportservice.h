/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef BLUETOOTH_TERMINAL_SERVICE
#define BLUETOOTH_TERMINAL_SERVICE

#include <qbluetoothabstractservice.h>
#include <qbluetoothnamespace.h>

class QBluetoothSerialPortServicePrivate;

class QBluetoothSerialPortService : public QBluetoothAbstractService
{
    Q_OBJECT
public:
    QBluetoothSerialPortService( const QString& serviceID,
                                 const QString& serviceName,
                                 const QBluetoothSdpRecord &record,
                                 QObject* parent = 0 );
    ~QBluetoothSerialPortService();

    void start();
    void stop();
    void setSecurityOptions( QBluetooth::SecurityOptions options );

protected slots:
    void newConnection();

private slots:
    void initiateModemEmulator();
    void emulatorStateChanged();

private:
    QBluetoothSerialPortServicePrivate* d;
};

#endif ///BLUETOOTH_TERMINAL_SERVICE
