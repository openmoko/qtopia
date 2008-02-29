/****************************************************************************
** $Id: qt/src/kernel/qlock_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Definition of QLock class. This manages interprocess locking
**
** Created : 20000406
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qlock_qws.h"
#ifndef QT_NO_QWS_MULTIPROCESS
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <errno.h>

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED) \
    || defined _OS_FREEBSD_
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
       int val;                    /* value for SETVAL */
       struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
       unsigned short int *array;  /* array for GETALL, SETALL */
       struct seminfo *__buf;      /* buffer for IPC_INFO */
};
#endif

#define MAX_LOCKS   200	    // maximum simultaneous read locks

class QLockData
{
public:
    int id;
    int count;
    bool owned;
};

#endif

/*!
  \class QLock qlock_qws.h
  \brief A QLock is a wrapper round a System V shared semaphore;
  it is used by Qt/Embedded for synchronising access to the graphics
  card and shared memory region between processes.
*/

/*!
  \fn QLock::QLock( const QString &filename, char id, bool create )
  Creates a lock. filename is the file path of the Unix-domain socket
  the Qt/Embedded client is using. Id is the name of the particular lock
  to be created on that socket. create is true if it is to be created
  (as the Qt/Embedded server does), false if it is expected to already
  exist (as the Qt/Embedded client does).
*/

QLock::QLock( const QString &filename, char id, bool create )
{
#ifndef QT_NO_QWS_MULTIPROCESS
    data = new QLockData;
    data->count = 0;
    int semkey = ftok(filename, id);
    data->id = semget(semkey,0,0);
    data->owned = create;
    if ( create ) {
	semun arg; arg.val = 0;
	if ( data->id != -1 )
	    semctl(data->id,0,IPC_RMID,arg);
	data->id = semget(semkey,1,IPC_CREAT|0600);
	arg.val = MAX_LOCKS;
	semctl(data->id,0,SETVAL,arg);
    }
    if ( data->id == -1 ) {
	qWarning( "Cannot %s semaphore %s \'%c\'",
	    create ? "create" : "get", filename.latin1(), id );
	qDebug("Error %d %s\n",errno,strerror(errno));
    }
#endif
}

/*!
\fn QLock::~QLock()
Destroys a lock
*/

QLock::~QLock()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if ( locked() )
	unlock();
    if ( data->owned ) {
	semun arg; arg.val = 0;
	semctl( data->id, 0, IPC_RMID, arg ); 
    }
    delete data;
#endif
}

/*!
\fn bool QLock::isValid() const
Returns true if the lock constructor was succesful, false if the lock
could not be created or was not available to connect to.
*/

bool QLock::isValid() const
{
#ifndef QT_NO_QWS_MULTIPROCESS    
    return (data->id != -1);
#else
    return TRUE;
#endif
}

/*!
  Locks the semaphore. Locks can either be Read or Write. If a lock is
  Read, attempts to lock by other processes as Read will succeed, Write
  attempts will block until the lock is unlocked. If locked as Write,
  all attempts to lock by other processes will lock until the lock is
  unlocked. Locks are recursive; that is a given QLock can be locked
  multiple times by the same process without blocking, and will only be
  unlocked after a corresponding number of unlock() calls.
*/

void QLock::lock( Type t )
{
#ifndef QT_NO_QWS_MULTIPROCESS    
    if ( !data->count ) {
	sembuf sops;
	sops.sem_num = 0;
	sops.sem_flg = SEM_UNDO;

	if ( t == Write ) {
	    sops.sem_op = -MAX_LOCKS;
	    type = Write;
	} else {
	    sops.sem_op = -1;
	    type = Read;
	}

	int rv;
	do {
	    rv = semop(data->id,&sops,1);
	    if (rv == -1 && errno != EINTR)
		qDebug("Semop lock failure %s",strerror(errno));
	} while ( rv == -1 && errno == EINTR );
    }
    data->count++;
#endif
}

/*!
\fn void QLock::unlock()
Unlocks the semaphore. If other processes were blocking waiting to lock()
the semaphore, one of them will wake up and succeed in lock()ing.
*/

void QLock::unlock()
{
#ifndef QT_NO_QWS_MULTIPROCESS    
    if ( data->count ) {
	data->count--;
	if ( !data->count ) {
	    sembuf sops;
	    sops.sem_num = 0;
	    sops.sem_op = 1;
	    sops.sem_flg = SEM_UNDO;

	    if ( type == Write )
		sops.sem_op = MAX_LOCKS;

	    int rv;
	    do {
		rv = semop(data->id,&sops,1);
		if (rv == -1 && errno != EINTR)
		    qDebug("Semop unlock failure %s",strerror(errno));
	    } while ( rv == -1 && errno == EINTR );
	}
	return;
    } else {
	qDebug("Unlock without corresponding lock");
    }
#endif
}

/*!
\fn bool QLock::locked() const
Returns true if the lock is currently held by the current process.
*/

bool QLock::locked() const
{
#ifndef QT_NO_QWS_MULTIPROCESS
    return (data->count > 0);
#else
    return FALSE;
#endif    
}

