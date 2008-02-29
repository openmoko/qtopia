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

#ifndef RUNNING_APP_BAR_H
#define RUNNING_APP_BAR_H

#include <qtopia/applnk.h>
#include <qtopia/global.h>

#include <qframe.h>
#include <qlist.h>
#include <qtimer.h>
#include <qmap.h>
#include <qguardedptr.h>

class AppLnkSet;
class QCString;
class QProcess;
class QMessageBox;
class TempScreenSaverMonitor;
class AppLauncher;

class RunningAppBar : public QFrame
{
    Q_OBJECT

public:
    RunningAppBar(QWidget* parent);
    ~RunningAppBar();

    void addTask(const AppLnk& appLnk);
    void removeTask(const AppLnk& appLnk);
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    QSize sizeHint() const;
    void reloadApps();

signals:
    void forceSuspend();

private slots:
    void received(const QCString& msg, const QByteArray& data);
    void applicationLaunched(int, const QString &);
    void applicationTerminated(int, const QString &);

private:
    AppLnkSet* m_AppLnkSet;
    QList<AppLnk> m_AppList;
    int m_SelectedAppIndex;
    int spacing;
    TempScreenSaverMonitor *tsmMonitor;
    AppLauncher *appLauncher;    
};

/**
 * Internal class that checks back in on the process when timerExpired is called
 * to make sure the process is on top.  If it's not it displays a dialog
 * box asking permission to kill it.
 */
class AppMonitor : public QObject {
  Q_OBJECT

 public:
  static const int RAISE_TIMEOUT_MS;

  AppMonitor(const AppLnk& app, RunningAppBar& owner);
  ~AppMonitor();
  
  private slots:
    void timerExpired();
    void received(const QCString& msg, const QByteArray& data);
    void psProcFinished();

 private:
  RunningAppBar& m_Owner;
  AppLnk m_App;
  QTimer m_Timer;
  QProcess* m_PsProc;
  QMessageBox* m_AppKillerBox;
};

class TempScreenSaverMonitor : public QObject
{
    Q_OBJECT
public:
    TempScreenSaverMonitor(QObject *parent = 0, const char *name = 0);

    void setTempMode(int,int);
    void applicationTerminated(int);

signals:
    void forceSuspend();

protected:
    void timerEvent(QTimerEvent *);

private:
    bool removeOld(int);
    void updateAll();
    int timerValue();

private:
    QValueList<int> sStatus[3];
    int currentMode;
    int timerId;
};

#endif

