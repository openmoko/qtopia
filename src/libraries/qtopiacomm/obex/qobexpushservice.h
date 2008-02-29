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

#ifndef __QOBEXPUSHSERVICE_H__
#define __QOBEXPUSHSERVICE_H__

#include <qobexnamespace.h>

#include <QObject>
#include <QString>

class QIODevice;
class QObexPushServicePrivate;

class QTOPIAOBEX_EXPORT QObexPushService : public QObject
{
    Q_OBJECT

public:

    enum State {
        Ready,
        Connecting,
        Disconnecting,
        Streaming,
        Closed = 100
    };

    enum Error {
        NoError,
        ConnectionError,
        Aborted,
        UnknownError = 100
    };

    explicit QObexPushService(QIODevice *device, QObject *parent = 0);
    ~QObexPushService();

    State state() const;
    Error error() const;

    QIODevice *sessionDevice() const;

    void setBusinessCard(const QByteArray &vCard);
    virtual QByteArray businessCard() const;

    QIODevice *currentDevice() const;

public slots:
    void abort();

protected:
    virtual QIODevice *acceptFile(const QString &name, const QString &type, qint64 size, const QString &description);

signals:
    void putRequested(const QString &name, const QString &type, qint64 size, const QString &description);
    void businessCardRequested();
    void stateChanged(QObexPushService::State);
    void dataTransferProgress(qint64 done, qint64 total);
    void requestFinished(bool error);
    void done(bool error);

private:
    friend class QObexPushServicePrivate;
    QObexPushServicePrivate *m_data;
    Q_DISABLE_COPY(QObexPushService)
};

#endif
