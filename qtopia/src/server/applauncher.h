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

#ifndef APP_LAUNCHER_H
#define APP_LAUNCHER_H

#include <qtopia/qpeglobal.h>
#include <qobject.h>
#include <qmap.h>
#ifdef Q_OS_WIN32
#include <qtopia/qprocess.h>
#include <qlist.h>
#endif

class QProcess;
class QMessageBox;

class AppLauncher : public QObject
{
    Q_OBJECT
public:
    AppLauncher(QObject *parent = 0, const char *name = 0);
    ~AppLauncher();

    bool isRunning(const QString &app);
    const QStringList &running() const { return runningList; }

    static const int RAISE_TIMEOUT_MS;
    static void delayMessages(bool);

    int pidForName( const QString & );
    static AppLauncher *appLauncherPtr;
public slots:
    void criticalKill(const QString &app);

signals:
    void raised(const QString &app);
    void launched(int pid, const QString &app);
    void terminated(int pid, const QString &app);
    void connected(const QString &app);
    
protected slots:
    void received(const QCString& msg, const QByteArray& data);
    void newQcopChannel(const QString& channel);
    void removedQcopChannel(const QString& channel);
    void createQuickLauncher();
    void processExited();

protected:
    void timerEvent( QTimerEvent * );

private:
    bool executeBuiltin(const QString &c, const QString &document);
    bool execute(const QString &c, const QString &document, bool noRaise = FALSE);
    void appLaunched(QProcess *proc, const QString &);
    void kill( int pid );
    void queueMessage(const QCString &, const QByteArray &);
private slots:
    void sendQueuedMessages();

private:

    struct QueuedItem
    {
	QCString msg;
	QByteArray data;
    };
    QValueList<QueuedItem> qmessages;

    static bool mDelayMessages;

    struct RunningApp {
	QString name;
	QProcess *proc;
    };

    QMap<int,RunningApp> runningApps;
    QMap<QString,int> waitingHeartbeat;
    QStringList runningList;
#ifdef Q_OS_WIN32
    QList<QProcess> runningAppsProc; 
#endif
    QProcess *qlProc;
    int qlPid;
    bool qlReady;
    QMessageBox *appKillerBox;
    QString appKillerName;

    static bool wasCritMemKill;

};

#endif

