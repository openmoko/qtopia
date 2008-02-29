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

	// Work out when to reschedule playing again
	struct timespec timeout;
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


	AutoLockUnlockMutex lock(&mutex);
/*
	// A small wait, implemented as a timedWait instead of ::usleep seems best
	long interval = timerInterval();
	interval = (interval) ? interval * 30 : 1000;
	mutex.timedWait(interval);
//	mutex.timedWait(1000);
*/

//	qDebug("play audio");
	play();

	// Doesn't seem to work nicely using these
	//pthread_yield();
	//sched_yield();

/*
	qDebug("timer interval: %i", timerInterval() );
	struct timeval now2;
	gettimeofday(&now2, 0);
	struct tms timesData;
	times(&timesData);
	qDebug("audio loop finished play iteration: pid: %d  now: %2d.%06d  times: %d + %d", getpid(),
	    now2.tv_sec % 60, now2.tv_usec,
	    timesData.tms_utime, timesData.tms_stime );
*/

	// This doesn't work because if it takes too long to decode, the timeout value
	// will be in the past, it won't do any waiting, and the realtime thread
	// can starve the whole system still
	mutex.timedWaitAbsolute(timeout);
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
    AutoLockUnlockMutex lock(&mutex);
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
    stop();

    AutoLockUnlockMutex lock(&mutex);
    if ( enabled ) {
	more = true;
	if ( threaded )
	    QTimer::singleShot(50, this, SLOT(signalThread())); // Wait 50 ms for playing loop to settle
	else
	    timerId = startTimer(timerInterval());
    }

    // Do atleast the first frame to ensure the decoder is initialised.
    // If we were to seek before having played anything, the decoder
    // can crash.
    // play();

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

