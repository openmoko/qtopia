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
#include <qdplugin.h>
#include <center.h>
#include <qtopiadesktoplog.h>
#include <qcopchannel_qd.h>
#include <qcopenvelope_qd.h>
#include <QApplication>
#include <trace.h>
QD_LOG_OPTION(Qtopia4Sync)

#define SEND_CHANNEL "QPE/Qtopia4Sync"
#define RECEIVE_CHANNEL "QD/Qtopia4Sync"

class Qtopia4Sync : public QDClientSyncPlugin
{
    Q_OBJECT
    QD_CONSTRUCT_PLUGIN(Qtopia4Sync,QDClientSyncPlugin)
public:
    // QDPlugin
    void init();

    // QDSyncPlugin
    void prepareForSync();
    void finishSync();

    // QDClientSyncPlugin
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

private slots:
    void handleMessage(const QString &message, const QByteArray &data);

private:
    struct FinishState {
        bool needFinished;
        bool needEnd;
    } finishState;
};

void Qtopia4Sync::init()
{
    QCopChannel *channel = new QCopChannel(RECEIVE_CHANNEL, this);
    connect(channel, SIGNAL(received(QString,QByteArray)),
            this, SLOT(handleMessage(QString,QByteArray)));
}

void Qtopia4Sync::prepareForSync()
{
    bool ret = false;
    if ( centerInterface()->currentDevice() && QCopChannel::isRegistered("QPE/*") ) {
        QCopEnvelope e( "QD/Connection", "setBusy()" );
        ret = true;
    }
    finishState.needFinished = false;
    finishState.needEnd = false;
    emit readyForSync( ret );
}

void Qtopia4Sync::finishSync()
{
    // If we haven't received a clientEnd() message wait for it before sending the finishedSync() message.
    if ( finishState.needEnd ) {
        finishState.needFinished = true;
        return;
    }

    QCopEnvelope e( "QD/Connection", "clearBusy()" );
    emit finishedSync();
}

void Qtopia4Sync::serverSyncRequest(const QString &source)
{
    QCopEnvelope e(SEND_CHANNEL, "serverSyncRequest(QString)");
    e << source;
}

void Qtopia4Sync::serverIdentity(const QString &server)
{
    QCopEnvelope e(SEND_CHANNEL, "serverIdentity(QString)");
    e << server;
}

void Qtopia4Sync::serverVersion(int major, int minor, int patch)
{
    QCopEnvelope e(SEND_CHANNEL, "serverVersion(int,int,int)");
    e << major;
    e << minor;
    e << patch;
}

void Qtopia4Sync::serverSyncAnchors(const QDateTime &serverLastSync, const QDateTime &serverNextSync)
{
    QCopEnvelope e(SEND_CHANNEL, "serverSyncAnchors(QDateTime,QDateTime)");
    e << serverLastSync;
    e << serverNextSync;
}

void Qtopia4Sync::createServerRecord(const QByteArray &record)
{
    QCopEnvelope e(SEND_CHANNEL, "createServerRecord(QByteArray)");
    e << record;
}

void Qtopia4Sync::replaceServerRecord(const QByteArray &record)
{
    QCopEnvelope e(SEND_CHANNEL, "replaceServerRecord(QByteArray)");
    e << record;
}

void Qtopia4Sync::removeServerRecord(const QString &serverId)
{
    QCopEnvelope e(SEND_CHANNEL, "removeServerRecord(QString)");
    e << serverId;
}

void Qtopia4Sync::requestTwoWaySync()
{
    QCopEnvelope e(SEND_CHANNEL, "requestTwoWaySync()");
}

void Qtopia4Sync::requestSlowSync()
{
    QCopEnvelope e(SEND_CHANNEL, "requestSlowSync()");
}

void Qtopia4Sync::serverError()
{
    QCopEnvelope e(SEND_CHANNEL, "serverError()");
}

