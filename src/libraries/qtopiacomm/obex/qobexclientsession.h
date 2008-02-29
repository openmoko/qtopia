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
#ifndef __QOBEXCLIENTSESSION_H__
#define __QOBEXCLIENTSESSION_H__

#include <qobexnamespace.h>
#include <qobexheader.h>

#include <qobexglobal.h>

#include <QByteArray>

class QIODevice;
class QObexClientSessionPrivate;
class QObexAuthenticationChallenge;
class QObexAuthenticationResponse;

class QTOPIAOBEX_EXPORT QObexClientSession : public QObject
{
    friend class QObexClientSessionPrivate;
    Q_OBJECT

public:
    enum Error {
        NoError,
        ConnectionError,
        RequestFailed,
        InvalidRequest,
        InvalidResponse,
        Aborted,
        AuthenticationFailed,
        UnknownError = 100
    };

    explicit QObexClientSession(QIODevice *device, QObject *parent = 0);
    virtual ~QObexClientSession();

    QIODevice *sessionDevice() const;

    int connect(const QObexHeader &header = QObexHeader());
    int disconnect(const QObexHeader &header = QObexHeader());
    int put(const QObexHeader &header, QIODevice *dev);
    int put(const QObexHeader &header, const QByteArray &data);
    int putDelete(const QObexHeader &header);
    int get(const QObexHeader &header, QIODevice *dev = 0);
    int setPath(const QObexHeader &header, QObex::SetPathFlags flags = 0);

    int currentId() const;
    QObex::Request currentRequest() const;
    QIODevice *currentDevice() const;

    void clearPendingRequests();
    bool hasPendingRequests() const;

    qint64 read(char *data, qint64 maxlen);
    QByteArray readAll();
    qint64 bytesAvailable() const;

    QObex::ResponseCode lastResponseCode() const;
    QObexHeader lastResponseHeader() const;

    quint32 connectionId() const;
    bool hasConnectionId() const;

    Error error() const;
    QString errorString() const;

public slots:
    void abort();

signals:
    void requestStarted(int id);
    void requestFinished(int id, bool error);

    void responseHeaderReceived(const QObexHeader &header);
    void dataTransferProgress(qint64 done, qint64 total);
    void readyRead();

    void done(bool error);

    void authenticationRequired(QObexAuthenticationChallenge *challenge);
    void authenticationResponse(const QObexAuthenticationResponse &response, bool *accept);

private:
    QObexClientSessionPrivate *m_data;
    Q_DISABLE_COPY(QObexClientSession)
};

#endif
