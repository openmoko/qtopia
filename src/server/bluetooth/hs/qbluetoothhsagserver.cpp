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

#include "qbluetoothhsagserver_p.h"
#include "qbluetoothhsservice_p.h"
#include <qtopia/comm/qbluetoothaddress.h>
#include <qtopialog.h>

/*!
    \class QBluetoothHeadsetAudioGatewayServer
    \ingroup QtopiaServer::Task::Bluetooth
    \brief The QBluetoothHeadsetAudioGatewayServer class implements the Bluetooth headset service Audio Gateway interface.

    The QBluetoothHandsfreeAudioGatewayServer class implements Bluetooth
    Headset Audio Gateway profile..

    QBluetoothHeadsetAudioGatewayServer implements the
    QBluetoothAudioGateway interface.  Client applications
    should use the QBluetoothAudioGateway class instead of
    this class to access the Bluetooth Audio Gateway functionality.

    Please note, this class only acts as a forwarding agent
    to the QBluetoothHeadsetService class, which handles all
    implementation details.
  
    This class is part of the Qtopia server and cannot be used by other QtopiaApplications.

    \sa QBluetoothAudioGateway, QBluetoothHeadsetService
*/

/*!
    Create a new Bluetooth Headset server for \a parent.  The name
    of the service is set to \a service.  The audio device to use
    is given by \a audioDev.
*/

QBluetoothHeadsetAudioGatewayServer::QBluetoothHeadsetAudioGatewayServer(
        QBluetoothHeadsetCommInterface *parent,
        const QString &audioDev,
        const QString &service)
    : QBluetoothAudioGateway(service, parent, QCommInterface::Server)
{
    m_parent = parent;

    QObject::connect(m_parent, SIGNAL(connectResult(bool,QString)),
                     this, SIGNAL(connectResult(bool,QString)));
    QObject::connect(m_parent, SIGNAL(newConnection(QBluetoothAddress)),
                     this, SIGNAL(newConnection(QBluetoothAddress)));
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
    setValue("AudioEnabled", false, Delayed);
    setValue("SpeakerVolume", 0, Delayed);
    setValue("MicrophoneVolume", 0, Delayed);
}

/*!
    Destructor.
*/
QBluetoothHeadsetAudioGatewayServer::~QBluetoothHeadsetAudioGatewayServer()
{

}

/*!
    \reimp
*/
void QBluetoothHeadsetAudioGatewayServer::connect(const QBluetoothAddress &addr,
        int rfcomm_channel)
{
    m_parent->connect(addr, rfcomm_channel);
}

/*!
    \reimp
*/
void QBluetoothHeadsetAudioGatewayServer::disconnect()
{
    qLog(Bluetooth) << "Headset AG got a disconnect request";
    m_parent->disconnect();
}

/*!
    \reimp
*/
void QBluetoothHeadsetAudioGatewayServer::setSpeakerVolume(int volume)
{
    m_parent->setSpeakerVolume(volume);
}

/*!
    \reimp
*/
void QBluetoothHeadsetAudioGatewayServer::setMicrophoneVolume(int volume)
{
    m_parent->setMicrophoneVolume(volume);
}

/*!
    \reimp
*/
void QBluetoothHeadsetAudioGatewayServer::releaseAudio()
{
    m_parent->releaseAudio();
}

/*!
    \reimp
*/
void QBluetoothHeadsetAudioGatewayServer::connectAudio()
{
    m_parent->connectAudio();
}
