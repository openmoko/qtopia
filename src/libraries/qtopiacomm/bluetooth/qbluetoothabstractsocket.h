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

#ifndef __QBLUETOOTHABSTRACTSOCKET_H__
#define __QBLUETOOTHABSTRACTSOCKET_H__

#include <QIODevice>
#include <QAbstractSocket>
#include <QtGlobal>

#include <qtopiaglobal.h>
#include <qbluetoothnamespace.h>


class QBluetoothAddress;
struct sockaddr;

class QBluetoothAbstractSocketPrivate;
class QTOPIACOMM_EXPORT QBluetoothAbstractSocket : public QIODevice
{
    Q_OBJECT
    friend class QBluetoothAbstractSocketPrivate;

public:
    enum SocketState {
        UnconnectedState,
        ConnectingState,
        ConnectedState,
        ClosingState,
    };

    enum SocketError {
        NoError,
        AccessError,
        ResourceError,
        BindError,
        ConnectionRefused,
        HostDownError,
        NetworkError,
        TimedoutError,
        RemoteHostClosedError,
        UnknownError
    };

    explicit QBluetoothAbstractSocket(QObject *parent = 0);
    ~QBluetoothAbstractSocket();

    void abort();
    virtual bool disconnect();

    int socketDescriptor() const;
    bool setSocketDescriptor(int socketDescriptor,
                             QBluetoothAbstractSocket::SocketState state,
                             QIODevice::OpenMode openMode = QIODevice::ReadWrite);

    SocketError lastError() const;
    SocketState state() const;

    qint64 readBufferSize() const;
    void setReadBufferSize(qint64 size);

    bool waitForConnected(int msecs = 30000);
    bool waitForDisconnected(int msecs = 30000);

// from QIODevice
    virtual void close();
    virtual bool isSequential() const;
    virtual bool atEnd() const;
    virtual bool flush();
    virtual bool waitForReadyRead(int msecs);
    virtual bool waitForBytesWritten(int msecs);
    qint64 bytesAvailable() const;
    qint64 bytesToWrite() const;
    bool canReadLine() const;

signals:
    void connected();
    void stateChanged(QBluetoothAbstractSocket::SocketState socketState);
    void error(QBluetoothAbstractSocket::SocketError error);
    void disconnected();

protected:
    virtual qint64 readLineData(char *data, qint64 maxsize);
    virtual qint64 readData(char *data, qint64 maxsize);
    virtual qint64 writeData(const char *data, qint64 size);

    bool initiateConnect(int socket, sockaddr *, int size);
    virtual bool readSocketParameters(int sockfd);
    virtual void resetSocketParameters();
    void setError(const QBluetoothAbstractSocket::SocketError &error);

private:
    Q_DISABLE_COPY(QBluetoothAbstractSocket)
    QBluetoothAbstractSocketPrivate *m_data;
};

#endif
