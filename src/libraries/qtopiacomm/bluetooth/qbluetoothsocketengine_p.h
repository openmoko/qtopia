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

#ifndef __QIRSOCKETENGINE_P_H__
#define __QIRSOCKETENGINE_P_H__

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

#include <qbluetoothabstractsocket.h>
#include <qbluetoothnamespace.h>

class QBluetoothLocalDevice;
class QByteArray;

class QBluetoothSocketEngine
{
public:
    enum SelectType { SelectRead = 0x1, SelectWrite = 0x2};
    enum SocketOption { NonBlockingOption };

    Q_DECLARE_FLAGS(SelectTypes, SelectType)

    QBluetoothSocketEngine();
    ~QBluetoothSocketEngine();

    bool waitFor(QBluetoothSocketEngine::SelectTypes types,
                 int fd, int timeout,
                 bool *timedOut,
                 bool *canRead = 0, bool *canWrite = 0);

    int select(QBluetoothSocketEngine::SelectTypes types, int fd, int timeout,
               bool *canRead = 0, bool *canWrite = 0) const;

    inline QBluetoothAbstractSocket::SocketError error() const
    {
        return m_error;
    }

    static QString getErrorString(QBluetoothAbstractSocket::SocketError error);

    void handleSocketError(int error);
    int scoSocket();
    int rfcommSocket();
    int l2capSocket();
    int l2capDgramSocket();

    int accept(int fd);

    QBluetoothAbstractSocket::SocketState connectSco(int fd,
            const QBluetoothAddress &local,
            const QBluetoothAddress &remote);
    QBluetoothAbstractSocket::SocketState connectL2Cap(int fd,
            const QBluetoothAddress &local,
            const QBluetoothAddress &remote, int psm,
            int incomingMtu, int outgoingMtu);
    QBluetoothAbstractSocket::SocketState connectRfcomm(int fd,
            const QBluetoothAddress &local,
            const QBluetoothAddress &remote, int channel);

    bool testConnected(int fd);
    bool listen(int fd, int backlog);

    void handleBindError(int error);
    bool rfcommBind(int fd, const QBluetoothAddress &device, int channel);
    bool scoBind(int fd, const QBluetoothAddress &device);
    bool l2capBind(int fd, const QBluetoothAddress &device, int psm, int mtu);

    qint64 writeToSocket(int fd, const char *data, qint64 len);
    qint64 readFromSocket(int fd, char *data, qint64 len);
    void close(int fd);

    qint64 bytesAvailable(int fd) const;

    bool setSocketOption(int fd, QBluetoothSocketEngine::SocketOption option);

    bool getScoMtu(int fd, int *mtu) const;
    bool getL2CapIncomingMtu(int fd, int *imtu) const;
    bool setL2CapIncomingMtu(int fd, int imtu);
    bool getL2CapOutgoingMtu(int fd, int *omtu) const;
    bool setL2CapOutgoingMtu(int fd, int omtu);

    bool getsocknameSco(int fd, QBluetoothAddress *address) const;
    bool getsocknameRfcomm(int fd, QBluetoothAddress *address, int *channel) const;
    bool getsocknameL2Cap(int fd, QBluetoothAddress *address, int *psm) const;

    bool getpeernameSco(int fd, QBluetoothAddress *address) const;
    bool getpeernameRfcomm(int fd, QBluetoothAddress *address, int *channel) const;
    bool getpeernameL2Cap(int fd, QBluetoothAddress *address, int *psm) const;

    qint64 recvfrom(int fd, char *data, qint64 maxSize,
                    QBluetoothAddress *address, int *psm);

private:
    void handleConnectError(int error);
    QBluetoothAbstractSocket::SocketError m_error;
};

#endif
