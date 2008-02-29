/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PARISC_QATOMIC_H
#define PARISC_QATOMIC_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

extern "C" {
    Q_CORE_EXPORT void q_atomic_lock(int *lock);
    Q_CORE_EXPORT void q_atomic_unlock(int *lock);
    Q_CORE_EXPORT int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval);
    Q_CORE_EXPORT void *q_atomic_set_ptr(volatile void *ptr, void *newval);
}

#define Q_SPECIALIZED_QATOMIC

struct QBasicAtomic
{
    int lock[4];
    int atomic;

    inline void init(int x = 0)
    {
        lock[0] = lock[1] = lock[2] = lock[3] = -1; atomic = x;
    }

    inline bool ref()
    {
	q_atomic_lock(lock);
	bool ret = (++atomic != 0);
	q_atomic_unlock(lock);
	return ret;
    }

    inline bool deref()
    {
	q_atomic_lock(lock);
	bool ret = (--atomic != 0);
	q_atomic_unlock(lock);
	return ret;
    }

    inline bool operator==(int x) const
    { return atomic == x; }

    inline bool operator!=(int x) const
    { return atomic != x; }

    inline bool operator!() const
    { return atomic == 0; }

    inline QBasicAtomic &operator=(int x)
    {
        q_atomic_lock(lock);
        atomic = x;
        q_atomic_unlock(lock);
        return *this;
    }

    inline operator int() const
    { return atomic; }

    inline bool testAndSet(int expected, int newval)
    {
	q_atomic_lock(lock);
	if (atomic == expected) {
            atomic = newval;
	    q_atomic_unlock(lock);
	    return true;
        }
	q_atomic_unlock(lock);
	return false;
    }
    
    inline bool testAndSetAcquire(int expected, int newval)
    {
        return testAndSet(expected, newval);
    }
    
    inline bool testAndSetRelease(int expected, int newval)
    {
        return testAndSet(expected, newval);
    }

    inline int exchange(int newval)
    {
	q_atomic_lock(lock);
	int oldval = atomic;
	atomic = newval;
	q_atomic_unlock(lock);
	return oldval;
    }
};

template <typename T>
struct QBasicAtomicPointer
{
    int lock[4];
    volatile T *pointer;

    inline void init(T *t = 0)
    {
        lock[0] = lock[1] = lock[2] = lock[3] = -1; pointer = t;
    }

    inline bool operator==(T *x) const
    {
	return pointer == x;
    }

    inline bool operator!=(T *x) const
    {
	return pointer != x;
    }

    inline bool operator!() const
    { return operator==(0); }

    inline QBasicAtomicPointer<T> &operator=(T *t)
    {
        q_atomic_lock(lock);
        pointer = const_cast<T *>(t);
        q_atomic_unlock(lock);
        return *this;
    }

    inline T *operator->()
    { return const_cast<T *>(pointer); }
    inline const T *operator->() const
    { return pointer; }

    inline operator T*() const
    { return const_cast<T *>(pointer); }

    inline bool testAndSet(T* expected, T *newval)
    {
	q_atomic_lock(lock);
	if (pointer == expected) {
	    pointer = newval;
	    q_atomic_unlock(lock);
	    return true;
	}
	q_atomic_unlock(lock);
	return false;
    }

    inline T *exchange(T *newval)
    {
	q_atomic_lock(lock);
	T *oldval = const_cast<T *>(pointer);
	pointer = newval;
	q_atomic_unlock(lock);
	return oldval;
    }
};

#define Q_ATOMIC_INIT(a) {{-1,-1,-1,-1},(a)}

QT_END_HEADER

#endif // PARISC_QATOMIC_H
