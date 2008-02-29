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

#include "qeventdispatcher_glib_p.h"
#include "qeventdispatcher_unix_p.h"

#include <private/qthread_p.h>

#include "qcoreapplication.h"
#include "qsocketnotifier.h"

#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qpair.h>

#include <glib.h>

struct GPollFDWithQSocketNotifier
{
    GPollFD pollfd;
    QSocketNotifier *socketNotifier;
};

struct GSocketNotifierSource
{
    GSource source;
    QList<GPollFDWithQSocketNotifier *> pollfds;
};

static gboolean socketNotifierSourcePrepare(GSource *, gint *timeout)
{
    if (timeout)
        *timeout = -1;
    return false;
}

static gboolean socketNotifierSourceCheck(GSource *source)
{
    GSocketNotifierSource *src = reinterpret_cast<GSocketNotifierSource *>(source);

    bool pending = false;
    for (int i = 0; !pending && i < src->pollfds.count(); ++i) {
        GPollFDWithQSocketNotifier *p = src->pollfds.at(i);

        if (p->pollfd.revents & G_IO_NVAL) {
            // disable the invalid socket notifier
            static const char *t[] = { "Read", "Write", "Exception" };
            qWarning("QSocketNotifier: Invalid socket %d and type '%s', disabling...",
                     p->pollfd.fd, t[int(p->socketNotifier->type())]);
            // ### note, modifies src->pollfds!
            p->socketNotifier->setEnabled(false);
        }

        pending = ((p->pollfd.revents & p->pollfd.events) != 0);
    }

    return pending;
}

static gboolean socketNotifierSourceDispatch(GSource *source, GSourceFunc, gpointer)
{
    QEvent event(QEvent::SockAct);

    GSocketNotifierSource *src = reinterpret_cast<GSocketNotifierSource *>(source);
    for (int i = 0; i < src->pollfds.count(); ++i) {
        GPollFDWithQSocketNotifier *p = src->pollfds.at(i);

        if ((p->pollfd.revents & p->pollfd.events) != 0)
            QCoreApplication::sendEvent(p->socketNotifier, &event);
    }

    return true; // ??? don't remove, right?
}

static GSourceFuncs socketNotifierSourceFuncs = {
    socketNotifierSourcePrepare,
    socketNotifierSourceCheck,
    socketNotifierSourceDispatch,
    NULL,
    NULL,
    NULL
};

struct GTimerSource
{
    GSource source;
    QTimerInfoList timerList;
};

static gboolean timerSourcePrepare(GSource *source, gint *timeout)
{
    gint dummy;
    if (!timeout)
        timeout = &dummy;

    GTimerSource *src = reinterpret_cast<GTimerSource *>(source);

    timeval tv = { 0l, 0l };
    if (src->timerList.timerWait(tv))
        *timeout = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
    else
        *timeout = -1;

    return false;
}

static gboolean timerSourceCheck(GSource *source)
{
    GTimerSource *src = reinterpret_cast<GTimerSource *>(source);

    if (src->timerList.isEmpty())
        return false;

    timeval currentTime;
    getTime(currentTime);
    if (currentTime < src->timerList.first()->timeout)
        return false;

    return true;
}

static gboolean timerSourceDispatch(GSource *source, GSourceFunc, gpointer)
{
    GTimerSource *src = reinterpret_cast<GTimerSource *>(source);

    bool first = true;
    timeval currentTime;
    int n_act = 0, maxCount = src->timerList.size();
    QTimerInfo *begin = 0;

    while (maxCount--) {
        getTime(currentTime);
        if (first) {
            src->timerList.updateWatchTime(currentTime);
            first = false;
        }

        if (src->timerList.isEmpty())
            break;
        QTimerInfo *t = src->timerList.first();
        if (currentTime < t->timeout)
            break; // no timer has expired

        if (!begin) {
            begin = t;
        } else if (begin == t) {
            // avoid sending the same timer multiple times
            break;
        } else if (t->interval <  begin->interval || t->interval == begin->interval) {
            begin = t;
        }

        // remove from list
        src->timerList.removeFirst();
        t->timeout += t->interval;
        if (t->timeout < currentTime)
            t->timeout = currentTime + t->interval;

        // reinsert timer
        src->timerList.timerInsert(t);
        if (t->interval.tv_usec > 0 || t->interval.tv_sec > 0)
            n_act++;

        if (!t->inTimerEvent) {
            // send event, but don't allow it to recurse
            t->inTimerEvent = true;

            QTimerEvent e(t->id);
            QCoreApplication::sendEvent(t->obj, &e);

            if (src->timerList.contains(t))
                t->inTimerEvent = false;
        }

        if (!src->timerList.contains(begin))
            begin = 0;
    }

    return true; // ??? don't remove, right again?
}

