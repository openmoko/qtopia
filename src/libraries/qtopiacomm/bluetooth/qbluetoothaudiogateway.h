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

#ifndef __QBLUETOOTHAUDIOGATEWAY_H__
#define __QBLUETOOTHAUDIOGATEWAY_H__

#include <qbluetoothglobal.h>
#include <qtopia/comm/qcomminterface.h>

class QBluetoothAddress;
class QBluetoothRfcommSocket;

class QBLUETOOTH_EXPORT QBluetoothAudioGateway : public QCommInterface
{
    Q_OBJECT

public:
    explicit QBluetoothAudioGateway(const QString& service = QString(),
                                    QObject *parent = 0,
                                    QAbstractIpcInterface::Mode mode = Client );
    ~QBluetoothAudioGateway();

    int speakerVolume() const;
    int microphoneVolume() const;
    bool isConnected() const;
    QBluetoothAddress remotePeer() const;
    bool audioEnabled() const;

public slots:
    virtual void connect(const QBluetoothAddress &addr, int rfcomm_channel);
    virtual void disconnect();
    virtual void setSpeakerVolume(int volume);
    virtual void setMicrophoneVolume(int volume);
    virtual void releaseAudio();
    virtual void connectAudio();

signals:
    void connectResult(bool success, const QString &msg);
    void newConnection(const QBluetoothAddress &addr);
    void headsetDisconnected();
    void speakerVolumeChanged();
    void microphoneVolumeChanged();
    void audioStateChanged();
};

#endif
