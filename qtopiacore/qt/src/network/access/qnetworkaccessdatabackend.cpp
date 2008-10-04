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

#include "qnetworkaccessdatabackend_p.h"
#include "qnetworkrequest.h"
#include "qnetworkreply.h"
#include "qurlinfo.h"

QT_BEGIN_NAMESPACE

QNetworkAccessBackend *
QNetworkAccessDataBackendFactory::create(QNetworkAccessManager::Operation,
                                         const QNetworkRequest &request) const
{
    if (request.url().scheme() == QLatin1String("data"))
        return new QNetworkAccessDataBackend;

    return 0;
}

QNetworkAccessDataBackend::QNetworkAccessDataBackend()
{
}

QNetworkAccessDataBackend::~QNetworkAccessDataBackend()
{
}

void QNetworkAccessDataBackend::open()
{
    QUrl uri = request().url();

    if (operation() != QNetworkAccessManager::GetOperation &&
        operation() != QNetworkAccessManager::HeadOperation) {
        // data: doesn't support anything but GET
        QString msg = QObject::tr("Operation not supported on %1")
                      .arg(uri.toString());
        error(QNetworkReply::ContentOperationNotPermittedError, msg);
        finished();
        return;
    }

    if (uri.host().isEmpty()) {
        setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("text/plain;charset=US-ASCII"));

        // the following would have been the correct thing, but
        // reality often differs from the specification. People have
        // data: URIs with ? and #
        //QByteArray data = QByteArray::fromPercentEncoding(uri.encodedPath());
        QByteArray data = QByteArray::fromPercentEncoding(uri.toEncoded());

        // remove the data: scheme
        data.remove(0, 5);

        // parse it:
        int pos = data.indexOf(',');
        if (pos != -1) {
            QByteArray payload = data.mid(pos + 1);
            data.truncate(pos);
            data = data.trimmed();

            // find out if the payload is encoded in Base64
            if (data.endsWith(";base64")) {
                payload = QByteArray::fromBase64(payload);
                data.chop(7);
            }

            if (data.toLower().startsWith("charset")) {
                int i = 7;      // strlen("charset")
                while (data.at(i) == ' ')
                    ++i;
                if (data.at(i) == '=')
                    data.prepend("text/plain;");
            }

            if (!data.isEmpty())
                setHeader(QNetworkRequest::ContentTypeHeader, data.trimmed());

            setHeader(QNetworkRequest::ContentLengthHeader, payload.size());
            emit metaDataChanged();

            writeDownstreamData(payload);
            finished();
            return;
        }
    }

    // something wrong with this URI
    QString msg = QObject::tr("Invalid URI: %1").arg(uri.toString());
    error(QNetworkReply::ProtocolFailure, msg);
    finished();
}

void QNetworkAccessDataBackend::closeDownstreamChannel()
{
}

void QNetworkAccessDataBackend::closeUpstreamChannel()
{
}

bool QNetworkAccessDataBackend::waitForDownstreamReadyRead(int)
{
    return false;
}

bool QNetworkAccessDataBackend::waitForUpstreamBytesWritten(int)
{
    return false;
}

QT_END_NAMESPACE
