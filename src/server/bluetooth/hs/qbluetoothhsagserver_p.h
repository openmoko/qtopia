/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef __QBLUETOOTHHSAGSERVER_P_H__
#define __QBLUETOOTHHSAGSERVER_P_H__

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

#include <qtopia/comm/qbluetoothaudiogateway.h>
#include <qtopiaglobal.h>

class QBluetoothHeadsetCommInterface;

class QBluetoothHeadsetAudioGatewayServer : public QBluetoothAudioGateway
{
    Q_OBJECT
    friend class QBluetoothHeadsetAudioGatewayServerPrivate;

public:
    QBluetoothHeadsetAudioGatewayServer(QBluetoothHeadsetCommInterface *parent,
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
    QBluetoothHeadsetCommInterface *m_parent;
};

#endif
