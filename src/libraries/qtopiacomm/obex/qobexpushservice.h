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

#ifndef __QOBEXPUSHSERVICE_H__
#define __QOBEXPUSHSERVICE_H__

#include <qobexnamespace.h>

#include <QObject>
#include <QString>

#include <qtopiaglobal.h>

class QObexSocket;
class QObexPushService_Private;

class QTOPIACOMM_EXPORT QObexPushService : public QObject
{
    Q_OBJECT

    friend class QObexPushService_Private;

public:

    explicit QObexPushService(QObexSocket *socket, QObject *parent = 0);
    ~QObexPushService();

    void close();

    void setAutoDelete(bool enable);
    bool autoDelete() const;

    QObex::State state() const;
    QObex::Error error() const;

    void setIncomingDirectory(const QString &dir);
    const QString &incomingDirectory() const;

    QObexSocket *socket();

    virtual QByteArray businessCard() const;


protected:
    virtual bool acceptFile(const QString &filename, const QString &mimetype, qint64 size);

signals:
    void putRequest(const QString &filename, const QString &mimetype);
    void getRequest(const QString &filename, const QString &mimetype);
    void requestComplete(bool error);
    void done(bool error);
    void progress(qint64, qint64);
    void stateChanged(QObex::State);
    void aboutToDelete();

private:
    QObexPushService_Private *m_data;
    Q_DISABLE_COPY(QObexPushService)
};

#endif
