/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef APP_LAUNCHER_H
#define APP_LAUNCHER_H

#include <qobject.h>
#include <qmap.h>

class AppLnkSet;

class AppLauncher : public QObject
{
    Q_OBJECT
public:
    AppLauncher(const AppLnkSet *, QObject *parent = 0, const char *name = 0);
    ~AppLauncher();

    void setAppLnkSet(const AppLnkSet*);

    bool isRunning(const QString &app);

signals:
    void launched(int pid, const QString &app);
    void terminated(int pid, const QString &app);
    void connected(const QString &app);
    
protected slots:
    void sigStopped(int sigPid, int sigStatus);
    void received(const QCString& msg, const QByteArray& data);
    void newQcopChannel(const QString& channel);

protected:
    bool event(QEvent *);

private:
    static void signalHandler(int sig);
    bool executeBuiltin(const QString &c, const QString &document);
    void execute(const QString &c, const QString &document);
    
private:    
    QMap<int,QString> runningApps;
    const AppLnkSet *appLnkSet;
};

#endif

