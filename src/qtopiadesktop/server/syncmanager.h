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
#ifndef SYNCMANAGER_H
#define SYNCMANAGER_H

#include <QProgressDialog>
#include <QMap>

class QDServerSyncPlugin;
class QDClientSyncPlugin;
class QSyncProtocol;

class SyncManager : public QProgressDialog
{
    Q_OBJECT
public:
    enum Status { Waiting, Ready, NotReady, Finished };

    SyncManager( QWidget *parent = 0 );
    ~SyncManager();

    int errors();
    QObject *syncObject();

public slots:
    void abort();

private slots:
    void next();
    void syncComplete();
    void syncError(const QString &error);
    void progress();
    void clientReadyForSync( bool ready );
    void serverReadyForSync( bool ready );
    void _finished( int result );
    void clientFinishedSync();
    void serverFinishedSync();

private:
    void showEvent( QShowEvent *e );
    void closeEvent( QCloseEvent *e );
    void readyForSync();
    void finishedSync();

    QMap<QDServerSyncPlugin*,QDClientSyncPlugin*> pending;
    QSyncProtocol *protocol;
    int mError;
    int completed;
    Status clientStatus;
    Status serverStatus;
    QDClientSyncPlugin *client;
    QDServerSyncPlugin *server;
    bool complete;
    QObject *mSyncObject;
    bool canAbort;
    bool abortLater;
};

#endif
