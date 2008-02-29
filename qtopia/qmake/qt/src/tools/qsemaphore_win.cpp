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

#include "qnamespace.h"
#include "qsemaphore.h"
#include "qwaitcondition.h"
#include "qt_windows.h"

#include <private/qcriticalsection_p.h>

/*
  QSemaphore implementation
*/

class QSemaphorePrivate
{
public:
    Qt::HANDLE handle;
    int count;
    int maxCount;
    QCriticalSection protect;
    QWaitCondition dontBlock;
};

QSemaphore::QSemaphore( int maxcount )
{
    d = new QSemaphorePrivate;
    d->maxCount = maxcount;
    QT_WA( {
	d->handle = CreateSemaphore( NULL, maxcount, maxcount, NULL );
    } , {
	d->handle = CreateSemaphoreA( NULL, maxcount, maxcount, NULL );
    } );

#ifdef QT_CHECK_RANGE
    if ( !d->handle )
	qSystemWarning( "Semaphore init failure" );
#endif
    d->count = maxcount;
}

QSemaphore::~QSemaphore()
{
    if ( !CloseHandle( d->handle ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Semaphore close failure" );
#endif
    }
    delete d;
}

int QSemaphore::available() const
{
    return d->count;
}

int QSemaphore::total() const
{
    return d->maxCount;
}

int QSemaphore::operator--(int)
{
    d->protect.enter();
    if ( d->count == d->maxCount ) {
	d->protect.leave();
	return d->count;
    }

    int c = d->count;
    if ( !ReleaseSemaphore( d->handle, 1, NULL ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Semaphore release failure" );
#endif
    } else {
	c = ++d->count;
	d->dontBlock.wakeAll();
    }
    d->protect.leave();

    return c;
}

int QSemaphore::operator -=(int s)
{
    if ( !ReleaseSemaphore( d->handle, s, NULL ) ) {
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Semaphore release failure" );
#endif
	d->protect.enter();
	int c = d->count; 
	d->protect.leave();
	return c;
    }

    d->protect.enter();
    int c = d->count;
    d->count += s;
    if ( d->count > d->maxCount )
	d->count = d->maxCount;
    c = d->count;
    d->dontBlock.wakeAll();
    d->protect.leave();	
    return c;
}

int QSemaphore::operator++(int)
{
    switch ( WaitForSingleObject( d->handle, INFINITE ) ) {
    case WAIT_TIMEOUT:
    case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
	qSystemWarning( "Semaphore wait failure" );
#endif
	return d->count;
    default:
	break;
    }
    d->protect.enter();
    int c = --d->count;
    d->protect.leave();

    return c;
}

int QSemaphore::operator +=(int s)
{
    while ( d->count < s )
	d->dontBlock.wait();

    d->protect.enter();
    for ( int i = 0; i < s; i++ ) {
	switch ( WaitForSingleObject( d->handle, INFINITE ) ) {
	case WAIT_TIMEOUT:
	case WAIT_FAILED:
#ifdef QT_CHECK_RANGE
	    qSystemWarning( "Semaphore wait failure" );
#endif
	    return d->count;
	default:
	    break;
	}
	d->count--;
    }
    int c = d->count;
    d->protect.leave();

    return c;
}

bool QSemaphore::tryAccess( int n )
{
    d->protect.enter();
    if ( available() < n ) {
	d->protect.leave();
	return FALSE;
    }
    d->protect.leave();
    operator+=( n );

    return TRUE;
}

#endif
