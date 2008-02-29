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

#ifndef __QBLUETOOTHHSAGSERVER_P_H__
#define __QBLUETOOTHHSAGSERVER_P_H__

#ifdef HAVE_ALSA

#include <qtopia/comm/qbluetoothaudiogateway.h>
#include <qtopiaglobal.h>

class QBluetoothHeadsetService;

class QBluetoothHeadsetAudioGatewayServer : public QBluetoothAudioGateway
{
    Q_OBJECT
    friend class QBluetoothHeadsetAudioGatewayServerPrivate;

public:
    QBluetoothHeadsetAudioGatewayServer(QBluetoothHeadsetService *parent,
                                        const QString &audioDev,
                                        const QString& service);
    ~QBluetoothHeadsetAudioGatewayServer();

    using QCommInterface::setValue;

public slots:
    void connect(const QBluetoothAddress &addr, int rfcomm_channel);
    void disconnect();
    void setSpeakerVolume(int volume);
    void setMicrophoneVolume(int volume);
    void releaseAudio();
    void connectAudio();

private:
    QBluetoothHeadsetService *m_parent;
};

#endif

#endif
