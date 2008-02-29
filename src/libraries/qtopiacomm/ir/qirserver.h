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

#ifndef __QIRSERVER_H__
#define __QIRSERVER_H__

#include <QObject>

#include <qirglobal.h>
#include <qirnamespace.h>

#include <qirsocket.h>

class QIrServerPrivate;

class QIR_EXPORT QIrServer : public QObject
{
    Q_OBJECT
    friend class QIrServerPrivate;

public:
    explicit QIrServer(QObject *parent = 0);
    ~QIrServer();

    void close();

    bool listen(const QByteArray &service, QIr::DeviceClasses classes = QIr::OBEX);

    QByteArray serverService() const;
    QIr::DeviceClasses serverDeviceClasses() const;

    bool isListening() const;

    QIrSocket::SocketError serverError() const;
    QString errorString() const;

    int maxPendingConnections() const;
    void setMaxPendingConnections(int max);

    bool waitForNewConnection(int msec = 0, bool *timedOut = 0);
    bool hasPendingConnections() const;
    QIrSocket *nextPendingConnection();

    int socketDescriptor() const;

signals:
    void newConnection();

private:
    Q_DISABLE_COPY(QIrServer)
    QIrServerPrivate *m_data;
};

#endif