static GSourceFuncs timerSourceFuncs = {
    timerSourcePrepare,
    timerSourceCheck,
    timerSourceDispatch,
    NULL,
    NULL,
    NULL
};

struct GPostEventSource
{
    GSource source;
    GPollFD pollfd;
    int wakeUpPipe[2];
    QEventLoop::ProcessEventsFlags flags, previousFlags;
};

static gboolean postEventSourcePrepare(GSource *s, gint *timeout)
{
    QThreadData *data = QThreadData::current();
    if (!data)
        return false;

    gint dummy;
    if (!timeout)
        timeout = &dummy;
    *timeout = data->canWait ? -1 : 0;

    GPostEventSource *source = reinterpret_cast<GPostEventSource *>(s);
    return (!data->canWait
            || (source->flags != source->previousFlags));
}

static gboolean postEventSourceCheck(GSource *source)
{
    return (postEventSourcePrepare(source, 0)
            || reinterpret_cast<GPostEventSource *>(source)->pollfd.revents != 0);
}

static gboolean postEventSourceDispatch(GSource *s, GSourceFunc, gpointer)
{
    GPostEventSource *source = reinterpret_cast<GPostEventSource *>(s);

    char c[16];
    while (::read(source->wakeUpPipe[0], c, sizeof(c)) > 0)
        ;

    source->previousFlags = source->flags;
    QCoreApplication::sendPostedEvents(0, (source->flags & QEventLoop::DeferredDeletion) ? -1 : 0);
    return true; // i dunno, george...
}

static GSourceFuncs postEventSourceFuncs = {
    postEventSourcePrepare,
    postEventSourceCheck,
    postEventSourceDispatch,
    NULL,
    NULL,
    NULL
};


QEventDispatcherGlibPrivate::QEventDispatcherGlibPrivate()
{
    if (!g_thread_supported())
        g_thread_init(NULL);

    QCoreApplication *app = QCoreApplication::instance();
    if (app && QThread::currentThread() == app->thread()) {
        mainContext = g_main_context_default();
        g_main_context_ref(mainContext);
    } else {
        mainContext = g_main_context_new();
    }

    postEventSource = reinterpret_cast<GPostEventSource *>(g_source_new(&postEventSourceFuncs,
                                                                        sizeof(GPostEventSource)));
    g_source_set_can_recurse(&postEventSource->source, true);

    pipe(postEventSource->wakeUpPipe);
    fcntl(postEventSource->wakeUpPipe[0], F_SETFD, FD_CLOEXEC);
    fcntl(postEventSource->wakeUpPipe[1], F_SETFD, FD_CLOEXEC);
    fcntl(postEventSource->wakeUpPipe[0], F_SETFL,
          fcntl(postEventSource->wakeUpPipe[0], F_GETFL) | O_NONBLOCK);
    fcntl(postEventSource->wakeUpPipe[1], F_SETFL,
          fcntl(postEventSource->wakeUpPipe[1], F_GETFL) | O_NONBLOCK);

    postEventSource->pollfd.fd = postEventSource->wakeUpPipe[0];
    postEventSource->pollfd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;
    postEventSource->flags = postEventSource->previousFlags = QEventLoop::AllEvents;

    g_source_add_poll(&postEventSource->source, &postEventSource->pollfd);
    g_source_attach(&postEventSource->source, mainContext);

    // setup socketNotifierSource
    socketNotifierSource =
        reinterpret_cast<GSocketNotifierSource *>(g_source_new(&socketNotifierSourceFuncs,
                                                               sizeof(GSocketNotifierSource)));
    (void) new (&socketNotifierSource->pollfds) QList<GPollFDWithQSocketNotifier *>();
    g_source_set_can_recurse(&socketNotifierSource->source, true);
    g_source_attach(&socketNotifierSource->source, mainContext);

    // setup timerSource
    timerSource = reinterpret_cast<GTimerSource *>(g_source_new(&timerSourceFuncs,
                                                                sizeof(GTimerSource)));
    (void) new (&timerSource->timerList) QTimerInfoList();
    g_source_set_can_recurse(&timerSource->source, true);
    g_source_attach(&timerSource->source, mainContext);
}

