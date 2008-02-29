/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore/qmutex.h>
#include <QtCore/qwaitcondition.h>

#include <stdlib.h>
#include <dbus/dbus.h>

struct DBusMutex: public QMutex
{
    inline DBusMutex()
        : QMutex( QMutex::NonRecursive )
    { }

    static DBusMutex* mutex_new()
    {
        return new DBusMutex;
    }
    
    static void mutex_free(DBusMutex *mutex)
    {
        delete mutex;
    }

    static dbus_bool_t mutex_lock(DBusMutex *mutex)
    {
        mutex->lock();
        return true;
    }

    static dbus_bool_t mutex_unlock(DBusMutex *mutex)
    {
        mutex->unlock();
        return true;
    }
};

struct DBusCondVar: public QWaitCondition
{
    inline DBusCondVar()
    { }

    static DBusCondVar* condvar_new()
    {
        return new DBusCondVar;
    }

    static void condvar_free(DBusCondVar *cond)
    {
        delete cond;
    }

    static void condvar_wait(DBusCondVar *cond, DBusMutex *mutex)
    {
        cond->wait(mutex);
    }

    static dbus_bool_t condvar_wait_timeout(DBusCondVar *cond, DBusMutex *mutex, int msec)
    {
        return cond->wait(mutex, msec);
    }

    static void condvar_wake_one(DBusCondVar *cond)
    {
        cond->wakeOne();
    }

    static void condvar_wake_all(DBusCondVar *cond)
    {
        cond->wakeAll();
    }
};

bool qDBusInitThreads()
{
    static DBusThreadFunctions fcn = {
        DBUS_THREAD_FUNCTIONS_MUTEX_NEW_MASK |
        DBUS_THREAD_FUNCTIONS_MUTEX_FREE_MASK |
        DBUS_THREAD_FUNCTIONS_MUTEX_LOCK_MASK |
        DBUS_THREAD_FUNCTIONS_MUTEX_UNLOCK_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_NEW_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_FREE_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_WAIT_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_WAIT_TIMEOUT_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_WAKE_ONE_MASK |
        DBUS_THREAD_FUNCTIONS_CONDVAR_WAKE_ALL_MASK,
        DBusMutex::mutex_new,
        DBusMutex::mutex_free,
        DBusMutex::mutex_lock,
        DBusMutex::mutex_unlock,
        DBusCondVar::condvar_new,
        DBusCondVar::condvar_free,
        DBusCondVar::condvar_wait,
        DBusCondVar::condvar_wait_timeout,
        DBusCondVar::condvar_wake_one,
        DBusCondVar::condvar_wake_all,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    dbus_threads_init(&fcn);
    return true;
}

        
