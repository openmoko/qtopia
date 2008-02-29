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

#ifndef __QBLUETOOTHABSTRACTSOCKET_P_H__
#define __QBLUETOOTHABSTRACTSOCKET_P_H__

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

#include <QObject>

#include <private/qringbuffer_p.h>
#include <qbluetoothabstractsocket.h>

class QSocketNotifier;
class QTimer;
class QBluetoothSocketEngine;

class QBluetoothAbstractSocketPrivate : public QObject
{
    Q_OBJECT

public:
    QBluetoothAbstractSocketPrivate(bool isBuffered);
    ~QBluetoothAbstractSocketPrivate();

    bool initiateDisconnect();

    bool flush();

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

    QBluetoothSocketEngine *m_engine;
};

#define SOCKET_DATA(Class) Class##Private * const m_data = static_cast<Class##Private *>(QBluetoothAbstractSocket::m_data)

#endif
