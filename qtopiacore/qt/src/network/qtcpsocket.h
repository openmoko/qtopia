/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTCPSOCKET_H
#define QTCPSOCKET_H

#include <QtNetwork/qabstractsocket.h>

QT_BEGIN_HEADER

QT_MODULE(Network)

class QTcpSocketPrivate;

class Q_NETWORK_EXPORT QTcpSocket : public QAbstractSocket
{
    Q_OBJECT
public:
    explicit QTcpSocket(QObject *parent = 0);
    virtual ~QTcpSocket();

private:
    Q_DISABLE_COPY(QTcpSocket)
    Q_DECLARE_PRIVATE(QTcpSocket)
};

QT_END_HEADER

#endif // QTCPSOCKET_H
