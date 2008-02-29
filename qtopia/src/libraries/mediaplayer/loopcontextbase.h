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
#ifndef LOOP_CONTEXT_BASE_H
#define LOOP_CONTEXT_BASE_H


#define THREAD_SUPPORT
#include <qobject.h>
#include "mutex.h"


class LoopContextBase : public QObject {
    Q_OBJECT
public:
    LoopContextBase(bool preferThreaded, bool realtime = FALSE, bool fifo = TRUE, int priority = 50);
    virtual ~LoopContextBase();

    virtual void init() = 0;
    virtual void stats() = 0;
    void start();
    void stop();
    bool isMore();
    bool isStopping();
    void setStopping(bool b);

    bool enabled;
    int streamId;
    long current;
    long size;

    long time() {
	return ( enabled && rate != 0.0 && size ) ? (long)(size / rate) : -1;
    }

    typedef struct {
	long rateNumerator;
        long rateDenominator;
    } rateStruct;

    union {
        double rate;
	rateStruct rateNumDen;	
    };

    Mutex mutex;
    Mutex stoppingMutex;

public slots:
    void signalThread();

protected:
    void timerEvent(QTimerEvent *te);

private:
    virtual long timerInterval() = 0;
    virtual bool playMore() = 0;

#ifdef THREAD_SUPPORT
    void setJoin(bool j);
    bool join();
    bool joinThread;
    pthread_attr_t threadAttr;
    pthread_t threadId;
    void createThread(void *(*func)(void *), void *arg, bool realtime, bool fifo, int priority);
    static void *startThreadLoop(LoopContextBase*);
    void playingLoop();
#endif

    int timerId;
    bool sleep;
    bool more;
    bool stopping;
    bool threaded;
    bool realtime;
    bool needFirstPlay;
    void play();
    //friend class LoopControl;
};


#endif // LOOP_CONTEXT_BASE_H

