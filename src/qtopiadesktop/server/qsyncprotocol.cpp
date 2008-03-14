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
#include "qsyncprotocol.h"
#include "merge.h"

#include <qdplugin.h>
#include <trace.h>
QD_LOG_OPTION(QDSyncProtocol)
#include <desktopsettings.h>

QSyncProtocol::QSyncProtocol(QObject *parent)
    : QObject(parent), pendingServerChanges(false), pendingClientChanges(false)
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::QSyncProtocol";
    state = None;
    merge = new QSyncMerge(this);
}

QSyncProtocol::~QSyncProtocol()
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::~QSyncProtocol";
    if (state != None && state != Aborted)
        abort(tr("Synchronization released while existing synchronization in progress"));
}

void QSyncProtocol::abort(const QString &message)
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::abort" << "message" << message;
    if ( state != Aborted ) {
        emit syncError(message);
        abortSync();
    } else {
        LOG() << "Already aborted... ignoring";
    }
}

void QSyncProtocol::abortSync()
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::abortSync";
    serverError();
    state = Aborted;
    merge->clearChanges();
    datasource.clear();
    clientid.clear();
}

void QSyncProtocol::serverSyncRequest(const QString &source)
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::serverSyncRequest" << "source" << source;
    client->serverSyncRequest(source);
}

void QSyncProtocol::serverIdentity(const QString &server)
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::serverIdentity" << "server" << server;
    client->serverIdentity(server);
}

void QSyncProtocol::serverVersion(int major, int minor, int patch)
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::serverVersion" << major << minor << patch;
    client->serverVersion(major, minor, patch);
}

void QSyncProtocol::serverSyncAnchors(const QDateTime &serverLastSync, const QDateTime &serverNextSync)
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::serverSyncAnchors" << "serverLastSync" << serverLastSync << "serverNextSync" << serverNextSync;
    client->serverSyncAnchors(serverLastSync, serverNextSync);
}

void QSyncProtocol::createServerRecord(const QByteArray &record)
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::createServerRecord";
    LOG() << "record" << record;
    client->createServerRecord(record);
}

void QSyncProtocol::replaceServerRecord(const QByteArray &record)
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::replaceServerRecord";
    LOG() << "record" << record;
    client->replaceServerRecord(record);
}

void QSyncProtocol::removeServerRecord(const QString &serverId)
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::removeServerRecord" << "serverId" << serverId;
    client->removeServerRecord(serverId);
}

void QSyncProtocol::requestTwoWaySync()
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::requestTwoWaySync";
    client->requestTwoWaySync();
}

void QSyncProtocol::requestSlowSync()
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::requestSlowSync";
    client->requestSlowSync();
}

void QSyncProtocol::serverError()
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::serverError";
    client->serverError();
}

void QSyncProtocol::serverEnd()
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::serverEnd";
    client->serverEnd();
}

/*
   Client initiated sync.  Essentially prepends to normal network flow
   and results in the server requesting a sync right back at the client
*/
void QSyncProtocol::startSync(QDClientSyncPlugin *_client, QDServerSyncPlugin *_server)
{
    client = _client;
    server = _server;
    TRACE(QDSyncProtocol) << "QSyncProtocol::startSync" << "client" << client << "server" << server;
    QString source = server->dataset();
    merge->setClientReferenceSchema(client->referenceSchema());
    merge->setServerReferenceSchema(server->referenceSchema());
    // assert state == None

    connect( client, SIGNAL(clientIdentity(QString)), this, SLOT(clientIdentity(QString)) );
    connect( client, SIGNAL(clientVersion(int,int,int)), this, SLOT(clientVersion(int,int,int)) );
    connect( client, SIGNAL(clientSyncAnchors(QDateTime,QDateTime)), this, SLOT(clientSyncAnchors(QDateTime,QDateTime)) );
    connect( client, SIGNAL(createClientRecord(QByteArray)), this, SLOT(createClientRecord(QByteArray)) );
    connect( client, SIGNAL(replaceClientRecord(QByteArray)), this, SLOT(replaceClientRecord(QByteArray)) );
    connect( client, SIGNAL(removeClientRecord(QString)), this, SLOT(removeClientRecord(QString)) );
    connect( client, SIGNAL(mappedId(QString,QString)), this, SLOT(clientMappedId(QString,QString)) );
    connect( client, SIGNAL(clientError()), this, SLOT(clientError()) );
    connect( client, SIGNAL(clientEnd()), this, SLOT(clientEnd()) );

    connect( server, SIGNAL(mappedId(QString,QString)), merge, SLOT(mapIdentifier(QString,QString)) );
    connect( server, SIGNAL(createServerRecord(QByteArray)), merge, SLOT(createServerRecord(QByteArray)) );
    connect( server, SIGNAL(replaceServerRecord(QByteArray)), merge, SLOT(replaceServerRecord(QByteArray)) );
    connect( server, SIGNAL(removeServerRecord(QString)), merge, SLOT(removeServerRecord(QString)) );
    connect( server, SIGNAL(serverChangesCompleted()), this, SLOT(markServerChangesComplete()) );

    merge->clearChanges();
    datasource.clear();
    clientid.clear();
    datasource = source;
    state = Header;
    serverSyncRequest(datasource);
    serverIdentity(DesktopSettings::deviceId());
    serverVersion(4, 3, 0);
    emit progress();
}

void QSyncProtocol::clientIdentity(const QString &id)
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::clientIdentity" << "id" << id;
    if (state != Header) {
        abort(tr("Protocol error - unexpected response from client"));
        return;
    }
    clientid = id;
}

