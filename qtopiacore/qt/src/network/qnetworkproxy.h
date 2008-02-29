/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtNetwork module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QNETWORKPROXY_H
#define QNETWORKPROXY_H

#include <QtNetwork/qhostaddress.h>

#ifndef QT_NO_NETWORKPROXY

QT_BEGIN_HEADER

QT_MODULE(Network)

class QNetworkProxyPrivate;

class Q_NETWORK_EXPORT QNetworkProxy
{
    Q_DECLARE_PRIVATE(QNetworkProxy)

public:
    enum ProxyType {
        DefaultProxy,
        Socks5Proxy,
        NoProxy
    };

    QNetworkProxy();
    QNetworkProxy(ProxyType type, const QString &hostName = QString(), quint16 port = 0,
                  const QString &user = QString(), const QString &password = QString());
    QNetworkProxy(const QNetworkProxy &other);
    QNetworkProxy &operator=(const QNetworkProxy &other);
    ~QNetworkProxy();

    void setType(QNetworkProxy::ProxyType type);
    QNetworkProxy::ProxyType type() const;

    void setUser(const QString &userName);
    QString user() const;

    void setPassword(const QString &password);
    QString password() const;

    void setHostName(const QString &hostName);
    QString hostName() const;

    void setPort(quint16 port);
    quint16 port() const;

    static void setApplicationProxy(const QNetworkProxy &proxy);
    static QNetworkProxy applicationProxy();

private:
    QNetworkProxyPrivate *d_ptr;
};

QT_END_HEADER

#endif // QT_NO_NETWORKPROXY

#endif // QHOSTINFO_H
