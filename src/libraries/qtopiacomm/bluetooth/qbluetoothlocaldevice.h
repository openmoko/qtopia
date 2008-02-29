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

#ifndef __QBLUETOOTHLOCALDEVICE_H__
#define __QBLUETOOTHLOCALDEVICE_H__

#include <qbluetoothnamespace.h>

#include <qobject.h>
#include <qglobal.h>
#include <QString>

class QBluetoothRemoteDevice;
template <class T> class QList;
class QBluetoothLocalDevice_Private;
class QDateTime;
class QBluetoothAddress;

template <typename T> class QBluetoothReply {
public:
    QBluetoothReply(const T &value) : m_isError(false), m_value(value) {}
    QBluetoothReply() : m_isError(true) {}

    bool isError() const { return m_isError; }
    const T &value() const { return m_value; }

    operator const T& () const { return m_value; }

private:
    bool m_isError;
    T m_value;
};

class QTOPIACOMM_EXPORT QBluetoothLocalDevice : public QObject
{
    Q_OBJECT
    friend class QBluetoothLocalDevice_Private;

public:

    enum State {
        Off,
        Connectable,
        Discoverable
    };

    enum Error {
        NoError = 0,
        InvalidArguments,
        NotAuthorized,
        OutOfMemory,
        NoSuchAdapter,
        UnknownAddress,
        ConnectionAttemptFailed,
        NotConnected,
        AlreadyExists,
        DoesNotExist,
        InProgress,
        AuthenticationFailed,
        AuthenticationTimeout,
        AuthenticationRejected,
        AuthenticationCancelled,
        UnknownError
    };

    explicit QBluetoothLocalDevice(QObject *parent = 0);
    explicit QBluetoothLocalDevice(const QBluetoothAddress &addr, QObject* parent=0 );
    explicit QBluetoothLocalDevice(const QString &devName, QObject* parent = 0 );

    ~QBluetoothLocalDevice();

    bool isValid() const;
    QBluetoothLocalDevice::Error lastError() const;

    QString deviceName() const;
    QBluetoothAddress address() const;

    QBluetoothReply<QString> manufacturer() const;
    QBluetoothReply<QString> version() const;
    QBluetoothReply<QString> revision() const;
    QBluetoothReply<QString> company() const;

    QBluetoothReply<QString> name() const;
    bool setName(const QString &name);

    bool setDiscoverable(uint timeout = 0);
    QBluetoothReply<uint> discoverableTimeout() const;
    QBluetoothReply<bool> discoverable() const;
    bool setConnectable();
    QBluetoothReply<bool> connectable() const;
    bool turnOff();
    QBluetoothReply<bool> isUp();

    QBluetoothReply<bool> isConnected(const QBluetoothAddress &addr) const;
    QBluetoothReply<QList<QBluetoothAddress> > connections() const;

    QBluetoothReply<QDateTime> lastSeen(const QBluetoothAddress &addr) const;
    QBluetoothReply<QDateTime> lastUsed(const QBluetoothAddress &addr) const;
    bool updateRemoteDevice(QBluetoothRemoteDevice &device) const;

    bool requestPairing(const QBluetoothAddress &addr);
    bool removePairing(const QBluetoothAddress &addr);
    QBluetoothReply<QList<QBluetoothAddress> > pairedDevices() const;
    QBluetoothReply<bool> isPaired(const QBluetoothAddress &addr) const;
    bool cancelPairing(const QBluetoothAddress &addr);

    QBluetoothReply<QString> remoteAlias(const QBluetoothAddress &addr) const;
    bool setRemoteAlias(const QBluetoothAddress &addr, const QString &alias);
    bool removeRemoteAlias(const QBluetoothAddress &addr);

public slots:
    bool discoverRemoteDevices();
    bool cancelDiscovery();

signals:
    void nameChanged(const QString &name);
    void stateChanged(QBluetoothLocalDevice::State state);
    void error(QBluetoothLocalDevice::Error error, const QString &msg);

    void remoteDeviceConnected(const QBluetoothAddress &addr);
    void remoteDeviceDisconnected(const QBluetoothAddress &addr);

    void discoveryStarted();
    void remoteDeviceFound(const QBluetoothRemoteDevice &device);
    void remoteDevicesFound(const QList<QBluetoothRemoteDevice> &devices);
    void discoveryCancelled();
    void discoveryCompleted();

    void pairingCreated(const QBluetoothAddress &addr);
    void pairingRemoved(const QBluetoothAddress &addr);
    void pairingFailed(const QBluetoothAddress &addr);

    void remoteAliasChanged(const QBluetoothAddress &addr, const QString &alias);
    void remoteAliasRemoved(const QBluetoothAddress &addr);

private:
    Q_DISABLE_COPY(QBluetoothLocalDevice)
    QBluetoothLocalDevice_Private *m_data;
};

#endif
