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

#include <qtopia/comm/qbluetoothaudiogateway.h>
#include "qbluetoothhsservice_p.h"
#include "qbluetoothhsagserver_p.h"
#include <qtopia/comm/qbluetoothrfcommserver.h>
#include <qtopia/comm/qbluetoothrfcommsocket.h>
#include <qtopia/comm/qbluetoothscosocket.h>
#include <qtopia/comm/qbluetoothlocaldevice.h>
#include <QBluetoothSdpRecord>
#include <qtopiacomm/private/qsdpxmlparser_p.h>
#include <qtopianamespace.h>
#include <qtopialog.h>
#include <qtopia/comm/qbluetoothaddress.h>
#include <qcommdevicesession.h>

#include <QFile>

#include <bluetooth/bluetooth.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "bluetooth/scomisc_p.h"

class QBluetoothHeadsetServicePrivate
{
public:
    QBluetoothRfcommServer *m_server;
    quint32 m_sdpRecordHandle;
    QBluetooth::SecurityOptions m_securityOptions;
    QBluetoothRfcommSocket *m_client;
    QBluetoothHeadsetCommInterface *m_interface;
    QBluetoothScoSocket *m_scoSocket;
    int m_scofd;
    bool m_connectInProgress;
    bool m_disconnectInProgress;
    int m_speakerVolume;
    int m_microphoneVolume;

    void *m_audioDev;
    QCommDeviceSession *m_session;

    QBluetoothAddress m_addr;
    int m_channel;
};

QBluetoothHeadsetService::QBluetoothHeadsetService(const QString &service, const QString &displayName, QObject *parent)
    : QBluetoothAbstractService(service, displayName, parent)
{
    m_data = new QBluetoothHeadsetServicePrivate();

    m_data->m_server = new QBluetoothRfcommServer(this);
    m_data->m_server->setMaxPendingConnections(1);
    m_data->m_securityOptions = 0;
    QObject::connect(m_data->m_server, SIGNAL(newConnection()),
                     this, SLOT(newConnection()));

    m_data->m_client = 0;
    m_data->m_connectInProgress = false;
    m_data->m_disconnectInProgress = false;

    m_data->m_speakerVolume = 0;
    m_data->m_microphoneVolume = 0;

    m_data->m_scoSocket = 0;
    m_data->m_scofd = -1;

    m_data->m_session = 0;
    m_data->m_interface = 0;


    QByteArray audioDev = find_btsco_device("Headset");
    if (audioDev.isEmpty()) {
        qWarning("No headset audio devices available...");
    }
    else if (!bt_sco_open(&m_data->m_audioDev, audioDev.constData())) {
        qWarning("Unable to open audio device: %s", audioDev.constData());
    }
    else {
        m_data->m_interface = new QBluetoothHeadsetCommInterface(audioDev, this);
        m_data->m_interface->initialize();
    }
}

QBluetoothHeadsetService::~QBluetoothHeadsetService()
{
    if (m_data) {
        bt_sco_close(m_data->m_audioDev);
        delete m_data->m_server;
        delete m_data->m_scoSocket;
        delete m_data->m_client;
        delete m_data->m_session;
        delete m_data;
    }
}

// Methods from the Control Interface
void QBluetoothHeadsetService::start()
{
    qLog(Bluetooth) << "QBluetoothHeadsetService::start";

    if (!m_data->m_interface) {
        emit started(true, tr("No suitable audio devices found!"));
        return;
    }

    if (m_data->m_server->isListening()) {
        emit started(true,
                     tr("Headset Audio Gateway already running."));
        return;
    }

    // register the SDP service
    m_data->m_sdpRecordHandle = registerRecord(Qtopia::qtopiaDir() + "etc/bluetooth/sdp/hs.xml");
    if (m_data->m_sdpRecordHandle == 0) {
        emit started(true,
                     tr("Error registering with SDP server"));
        return;
    }

    // For now, hard code in the channel, which has to be the same channel as
    // the one in the XML file passed in the registerRecord() call above
    int channel = 1;

    // start the server
    if (!m_data->m_server->listen(QBluetoothAddress::any, channel)) {
        unregisterRecord(m_data->m_sdpRecordHandle);
        emit started(true, tr("Could not listen on channel."));
        return;
    }

    m_data->m_server->setSecurityOptions(m_data->m_securityOptions);

    emit started(false, QString());
}

