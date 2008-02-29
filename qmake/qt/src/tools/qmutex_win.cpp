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

#include "qt_windows.h"
#include "qmutex.h"
#include "qnamespace.h"

#define Q_MUTEX_T void*
#include <private/qmutex_p.h>
#include <private/qcriticalsection_p.h>

QMutexPrivate::~QMutexPrivate()
{
    if ( !CloseHandle( handle ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex destroy failure" );
#endif
    }
}

/*
  QRecursiveMutexPrivate - implements a recursive mutex
*/

class QRecursiveMutexPrivate : public QMutexPrivate
{
public:
    QRecursiveMutexPrivate();

    void lock();
    void unlock();
    bool locked();
    bool trylock();
    int type() const { return Q_MUTEX_RECURSIVE; }
};

QRecursiveMutexPrivate::QRecursiveMutexPrivate()
{
    QT_WA( {
	handle = CreateMutex( NULL, FALSE, NULL );
    } , {
	handle = CreateMutexA( NULL, FALSE, NULL );
    } );
#ifdef QT_CHECK_RANGE
    if ( !handle )
	qSystemWarning( "Mutex init failure" );
#endif

}

void QRecursiveMutexPrivate::lock()
{
    switch ( WaitForSingleObject( handle, INFINITE ) ) {
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex lock failure" );
#endif
	break;
    case WAIT_ABANDONED:
#ifdef QT_CHECK_RANGE
	qWarning( "Thread terminated while locking mutex!" );
#endif
	// Fall through
    default:
	break;
    }
}

void QRecursiveMutexPrivate::unlock()
{
    if ( !ReleaseMutex( handle ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex unlock failure" );
#endif
    }
}

bool QRecursiveMutexPrivate::locked()
{
    switch ( WaitForSingleObject( handle, 0) ) {
    case WAIT_TIMEOUT:
	return TRUE;
    case WAIT_ABANDONED_0:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex locktest failure" );
#endif
	ReleaseMutex( handle );
	return FALSE;
    case WAIT_OBJECT_0:
	ReleaseMutex( handle );
	return FALSE;
    case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex locktest failure" );
#endif
	break;
    default:
	break;
    }
    return TRUE;
}

bool QRecursiveMutexPrivate::trylock()
{
    switch ( WaitForSingleObject( handle, 0) ) {
    case WAIT_FAILED:
    case WAIT_TIMEOUT:
	return FALSE;
    case WAIT_ABANDONED_0:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex locktest failure" );
#endif
	return FALSE;
    case WAIT_OBJECT_0:
	break;
    default:
	break;
    }
    return TRUE;
}

/*
  QNonRecursiveMutexPrivate - implements a non-recursive mutex
*/

class QNonRecursiveMutexPrivate : public QRecursiveMutexPrivate
{
public:
    QNonRecursiveMutexPrivate();

    void lock();
    void unlock();
    bool trylock();
    int type() const { return Q_MUTEX_NORMAL; };

    unsigned int threadID;
    QCriticalSection protect;
};

QNonRecursiveMutexPrivate::QNonRecursiveMutexPrivate()
    : QRecursiveMutexPrivate()
{
    threadID = 0;
}

void QNonRecursiveMutexPrivate::lock()
{
    protect.enter();

    if ( threadID == GetCurrentThreadId() ) {
	protect.leave();
#ifdef QT_CHECK_RANGE
	qWarning( "Non-recursive mutex already locked by this thread" );
#endif
    } else {
	protect.leave();
	switch ( WaitForSingleObject( handle, INFINITE ) ) {
	case WAIT_TIMEOUT:
	case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
	    qSystemWarning( "Mutex lock failure" );
#endif
	    break;
	case WAIT_ABANDONED:
#ifdef QT_CHECK_RANGE
	    qWarning( "Thread terminated while locking mutex!" );
#endif
	    // Fall through
	default:
	    protect.enter();
	    threadID = GetCurrentThreadId();
	    protect.leave();
	    break;
	}
    }
}

void QNonRecursiveMutexPrivate::unlock()
{
    protect.enter();
    QRecursiveMutexPrivate::unlock();
    threadID = 0;
    protect.leave();
}

bool QNonRecursiveMutexPrivate::trylock()
{
    protect.enter();

    if (threadID == GetCurrentThreadId()) {
	// locked by this thread already, return FALSE
	protect.leave();
	return FALSE;
    }

    protect.leave();

    switch (WaitForSingleObject(handle, 0)) {
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
	return FALSE;
    case WAIT_ABANDONED_0:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Mutex locktest failure" );
#endif
	return FALSE;
    default:
	protect.enter();
	threadID = GetCurrentThreadId();
	protect.leave();
	break;
    }

    return TRUE;
}

/*
  QMutex implementation
*/

QMutex::QMutex(bool recursive)
{
    if ( recursive )
	d = new QRecursiveMutexPrivate();
    else
	d = new QNonRecursiveMutexPrivate();
}

QMutex::~QMutex()
{
    delete d;
}

void QMutex::lock()
{
    d->lock();
}

void QMutex::unlock()
{
    d->unlock();
}

bool QMutex::locked()
{
    return d->locked();
}

bool QMutex::tryLock()
{
    return d->trylock();
}

#endif
