/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#ifndef QEVENTDISPATCHER_UNIX_P_H
#define QEVENTDISPATCHER_UNIX_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qabstracteventdispatcher.h"
#include "QtCore/qlist.h"
#include "private/qabstracteventdispatcher_p.h"

#include <sys/types.h>
#include <sys/time.h>

// get time of day
inline void getTime(timeval &t)
{
    gettimeofday(&t, 0);
    // NTP-related fix
    while (t.tv_usec >= 1000000l) {
        t.tv_usec -= 1000000l;
        ++t.tv_sec;
    }
    while (t.tv_usec < 0l) {
        if (t.tv_sec > 0l) {
            t.tv_usec += 1000000l;
            --t.tv_sec;
        } else {
            t.tv_usec = 0l;
            break;
        }
    }
}

// Internal operator functions for timevals
inline bool operator<(const timeval &t1, const timeval &t2)
{ return t1.tv_sec < t2.tv_sec || (t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec); }
inline bool operator==(const timeval &t1, const timeval &t2)
{ return t1.tv_sec == t2.tv_sec && t1.tv_usec == t2.tv_usec; }
inline timeval &operator+=(timeval &t1, const timeval &t2)
{
    t1.tv_sec += t2.tv_sec;
    if ((t1.tv_usec += t2.tv_usec) >= 1000000l) {
        ++t1.tv_sec;
        t1.tv_usec -= 1000000l;
    }
    return t1;
}
inline timeval operator+(const timeval &t1, const timeval &t2)
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec + t2.tv_sec;
    if ((tmp.tv_usec = t1.tv_usec + t2.tv_usec) >= 1000000l) {
        ++tmp.tv_sec;
        tmp.tv_usec -= 1000000l;
    }
    return tmp;
}
inline timeval operator-(const timeval &t1, const timeval &t2)
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec - t2.tv_sec;
    if ((tmp.tv_usec = t1.tv_usec - t2.tv_usec) < 0l) {
        --tmp.tv_sec;
        tmp.tv_usec += 1000000l;
    }
    return tmp;
}

// internal timer info
struct QTimerInfo {
    int id;           // - timer identifier
    timeval interval; // - timer interval
    timeval timeout;  // - when to sent event
    QObject *obj;     // - object to receive event
    bool inTimerEvent;
};

class QTimerInfoList : public QList<QTimerInfo*>
{

public:
    QTimerInfoList();

    timeval watchtime;
    void updateWatchTime(const timeval &currentTime);

    bool timerWait(timeval &);
    void timerInsert(QTimerInfo *);
    void timerRepair(const timeval &);
};

struct Q_CORE_EXPORT QSockNot
{
    QSocketNotifier *obj;
    int fd;
    fd_set *queue;
};

class Q_CORE_EXPORT QSockNotType
{
public:
    QSockNotType();
    ~QSockNotType();

    QList<QSockNot*> list;
    fd_set select_fds;
    fd_set enabled_fds;
    fd_set pending_fds;

};

class QEventDispatcherUNIXPrivate;

class Q_CORE_EXPORT QEventDispatcherUNIX : public QAbstractEventDispatcher
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QEventDispatcherUNIX)

public:
    explicit QEventDispatcherUNIX(QObject *parent = 0);
    ~QEventDispatcherUNIX();

    bool processEvents(QEventLoop::ProcessEventsFlags flags);
    bool hasPendingEvents();

    void registerSocketNotifier(QSocketNotifier *notifier);
    void unregisterSocketNotifier(QSocketNotifier *notifier);

    void registerTimer(int timerId, int interval, QObject *object);
    bool unregisterTimer(int timerId);
    bool unregisterTimers(QObject *object);
    QList<TimerInfo> registeredTimers(QObject *object) const;

    void wakeUp();
    void interrupt();
    void flush();

protected:
    QEventDispatcherUNIX(QEventDispatcherUNIXPrivate &dd, QObject *parent = 0);

    void setSocketNotifierPending(QSocketNotifier *notifier);

    int activateTimers();
    int activateSocketNotifiers();

    virtual int select(int nfds,
                       fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                       timeval *timeout);
};

class Q_CORE_EXPORT QEventDispatcherUNIXPrivate : public QAbstractEventDispatcherPrivate
{
    Q_DECLARE_PUBLIC(QEventDispatcherUNIX)

public:
    QEventDispatcherUNIXPrivate();
    ~QEventDispatcherUNIXPrivate();

    int doSelect(QEventLoop::ProcessEventsFlags flags, timeval *timeout);

    bool mainThread;
    int thread_pipe[2];

    // highest fd for all socket notifiers
    int sn_highest;
    // 3 socket notifier types - read, write and exception
    QSockNotType sn_vec[3];

    QTimerInfoList timerList;

    // pending socket notifiers list
    QList<QSockNot*> sn_pending_list;

    QAtomic wakeUps;
    bool interrupt;
};

#endif // QEVENTDISPATCHER_UNIX_P_H