void QBluetoothHeadsetService::stop()
{
    qLog(Bluetooth) << "QBluetoothHeadsetService::stop";

    if (m_data->m_server->isListening()) {
        m_data->m_server->close();
        disconnect();
    }

    if (!unregisterRecord(m_data->m_sdpRecordHandle))
        qLog(Bluetooth) << "QBluetoothHeadsetService::stop() error unregistering SDP service";

    emit stopped();
}

void QBluetoothHeadsetService::setSecurityOptions(QBluetooth::SecurityOptions options)
{
    m_data->m_securityOptions = options;
    if (m_data->m_server->isListening())
        m_data->m_server->setSecurityOptions(options);
}

void QBluetoothHeadsetService::sessionOpen()
{
    if (m_data->m_connectInProgress) {
        QBluetoothRfcommSocket *socket = new QBluetoothRfcommSocket(this);
        hookupSocket(socket);

        bool ret = m_data->m_client->connect(QBluetoothAddress::any,
                                            m_data->m_addr, m_data->m_channel);

        if (!ret) {
            delete m_data->m_client;
            m_data->m_client = 0;
            emit connectResult(false, tr("Connect failed."));
            return;
        }

        if (m_data->m_client->state() == QBluetoothRfcommSocket::ConnectedState) {
            m_data->m_interface->setValue("IsConnected", true);
            m_data->m_interface->setValue("RemotePeer",
                                        QVariant::fromValue(socket->remoteAddress()));
            emit connectResult(true, QString());
            return;
        }
    }
}

void QBluetoothHeadsetService::sessionFailed()
{
    if (m_data->m_connectInProgress) {
        m_data->m_connectInProgress = false;
        emit connectResult(false, tr("Bluetooth Session could not be started"));
        return;
    }
}

// Methods from the Headset AG Interface
void QBluetoothHeadsetService::connect(const QBluetoothAddress &addr,
                                       int rfcomm_channel)
{
    // If the service is stop, deny connect requests
    if (!m_data->m_server->isListening()) {
        emit connectResult(false, tr("Service not available."));
        return;
    }

    // If we're still connecting or disconnecting, return
    if (m_data->m_connectInProgress || m_data->m_disconnectInProgress) {
        emit connectResult(false, tr("Connection in progress."));
        return;
    }

    // If we're connected, return, caller should call disconnect first
    if (m_data->m_client &&
        (m_data->m_client->state() == QBluetoothRfcommSocket::ConnectedState)) {
        emit connectResult(false, tr("Already connected."));
        return;
    }

    m_data->m_connectInProgress = true;
    m_data->m_addr = addr;
    m_data->m_channel = rfcomm_channel;
    qLog(Bluetooth) << "Starting session for headset.";

    if (!m_data->m_session) {
        qLog(Bluetooth) << "Lazy initializing the QCommDeviceSession object";
        QBluetoothLocalDevice local;
        m_data->m_session = new QCommDeviceSession(local.deviceName().toLatin1());
        QObject::connect(m_data->m_session, SIGNAL(sessionOpen()), this, SLOT(sessionOpen()));
        QObject::connect(m_data->m_session, SIGNAL(sessionFailed()), this, SLOT(sessionFailed()));
    }

    m_data->m_session->startSession();
}

void QBluetoothHeadsetService::disconnect()
{
    releaseAudio();

    if (m_data->m_client &&
        (m_data->m_client->state() == QBluetoothRfcommSocket::ConnectedState)) {
        m_data->m_client->disconnect();
    }

    if (m_data->m_client &&
        m_data->m_client->state() == QBluetoothRfcommSocket::ClosingState) {
        m_data->m_disconnectInProgress = true;
    }
}

void QBluetoothHeadsetService::setSpeakerVolume(int volume)
{
    qLog(Bluetooth) << "setSpeakerVolume:" << volume;

    if (!m_data->m_client)
        return;

    if (m_data->m_client->state() != QBluetoothRfcommSocket::ConnectedState) {
        return;
    }

    if (volume == m_data->m_speakerVolume)
        return;

    char data[64];
    int len = sprintf(data, "\r\n+VGS=%d\r\n", volume);
    qLog(Bluetooth) << "Writing" << data << "(len =" << volume << "bytes) to the headset";
    m_data->m_client->write(data, len);

    m_data->m_speakerVolume = volume;
    m_data->m_interface->setValue("SpeakerVolume", volume);
    emit speakerVolumeChanged();
}

