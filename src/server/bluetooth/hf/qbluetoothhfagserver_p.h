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

#ifndef __QBLUETOOTHHFAGSERVER_P_H__
#define __QBLUETOOTHHFAGSERVER_P_H__

#include <qtopia/comm/qbluetoothaudiogateway.h>
#include <qtopiaglobal.h>

class QBluetoothHandsfreeCommInterface;

class QBluetoothHandsfreeAudioGatewayServer : public QBluetoothAudioGateway
{
    Q_OBJECT

public:
    QBluetoothHandsfreeAudioGatewayServer(QBluetoothHandsfreeCommInterface *parent,
                                        const QString &audioDev,
                                        const QString& service);
    ~QBluetoothHandsfreeAudioGatewayServer();

    using QCommInterface::setValue;

public slots:
    void connect(const QBluetoothAddress &addr, int rfcomm_channel);
    void disconnect();
    void setSpeakerVolume(int volume);
    void setMicrophoneVolume(int volume);
    void releaseAudio();
    void connectAudio();

private:
    QBluetoothHandsfreeCommInterface *m_parent;
};

#endif