void QSyncProtocol::clientVersion(int major, int minor, int patch)
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::clientVersion" << major << minor << patch;
    if (state != Header) {
        abort(tr("Protocol error - unexpected response from client"));
        return;
    }
    if (major != 4 || minor != 3 || patch != 0) {
        abort(tr("Invalid client version %1.%2.%3, client version 4.3.0 required").arg(major).arg(minor).arg(patch));
        return;
    }


    lastSync = merge->lastSync(clientid, datasource);
    // TODO nextSync

    serverSyncAnchors(lastSync, nextSync);
}

void QSyncProtocol::clientSyncAnchors(const QDateTime &clientLastSync, const QDateTime &clientNextSync)
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::clientSyncAnchors" << "clientLastSync" << clientLastSync << "clientNextSync" << clientNextSync;
    if (state != Header) {
        abort(tr("Protocol error - unexpected response from client"));
        return;
    }
    state = ClientDiff;
    emit progress();
    pendingServerChanges = true;
    pendingClientChanges = true;
    LOG() << "compare last sync" << lastSync << clientLastSync;
    nextSync = clientNextSync; // so last sync when stored will match client
    if (lastSync.isNull()
            // We need to use a reduced resolution for comparrison because QDateTime
            // cannot be saved and restored to a database without losing resolution.
            || lastSync.toString("yyyy.MM.dd hh:mm:ss") != clientLastSync.toString("yyyy.MM.dd hh:mm:ss")) {
        SyncLog() << "Performing slow sync" << endl;
        // clear id mapping for this sync.
        // TODO merge should have specific mappings for specific plugins
        merge->clearIdentifierMap();
        requestSlowSync();
        server->fetchChangesSince(QDateTime());
    } else {
        SyncLog() << "Performing fast sync" << endl;
        requestTwoWaySync();
        server->fetchChangesSince(lastSync.addSecs(1)); // don't re-sync items matching last time-stamp
    }
}

void QSyncProtocol::createClientRecord(const QByteArray &record)
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::createClientRecord";
    LOG() << "record" << record;
    if (state != ClientDiff) {
        abort(tr("Protocol error - unexpected response from client"));
        return;
    }
    merge->createClientRecord(record);
}

void QSyncProtocol::replaceClientRecord(const QByteArray &record)
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::replaceClientRecord";
    LOG() << "record" << record;
    if (state != ClientDiff) {
        abort(tr("Protocol error - unexpected response from client"));
        return;
    }
    merge->replaceClientRecord(record);
}

void QSyncProtocol::removeClientRecord(const QString &clientId)
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::removeClientRecord" << "clientId" << clientId;
    if (state != ClientDiff) {
        abort(tr("Protocol error - unexpected response from client"));
        return;
    }
    merge->removeClientRecord(clientId);
}

void QSyncProtocol::clientMappedId(const QString &serverId, const QString &clientId)
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::clientMappedId" << "serverId" << serverId << "clientId" << clientId;
    if (state != IdMapping) {
        abort(tr("Protocol error - unexpected response from client"));
        return;
    }
    merge->mapIdentifier(serverId, clientId);
}

void QSyncProtocol::clientError()
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::clientError";
    state = None;
    emit syncError(tr("Client indicated synchronization failure"));
    merge->clearChanges();
    datasource.clear();
    clientid.clear();
}

void QSyncProtocol::markServerChangesComplete()
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::markServerChangesComplete";
    pendingServerChanges = false;
    if (!pendingClientChanges)
        mergeAndApply();
}

void QSyncProtocol::clientEnd()
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::clientEnd";
    switch(state) {
        case ClientDiff:
            pendingClientChanges = false;
            if (!pendingServerChanges)
                mergeAndApply();
            break;
        case IdMapping:
            state = None;
            emit syncComplete();
            merge->recordLastSync(clientid, datasource, nextSync);
            break;
        default:
            abort(tr("Protocol error - unexpected response from client"));
            break;
    }
}

void QSyncProtocol::mergeAndApply()
{
    TRACE(QDSyncProtocol) << "QSyncProtocol::mergeAndApply";
    QList<Change> serverChanges, clientChanges;

    serverChanges = merge->serverDiff();
    clientChanges = merge->clientDiff();

    SyncLog() << "There are" << serverChanges.count() << "server changes and" << clientChanges.count() << "client changes to resolve" << endl;
    LOG() << "pre-resolve: sc" << serverChanges.count() << "cc" << clientChanges.count();

    // bias client... later based from config.
    merge->resolveAllServer();
    merge->resolveAllClient(); // not all changes are server-resolvable

    serverChanges = merge->serverDiff();
    clientChanges = merge->clientDiff();

    SyncLog() << "After resolving there are" << serverChanges.count() << "server changes and" << clientChanges.count() << "client changes to apply" << endl;
    LOG() << "post-resolve: sc" << serverChanges.count() << "cc" << clientChanges.count();

    state = IdMapping;
    emit progress();
    foreach(const Change &c, serverChanges) {
        switch(c.type) {
            case Change::Create:
                createServerRecord(c.record);
                break;
            case Change::Replace:
                replaceServerRecord(c.record);
                break;
            case Change::Remove:
                removeServerRecord(c.id);
                break;
        }
    }

    emit progress();
    foreach(const Change &c, clientChanges) {
        switch(c.type) {
            case Change::Create:
                server->createClientRecord(c.record);
                break;
            case Change::Replace:
                server->replaceClientRecord(c.record);
                break;
            case Change::Remove:
                server->removeClientRecord(c.id);
                break;
        }
    }

    emit progress();
    serverEnd(); // indicate end of diff.
}

