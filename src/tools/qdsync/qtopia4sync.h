/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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
#ifndef QTOPIASYNC_H
#define QTOPIASYNC_H

#include <QObject>

class Qtopia4SyncPlugin;
class Qtopia4SyncPrivate;
class QDateTime;
class QByteArray;

class Qtopia4SyncPluginFactory : public QObject
{
    Q_OBJECT
public:
    Qtopia4SyncPluginFactory( QObject *parent = 0 );
    virtual ~Qtopia4SyncPluginFactory();

    virtual QStringList keys() = 0;
    virtual Qtopia4SyncPlugin *plugin( const QString &key ) = 0;
};

class Qtopia4SyncPlugin : public QObject
{
    Q_OBJECT
public:
    Qtopia4SyncPlugin( QObject *parent = 0 );
    virtual ~Qtopia4SyncPlugin();

    virtual QString dataset() = 0;

    virtual void fetchChangesSince(const QDateTime &timestamp) = 0;

    virtual void createServerRecord(const QByteArray &record) = 0;
    virtual void replaceServerRecord(const QByteArray &record) = 0;
    virtual void removeServerRecord(const QString &identifier) = 0;

    virtual void beginTransaction(const QDateTime &timestamp) = 0;
    virtual void abortTransaction() = 0;
    virtual void commitTransaction() = 0;

signals:
    void mappedId(const QString &serverId, const QString &clientId);
    void createClientRecord(const QByteArray &record);
    void replaceClientRecord(const QByteArray &record);
    void removeClientRecord(const QString &identifier);
    void clientChangesCompleted();
    void clientError();
};

class Qtopia4Sync : public QObject
{
    Q_OBJECT
private:
    Qtopia4Sync();
public:
    virtual ~Qtopia4Sync();

    static Qtopia4Sync *instance();
    void registerPlugin( Qtopia4SyncPlugin *plugin );
    QStringList datasets();

private slots:
    void handleMessage( const QString &message, const QByteArray &data );

    void clientIdentity(const QString &id);
    void clientVersion(int major, int minor, int patch);
    void clientSyncAnchors(const QDateTime &clientLastSync, const QDateTime &clientNextSync);
    void createClientRecord(const QByteArray &record);
    void replaceClientRecord(const QByteArray &record);
    void removeClientRecord(const QString &clientId);
    void mappedId(const QString &serverId, const QString &clientId);
    void clientError();
    void clientEnd();
    void abort();

private:
    void serverSyncRequest(const QString &source);
    void serverIdentity(const QString &server);
    void serverVersion(int major, int minor, int patch);
    void serverSyncAnchors(const QDateTime &serverLastSync, const QDateTime &serverNextSync);
    void createServerRecord(const QByteArray &record);
    void replaceServerRecord(const QByteArray &record);
    void removeServerRecord(const QString &serverId);
    void requestTwoWaySync();
    void requestSlowSync();
    void serverError();
    void serverEnd();

    void initsyncVars();
    void cleanupPlugin();
    void checkTransaction();

    Qtopia4SyncPrivate *d;
};

#endif
