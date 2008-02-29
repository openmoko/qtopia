/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef BOUNDSCHECKER_QATOMIC_H
#define BOUNDSCHECKER_QATOMIC_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

__forceinline int q_atomic_xchg(int newval)
{
    static int lockvar = 0;
    int *pointer = &lockvar;

    __asm {
        mov EDX,pointer
        mov ECX,newval
        xchg dword ptr[EDX],ECX
        mov newval,ECX
    }
    return newval;
}

inline void q_atomic_lock()
{
    while (q_atomic_xchg(~0) != 0)
        ;
}

inline void q_atomic_unlock()
{
    q_atomic_xchg(0);
}

inline int q_atomic_test_and_set_int(volatile int *pointer, int expected, int newval)
{
    q_atomic_lock();
    if (*pointer == expected) {
        *pointer = newval;
        q_atomic_unlock();
        return 1;
    }
    q_atomic_unlock();
    return 0;
}

inline int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval)
{
    return q_atomic_test_and_set_int(ptr, expected, newval);
}

inline int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval)
{
    return q_atomic_test_and_set_int(ptr, expected, newval);
}

inline int q_atomic_test_and_set_ptr(volatile void *pointer, void *expected, void *newval)
{
    q_atomic_lock();
    if (*reinterpret_cast<void * volatile *>(pointer) == expected) {
        *reinterpret_cast<void * volatile *>(pointer) = newval;
        q_atomic_unlock();
        return 1;
    }
    q_atomic_unlock();
    return 0;
}

inline int q_atomic_increment(volatile int *pointer)
{
    q_atomic_lock();
    int ret = ++(*pointer);
    q_atomic_unlock();
    return ret;
}

inline int q_atomic_decrement(volatile int *pointer)
{
    q_atomic_lock();
    int ret = --(*pointer);
    q_atomic_unlock();
    return ret;
}

inline int q_atomic_set_int(volatile int *pointer, int newval)
{
    q_atomic_lock();
    int ret = *pointer;
    *pointer = newval;
    q_atomic_unlock();
    return ret;
}

inline void *q_atomic_set_ptr(volatile void *pointer, void *newval)
{
    q_atomic_lock();
    void *ret = *reinterpret_cast<void * volatile *>(pointer);
    *reinterpret_cast<void * volatile *>(pointer) = newval;
    q_atomic_unlock();
    return ret;

}

QT_END_HEADER

#endif // BOUNDSCHECKER_QATOMIC_H
