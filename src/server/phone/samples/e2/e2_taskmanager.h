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

#ifndef HAVE_E2TASKMANAGER_H
#define HAVE_E2TASKMANAGER_H

#include "phone/taskmanagerservice.h"
#include "e2_frames.h"

#include "applicationmonitor.h"
#include <QTimer>
#include <QPair>

class E2TaskManagerService : public TaskManagerService
{
public:
    E2TaskManagerService(QObject* par);
    void multitask();
    void showRunningTasks();
};

class QTreeWidget;
class QTreeWidgetItem;
class E2Bar;
class E2MemoryBar;
class QContent;
class E2TaskManager : public E2PopupFrame
{
    Q_OBJECT
public:
    static E2TaskManager* instance();

protected:
    void showEvent(QShowEvent* e);

private slots:
    void switchToTask();
    void endTask();
    void memoryUpdate();
    void doUpdate();
private:
    QList<QPair<QTreeWidgetItem*,QContent*> > m_items;
    E2TaskManager(QWidget* par);
    int getMemoryPercentUsed() const;
    E2Bar* m_bar;
    QTreeWidget* m_taskList;
    QTimer* m_memoryTimer;
    E2MemoryBar* m_memoryBar;
    UIApplicationMonitor m_appMonitor;
};

#endif
