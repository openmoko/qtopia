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

#include "qbluetoothhfagserver_p.h"
#include "qbluetoothhfservice_p.h"
#include <qtopia/comm/qbluetoothaddress.h>
#include <qtopialog.h>

QBluetoothHandsfreeAudioGatewayServer::QBluetoothHandsfreeAudioGatewayServer(
        QBluetoothHandsfreeCommInterface *parent,
        const QString &audioDev,
        const QString &service)
    : QBluetoothAudioGateway(service, parent, QCommInterface::Server)
{
    m_parent = parent;

    QObject::connect(m_parent, SIGNAL(connectResult(bool, const QString &)),
                     this, SIGNAL(connectResult(bool, const QString &)));
    QObject::connect(m_parent, SIGNAL(newConnection(const QBluetoothAddress &)),
                     this, SIGNAL(newConnection(const QBluetoothAddress &)));
    QObject::connect(m_parent, SIGNAL(disconnected()),
                     this, SIGNAL(headsetDisconnected()));
    QObject::connect(m_parent, SIGNAL(speakerVolumeChanged()),
                     this, SIGNAL(speakerVolumeChanged()));
    QObject::connect(m_parent, SIGNAL(microphoneVolumeChanged()),
                     this, SIGNAL(microphoneVolumeChanged()));
    QObject::connect(m_parent, SIGNAL(audioStateChanged()),
                     this, SIGNAL(audioStateChanged()));

    setValue("AudioDevice", audioDev, Delayed);
    setValue("IsConnected", false, Delayed);
    setValue("RemotePeer", QVariant::fromValue(QBluetoothAddress::invalid), Delayed);
}

QBluetoothHandsfreeAudioGatewayServer::~QBluetoothHandsfreeAudioGatewayServer()
{

}

void QBluetoothHandsfreeAudioGatewayServer::connect(const QBluetoothAddress &addr,
        int rfcomm_channel)
{
    m_parent->connect(addr, rfcomm_channel);
}

void QBluetoothHandsfreeAudioGatewayServer::disconnect()
{
    m_parent->disconnect();
}

void QBluetoothHandsfreeAudioGatewayServer::setSpeakerVolume(int volume)
{
    m_parent->setSpeakerVolume(volume);
}

void QBluetoothHandsfreeAudioGatewayServer::setMicrophoneVolume(int volume)
{
    m_parent->setMicrophoneVolume(volume);
}

void QBluetoothHandsfreeAudioGatewayServer::releaseAudio()
{
    m_parent->releaseAudio();
}

void QBluetoothHandsfreeAudioGatewayServer::connectAudio()
{
    m_parent->connectAudio();
}
