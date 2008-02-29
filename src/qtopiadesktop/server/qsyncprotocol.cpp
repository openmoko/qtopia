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
#include "qsyncprotocol.h"
#include "merge.h"

#include <qdplugin.h>
#include <trace.h>

QSyncProtocol::QSyncProtocol(QObject *parent)
    : QObject(parent), pendingServerChanges(false), pendingClientChanges(false)
{ 
    TRACE(QDSync) << "QSyncProtocol::QSyncProtocol";
    state = None;
    merge = new QSyncMerge(this);
}

QSyncProtocol::~QSyncProtocol()
{
    TRACE(QDSync) << "QSyncProtocol::~QSyncProtocol";
    if (state != None && state != Aborted)
        abort(tr("Synchronization released while existing synchronization in progress"));
}

void QSyncProtocol::abort(const QString &message)
{
    TRACE(QDSync) << "QSyncProtocol::abort" << "message" << message;
    if ( state != Aborted ) {
        emit syncError(message);
        abortSync();
    } else {
        LOG() << "Already aborted... ignoring";
    }
}

void QSyncProtocol::abortSync()
{
    TRACE(QDSync) << "QSyncProtocol::abortSync";
    serverError();
    state = Aborted;
    merge->clearChanges();
    datasource.clear();
    clientid.clear();
}

void QSyncProtocol::serverSyncRequest(const QString &source)
{
    TRACE(QDSync) << "QSyncProtocol::serverSyncRequest" << "source" << source;
    client->serverSyncRequest(source);
}

void QSyncProtocol::serverIdentity(const QString &server)
{
    TRACE(QDSync) << "QSyncProtocol::serverIdentity" << "server" << server;
    client->serverIdentity(server);
}

void QSyncProtocol::serverVersion(int major, int minor, int patch)
{
    TRACE(QDSync) << "QSyncProtocol::serverVersion" << major << minor << patch;
    client->serverVersion(major, minor, patch);
}

void QSyncProtocol::serverSyncAnchors(const QDateTime &serverLastSync, const QDateTime &serverNextSync)
{
    TRACE(QDSync) << "QSyncProtocol::serverSyncAnchors" << "serverLastSync" << serverLastSync << "serverNextSync" << serverNextSync;
    client->serverSyncAnchors(serverLastSync, serverNextSync);
}

void QSyncProtocol::createServerRecord(const QByteArray &record)
{
    TRACE(QDSync) << "QSyncProtocol::createServerRecord";
    LOG() << "record" << record;
    client->createServerRecord(record);
}

void QSyncProtocol::replaceServerRecord(const QByteArray &record)
{
    TRACE(QDSync) << "QSyncProtocol::replaceServerRecord";
    LOG() << "record" << record;
    client->replaceServerRecord(record);
}

void QSyncProtocol::removeServerRecord(const QString &serverId)
{
    TRACE(QDSync) << "QSyncProtocol::removeServerRecord" << "serverId" << serverId;
    client->removeServerRecord(serverId);
}

void QSyncProtocol::requestTwoWaySync()
{
    TRACE(QDSync) << "QSyncProtocol::requestTwoWaySync";
    client->requestTwoWaySync();
}

void QSyncProtocol::requestSlowSync()
{
    TRACE(QDSync) << "QSyncProtocol::requestSlowSync";
    client->requestSlowSync();
}

void QSyncProtocol::serverError()
{
    TRACE(QDSync) << "QSyncProtocol::serverError";
    client->serverError();
}

void QSyncProtocol::serverEnd()
{
    TRACE(QDSync) << "QSyncProtocol::serverEnd";
    client->serverEnd();
}

/*
   Client initiated sync.  Essentially prepends to normal network flow
   and results in the server requesting a sync right back at the client
*/
void QSyncProtocol::clientSyncRequest(const QString &source)
{
    TRACE(QDSync) << "QSyncProtocol::clientSyncRequest" << "source" << source;
    if (state != None) {
        abort(tr("Protocol error - unexpected response from client"));
        return;
    }
    //startSync(source);
}

void QSyncProtocol::startSync(QDClientSyncPlugin *_client, QDServerSyncPlugin *server)
{
    TRACE(QDSync) << "QSyncProtocol::startSync" << "client" << _client << "server" << server;
    client = _client;
    storage = server;
    QString source = storage->dataset();
    merge->setClientReferenceSchema(client->referenceSchema());
    merge->setServerReferenceSchema(storage->referenceSchema());
    // assert state == None

    connect( client, SIGNAL(clientSyncRequest(QString)), this, SLOT(clientSyncRequest(QString)) );
    connect( client, SIGNAL(clientIdentity(QString)), this, SLOT(clientIdentity(QString)) );
    connect( client, SIGNAL(clientVersion(int,int,int)), this, SLOT(clientVersion(int,int,int)) );
    connect( client, SIGNAL(clientSyncAnchors(QDateTime,QDateTime)), this, SLOT(clientSyncAnchors(QDateTime,QDateTime)) );
    connect( client, SIGNAL(createClientRecord(QByteArray)), this, SLOT(createClientRecord(QByteArray)) );
    connect( client, SIGNAL(replaceClientRecord(QByteArray)), this, SLOT(replaceClientRecord(QByteArray)) );
    connect( client, SIGNAL(removeClientRecord(QString)), this, SLOT(removeClientRecord(QString)) );
    connect( client, SIGNAL(mapId(QString,QString)), this, SLOT(mapId(QString,QString)) );
    connect( client, SIGNAL(clientError()), this, SLOT(clientError()) );
    connect( client, SIGNAL(clientEnd()), this, SLOT(clientEnd()) );

    connect( storage, SIGNAL(mappedId(QString,QString)), merge, SLOT(mapIdentifier(QString,QString)) );
    connect( storage, SIGNAL(createServerRecord(QByteArray)), merge, SLOT(createServerRecord(QByteArray)) );
    connect( storage, SIGNAL(replaceServerRecord(QByteArray)), merge, SLOT(replaceServerRecord(QByteArray)) );
    connect( storage, SIGNAL(removeServerRecord(QString)), merge, SLOT(removeServerRecord(QString)) );
    connect( storage, SIGNAL(serverChangesCompleted()), this, SLOT(markServerChangesComplete()) );

    merge->clearChanges();
    datasource.clear();
    clientid.clear();
    datasource = source;
    serverSyncRequest(datasource);
    serverIdentity("trolltech.qtopia.desktop");// possible to configure for multiple later.
    serverVersion(4, 3, 0);
    state = Header;
    emit progress();
}

