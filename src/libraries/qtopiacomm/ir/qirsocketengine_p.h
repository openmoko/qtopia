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

#include "qirsocket.h"
#include <qirnamespace.h>

class QIrSocketEngine
{
public:
    enum SelectType { SelectRead = 0x1, SelectWrite = 0x2};
    enum SocketOption { NonBlockingOption };

    Q_DECLARE_FLAGS(SelectTypes, SelectType)

    QIrSocketEngine();
    ~QIrSocketEngine();

    bool waitFor(QIrSocketEngine::SelectTypes types,
                 int fd, int timeout,
                 bool *timedOut,
                 bool *canRead = 0, bool *canWrite = 0);

    int select(QIrSocketEngine::SelectTypes types, int fd, int timeout,
               bool *canRead = 0, bool *canWrite = 0) const;

    inline QIrSocket::SocketError error() const
    {
        return m_error;
    }

    static QString getErrorString(QIrSocket::SocketError error);

    int socket();
    int accept(int fd);
    QIrSocket::SocketState connect(int fd, const QByteArray &service, quint32 remote);
    bool testConnected(int fd);
    bool listen(int fd, int backlog);
    bool bind(int fd, const QByteArray &service);
    qint64 writeToSocket(int fd, const char *data, qint64 len);
    qint64 readFromSocket(int fd, char *data, qint64 len);
    void close(int fd);

    qint64 bytesAvailable(int fd) const;
    void readSocketParameters(int fd, quint32 *remote) const;

    bool setSocketOption(int fd, QIrSocketEngine::SocketOption option);
    bool setServiceHints(int fd, QIr::DeviceClasses classes);

private:
    void handleConnectError(int error);

    QIrSocket::SocketError m_error;
};

#endif
