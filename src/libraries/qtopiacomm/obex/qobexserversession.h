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
#ifndef __QOBEXSERVERSESSION_H__
#define __QOBEXSERVERSESSION_H__

#include <qobexnamespace.h>
#include <QObject>

class QIODevice;
class QObexHeader;
class QObexServerSessionPrivate;
class QObexAuthenticationChallenge;
class QObexAuthenticationResponse;

class QTOPIAOBEX_EXPORT QObexServerSession : public QObject
{
    friend class QObexServerSessionPrivate;
    Q_OBJECT

public:
    enum Error {
        ConnectionError = 1,
        InvalidRequest,
        Aborted,
        AuthenticationFailed,
        UnknownError = 100
    };

    explicit QObexServerSession(QIODevice *device, QObject *parent = 0);
    virtual ~QObexServerSession() = 0;

    QIODevice *sessionDevice();
    void close();

signals:
    void finalResponseSent(QObex::Request request);
    void authenticationRequired(QObexAuthenticationChallenge *challenge);
    void authenticationResponse(const QObexAuthenticationResponse &response, bool *accept);

protected:
    void setNextResponseHeader(const QObexHeader &header);
    virtual void error(QObexServerSession::Error error, const QString &errorString);

    virtual QObex::ResponseCode dataAvailable(const char *data, qint64 size);
    virtual QObex::ResponseCode provideData(const char **data, qint64 *size);

protected slots:
    QObex::ResponseCode connect(const QObexHeader &header);
    QObex::ResponseCode disconnect(const QObexHeader &header);
    QObex::ResponseCode put(const QObexHeader &header);
    QObex::ResponseCode putDelete(const QObexHeader &header);
    QObex::ResponseCode get(const QObexHeader &header);
    QObex::ResponseCode setPath(const QObexHeader &header, QObex::SetPathFlags flags);

private:
    QObexServerSessionPrivate *m_data;
    Q_DISABLE_COPY(QObexServerSession)
};

#endif