void QBluetoothHeadsetService::setMicrophoneVolume(int volume)
{
    qLog(Bluetooth) << "setMicrophoneVolume:" << volume;

    if (!m_data->m_client)
        return;

    if (m_data->m_client->state() != QBluetoothRfcommSocket::ConnectedState) {
        return;
    }

    if (volume == m_data->m_microphoneVolume)
        return;

    char data[64];
    int len = sprintf(data, "\r\n+VGM=%d\r\n", volume);
    qLog(Bluetooth) << "Writing" << data << "(len =" << volume << "bytes) to the headset";
    m_data->m_client->write(data, len);

    m_data->m_microphoneVolume = volume;
    m_data->m_interface->setValue("MicrophoneVolume", volume);
    emit microphoneVolumeChanged();
}

void QBluetoothHeadsetService::releaseAudio()
{
    if (m_data->m_scoSocket) {
        delete m_data->m_scoSocket;
        m_data->m_scoSocket = 0;
    }

    if (m_data->m_scofd == -1)
        return;

    bt_sco_set_fd(m_data->m_audioDev, -1);
    ::close(m_data->m_scofd);
    m_data->m_scofd = -1;

    m_data->m_interface->setValue("AudioEnabled", false);
    emit audioStateChanged();
}

void QBluetoothHeadsetService::scoStateChanged(QBluetoothAbstractSocket::SocketState socketState)
{
    switch (socketState) {
        case QBluetoothAbstractSocket::ConnectingState:
            break;
        case QBluetoothAbstractSocket::ConnectedState:
            // This is gonna be ugly...
            m_data->m_scofd = dup(m_data->m_scoSocket->socketDescriptor());
            ::fcntl(m_data->m_scofd, F_SETFD, FD_CLOEXEC);

            delete m_data->m_scoSocket;
            m_data->m_scoSocket = 0;

            bt_sco_set_fd(m_data->m_audioDev, m_data->m_scofd);
            m_data->m_interface->setValue("AudioEnabled", true);
            emit audioStateChanged();
            break;
        case QBluetoothAbstractSocket::ClosingState:
            break;
        case QBluetoothAbstractSocket::UnconnectedState:
            // This happens on a failed connect
            if (m_data->m_scoSocket) {
                m_data->m_scoSocket->deleteLater();
                m_data->m_scoSocket = 0;
            }

            break;
    };
}

void QBluetoothHeadsetService::connectAudio()
{
    doConnectAudio();
}

bool QBluetoothHeadsetService::doConnectAudio()
{
    if (m_data->m_scofd != -1) {
        qWarning("Already connected.");
        return false;
    }

    if (m_data->m_scoSocket != 0) {
        qWarning("Already connecting!");
        return false;
    }

    if (!canConnectAudio()) {
        return false;
    }

    m_data->m_scoSocket = new QBluetoothScoSocket(this);
    QObject::connect(m_data->m_scoSocket,
                     SIGNAL(stateChanged(QBluetoothAbstractSocket::SocketState)),
                     this, SLOT(scoStateChanged(QBluetoothAbstractSocket::SocketState)));

    bool ret = m_data->m_scoSocket->connect(QBluetoothAddress::any,
                                            m_data->m_client->remoteAddress());
    if (ret == false) {
        delete m_data->m_scoSocket;
        m_data->m_scoSocket = 0;
    }

    return ret;
}

