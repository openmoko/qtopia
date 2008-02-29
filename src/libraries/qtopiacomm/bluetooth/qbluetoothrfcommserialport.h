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

#ifndef __QBLUETOOHRFCOMMSERIALPORT__H
#define __QBLUETOOHRFCOMMSERIALPORT__H

#include <QObject>
#include <QList>

class QString;

#include <qbluetoothglobal.h>
#include <qbluetoothaddress.h>
#include <qbluetoothrfcommsocket.h>

class QBluetoothRfcommSerialPortPrivate;

class QBLUETOOTH_EXPORT QBluetoothRfcommSerialPort : public QObject
{
    Q_OBJECT
    friend class QBluetoothRfcommSerialPortPrivate;

public:
    enum Flag { KeepAlive = 0x01 };
    Q_DECLARE_FLAGS(Flags, Flag)

    enum Error {
        NoError,
        SocketNotConnected,
        ConnectionFailed,
        ConnectionCancelled,
        CreationError
    };

    explicit QBluetoothRfcommSerialPort(QObject* parent = 0);
    explicit QBluetoothRfcommSerialPort(QBluetoothRfcommSocket *socket,
                                        QBluetoothRfcommSerialPort::Flags flags = 0,
                                        QObject *parent = 0);
    ~QBluetoothRfcommSerialPort();

    bool connect(const QBluetoothAddress &local,
                 const QBluetoothAddress &remote,
                 int channel);
    bool disconnect();

    QString device() const;
    int id() const;
    QBluetoothRfcommSerialPort::Flags flags() const;

    QBluetoothRfcommSerialPort::Error error() const;
    QString errorString() const;

    QBluetoothAddress remoteAddress() const;
    int remoteChannel() const;
    QBluetoothAddress localAddress() const;

    static QList<int> listDevices();
    static QList<int> listDevices(const QBluetoothLocalDevice &device);
    static bool releaseDevice(int id);

signals:
    void connected(const QString &boundDevice);
    void error(QBluetoothRfcommSerialPort::Error err);
    void disconnected();

protected:
    void setError(QBluetoothRfcommSerialPort::Error err);

private:
    QBluetoothRfcommSerialPortPrivate* d;
};

#endif //__QBLUETOOHRFCOMMSERIALPORT__H
