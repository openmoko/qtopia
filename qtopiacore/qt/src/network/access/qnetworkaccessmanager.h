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

#ifndef QNETWORKACCESSMANAGER_H
#define QNETWORKACCESSMANAGER_H

#include <QtCore/QObject>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Network)

class QIODevice;
class QAuthenticator;
class QByteArray;
template<typename T> class QList;
class QNetworkCookie;
class QNetworkCookieJar;
class QNetworkRequest;
class QNetworkReply;
class QNetworkProxy;
class QSslError;

class QNetworkReplyImplPrivate;
class QNetworkAccessManagerPrivate;
class Q_NETWORK_EXPORT QNetworkAccessManager: public QObject
{
    Q_OBJECT
public:
    enum Operation {
        HeadOperation = 1,
        GetOperation,
        PutOperation,
        PostOperation,

        UnknownOperation = 0
    };

    explicit QNetworkAccessManager(QObject *parent = 0);
    ~QNetworkAccessManager();

#ifndef QT_NO_NETWORKPROXY
    QNetworkProxy proxy() const;
    void setProxy(const QNetworkProxy &proxy);
#endif

    QNetworkCookieJar *cookieJar() const;
    void setCookieJar(QNetworkCookieJar *cookieJar);

    QNetworkReply *head(const QNetworkRequest &request);
    QNetworkReply *get(const QNetworkRequest &request);
    QNetworkReply *post(const QNetworkRequest &request, QIODevice *data);
    QNetworkReply *post(const QNetworkRequest &request, const QByteArray &data);
    QNetworkReply *put(const QNetworkRequest &request, QIODevice *data);
    QNetworkReply *put(const QNetworkRequest &request, const QByteArray &data);

Q_SIGNALS:
#ifndef QT_NO_NETWORKPROXY
    void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator);
#endif
    void authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);
    void finished(QNetworkReply *reply);
#ifndef QT_NO_OPENSSL
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
#endif

protected:
    virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &request,
                                         QIODevice *outgoingData = 0);

private:
    friend class QNetworkReplyImplPrivate;
    Q_DECLARE_PRIVATE(QNetworkAccessManager)
    Q_PRIVATE_SLOT(d_func(), void _q_replyFinished())
    Q_PRIVATE_SLOT(d_func(), void _q_replySslErrors(QList<QSslError>))
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
