/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef __QOBEXPUSHCLIENT_H__
#define __QOBEXPUSHCLIENT_H__

#include <QObject>
#include <QString>

#include <qtopiaglobal.h>

class QObexPushClientPrivate;
class QByteArray;
class QIODevice;
class QObexSocket;

class QTOPIACOMM_EXPORT QObexPushClient : public QObject
{
    friend class QObexPushClientPrivate;
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
        LinkError,
        TransportConnectionError,
        RequestFailed,
        Aborted,
        UnknownError = 100
    };

    explicit QObexPushClient(QObexSocket *socket, QObject *parent = 0);
    virtual ~QObexPushClient();

    State state() const;
    Error error() const;

    int currentId() const;
    bool hasPendingCommands() const;
    void clearPendingCommands();

    int connect();
    int disconnect();
    int send(QIODevice *device, const QString &filename,
             const QString &mimetype = QString());
    int send(const QByteArray &array, const QString &filename,
             const QString &mimetype = QString());

    int sendBusinessCard(QIODevice *vcard);
    int requestBusinessCard(QIODevice *vcard);
    void exchangeBusinessCard(QIODevice *mine, QIODevice *theirs,
                              int *putId = 0, int *getId = 0);

public slots:
    void abort();

signals:
    void done(bool error);
    void progress(qint64, qint64);
    void stateChanged(QObexPushClient::State state);
    void commandFinished(int id, bool error);
    void commandStarted(int id);

private:
    QObexPushClientPrivate *m_data;
    Q_DISABLE_COPY(QObexPushClient)
};

#endif
