/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
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

#ifndef QCOPCHANNEL_X11_H
#define QCOPCHANNEL_X11_H

#include <QtCore/qobject.h>
#include <qtopiaglobal.h>

#if defined(Q_WS_X11)

class QCopChannelPrivate;
class QCopX11Client;
class QCopX11Server;

class QTOPIABASE_EXPORT QCopChannel : public QObject
{
    Q_OBJECT
public:
    explicit QCopChannel(const QString& channel, QObject *parent=0);
    virtual ~QCopChannel();

    QString channel() const;

    static bool isRegistered(const QString&  channel);
    static bool send(const QString& channel, const QString& msg);
    static bool send(const QString& channel, const QString& msg,
                      const QByteArray &data);

    static bool flush();

    static void sendLocally( const QString& ch, const QString& msg,
                               const QByteArray &data);
    static void reregisterAll();

    virtual void receive(const QString& msg, const QByteArray &data);

Q_SIGNALS:
    void received(const QString& msg, const QByteArray &data);

private:
    void init(const QString& channel);

    // server side
    static void registerChannel(const QString& ch, QCopX11Client *cl);
    static void requestRegistered(const QString& ch, QCopX11Client *cl);
    static void detach(QCopX11Client *cl);
    static void detach(const QString& ch, QCopX11Client *cl);
    static void answer(QCopX11Client *cl, const QString& ch,
                        const QString& msg, const QByteArray &data);
    // client side
    QCopChannelPrivate* d;

    friend class QCopX11Server;
    friend class QCopX11Client;
};

class QTOPIABASE_EXPORT QCopServer : public QObject
{
    Q_OBJECT
public:
    QCopServer(QObject *parent);
    ~QCopServer();

    static QCopServer *instance();

signals:
    void newChannel(const QString& channel);
    void removedChannel(const QString& channel);

private:
    QCopX11Server *server;

    friend class QCopChannel;
};

#endif // Q_WS_X11

#endif // QCOPCHANNEL_X11_H
