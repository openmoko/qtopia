/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef DESKTOP_H
#define DESKTOP_H

#include <qwidget.h>
#include <qdatetime.h>
#include "serverapp.h"

//#include "qcopbridge.h"

class QCopBridge;
class QHostAddress;
class TransferServer;
class PackageHandler;
class DeviceButton;
class ServiceRequest;
class TempScreenSaverMonitor;
class AppLauncher;
class AppLnkSet;
class StorageInfo;
class SyncDialog;
class DocumentList;
class ServerInterface;
class QueuedRequestRunner;
class QProcess;


class Server : public QWidget {
    Q_OBJECT
public:
    Server();
    ~Server();

    static bool mkdir(const QString &path);

    void show();

    static void soundAlarm();
    static bool setKeyboardLayout( const QString &kb );

    static Server *instance();
    DocumentList *documentList() const;

public slots:
    void systemMsg(const QCString &, const QByteArray &);
    void receiveTaskBar(const QCString &msg, const QByteArray &data);
    void terminateServers();
    void pokeTimeMonitors();
    void recoverMemory(ServerApplication::MemState);

private slots:
    void activate(const DeviceButton*,bool);
    void syncConnectionClosed( const QHostAddress & );
    void applicationRaised(const QString &app);
    void applicationLaunched(int pid, const QString &app);
    void applicationTerminated(int pid, const QString &app);
    void applicationConnected(const QString &app);
    void storageChanged();
    void cancelSync();
    void desktopMessage( const QCString &, const QByteArray & );
    void runDirectAccess();
    void finishedQueuedRequests();
    void soundServerExited();
    void soundServerReadyStdout();
    void soundServerReadyStderr();

protected:
    void styleChange( QStyle & );
    void timerEvent( QTimerEvent *e );

private:
    void startTransferServer();
    void preloadApps();
    void prepareDirectAccess();
    void postDirectAccess();
    QString cardInfoString();
    QString installLocationsString();

    QCopBridge *qcopBridge;
    TransferServer *transferServer;
    PackageHandler *packageHandler;
    int tid_xfer;
    int qssTimerId;
    TempScreenSaverMonitor *tsmMonitor;
    StorageInfo *storage;
    SyncDialog *syncDialog;
    AppLauncher *appLauncher;
    DocumentList *docList;
    ServerInterface *serverGui;
    QProcess *soundserver;

    int pendingFlushes;
    bool directAccessRun;
    QueuedRequestRunner *qrr;

    QString *lastStartedApp;
};

#endif // DESKTOP_H

