/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qwineventnotifier_p.h"

#include "qeventdispatcher_win_p.h"
#include "qcoreapplication.h"

#include <private/qthread_p.h>

QT_BEGIN_NAMESPACE

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

QT_END_NAMESPACE
