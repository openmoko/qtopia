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

#ifndef __QOBEXSERVER_H__
#define __QOBEXSERVER_H__

#include <QObject>
#include <QString>

#include <qtopiaglobal.h>

class QObexServer_Private;
class QObexSocket;

class QTOPIACOMM_EXPORT QObexServer : public QObject
{
    Q_OBJECT

    friend class QObexServer_Private;

public:
    explicit QObexServer(QObject *parent = 0);
    virtual ~QObexServer();

    void close();

    bool listen();
    bool isListening() const;

    bool hasPendingConnections() const;
    virtual QObexSocket *nextPendingConnection();

signals:
    void newConnection();

protected:
    virtual void *registerServer() = 0;
    void *spawnReceiver();

private:
    QObexServer_Private *m_data;
    Q_DISABLE_COPY(QObexServer)
};

#endif
