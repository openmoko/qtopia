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

#ifndef __QIRSOCKET_H__
#define __QIRSOCKET_H__

#include <QIODevice>
#include <QtGlobal>

#include <qglobal.h>
#include <qirglobal.h>

class QIrSocketPrivate;
class QIR_EXPORT QIrSocket : public QIODevice
{
    Q_OBJECT
    friend class QIrSocketPrivate;

public:
    enum SocketState {
        UnconnectedState,
        ConnectingState,
        ConnectedState,
        BoundState,
        ClosingState,
    };

    enum SocketError {
        NoError,
        AccessError,
        ResourceError,
        BusyError,
        HostUnreachableError,
        ServiceUnavailableError,
        ConnectionRefused,
        NetworkError,
        TimeoutError,
        RemoteHostClosedError,
        UnsupportedOperationError,
        AddressInUseError,
        AddressNotAvailableError,
        UnknownError
    };

    explicit QIrSocket(QObject *parent = 0);
    ~QIrSocket();

    bool connect(const QByteArray &service, quint32 addr);
    quint32 remoteAddress() const;

    void abort();
    virtual bool disconnect();

    int socketDescriptor() const;
    bool setSocketDescriptor(int socketDescriptor,
                             QIrSocket::SocketState state,
                             QIODevice::OpenMode openMode = QIODevice::ReadWrite);

    SocketError error() const;
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
    void stateChanged(QIrSocket::SocketState socketState);
    void error(QIrSocket::SocketError socketError);
    void disconnected();

protected:
    virtual qint64 readLineData(char *data, qint64 maxsize);
    virtual qint64 readData(char *data, qint64 maxsize);
    virtual qint64 writeData(const char *data, qint64 size);

private:
    QIrSocketPrivate *m_data;
    Q_DISABLE_COPY(QIrSocket)
};

#endif
