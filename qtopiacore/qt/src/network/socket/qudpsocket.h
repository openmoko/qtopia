/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtNetwork module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QUDPSOCKET_H
#define QUDPSOCKET_H

#include <QtNetwork/qabstractsocket.h>
#include <QtNetwork/qhostaddress.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Network)

#ifndef QT_NO_UDPSOCKET

class QUdpSocketPrivate;

class Q_NETWORK_EXPORT QUdpSocket : public QAbstractSocket
{
    Q_OBJECT
public:
    enum BindFlag {
        DefaultForPlatform = 0x0,
        ShareAddress = 0x1,
        DontShareAddress = 0x2,
        ReuseAddressHint = 0x4
    };
    Q_DECLARE_FLAGS(BindMode, BindFlag)

    explicit QUdpSocket(QObject *parent = 0);
    virtual ~QUdpSocket();

    bool bind(const QHostAddress &address, quint16 port);
    bool bind(quint16 port = 0);
    bool bind(const QHostAddress &address, quint16 port, BindMode mode);
    bool bind(quint16 port, BindMode mode);
    // ### Qt 5: Merge the bind functions

    bool hasPendingDatagrams() const;
    qint64 pendingDatagramSize() const;
    qint64 readDatagram(char *data, qint64 maxlen, QHostAddress *host = 0, quint16 *port = 0);
    qint64 writeDatagram(const char *data, qint64 len, const QHostAddress &host, quint16 port);
    inline qint64 writeDatagram(const QByteArray &datagram, const QHostAddress &host, quint16 port)
        { return writeDatagram(datagram.constData(), datagram.size(), host, port); }

private:
    Q_DISABLE_COPY(QUdpSocket)
    Q_DECLARE_PRIVATE(QUdpSocket)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QUdpSocket::BindMode)

#endif // QT_NO_UDPSOCKET

QT_END_NAMESPACE

QT_END_HEADER

#endif // QUDPSOCKET_H
