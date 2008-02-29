/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef __QOBEXPUSHCLIENT_H__
#define __QOBEXPUSHCLIENT_H__

#include <qobexnamespace.h>

#include <QObject>
#include <QString>

class QObexPushClientPrivate;
class QByteArray;
class QIODevice;

class QTOPIAOBEX_EXPORT QObexPushClient : public QObject
{
    Q_OBJECT
public:

    enum Command {
        None,
        Connect,
        Disconnect,
        Send,
        SendBusinessCard,
        RequestBusinessCard
    };

    enum Error {
        NoError,
        ConnectionError,
        RequestFailed,
        Aborted,
        UnknownError = 100
    };

    explicit QObexPushClient(QIODevice *device, QObject *parent = 0);
    virtual ~QObexPushClient();

    QIODevice *sessionDevice() const;

    Error error() const;
    QObex::ResponseCode lastCommandResponse() const;

    int currentId() const;
    Command currentCommand() const;
    bool hasPendingCommands() const;
    void clearPendingCommands();

    int connect();
    int disconnect();

    int send(QIODevice *device,
             const QString &name,
             const QString &type = QString(),
             const QString &description = QString());

    int send(const QByteArray &array,
             const QString &name,
             const QString &type = QString(),
             const QString &description = QString());

    int sendBusinessCard(QIODevice *vcard);
    int requestBusinessCard(QIODevice *vcard);
    void exchangeBusinessCard(QIODevice *mine, QIODevice *theirs,
                                int *putId = 0, int *getId = 0);

public slots:
    void abort();

signals:
    void commandStarted(int id);
    void commandFinished(int id, bool error);
    void dataTransferProgress(qint64, qint64);
    void done(bool error);

private:
    friend class QObexPushClientPrivate;
    QObexPushClientPrivate *m_data;
    Q_DISABLE_COPY(QObexPushClient)
};

#endif
