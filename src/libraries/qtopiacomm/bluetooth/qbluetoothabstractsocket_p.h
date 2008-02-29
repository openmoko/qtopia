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

#ifndef __QBLUETOOTHABSTRACTSOCKET_P_H__
#define __QBLUETOOTHABSTRACTSOCKET_P_H__

#include <QObject>

#include <private/qringbuffer_p.h>
#include <qtopiacomm/qbluetoothabstractsocket.h>

class QSocketNotifier;
class QTimer;

class QBluetoothAbstractSocketPrivate : public QObject
{
    Q_OBJECT

public:
    QBluetoothAbstractSocketPrivate(bool isBuffered);
    ~QBluetoothAbstractSocketPrivate();

    bool initiateConnect(int socket, struct sockaddr *addr, int size);
    bool initiateDisconnect();

    bool flush();

    qint64 writeToSocket(const char *data, qint64 len);
    qint64 readFromSocket(char *data, qint64 len);
    bool readData();

    void resetNotifiers();
    void setupNotifiers();

    qint64 bytesAvailable() const;

public slots:
    void testConnected();
    void abortConnectionAttempt();
    bool readActivated();
    bool writeActivated();

public:
    QBluetoothAbstractSocket *m_parent;
    QBluetoothAbstractSocket::SocketError m_error;
    QBluetoothAbstractSocket::SocketState m_state;
    int m_fd;
    QSocketNotifier *m_readNotifier;
    QSocketNotifier *m_writeNotifier;
    QTimer *m_timer;

    QRingBuffer m_writeBuffer;
    QRingBuffer m_readBuffer;

    bool m_readSocketNotifierCalled;
    bool m_readSocketNotifierState;
    bool m_readSocketNotifierStateSet;
    bool m_emittedReadyRead;
    bool m_emittedBytesWritten;
    qint64 m_readBufferCapacity;

    bool m_isBuffered;

    int m_readMtu;
    int m_writeMtu;
};

#define SOCKET_DATA(Class) Class##Private * const m_data = static_cast<Class##Private *>(QBluetoothAbstractSocket::m_data)

#endif
