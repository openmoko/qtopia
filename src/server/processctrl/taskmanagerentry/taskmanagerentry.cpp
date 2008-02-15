/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#include "taskmanagerentry.h"
#include <qtopiaipcadaptor.h>


class TaskManagerEntryPrivate : public QtopiaIpcAdaptor
{
    Q_OBJECT
public:
    TaskManagerEntryPrivate(TaskManagerEntry *parent, const QString &description, const QString &iconPath);
    void show();
    void hide();

signals:
    void addDynamicLauncherItem(int id, const QString &description, const QString &iconPath);
    void removeDynamicLauncherItem(int id);

public slots:
    void runningApplicationsViewLoaded();
    void activatedLaunchItem(int id);

private:
    TaskManagerEntry *m_parent;
    int m_id;
    QString m_description;
    QString m_iconPath;

    bool m_runningAppsViewLoaded;
    bool m_showWhenViewLoaded;

#if QT_VERSION < 0x040400
    static QBasicAtomic idCounter;
#else
    static QAtomicInt idCounter;
#endif
    static int nextId();
};


#if QT_VERSION < 0x040400
QBasicAtomic TaskManagerEntryPrivate::idCounter = Q_ATOMIC_INIT(1);
#else
QAtomicInt TaskManagerEntryPrivate::idCounter(1);
#endif


/*
    This talks over IPC to the RunningAppsLauncherView class in
    src/server/phone/phonebrowser.h.
*/

TaskManagerEntryPrivate::TaskManagerEntryPrivate(TaskManagerEntry *parent, const QString &description, const QString &iconPath)
    : QtopiaIpcAdaptor("QPE/RunningAppsLauncherViewService", parent),
      m_parent(parent),
      m_id(nextId()),
      m_description(description),
      m_iconPath(iconPath),
      m_runningAppsViewLoaded(false),
      m_showWhenViewLoaded(false)
{
    publishAll(SignalsAndSlots);
}

void TaskManagerEntryPrivate::show()
{
    if (!m_runningAppsViewLoaded) {
        m_showWhenViewLoaded = true;
        return;
    }
    emit addDynamicLauncherItem(m_id, m_description, m_iconPath);
}

void TaskManagerEntryPrivate::hide()
{
    if (!m_runningAppsViewLoaded && m_showWhenViewLoaded) {
        m_showWhenViewLoaded = false;
        return;
    }
    emit removeDynamicLauncherItem(m_id);
}

void TaskManagerEntryPrivate::runningApplicationsViewLoaded()
{
    m_runningAppsViewLoaded = true;
    if (m_showWhenViewLoaded)
        show();
}

void TaskManagerEntryPrivate::activatedLaunchItem(int id)
{
    if (id == m_id)
        emit m_parent->activated();
}

/*
    Generates an ID for an item.
*/
int TaskManagerEntryPrivate::nextId()
{
#if QT_VERSION < 0x040400
    register int id;
    for (;;) {
        id = idCounter;
        if (idCounter.testAndSet(id, id + 1))
            break;
    }
    return id;
#else
    register int id;
    for (;;) {
        id = idCounter;
        if (idCounter.testAndSetOrdered(id, id + 1))
            break;
    }
    return id;
#endif
}


/*!
    \class TaskManagerEntry
    \brief The TaskManagerEntry class is used to insert non-application items into Qtopia's Running Applications/TaskManager window.
    \ingroup QtopiaServer::AppLaunch

    Normally, the TaskManager window only shows Qtopia applications.
    However, there may be times when you need to show other items in this
    window. For example, if you have a server task that displays an
    informational dialog about a long-running activity, you might want the dialog
    to be accessible via the TaskManager window so the user can return
    to the dialog if it is backgrounded. However, in this situation, the
    dialog will not automatically be displayed in the TaskManager
    because it is an ordinary dialog and not a Qtopia application.

    In these cases, you can use the TaskManagerEntry class. For
    example:

    \code
    TaskManagerEntry *cpuMonitorItem =
            new TaskManagerEntry(tr("CPU Monitor"), "cpuMonitorIcon");
    connect(cpuMonitorItem, SIGNAL(activated()),
            this, SLOT(raiseCpuMonitorDialog()));
    cpuMonitorItem->show();
    \endcode

    The activated() is emitted when the user activates the item in the
    TaskManager window.
*/

/*!
  Constructs a new TaskManagerEntry. \a description represents the string under which 
  the entry should appear in the task task manager launcher view and \a iconPath describes the 
  icon to be used. \a parent is the standard QObject parent parameter.
*/
TaskManagerEntry::TaskManagerEntry(const QString &description, const QString &iconPath, QObject *parent)
    : QObject(parent),
      m_data(new TaskManagerEntryPrivate(this, description, iconPath))
{
}

/*!
    Shows this item in the Running Applications window.
*/
void TaskManagerEntry::show()
{
    m_data->show();
}

/*!
    Hides this item in the Running Applications window.
*/
void TaskManagerEntry::hide()
{
    m_data->hide();
}

/*!
    \fn TaskManagerEntry::activated()

    Emitted when the user has activated this item from the Running
    Applications window.
*/

#include "taskmanagerentry.moc"
