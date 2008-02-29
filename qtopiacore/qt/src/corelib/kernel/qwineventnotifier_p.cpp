/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qwineventnotifier_p.h"

#include "qeventdispatcher_win_p.h"
#include "qcoreapplication.h"

#include <private/qthread_p.h>

/*
    \class QWinEventNotifier qwineventnotifier.h
    \brief The QWinEventNotifier class provides support for the Windows Wait functions.

    \ingroup io

    The QWinEventNotifier class makes it possible to use the wait
    functions on windows in a asynchronous manner. With this class
    you can register a HANDLE to an event and get notification when
    that event becomes signalled. The state of the event is not modified
    in the process so if it is a manual reset event you will need to
    reset it after the notification.
*/


QWinEventNotifier::QWinEventNotifier(QObject *parent)
  : QObject(parent), handleToEvent(0), enabled(false)
{}

QWinEventNotifier::QWinEventNotifier(HANDLE hEvent, QObject *parent)
 : QObject(parent), handleToEvent(hEvent), enabled(false)
{
    Q_D(QObject);
    QEventDispatcherWin32 *eventDispatcher = qobject_cast<QEventDispatcherWin32 *>(d->threadData->eventDispatcher);
    Q_ASSERT_X(eventDispatcher, "QWinEventNotifier::QWinEventNotifier()",
               "Cannot create a win event notifier without a QEventDispatcherWin32");
    eventDispatcher->registerEventNotifier(this);
    enabled = true;
}

QWinEventNotifier::~QWinEventNotifier()
{
    setEnabled(false);
}

void QWinEventNotifier::setHandle(HANDLE hEvent)
{
    setEnabled(false);
    handleToEvent = hEvent;
}

HANDLE  QWinEventNotifier::handle() const
{
    return handleToEvent;
}

bool QWinEventNotifier::isEnabled() const
{
    return enabled;
}

void QWinEventNotifier::setEnabled(bool enable)
{
    if (enabled == enable)                        // no change
        return;
    enabled = enable;

    Q_D(QObject);
    QEventDispatcherWin32 *eventDispatcher = qobject_cast<QEventDispatcherWin32 *>(d->threadData->eventDispatcher);
    if (!eventDispatcher) // perhaps application is shutting down
        return;

    if (enabled)
        eventDispatcher->registerEventNotifier(this);
    else
        eventDispatcher->unregisterEventNotifier(this);
}

bool QWinEventNotifier::event(QEvent * e)
{
    if (e->type() == QEvent::ThreadChange) {
        if (enabled) {
            QMetaObject::invokeMethod(this, "setEnabled", Qt::QueuedConnection,
                                      Q_ARG(bool, enabled));
            setEnabled(false);
        }
    }
    QObject::event(e);                        // will activate filters
    if (e->type() == QEvent::WinEventAct) {
        emit activated(handleToEvent);
        return true;
    }
    return false;
}