void QBluetoothHeadsetService::readyRead()
{
    char buf[512];

    int size = m_data->m_client->read(buf, 512);
    buf[size] = '\0';
    int volume = 0;

    if (!strncmp("AT+CKPD=200\r", buf, 512)) {
        qLog(Bluetooth) << "QBluetoothHeadsetService::readyRead: Got an AT+CKPD=200";
        // Audio not connected
        if (m_data->m_scofd == -1) {
            qLog(Bluetooth) << "QBluetoothHeadsetService::readyRead: No audio.";
            if (doConnectAudio()) {
                m_data->m_client->write("\r\nOK\r\n");
            }
            else {
                m_data->m_client->write("\r\nERROR\r\n");
            }
        }
        else {
            qLog(Bluetooth) << "QBluetoothHeadsetService::readyRead: Releasing audio.";
            releaseAudio();
            m_data->m_client->write("\r\nOK\r\n");
        }
    }
    else if (sscanf(buf, "AT+VGS=%d", &volume) == 1) {
        qLog(Bluetooth) << "QBluetoothHeadsetService::readyRead: Got a SpeakerVolume change.";
        if (m_data->m_speakerVolume != volume) {
            m_data->m_speakerVolume = volume;
            m_data->m_interface->setValue("SpeakerVolume", volume);
            emit speakerVolumeChanged();
        }

        m_data->m_client->write("\r\nOK\r\n");
    }
    else if (sscanf(buf, "AT+VGM=%d", &volume) == 1) {
        qLog(Bluetooth) << "QBluetoothHeadsetService::readyRead: Got a MicrophoneVolume change.";
        if (m_data->m_microphoneVolume != volume) {
            m_data->m_microphoneVolume = volume;
            m_data->m_interface->setValue("MicrophoneVolume", volume);
            emit microphoneVolumeChanged();
        }

        m_data->m_client->write("\r\nOK\r\n");
    }
}

void QBluetoothHeadsetService::bytesWritten(qint64)
{

}

void QBluetoothHeadsetService::newConnection()
{
    qLog(Bluetooth) << "QBluetoothHeadsetService::New client has connected.";

    // New client has connected
    QBluetoothRfcommSocket *socket =
        static_cast<QBluetoothRfcommSocket *>(m_data->m_server->nextPendingConnection());

    qLog(Bluetooth) << "Socket is:" << socket->socketDescriptor();
    if (m_data->m_client &&
        (m_data->m_connectInProgress ||
        (m_data->m_client->state() == QBluetoothRfcommSocket::ConnectedState))) {

        qLog(Bluetooth) << "Already connected, closing client socket.";
        socket->close();
        delete socket;
        return;
    }

    hookupSocket(socket);
    m_data->m_interface->setValue("IsConnected", true);
    qLog(Bluetooth) << "Starting Bluetooth session for Headset";

    if (!m_data->m_session) {
        qLog(Bluetooth) << "Lazy initializing the QCommDeviceSession object";
        QBluetoothLocalDevice local;
        m_data->m_session = new QCommDeviceSession(local.deviceName().toLatin1());
        QObject::connect(m_data->m_session, SIGNAL(sessionOpen()), this, SLOT(sessionOpen()));
        QObject::connect(m_data->m_session, SIGNAL(sessionFailed()), this, SLOT(sessionFailed()));
    }

    m_data->m_session->startSession();

    qLog(Bluetooth) << "The socket remoteAddress is:" << socket->remoteAddress().toString();
    m_data->m_interface->setValue("RemotePeer",
                                  QVariant::fromValue(socket->remoteAddress()));

    emit newConnection(socket->remoteAddress());

    qLog(Bluetooth) << "The socket has bytesAvailable:" << socket->bytesAvailable();

    if (socket->bytesAvailable()) {
        readyRead();
    }
}

void QBluetoothHeadsetService::error(QBluetoothAbstractSocket::SocketError)
{
    if (m_data->m_connectInProgress) {
        return;
    }

    qWarning("Unknown error occrred in headset service");
}

void QBluetoothHeadsetService::stateChanged(QBluetoothAbstractSocket::SocketState socketState)
{
    qLog(Bluetooth) << "QBluetoothHeadsetService::stateChanged...";

    switch (socketState) {
        case QBluetoothRfcommSocket::ConnectingState:
            break;
        case QBluetoothRfcommSocket::ConnectedState:
            m_data->m_interface->setValue("IsConnected", true);
            m_data->m_interface->setValue("RemotePeer",
                                          QVariant::fromValue(m_data->m_client->remoteAddress()));
            emit connectResult(true, QString());
            m_data->m_client->write("\r\nRING\r\n");
            m_data->m_connectInProgress = false;
            break;
        case QBluetoothRfcommSocket::ClosingState:
            break;
        case QBluetoothRfcommSocket::UnconnectedState:
        {
            bool emitDisconnected = true;
            qLog(Bluetooth) << "Client socket is now in disconnected state.";
            if (m_data->m_connectInProgress) {
                qLog(Bluetooth) << "Connect was in progress, connection failed.";
                m_data->m_connectInProgress = false;
                emitDisconnected = false;
                emit connectResult(false, "Connection failed.");
            }
            else if (m_data->m_disconnectInProgress) {
                qLog(Bluetooth) << "Disconnect was in progress, connection failed.";
                m_data->m_disconnectInProgress = false;
            }
            else {
                releaseAudio();
                qWarning("Headset unexpectedly closed rfcomm connection");
            }

            m_data->m_client->deleteLater();
            m_data->m_client = 0;

            m_data->m_interface->setValue("IsConnected", false);
            m_data->m_interface->setValue("RemotePeer",
                                          QVariant::fromValue(QBluetoothAddress::invalid));

            qLog(Bluetooth) << "Deleting session for headset";
            m_data->m_session->endSession();
            if (emitDisconnected)
                emit disconnected();

            break;
        }
        default:
            break;
    };
}

