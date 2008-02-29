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
#include "syncmanager.h"
#include "pluginmanager.h"
#ifndef NO_SYNC
#include "qsyncprotocol.h"
#endif

#include <qdplugin.h>
#include <trace.h>
#include <qcopenvelope_qd.h>

#include <QTimer>
#include <QApplication>
#include <QCloseEvent>

#ifndef SYNC_STEPS
#define SYNC_STEPS 6
#endif

SyncManager::SyncManager( QWidget *parent )
    : QProgressDialog( parent ), protocol( 0 ), mError( 0 ), mSyncObject( 0 ),
    canAbort( true ), abortLater( false )
{
    TRACE(QDSync) << "SyncManager::SyncManager";
    setWindowTitle( tr("Sync All") );
    setModal( true );
    //setAutoClose( false );
    setAutoReset( false );
    connect( this, SIGNAL(canceled()), this, SLOT(abort()) );
    connect( this, SIGNAL(finished(int)), this, SLOT(_finished(int)) );
    connect( qApp, SIGNAL(setConnectionState(int)), this, SLOT(abort()) );

    foreach ( QDSyncPlugin *plugin, qdPluginManager()->syncPlugins() ) {
        server = qobject_cast<QDServerSyncPlugin*>(plugin);
        if ( server ) {
            LOG() << "server" << server->id() << server->dataset();
            foreach ( QDSyncPlugin *plugin, qdPluginManager()->syncPlugins() ) {
                client = qobject_cast<QDClientSyncPlugin*>(plugin);
                if ( client && client->dataset() == server->dataset() ) {
                    Q_ASSERT(!pending.contains(server));
                    LOG() << "matched to" << client->id();
                    pending[server] = client;
                }
            }
        }
    }

    setMinimum( 0 );
    setMaximum( pending.count() * SYNC_STEPS );
    QCopEnvelope e("QPE/QDSync", "syncSteps(int)");
    e << pending.count() * SYNC_STEPS;
    setValue( 0 );
    completed = 0;
    client = 0;
    server = 0;
}

SyncManager::~SyncManager()
{
    TRACE(QDSync) << "SyncManager::~SyncManager";
#ifndef NO_SYNC
    if ( protocol )
        delete protocol;
#endif
}

void SyncManager::showEvent( QShowEvent * /*e*/ )
{
    TRACE(QDSync) << "SyncManager::showEvent";
    if ( pending.count() )
        QTimer::singleShot( 0, this, SLOT(next()) );
    else
        QTimer::singleShot( 0, this, SLOT(reject()) );
}

void SyncManager::next()
{
    TRACE(QDSync) << "SyncManager::next";
    Q_ASSERT( pending.count() );
    server = pending.begin().key();
    client = pending[server];
    pending.remove( server );
    setLabelText( tr("Syncing %1 with %2", "1=plugin, 2=plugin").arg(server->displayName()).arg(client->displayName()) );
    canAbort = false; // Don't process the abort until after the plugins are ready
    abortLater = false;
    qApp->processEvents();

    clientStatus = Waiting;
    serverStatus = Waiting;
    connect( client, SIGNAL(readyForSync(bool)), this, SLOT(clientReadyForSync(bool)) );
    connect( server, SIGNAL(readyForSync(bool)), this, SLOT(serverReadyForSync(bool)) );
    client->prepareForSync();
    server->prepareForSync();
}

void SyncManager::clientReadyForSync( bool ready )
{
    TRACE(QDSync) << "SyncManager::clientReadyForSync" << ready;
    disconnect( client, SIGNAL(readyForSync(bool)), this, SLOT(clientReadyForSync(bool)) );
    Q_ASSERT( clientStatus == Waiting );
    clientStatus = ready?Ready:NotReady;
    if ( serverStatus != Waiting )
        readyForSync();
}

void SyncManager::serverReadyForSync( bool ready )
{
    TRACE(QDSync) << "SyncManager::serverReadyForSync" << ready;
    disconnect( server, SIGNAL(readyForSync(bool)), this, SLOT(serverReadyForSync(bool)) );
    Q_ASSERT( serverStatus == Waiting );
    serverStatus = ready?Ready:NotReady;
    if ( clientStatus != Waiting )
        readyForSync();
}