QEventDispatcherGlib::QEventDispatcherGlib(QObject *parent)
    : QAbstractEventDispatcher(*(new QEventDispatcherGlibPrivate), parent)
{
}

QEventDispatcherGlib::~QEventDispatcherGlib()
{
    Q_D(QEventDispatcherGlib);

    // destroy all timer sources
    qDeleteAll(d->timerSource->timerList);
    d->timerSource->timerList.~QTimerInfoList();
    g_source_destroy(&d->timerSource->source);
    g_source_unref(&d->timerSource->source);
    d->timerSource = 0;

    // destroy socket notifier source
    for (int i = 0; i < d->socketNotifierSource->pollfds.count(); ++i) {
        GPollFDWithQSocketNotifier *p = d->socketNotifierSource->pollfds[i];
        g_source_remove_poll(&d->socketNotifierSource->source, &p->pollfd);
        delete p;
    }
    d->socketNotifierSource->pollfds.~QList<GPollFDWithQSocketNotifier *>();
    g_source_destroy(&d->socketNotifierSource->source);
    g_source_unref(&d->socketNotifierSource->source);
    d->socketNotifierSource = 0;

    // destroy post event source
    g_source_remove_poll(&d->postEventSource->source, &d->postEventSource->pollfd);
    close(d->postEventSource->wakeUpPipe[0]);
    close(d->postEventSource->wakeUpPipe[1]);
    d->postEventSource->wakeUpPipe[0] = 0;
    d->postEventSource->wakeUpPipe[1] = 0;

    g_source_destroy(&d->postEventSource->source);
    g_source_unref(&d->postEventSource->source);
    d->postEventSource = 0;

    g_main_context_unref(d->mainContext);
    d->mainContext = 0;
}

bool QEventDispatcherGlib::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    Q_D(QEventDispatcherGlib);

    const bool canWait = (flags & QEventLoop::WaitForMoreEvents);
    if (canWait)
        emit aboutToBlock();
    else
        emit awake();

    // tell postEventSourcePrepare() about any new flags
    d->postEventSource->flags = flags;
    bool result = g_main_context_iteration(d->mainContext, canWait);
    while (!result && canWait)
        result = g_main_context_iteration(d->mainContext, canWait);

    if (canWait)
        emit awake();

    return result;
}

bool QEventDispatcherGlib::hasPendingEvents()
{
    Q_D(QEventDispatcherGlib);
    return g_main_context_pending(d->mainContext);
}

void QEventDispatcherGlib::registerSocketNotifier(QSocketNotifier *socketNotifier)
{
    int sockfd;
    int type;
    if (!socketNotifier
        || (sockfd = socketNotifier->socket()) < 0
        || unsigned(sockfd) >= FD_SETSIZE
        || (type = socketNotifier->type()) < 0
        || type > 2) {
        qWarning("QSocketNotifier: Internal error");
        return;
    } else if (socketNotifier->thread() != thread()
               || thread() != QThread::currentThread()) {
        qWarning("QSocketNotifier: socket notifiers cannot be enabled from another thread");
        return;
    }

    Q_D(QEventDispatcherGlib);


    GPollFDWithQSocketNotifier *p = new GPollFDWithQSocketNotifier;
    p->pollfd.fd = sockfd;
    switch (type) {
    case QSocketNotifier::Read:
        p->pollfd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;
        break;
    case QSocketNotifier::Write:
        p->pollfd.events = G_IO_OUT | G_IO_ERR;
        break;
    case QSocketNotifier::Exception:
        p->pollfd.events = G_IO_PRI | G_IO_ERR;
        break;
    }
    p->socketNotifier = socketNotifier;

    d->socketNotifierSource->pollfds.append(p);

    g_source_add_poll(&d->socketNotifierSource->source, &p->pollfd);
}

