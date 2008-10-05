/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include "qbluetoothhfagserver_p.h"
#include "qbluetoothhfservice_p.h"
#include <qtopiacomm/qbluetoothaddress.h>
#include <qtopialog.h>

/*!
    \class QBluetoothHandsfreeAudioGatewayServer
    \inpublicgroup QtBluetoothModule
    \ingroup QtopiaServer::Task::Bluetooth
    \brief The QBluetoothHandsfreeAudioGatewayServer class implements the Bluetooth handsfree service Audio Gateway interface.

    The QBluetoothHandsfreeAudioGatewayServer class implements Bluetooth
    Handsfree Audio Gateway profile..

    QBluetoothHandsfreeAudioGatewayServer implements the
    QBluetoothAudioGateway interface.  Client applications
    should use the QBluetoothAudioGateway class instead of
    this class to access the Bluetooth Audio Gateway functionality.

    Please note, this class only acts as a forwarding agent
    to the QBluetoothHandsfreeService class, which handles all
    implementation details.
  
    This class is part of the Qt Extended server and cannot be used by other QtopiaApplications.

    \sa QBluetoothAudioGateway, QBluetoothHandsfreeService
*/

/*!
    Create a new Bluetooth Handsfree server for \a parent.  The name
    of the service is set to \a service.  The audio device to use
    is given by \a audioDev.
*/
QBluetoothHandsfreeAudioGatewayServer::QBluetoothHandsfreeAudioGatewayServer(
        QBluetoothHandsfreeCommInterface *parent,
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
QBluetoothHandsfreeAudioGatewayServer::~QBluetoothHandsfreeAudioGatewayServer()
{

}

/*!
    \reimp
*/
void QBluetoothHandsfreeAudioGatewayServer::connect(const QBluetoothAddress &addr,
        int rfcomm_channel)
{
    m_parent->connect(addr, rfcomm_channel);
}

/*!
    \reimp
*/
void QBluetoothHandsfreeAudioGatewayServer::disconnect()
{
    m_parent->disconnect();
}

/*!
    \reimp
*/
void QBluetoothHandsfreeAudioGatewayServer::setSpeakerVolume(int volume)
{
    m_parent->setSpeakerVolume(volume);
}

/*!
    \reimp
*/
void QBluetoothHandsfreeAudioGatewayServer::setMicrophoneVolume(int volume)
{
    m_parent->setMicrophoneVolume(volume);
}

/*!
    \reimp
*/
void QBluetoothHandsfreeAudioGatewayServer::releaseAudio()
{
    m_parent->releaseAudio();
}

/*!
    \reimp
*/
void QBluetoothHandsfreeAudioGatewayServer::connectAudio()
{
    m_parent->connectAudio();
}