void SyncManager::readyForSync()
{
    TRACE(QDSync) << "SyncManager::readyForSync";
    canAbort = true;
    if ( abortLater ) {
        LOG() << "Processing delayed abort()";
        abort();
        return;
    }
    bool clientOk = ( clientStatus == Ready );
    bool serverOk = ( serverStatus == Ready );
    LOG() << "client is" << (clientOk?"ok":"not ok") << "server is" << (serverOk?"ok":"not ok");
    complete = false;
    if ( clientOk && serverOk ) {
#ifndef NO_SYNC
        protocol = new QSyncProtocol;
        connect( protocol, SIGNAL(syncComplete()), this, SLOT(syncComplete()) );
        connect( protocol, SIGNAL(syncError(QString)), this, SLOT(syncError(QString)) );
        connect( protocol, SIGNAL(progress()), this, SLOT(progress()) );

        protocol->startSync( client, server );
#else
        QTimer::singleShot( 5000, this, SLOT(syncComplete()) );
#endif
    } else {
        mError++;
        QTimer::singleShot( 0, this, SLOT(syncComplete()) );
    }
}

void SyncManager::syncComplete()
{
    TRACE(QDSync) << "SyncManager::syncComplete";
    if ( complete ) {
        LOG() << "Can't call syncComplete again!";
        return;
    }
    complete = true;

    clientStatus = Waiting;
    serverStatus = Waiting;
    connect( client, SIGNAL(finishedSync()), this, SLOT(clientFinishedSync()) );
    connect( server, SIGNAL(finishedSync()), this, SLOT(serverFinishedSync()) );
    client->finishSync();
    server->finishSync();
}

void SyncManager::clientFinishedSync()
{
    TRACE(QDSync) << "SyncManager::clientFinishedSync";
    disconnect( client, SIGNAL(finishedSync()), this, SLOT(clientFinishedSync()) );
    Q_ASSERT( clientStatus == Waiting );
    clientStatus = Finished;
    if ( serverStatus != Waiting )
        finishedSync();
}

void SyncManager::serverFinishedSync()
{
    TRACE(QDSync) << "SyncManager::serverFinishedSync";
    disconnect( server, SIGNAL(finishedSync()), this, SLOT(serverFinishedSync()) );
    Q_ASSERT( serverStatus == Waiting );
    serverStatus = Finished;
    if ( clientStatus != Waiting )
        finishedSync();
}

void SyncManager::finishedSync()
{
    client = 0;
    server = 0;

    if ( protocol )
        protocol->deleteLater();

    setValue( ++completed * SYNC_STEPS );

    if ( pending.count() ) {
        if ( protocol ) {
            connect( protocol, SIGNAL(destroyed()), this, SLOT(next()) );
            protocol = 0;
        } else {
            QTimer::singleShot( 0, this, SLOT(next()) );
        }
    } else {
        protocol = 0;
        reset();
        accept();
    }
}

void SyncManager::syncError( const QString &error )
{
    TRACE(QDSync) << "SyncManager::syncError" << "error" << error;
    mError++;
    if ( protocol )
        protocol->abortSync();
    syncComplete();
}

void SyncManager::abort()
{
    TRACE(QDSync) << "SyncManager::abort";
    if ( !canAbort ) {
        LOG() << "Can't abort now, registering delayed abort";
        abortLater = true;
        return;
    }
#ifndef NO_SYNC
    if ( protocol )
        protocol->abortSync();
    syncComplete();
#endif
    reject();
}

int SyncManager::errors()
{
    return mError;
}

void SyncManager::progress()
{
    TRACE(QDSync) << "SyncManager::progress";
    QCopEnvelope e("QPE/QDSync", "syncProgress(int)");
    e << value()+1;
    setValue( value()+1 );
}

void SyncManager::closeEvent( QCloseEvent *e )
{
    TRACE(QDSync) << "SyncManager::closeEvent";
    e->ignore();
    abort();
}

QObject *SyncManager::syncObject()
{
    if ( !mSyncObject )
        mSyncObject = new QObject( this );
    return mSyncObject;
}

void SyncManager::_finished( int )
{
    TRACE(QDSync) << "SyncManager::_finished";
    if ( mSyncObject ) {
        delete mSyncObject;
        mSyncObject = 0;
    }
}

