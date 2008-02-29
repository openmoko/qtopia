/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef FF_MUTEX_H
#define FF_MUTEX_H


#include <pthread.h>
#include <sys/time.h>


//#define MANUAL_ERROR_CHECKING_MUTEX
//#define HAVE_COND_TIMEDWAIT_RELATIVE
//#define SLOPPY_WAIT
#define DISALLOW_ASSIGNMENT


class FFMutex {
public:
    FFMutex() {
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

    ~FFMutex() {
	pthread_mutex_destroy( &mutex );
    }

    void lock() {
#ifdef MANUAL_ERROR_CHECKING_MUTEX
	while ( pthread_mutex_trylock( &mutex ) == EBUSY ) {
	    long tid = pthread_self();
	    if ( tid == threadIdHoldingLock )
		qFatal("FFMutex: deadlock detected");
	    qWarning("FFMutex: waiting for another thread to unlock the mutex");
	    sched_yield();
	}
	threadIdHoldingLock = pthread_self();
#else
# ifdef EDEADLOCK
	if ( pthread_mutex_lock( &mutex ) == EDEADLOCK ) // Valgrind apparently creates all mutexes as ERROR CHECKING
	    qFatal("FFMutex: deadlock detected");
# else
	pthread_mutex_lock( &mutex ); // Some pthread implementation may not contain the required extentions to POSIX threads
# endif
#endif
    }

    void unlock() {
#ifdef MANUAL_ERROR_CHECKING_MUTEX
	long tid = pthread_self();
	if ( tid != threadIdHoldingLock )
	    qFatal("FFMutex: Wrong thread is unlocking the mutex!!");
#endif
	pthread_mutex_unlock( &mutex );
    }

    void wait() {
#ifdef SLOPPY_WAIT
	timedWait(30000000); // Release after 30 seconds
#else
	lock();
	pthread_cond_wait( &cond, &mutex );
# ifdef MANUAL_ERROR_CHECKING_MUTEX
	threadIdHoldingLock = pthread_self(); // cond_wait releases and reaquires the mutex
# endif
	unlock();
#endif
    }

    void timedWait(long usec) {
	lock();
#ifdef HAVE_COND_TIMEDWAIT_RELATIVE
	struct timespec reltime;
	reltime.tv_sec = usec / 1000000;
	reltime.tv_nsec = (usec % 1000000) * 1000000;
	pthread_cond_timedwait_relative(&cond, &mutex, &reltime);
#else
	struct timeval now;
	struct timespec timeout;
	//threadsafe_gettimeofday(&now, 0);
	gettimeofday(&now, 0);
	now.tv_usec += usec;
	now.tv_sec += now.tv_usec / 1000000;
	now.tv_usec %= 1000000;
	timeout.tv_sec = now.tv_sec;
	timeout.tv_nsec = now.tv_usec * 1000;
	pthread_cond_timedwait( &cond, &mutex, &timeout );
#endif
#ifdef MANUAL_ERROR_CHECKING_MUTEX
	threadIdHoldingLock = pthread_self(); // cond_wait releases and reaquires the mutex
#endif
	unlock();
    }

    void signal() {
	//qDebug("signalling condition");
	pthread_cond_broadcast( &cond );
	//qDebug("signalled condition");
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


//
// C++ automatic unlocking of mutex when object of this type goes out of scope.
// Nice in that you don't have to find all the exit points of a function to
// ensure you don't miss putting a mutex unlock in somewhere where it belongs.
//
class FFAutoLockUnlockMutex {
public:
    FFAutoLockUnlockMutex( FFMutex &m ) : mutex( m ) {
	mutex.lock();
    }
    ~FFAutoLockUnlockMutex() {
	mutex.unlock();
    }
private:
    FFMutex &mutex;
};


//
// Use sparingly and carefully
//
// The purpose is to have fine grain locking over individual variables, but at a cost.
// Obviously this is a bad idea for most variables, but where it is useful is to
// quickly gain access to a variable that belongs to an object that has its own mutex,
// but it can have that mutex locked for significant periods of time while it is doing
// some processor intensive task that does not modify this variable and hence you
// wouldn't want to block another thread from continuing simply to read this variable.
//
class ThreadSafeInteger {
public:
    ThreadSafeInteger() : mutex() {
	FFAutoLockUnlockMutex lock(mutex);
	variable = 0;
    }

    // Might be a mistake to modify the variable out of context.
    // The context might require a mutex for what this belongs to, to be locked first outside this scope.
#ifdef DISALLOW_ASSIGNMENT
    ThreadSafeInteger &operator=(const ThreadSafeInteger&) {
	abort();
    }
#else
    ThreadSafeInteger(int i) {
	FFAutoLockUnlockMutex lock(mutex);
	variable = i;
    }
    int operator=(int i) {
	FFAutoLockUnlockMutex lock(mutex);
	variable = i;
	return variable;
    }
#endif
    void assign(int i) {
	FFAutoLockUnlockMutex lock(mutex);
	variable = i;
    }

    bool operator==(int i) {
	FFAutoLockUnlockMutex lock(mutex);
	return variable == i;
    }
    bool operator!=(int i) {
	FFAutoLockUnlockMutex lock(mutex);
	return variable != i;
    }
    bool operator>=(int i) {
	FFAutoLockUnlockMutex lock(mutex);
	return variable >= i;
    }
    bool operator<=(int i) {
	FFAutoLockUnlockMutex lock(mutex);
	return variable <= i;
    }
    bool operator>(int i) {
	FFAutoLockUnlockMutex lock(mutex);
	return variable > i;
    }
    bool operator<(int i) {
	FFAutoLockUnlockMutex lock(mutex);
	return variable < i;
    }
    operator bool() {
	FFAutoLockUnlockMutex lock(mutex);
	return variable != 0;
    }
    operator int() {
	FFAutoLockUnlockMutex lock(mutex);
	return variable;
    }
private:
    FFMutex mutex;
    int variable;
};


#endif // FF_MUTEX_H

