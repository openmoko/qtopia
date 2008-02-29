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
#include "loopcontextbase.h"
#ifdef THREAD_SUPPORT
#include "mutex.h"
#include <sys/time.h>
#include <sched.h>
#include <unistd.h>
#include <sys/resource.h>
#include <qtopia/global.h>
#include <qtimer.h>
#endif // THREAD_SUPPORT


LoopContextBase::LoopContextBase(bool preferThreaded, bool preferRealTime, bool fifo, int priority)
{
    setStopping(false);
    AutoLockUnlockMutex lock(&mutex);
    // Initalise all the member variables inside the lock so valrgind is happy
    streamId = 0;
    current = 0;
    more = false;
    sleep = false;
    enabled = false;
    timerId = 0;
    size = 0;
    joinThread = false;
    threaded = preferThreaded;
    
    realtime = preferRealTime;

    rateNumDen.rateNumerator = 0;
    rateNumDen.rateDenominator = 0;

    if ( threaded ) {
#ifdef THREAD_SUPPORT
	createThread((void *(*)(void *))LoopContextBase::startThreadLoop, this, preferRealTime, fifo, priority);
#else
	threaded = false;
#endif
    }
}


LoopContextBase::~LoopContextBase()
{
    stop();

    mutex.lock();
    bool localThreaded = threaded; // avoid race condition
    if ( threaded ) {
	more = false;
	joinThread = true;
    }
    mutex.unlock();

    if ( localThreaded ) {
	signalThread();
	pthread_join(threadId,0);
    }
}


bool LoopContextBase::join()
{
    AutoLockUnlockMutex lock(&mutex);
    return joinThread;
}


#ifdef THREAD_SUPPORT
void LoopContextBase::createThread(void *(*func)(void *), void *arg, bool realtime, bool fifo, int priority)
{
    pthread_attr_init(&threadAttr);
    if ( getuid() == 0 ) {
	if ( realtime ) {
	    qDebug("running as root, can set realtime priority");
	    // Attempt to set it to real-time FIFO 
	    if ( pthread_attr_setschedpolicy(&threadAttr, fifo ? SCHED_FIFO : SCHED_RR) == 0 ) {
		sched_param params;
		params.sched_priority = priority;
		pthread_attr_setschedparam(&threadAttr,&params);
	    } else {
		qDebug("error setting up a realtime thread, reverting to using a normal thread.");
		pthread_attr_destroy(&threadAttr);
		pthread_attr_init(&threadAttr);
	    }
	} else {
	    setpriority(PRIO_PROCESS, 0, getpriority(PRIO_PROCESS, 0) - priority);
	}
    }
    pthread_create(&threadId, &threadAttr, func, arg);
}


void *LoopContextBase::startThreadLoop(LoopContextBase *loopContext)
{
    qDebug("playing loop thread started");
    while ( !loopContext->join() )
	loopContext->playingLoop();
    qDebug("playing loop thread ended");
    return 0;
}


void LoopContextBase::playingLoop()
{
    mutex.wait();
    qDebug("playing loop thread resumed");
    while ( isMore() && !isStopping() ) {
	struct timespec timeout;
    
        if( realtime ) {
            // Work out when to reschedule playing again
            struct timeval now;
            gettimeofday(&now, 0);
            // qDebug("Audio loop now: %2d.%6d", now.tv_sec % 60, now.tv_usec);
            now.tv_usec += timerInterval() * 100; // Under estimate when needed again
            while (now.tv_usec >= 1000000) {
                now.tv_usec -= 1000000;
                now.tv_sec++;
            }
            timeout.tv_sec = now.tv_sec;
            timeout.tv_nsec = now.tv_usec * 1000;
        }


	AutoLockUnlockMutex lock(&mutex);
	play();

        // Starvation of non-realtime processes may occur if timout not sufficiently large
        if( realtime ) {
            mutex.timedWaitAbsolute(timeout);
        }
    }
    qDebug("playing loop thread paused");
}
#endif


void LoopContextBase::play()
{
    more = (more) ? playMore() : false;
    stats();
}


bool LoopContextBase::isMore()
{
    return more;
}


bool LoopContextBase::isStopping()
{
    AutoLockUnlockMutex lock(&stoppingMutex);
    return stopping;
}


void LoopContextBase::setStopping(bool b)
{
    AutoLockUnlockMutex lock(&stoppingMutex);
    stopping = b;
}


void LoopContextBase::start()
{
#define BUFFER_TIME 500

    stop();
    
    AutoLockUnlockMutex lock(&mutex);
    if ( enabled ) {
	more = true;
	if ( threaded ) {
            // Allow time for initial packets to buffer
            struct timespec time, elapsed;
            time.tv_sec = 0;
            time.tv_nsec = 1000000*BUFFER_TIME;
            int ret = nanosleep( &time, &elapsed );
            while( ret == -1 && errno == EINTR ) {
                struct timespec remaining = elapsed;
                nanosleep( &remaining, &elapsed );
            }
            // Begin playing
            signalThread();
	} else
	    timerId = startTimer(timerInterval());
    }

    qDebug("started");
}


void LoopContextBase::signalThread()
{
    sched_yield();
    mutex.signal();
    sched_yield();
    mutex.signal();
}


void LoopContextBase::stop()
{
    setStopping(true);
    mutex.lock();
    if ( timerId ) { 
	killTimer(timerId);
	timerId = 0;
    }
    more = false;
    bool localThreaded = threaded; // avoid race condition
    mutex.unlock();
    setStopping(false);

    // ensure it is in the outer most waiting state
    if ( localThreaded )
	signalThread();

    qDebug("stopped");
}


void LoopContextBase::timerEvent(QTimerEvent *te)
{
    AutoLockUnlockMutex lock(&mutex);
    if ( !threaded && te->timerId() == timerId )
	play();
}

