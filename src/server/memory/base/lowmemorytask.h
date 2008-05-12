// -*-C++-*-
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

#ifndef _LOWMEMORYTASK_H_
#define _LOWMEMORYTASK_H_

#include "memorymonitor.h"
#include "applicationlauncher.h"
#include "oommanager.h"

class LowMemoryTask : public QObject
{
    Q_OBJECT

  public:
    LowMemoryTask();
    virtual ~LowMemoryTask();

  signals:
    void showWarning(const QString &title, const QString &text);

  private:
    void handleCriticalMemory();
    void handleVeryLowMemory();
    void handleLowMemory();
    bool kill(const QString& proc);
    bool quit(const QString& proc);
    int quitIfInvisible(const QMap<QString,int>& procs);
    QString selectProcess();

  private slots:
    void avoidOutOfMemory(MemoryMonitor::MemState newState);

  private:
    MemoryMonitor::MemState     m_state;
    MemoryMonitor::MemState     m_prevState;
    OomManager                  m_oomManager;
};

QTOPIA_TASK_INTERFACE(LowMemoryTask);

#endif // _LOWMEMORYTASK_H_
