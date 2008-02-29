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

#include "qsemaphore.h"
#include "qmutex.h"
#include "qwaitcondition.h"


/*!
    \class QSemaphore qsemaphore.h
    \threadsafe
    \brief The QSemaphore class provides a robust integer semaphore.

    \ingroup thread
    \ingroup environment

    A QSemaphore can be used to serialize thread execution, in a
    similar way to a QMutex. A semaphore differs from a mutex, in
    that a semaphore can be accessed by more than one thread at a
    time.

    For example, suppose we have an application that stores data in a
    large tree structure. The application creates 10 threads
    (commonly called a thread pool) to perform searches on the tree.
    When the application searches the tree for some piece of data, it
    uses one thread per base node to do the searching. A semaphore
    could be used to make sure that two threads don't try to search
    the same branch of the tree at the same time.

    A non-computing example of a semaphore would be dining at a
    restuarant. A semaphore is initialized to have a maximum count
    equal to the number of chairs in the restuarant. As people
    arrive, they want a seat. As seats are filled, the semaphore is
    accessed, once per person. As people leave, the access is
    released, allowing more people to enter. If a party of 10 people
    want to be seated, but there are only 9 seats, those 10 people
    will wait, but a party of 4 people would be seated (taking the
    available seats to 5, making the party of 10 people wait longer).

    When a semaphore is created it is given a number which is the
    maximum number of concurrent accesses it will permit. This amount
    may be changed using operator++(), operator--(), operator+=() and
    operator-=(). The number of accesses allowed is retrieved with
    available(), and the total number with total(). Note that the
    incrementing functions will block if there aren't enough available
    accesses. Use tryAccess() if you want to acquire accesses without
    blocking.
*/


class QSemaphorePrivate {
public:
    QSemaphorePrivate(int);

    QMutex mutex;
    QWaitCondition cond;

    int value, max;
};


QSemaphorePrivate::QSemaphorePrivate(int m)
    : mutex(FALSE), value(0), max(m)
{
}


/*!
    Creates a new semaphore. The semaphore can be concurrently
    accessed at most \a maxcount times.
*/
QSemaphore::QSemaphore(int maxcount)
{
    d = new QSemaphorePrivate(maxcount);
}


/*!
    Destroys the semaphore.

    \warning If you destroy a semaphore that has accesses in use the
    resultant behavior is undefined.
*/
QSemaphore::~QSemaphore()
{
    delete d;
}


/*!
    Postfix ++ operator.

    Try to get access to the semaphore. If \l available() == 0, this
    call will block until it can get access, i.e. until available() \>
    0.
*/
int QSemaphore::operator++(int)
{
    int ret;

    d->mutex.lock();

    while (d->value >= d->max)
	d->cond.wait(&(d->mutex));

    ++(d->value);
    if (d->value > d->max) d->value = d->max;
    ret = d->value;

    d->mutex.unlock();

    return ret;
}


/*!
    Postfix -- operator.

    Release access of the semaphore. This wakes all threads waiting
    for access to the semaphore.
*/
int QSemaphore::operator--(int)
{
    int ret;

    d->mutex.lock();

    --(d->value);
    if (d->value < 0) d->value = 0;
    ret = d->value;

    d->cond.wakeAll();
    d->mutex.unlock();

    return ret;
}


/*!
    Try to get access to the semaphore. If \l available() \< \a n, this
    call will block until it can get all the accesses it wants, i.e.
    until available() \>= \a n.
*/
int QSemaphore::operator+=(int n)
{
    int ret;

    d->mutex.lock();

    if ( n < 0 || n > d->max ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QSemaphore::operator+=: paramter %d out of range", n );
#endif // QT_CHECK_RANGE
	n = n < 0 ? 0 : d->max;
    }

    while (d->value + n > d->max)
	d->cond.wait(&(d->mutex));

    d->value += n;
    ret = d->value;

    d->mutex.unlock();

    return ret;
}


/*!
    Release \a n accesses to the semaphore.
*/
int QSemaphore::operator-=(int n)
{
    int ret;

    d->mutex.lock();

    if ( n < 0 || n > d->value ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QSemaphore::operator-=: paramter %d out of range", n );
#endif // QT_CHECK_RANGE
	n = n < 0 ? 0 : d->value;
    }

    d->value -= n;
    ret = d->value;

    d->cond.wakeOne();
    d->mutex.unlock();

    return ret;
}


/*!
    Returns the number of accesses currently available to the
    semaphore.
*/
int QSemaphore::available() const {
    int ret;

    d->mutex.lock();
    ret = d->max - d->value;
    d->mutex.unlock();

    return ret;
}


/*!
    Returns the total number of accesses to the semaphore.
*/
int QSemaphore::total() const {
    int ret;

    d->mutex.lock();
    ret = d->max;
    d->mutex.unlock();

    return ret;
}


/*!
    Try to get access to the semaphore. If \l available() \< \a n, this
    function will return FALSE immediately. If \l available() \>= \a n,
    this function will take \a n accesses and return TRUE. This
    function does \e not block.
*/
bool QSemaphore::tryAccess(int n)
{
    if (! d->mutex.tryLock())
	return FALSE;

    if (d->value + n > d->max) {
	d->mutex.unlock();
	return FALSE;
    }

    d->value += n;

#ifdef QT_CHECK_RANGE
    if (d->value > d->max) {
	qWarning("QSemaphore::operator+=: attempt to allocate more resources than available");
	d->value = d->max;
    }
#endif

    d->mutex.unlock();

    return TRUE;
}

#endif // QT_THREAD_SUPPORT
