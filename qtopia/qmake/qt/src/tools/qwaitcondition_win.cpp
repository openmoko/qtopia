/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#if defined(QT_THREAD_SUPPORT)

#include "qwaitcondition.h"
#include "qnamespace.h"
#include "qmutex.h"
#include "qptrlist.h"
#include "qt_windows.h"

#define Q_MUTEX_T void*
#include <private/qmutex_p.h>
#include <private/qcriticalsection_p.h>

//***********************************************************************
// QWaitConditionPrivate
// **********************************************************************

class QWaitConditionEvent
{
public:
    inline QWaitConditionEvent() : priority(0)
    {
	QT_WA ({
	    event = CreateEvent(NULL, TRUE, FALSE, NULL);
	}, {
	    event = CreateEventA(NULL, TRUE, FALSE, NULL);
	});
    }
    inline ~QWaitConditionEvent() { CloseHandle(event); }
    int priority;
    HANDLE event;
};

typedef QPtrList<QWaitConditionEvent> EventQueue;

class QWaitConditionPrivate
{
public:
    QCriticalSection cs;
    EventQueue queue;
    EventQueue freeQueue;

    bool wait(QMutex *mutex, unsigned long time);
};

bool QWaitConditionPrivate::wait(QMutex *mutex, unsigned long time)
{
    bool ret = FALSE;

    cs.enter();
    QWaitConditionEvent *wce = freeQueue.take();
    if (!wce)
	wce = new QWaitConditionEvent;
    wce->priority = GetThreadPriority(GetCurrentThread());

    // insert 'wce' into the queue (sorted by priority)
    QWaitConditionEvent *current = queue.first();
    int index = 0;
    while (current && current->priority >= wce->priority) {
	current = queue.next();
	++index;
    }
    queue.insert(index, wce);
    cs.leave();

    if (mutex) mutex->unlock();

    // wait for the event
    switch (WaitForSingleObject(wce->event, time)) {
    default: break;

    case WAIT_OBJECT_0:
	ret = TRUE;
	break;
    }

    if (mutex) mutex->lock();

    cs.enter();
    // remove 'wce' from the queue
    queue.removeRef(wce);
    ResetEvent(wce->event);
    freeQueue.append(wce);
    cs.leave();

    return ret;
}

//***********************************************************************
// QWaitCondition implementation
//***********************************************************************

QWaitCondition::QWaitCondition()
{
    d = new QWaitConditionPrivate;
    d->freeQueue.setAutoDelete(TRUE);
}

QWaitCondition::~QWaitCondition()
{ 
    Q_ASSERT(d->queue.isEmpty());
    delete d; 
}

bool QWaitCondition::wait( unsigned long time )
{
    return d->wait(0, time);
}

bool QWaitCondition::wait( QMutex *mutex, unsigned long time)
{
    if ( !mutex )
	return FALSE;

    if ( mutex->d->type() == Q_MUTEX_RECURSIVE ) {
#ifdef QT_CHECK_RANGE
	qWarning("QWaitCondition::wait: Cannot wait on recursive mutexes.");
#endif
	return FALSE;
    }
    return d->wait(mutex, time);
}

void QWaitCondition::wakeOne()
{
    // wake up the first thread in the queue
    d->cs.enter();
    QWaitConditionEvent *first = d->queue.first();
    if (first)
	SetEvent(first->event);
    d->cs.leave();
}

void QWaitCondition::wakeAll()
{
    // wake up the all threads in the queue
    d->cs.enter();
    QWaitConditionEvent *current = d->queue.first();
    while (current) {
	SetEvent(current->event);
	current = d->queue.next();
    }
    d->cs.leave();
}

#endif // QT_THREAD_SUPPORT
