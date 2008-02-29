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
#ifndef MUTEX_H
#define MUTEX_H


#include <pthread.h>
#include <errno.h>
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>
#include <qtopia/global.h>


//#define MANUAL_ERROR_CHECKING_MUTEX
//#define HAVE_COND_TIMEDWAIT_RELATIVE


class Mutex {
public:
    Mutex() {
	pthread_mutexattr_init( &attr );
	pthread_mutex_init( &mutex, &attr );
	pthread_mutexattr_destroy( &attr );

        pthread_condattr_init(&cattr);
        pthread_cond_init(&cond, &cattr);
        pthread_condattr_destroy(&cattr);

	pthread_mutex_lock( &mutex );
	pthread_mutex_unlock( &mutex );

#ifdef MANUAL_ERROR_CHECKING_MUTEX
	threadIdHoldingLock = pthread_self();
#endif
    }

    ~Mutex() {
	pthread_mutex_destroy( &mutex );
    }

    void lock() {
#ifdef MANUAL_ERROR_CHECKING_MUTEX
	while ( pthread_mutex_trylock( &mutex ) == EBUSY ) {
	    long tid = pthread_self();
	    if ( tid == threadIdHoldingLock )
		qFatal("Mutex::lock() deadlock detected");
	    qWarning("Mutex::lock() waiting for another thread to unlock the mutex");
	    sched_yield();
	}
	threadIdHoldingLock = pthread_self();
#else
# ifdef EDEADLOCK
	if ( pthread_mutex_lock( &mutex ) == EDEADLOCK ) // Valgrind apparently creates all mutexes as ERROR CHECKING
	    qFatal("Mutex::lock() deadlock detected");
# else
	pthread_mutex_lock( &mutex ); // Some POSIX threads implementations may not have error checking
# endif
#endif
    }

    void unlock() {
#ifdef MANUAL_ERROR_CHECKING_MUTEX
	long tid = pthread_self();
	if ( tid != threadIdHoldingLock )
	    qFatal("Wrong thread is unlocking the mutex!!");
#endif
	pthread_mutex_unlock( &mutex );
    }

    void wait() {
	lock();
	pthread_cond_wait( &cond, &mutex );
#ifdef MANUAL_ERROR_CHECKING_MUTEX
	threadIdHoldingLock = pthread_self(); // cond_wait releases and reaquires the mutex
#endif
	unlock();
    }

    void timedWaitAbsolute(struct timespec timeout) {
	//lock();
	pthread_cond_timedwait( &cond, &mutex, &timeout );
#ifdef MANUAL_ERROR_CHECKING_MUTEX
	threadIdHoldingLock = pthread_self(); // cond_wait releases and reaquires the mutex
#endif
	//unlock();
    }

    void timedWait(long usec) {
	//lock();
	qDebug("entering condition timed wait of %li us", usec);
#ifdef HAVE_COND_TIMEDWAIT_RELATIVE
	struct timespec reltime;
	reltime.tv_sec = usec / 1000000;
	reltime.tv_nsec = (usec % 1000000) * 1000000;
	pthread_cond_timedwait_relative(cond, mutex, &reltime);
#else
	struct timeval now;
	struct timespec timeout;
	gettimeofday(&now, 0);
	now.tv_usec += usec;
	now.tv_sec += now.tv_usec / 1000000;
	now.tv_usec %= 1000000;
	timeout.tv_sec = now.tv_sec;
	timeout.tv_nsec = now.tv_usec * 1000;
	pthread_cond_timedwait( &cond, &mutex, &timeout );
#endif
	qDebug("finished condition timed wait");
#ifdef MANUAL_ERROR_CHECKING_MUTEX
	threadIdHoldingLock = pthread_self(); // cond_wait releases and reaquires the mutex
#endif
	//unlock();
    }

    void signal() {
	pthread_cond_broadcast( &cond );
    }

private:
    pthread_mutexattr_t attr;
    pthread_mutex_t mutex;
    pthread_condattr_t cattr;
    pthread_cond_t cond;
#ifdef MANUAL_ERROR_CHECKING_MUTEX
    long threadIdHoldingLock;
#endif
};


class AutoLockUnlockMutex {
public:
    AutoLockUnlockMutex(Mutex *m) : mutex( m ) {
	mutex->lock();
    }
    ~AutoLockUnlockMutex() {
	mutex->unlock();
    }
private:
    Mutex *mutex;
};


#endif // MUTEX_H