void QBluetoothHeadsetService::hookupSocket(QBluetoothRfcommSocket *socket)
{
    m_data->m_client = socket;
    QObject::connect(m_data->m_client, SIGNAL(stateChanged(QBluetoothAbstractSocket::SocketState)),
                     this, SLOT(stateChanged(QBluetoothAbstractSocket::SocketState)));
    QObject::connect(m_data->m_client, SIGNAL(error(QBluetoothAbstractSocket::SocketError)),
                     this, SLOT(error(QBluetoothAbstractSocket::SocketError)));
    QObject::connect(m_data->m_client, SIGNAL(readyRead()),
                     this, SLOT(readyRead()));
    QObject::connect(m_data->m_client, SIGNAL(bytesWritten(qint64)),
                     this, SLOT(bytesWritten(qint64)));
}

bool QBluetoothHeadsetService::canConnectAudio()
{
    return true;
}


//==========================================================

class QBluetoothHeadsetCommInterfacePrivate
{
public:
    QBluetoothHeadsetService *m_service;
    QBluetoothHeadsetAudioGatewayServer *m_gatewayServer;
};



QBluetoothHeadsetCommInterface::QBluetoothHeadsetCommInterface(const QByteArray &audioDev, QBluetoothHeadsetService *parent)
    : QAbstractIpcInterfaceGroup(parent->name(), parent),
      m_data(new QBluetoothHeadsetCommInterfacePrivate)
{
    m_data->m_service = parent;
    m_data->m_gatewayServer = new QBluetoothHeadsetAudioGatewayServer(this, audioDev,
            parent->name());

    QObject::connect(parent, SIGNAL(connectResult(bool, const QString &)),
                     SIGNAL(connectResult(bool, const QString &)));
    QObject::connect(parent, SIGNAL(newConnection(const QBluetoothAddress &)),
                     SIGNAL(newConnection(const QBluetoothAddress &)));
    QObject::connect(parent, SIGNAL(disconnected()),
                     SIGNAL(disconnected()));
    QObject::connect(parent, SIGNAL(speakerVolumeChanged()),
                     SIGNAL(speakerVolumeChanged()));
    QObject::connect(parent, SIGNAL(microphoneVolumeChanged()),
                     SIGNAL(microphoneVolumeChanged()));
    QObject::connect(parent, SIGNAL(audioStateChanged()),
                     SIGNAL(audioStateChanged()));
}

QBluetoothHeadsetCommInterface::~QBluetoothHeadsetCommInterface()
{
    delete m_data;
}

void QBluetoothHeadsetCommInterface::initialize()
{
    if ( !supports<QBluetoothAudioGateway>() )
        addInterface(m_data->m_gatewayServer);
}

void QBluetoothHeadsetCommInterface::setValue(const QString &key, const QVariant &value)
{
    m_data->m_gatewayServer->setValue(key, value);
}

void QBluetoothHeadsetCommInterface::connect(const QBluetoothAddress &addr, int rfcomm_channel)
{
    m_data->m_service->connect(addr, rfcomm_channel);
}

void QBluetoothHeadsetCommInterface::disconnect()
{
    m_data->m_service->disconnect();
}

void QBluetoothHeadsetCommInterface::setSpeakerVolume(int volume)
{
    m_data->m_service->setSpeakerVolume(volume);
}

void QBluetoothHeadsetCommInterface::setMicrophoneVolume(int volume)
{
    m_data->m_service->setMicrophoneVolume(volume);
}

void QBluetoothHeadsetCommInterface::releaseAudio()
{
    m_data->m_service->releaseAudio();
}

void QBluetoothHeadsetCommInterface::connectAudio()
{
    m_data->m_service->connectAudio();
}
