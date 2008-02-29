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
#include <qdplugin.h>
#include <center.h>
#include <qtopiadesktoplog.h>
#include <qcopchannel_qd.h>
#include <qcopenvelope_qd.h>

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
    QCopChannel *channel = new QCopChannel("QD/PimSync", this);
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
    QCopEnvelope e("QPE/PimSync", "serverSyncRequest(QString)");
    e << source;
}

void Qtopia4Sync::serverIdentity(const QString &server)
{
    QCopEnvelope e("QPE/PimSync", "serverIdentity(QString)");
    e << server;
}

void Qtopia4Sync::serverVersion(int major, int minor, int patch)
{
    QCopEnvelope e("QPE/PimSync", "serverVersion(int,int,int)");
    e << major;
    e << minor;
    e << patch;
}

void Qtopia4Sync::serverSyncAnchors(const QDateTime &serverLastSync, const QDateTime &serverNextSync)
{
    QCopEnvelope e("QPE/PimSync", "serverSyncAnchors(QDateTime,QDateTime)");
    e << serverLastSync;
    e << serverNextSync;
}

void Qtopia4Sync::createServerRecord(const QByteArray &record)
{
    QCopEnvelope e("QPE/PimSync", "createServerRecord(QByteArray)");
    e << record;
}

void Qtopia4Sync::replaceServerRecord(const QByteArray &record)
{
    QCopEnvelope e("QPE/PimSync", "replaceServerRecord(QByteArray)");
    e << record;
}

void Qtopia4Sync::removeServerRecord(const QString &serverId)
{
    QCopEnvelope e("QPE/PimSync", "removeServerRecord(QString)");
    e << serverId;
}

void Qtopia4Sync::requestTwoWaySync()
{
    QCopEnvelope e("QPE/PimSync", "requestTwoWaySync()");
}

void Qtopia4Sync::requestSlowSync()
{
    QCopEnvelope e("QPE/PimSync", "requestSlowSync()");
}

void Qtopia4Sync::serverError()
{
    QCopEnvelope e("QPE/PimSync", "serverError()");
}

void Qtopia4Sync::serverEnd()
{
    QCopEnvelope e("QPE/PimSync", "serverEnd()");
}

void Qtopia4Sync::handleMessage(const QString &message, const QByteArray &data)
{
    QDataStream stream(data);
    QString s1, s2;
    int i1, i2, i3;
    QDateTime ts1, ts2;
    QByteArray r;
    if (message == "clientSyncRequest(QString)") {
        stream >> s1;
        emit clientSyncRequest(s1);
    } else if (message == "clientIdentity(QString)") {
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
    } else if (message == "mapId(QString,QString)") {
        stream >> s1 >> s2;
        emit mapId(s1, s2);
    } else if (message == "clientError()") {
        emit clientError();
    } else if (message == "clientEnd()") {
        emit clientEnd();
        finishState.needEnd = false;
        if ( finishState.needFinished )
            finishSync();
    }
}

class Qtopia4DatebookSync : public Qtopia4Sync
{
    Q_OBJECT
    QD_CONSTRUCT_PLUGIN(Qtopia4DatebookSync,Qtopia4Sync)
public:
    // QDPlugin
    QString id() { return "com.trolltech.sync.qtopia4.datebook"; }
    QString displayName() { return tr("Qtopia Appointments"); }

    // QDClientSyncPlugin
    QString dataset() { return "calendar"; }
};

QD_REGISTER_PLUGIN(Qtopia4DatebookSync)

class Qtopia4AddressbookSync : public Qtopia4Sync
{
    Q_OBJECT
    QD_CONSTRUCT_PLUGIN(Qtopia4AddressbookSync,Qtopia4Sync)
public:
    // QDPlugin
    QString id() { return "com.trolltech.sync.qtopia4.addressbook"; }
    QString displayName() { return tr("Qtopia Contacts"); }

    // QDClientSyncPlugin
    QString dataset() { return "contacts"; }
};

QD_REGISTER_PLUGIN(Qtopia4AddressbookSync)

class Qtopia4TodoSync : public Qtopia4Sync
{
    Q_OBJECT
    QD_CONSTRUCT_PLUGIN(Qtopia4TodoSync,Qtopia4Sync)
public:
    // QDPlugin
    QString id() { return "com.trolltech.sync.qtopia4.todo"; }
    QString displayName() { return tr("Qtopia Tasks"); }

    // QDClientSyncPlugin
    QString dataset() { return "tasks"; }
};

QD_REGISTER_PLUGIN(Qtopia4TodoSync)

#include "qtopia4sync.moc"