void QEventDispatcherGlib::unregisterSocketNotifier(QSocketNotifier *socketNotifier)
{
    int sockfd;
    int type;
    if (!socketNotifier
        || (sockfd = socketNotifier->socket()) < 0
        || unsigned(sockfd) >= FD_SETSIZE
        || (type = socketNotifier->type()) < 0
        || type > 2) {
        qWarning("QSocketNotifier: Internal error");
        return;
    } else if (socketNotifier->thread() != thread()
               || thread() != QThread::currentThread()) {
        qWarning("QSocketNotifier: socket notifiers cannot be disabled from another thread");
        return;
    }

    Q_D(QEventDispatcherGlib);

    for (int i = 0; i < d->socketNotifierSource->pollfds.count(); ++i) {
        GPollFDWithQSocketNotifier *p = d->socketNotifierSource->pollfds.at(i);
        if (p->socketNotifier == socketNotifier) {
            // found it
            g_source_remove_poll(&d->socketNotifierSource->source, &p->pollfd);

            d->socketNotifierSource->pollfds.removeAt(i);
            delete p;

            return;
        }
    }
}

void QEventDispatcherGlib::registerTimer(int timerId, int interval, QObject *object)
{
    if (timerId < 1 || interval < 0 || !object) {
        qWarning("QEventDispatcherUNIX::registerTimer: invalid arguments");
        return;
    } else if (object->thread() != thread() || thread() != QThread::currentThread()) {
        qWarning("QObject::startTimer: timers cannot be started from another thread");
        return;
    }

    Q_D(QEventDispatcherGlib);

    QTimerInfo *t = new QTimerInfo;
    t->id = timerId;
    t->interval.tv_sec  = interval / 1000;
    t->interval.tv_usec = (interval % 1000) * 1000;
    timeval currentTime;
    getTime(currentTime);
    t->timeout = currentTime + t->interval;
    t->obj = object;
    t->inTimerEvent = false;

    d->timerSource->timerList.timerInsert(t);
}

bool QEventDispatcherGlib::unregisterTimer(int timerId)
{
    if (timerId < 1) {
        qWarning("QEventDispatcherUNIX::unregisterTimer: invalid argument");
        return false;
    } else if (thread() != QThread::currentThread()) {
        qWarning("QObject::killTimer: timers cannot be stopped from another thread");
        return false;
    }

    Q_D(QEventDispatcherGlib);
    // set timer inactive
    for (int i = 0; i < d->timerSource->timerList.size(); ++i) {
        register QTimerInfo *t = d->timerSource->timerList.at(i);
        if (t->id == timerId) {
            d->timerSource->timerList.removeAt(i);
            delete t;
            return true;
        }
    }
    // id not found
    return false;
}

bool QEventDispatcherGlib::unregisterTimers(QObject *object)
{
    if (!object) {
        qWarning("QEventDispatcherUNIX::unregisterTimers: invalid argument");
        return false;
    } else if (object->thread() != thread() || thread() != QThread::currentThread()) {
        qWarning("QObject::killTimers: timers cannot be stopped from another thread");
        return false;
    }

    Q_D(QEventDispatcherGlib);
    if (d->timerSource->timerList.isEmpty())
        return false;
    for (int i = 0; i < d->timerSource->timerList.size(); ++i) {
        register QTimerInfo *t = d->timerSource->timerList.at(i);
        if (t->obj == object) {
            // object found
            d->timerSource->timerList.removeAt(i);
            delete t;
            // move back one so that we don't skip the new current item
            --i;
        }
    }
    return true;
}

QList<QEventDispatcherGlib::TimerInfo> QEventDispatcherGlib::registeredTimers(QObject *object) const
{
    if (!object) {
        qWarning("QEventDispatcherUNIX:registeredTimers: invalid argument");
        return QList<TimerInfo>();
    }

    Q_D(const QEventDispatcherGlib);
    QList<TimerInfo> list;
    for (int i = 0; i < d->timerSource->timerList.size(); ++i) {
        register const QTimerInfo * const t = d->timerSource->timerList.at(i);
        if (t->obj == object)
            list << TimerInfo(t->id, t->interval.tv_sec * 1000 + t->interval.tv_usec / 1000);
    }
    return list;
}

void QEventDispatcherGlib::interrupt()
{
    wakeUp();
}

void QEventDispatcherGlib::wakeUp()
{
    Q_D(QEventDispatcherGlib);
    char c = 0;
    ::write(d->postEventSource->wakeUpPipe[1], &c, 1);
}

void QEventDispatcherGlib::flush()
{
}

QEventDispatcherGlib::QEventDispatcherGlib(QEventDispatcherGlibPrivate &dd, QObject *parent)
    : QAbstractEventDispatcher(dd, parent)
{
}
