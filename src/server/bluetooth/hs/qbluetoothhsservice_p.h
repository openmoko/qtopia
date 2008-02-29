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

#ifndef __QBLUETOOTHHSSERVICE_H__
#define __QBLUETOOTHHSSERVICE_H__

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

#include <qabstractipcinterfacegroup.h>
#include <qtopia/comm/qbluetoothnamespace.h>
#include <qtopia/comm/qbluetoothabstractservice.h>
#include <qtopia/comm/qbluetoothrfcommsocket.h>
#include <qtopiaglobal.h>


class QBluetoothHeadsetServicePrivate;
class QBluetoothHeadsetService : public QBluetoothAbstractService
{
    Q_OBJECT

public:
    QBluetoothHeadsetService(const QString &service, const QString &displayName, QObject *parent = 0);
    ~QBluetoothHeadsetService();

    void start();
    void stop();
    void setSecurityOptions(QBluetooth::SecurityOptions options);

    // Methods from the Headset AG Interface
    void connect(const QBluetoothAddress &addr, int rfcomm_channel);
    void disconnect();
    void setSpeakerVolume(int volume);
    void setMicrophoneVolume(int volume);
    void releaseAudio();
    void connectAudio();

signals:
    // Signals from the Headset AG interface
    void connectResult(bool success, const QString &msg);
    void newConnection(const QBluetoothAddress &addr);
    void disconnected();
    void speakerVolumeChanged();
    void microphoneVolumeChanged();
    void audioStateChanged();

protected:
    virtual bool canConnectAudio();

private slots:
    // Server connection
    void newConnection();

    // Client connection
    void stateChanged(QBluetoothAbstractSocket::SocketState socketState);
    void error(QBluetoothAbstractSocket::SocketError error);
    void readyRead();
    void bytesWritten(qint64 bytes);

    // Sco connection
    void scoStateChanged(QBluetoothAbstractSocket::SocketState socketState);

    void sessionOpen();
    void sessionFailed();

private:
    void hookupSocket(QBluetoothRfcommSocket *socket);
    bool doConnectAudio();

    QBluetoothHeadsetServicePrivate *m_data;
};


class QBluetoothHeadsetCommInterfacePrivate;
class QBluetoothHeadsetCommInterface : public QAbstractIpcInterfaceGroup
{
    Q_OBJECT

public:
    QBluetoothHeadsetCommInterface(const QByteArray &audioDev, QBluetoothHeadsetService *parent);
    ~QBluetoothHeadsetCommInterface();

    void initialize();

    void setValue(const QString &key, const QVariant &value);

    // Methods from the Headset AG Interface
    void connect(const QBluetoothAddress &addr, int rfcomm_channel);
    void disconnect();
    void setSpeakerVolume(int volume);
    void setMicrophoneVolume(int volume);
    void releaseAudio();
    void connectAudio();

signals:
    // Signals from the Headset AG interface
    void connectResult(bool success, const QString &msg);
    void newConnection(const QBluetoothAddress &addr);
    void disconnected();
    void speakerVolumeChanged();
    void microphoneVolumeChanged();
    void audioStateChanged();

private:
    QBluetoothHeadsetCommInterfacePrivate *m_data;
};


#endif