void Qtopia4Sync::serverEnd()
{
    QCopEnvelope e(SEND_CHANNEL, "serverEnd()");
}

void Qtopia4Sync::handleMessage(const QString &message, const QByteArray &data)
{
    QDataStream stream(data);
    QString s1, s2;
    int i1, i2, i3;
    QDateTime ts1, ts2;
    QByteArray r;
    if (message == "clientIdentity(QString)") {
        stream >> s1;
        emit clientIdentity(s1);
        finishState.needEnd = true;
    } else if (message == "clientVersion(int,int,int)") {
        stream >> i1 >> i2 >> i3;
        emit clientVersion(i1, i2, i3);
    } else if (message == "clientSyncAnchors(QDateTime,QDateTime)") {
        stream >> ts1 >> ts2;
        emit clientSyncAnchors(ts1, ts2);
    } else if (message == "createClientRecord(QByteArray)") {
        stream >> r;
        emit createClientRecord(r);
    } else if (message == "replaceClientRecord(QByteArray)") {
        stream >> r;
        emit replaceClientRecord(r);
    } else if (message == "removeClientRecord(QString)") {
        stream >> s1;
        emit removeClientRecord(s1);
    } else if (message == "mappedId(QString,QString)") {
        stream >> s1 >> s2;
        emit mappedId(s1, s2);
    } else if (message == "clientError()") {
        emit clientError();
    } else if (message == "clientEnd()") {
        emit clientEnd();
        finishState.needEnd = false;
        if ( finishState.needFinished )
            finishSync();
    }
}

// =====================================================================

class Qtopia4SyncImpl : public Qtopia4Sync
{
    Q_OBJECT
public:
    Qtopia4SyncImpl( const QString &dataset, QObject *parent = 0 )
        : Qtopia4Sync( parent ), mDataset( dataset )
    {
    }
    ~Qtopia4SyncImpl()
    {
    }

    // QDPlugin
    QString id() { return QString("com.trolltech.sync.qtopia4.%1").arg(mDataset); }
    QString displayName() { return QString("Qtopia Sync (%1)").arg(mDataset); } // FIXME tr()

    // QDClientSyncPlugin
    QString dataset() { return mDataset; }

private:
    QString mDataset;
};

// =====================================================================

class Qtopia4MultiSync : public QDClientSyncPluginFactory
{
    Q_OBJECT
    QD_CONSTRUCT_PLUGIN(Qtopia4MultiSync,QDClientSyncPluginFactory)
public:
    // QDPlugin
    QString id() { return "com.trolltech.multisync.qtopia4"; }
    QString displayName() { return QString("Qtopia Sync"); } // FIXME tr()
    void init();

    // QDClientSyncPluginFactory
    QStringList datasets();
    QDClientSyncPlugin *pluginForDataset( const QString &dataset );

private slots:
    void checkForDatasets();

private:
    QStringList mDatasets;
};

QD_REGISTER_PLUGIN(Qtopia4MultiSync)

// =====================================================================

void Qtopia4MultiSync::init()
{
    connect( qApp, SIGNAL(setConnectionState(int)), this, SLOT(checkForDatasets()) );
}

void Qtopia4MultiSync::checkForDatasets()
{
    TRACE(Qtopia4Sync) << "Qtopia4MultiSync::checkForDatasets";
    mDatasets = QStringList();
    QDDevPlugin *dev = centerInterface()->currentDevice();
    if ( !dev || dev != centerInterface()->getPlugin("com.trolltech.plugin.dev.qtopia4") ) return;
    QDConPlugin *con = dev->connection();
    if ( !con ) return;
    mDatasets = con->conProperty("datasets").split(" ");
}

QStringList Qtopia4MultiSync::datasets()
{
    return mDatasets;
}

QDClientSyncPlugin *Qtopia4MultiSync::pluginForDataset( const QString &dataset )
{
    return new Qtopia4SyncImpl(dataset, centerInterface()->syncObject());
}

#include "qtopia4sync.moc"