void QSyncProtocol::clientIdentity(const QString &id)
{
    TRACE(QDSync) << "QSyncProtocol::clientIdentity" << "id" << id;
    if (state != Header) {
        abort(tr("Protocol error - unexpected response from client"));
        return;
    }
    clientid = id;
}

void QSyncProtocol::clientVersion(int major, int minor, int patch)
{
    TRACE(QDSync) << "QSyncProtocol::clientVersion" << major << minor << patch;
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
    TRACE(QDSync) << "QSyncProtocol::clientSyncAnchors" << "clientLastSync" << clientLastSync << "clientNextSync" << clientNextSync;
    if (state != Header) {
        abort(tr("Protocol error - unexpected response from client"));
        return;
    }
    state = ClientDiff;
    emit progress();
    pendingServerChanges = true;
    pendingClientChanges = true;
    LOG() << "compare last sync" << lastSync << clientLastSync;
    if (lastSync.isNull() || lastSync != clientLastSync) {
        // clear id mapping for this sync.
        // TODO merge should have specific mappings for specific plugins
        merge->clearIdentifierMap();
        requestSlowSync();
        storage->performSync(QDateTime());
    } else {
        requestTwoWaySync();
        storage->performSync(lastSync.addSecs(1)); // don't re-sync items matching last time-stamp
    }
    nextSync = clientNextSync; // so last sync when stored will match client
}

void QSyncProtocol::createClientRecord(const QByteArray &record)
{
    TRACE(QDSync) << "QSyncProtocol::createClientRecord";
    LOG() << "record" << record;
    if (state != ClientDiff) {
        abort(tr("Protocol error - unexpected response from client"));
        return;
    }
    merge->createClientRecord(record);
}

void QSyncProtocol::replaceClientRecord(const QByteArray &record)
{
    TRACE(QDSync) << "QSyncProtocol::replaceClientRecord";
    LOG() << "record" << record;
    if (state != ClientDiff) {
        abort(tr("Protocol error - unexpected response from client"));
        return;
    }
    merge->replaceClientRecord(record);
}

void QSyncProtocol::removeClientRecord(const QString &clientId)
{
    TRACE(QDSync) << "QSyncProtocol::removeClientRecord" << "clientId" << clientId;
    if (state != ClientDiff) {
        abort(tr("Protocol error - unexpected response from client"));
        return;
    }
    merge->removeClientRecord(clientId);
}

void QSyncProtocol::mapId(const QString &serverId, const QString &clientId)
{
    TRACE(QDSync) << "QSyncProtocol::mapId" << "serverId" << serverId << "clientId" << clientId;
    if (state != IdMapping) {
        abort(tr("Protocol error - unexpected response from client"));
        return;
    }
    merge->mapIdentifier(serverId, clientId);
}

void QSyncProtocol::clientError()
{
    TRACE(QDSync) << "QSyncProtocol::clientError";
    emit syncError(tr("Client indicated synchronization failure"));
    state = None;
    merge->clearChanges();
    datasource.clear();
    clientid.clear();
}

void QSyncProtocol::markServerChangesComplete()
{
    TRACE(QDSync) << "QSyncProtocol::markServerChangesComplete";
    pendingServerChanges = false;
    if (!pendingClientChanges)
        mergeAndApply();
}

void QSyncProtocol::clientEnd()
{
    TRACE(QDSync) << "QSyncProtocol::clientEnd";
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
    TRACE(QDSync) << "QSyncProtocol::mergeAndApply";
    QList<Change> serverChanges, clientChanges;

    serverChanges = merge->serverDiff();
    clientChanges = merge->clientDiff();

    LOG() << "pre-resolve: sc" << serverChanges.count() << "cc" << clientChanges.count();

    // bias client... later based from config.
    merge->resolveAllServer();
    merge->resolveAllClient(); // not all changes are server-resolvable

    serverChanges = merge->serverDiff();
    clientChanges = merge->clientDiff();

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
                storage->createClientRecord(c.record);
                break;
            case Change::Replace:
                storage->replaceClientRecord(c.record);
                break;
            case Change::Remove:
                storage->removeClientRecord(c.id);
                break;
        }
    }

    emit progress();
    serverEnd(); // indicate end of diff.
}

