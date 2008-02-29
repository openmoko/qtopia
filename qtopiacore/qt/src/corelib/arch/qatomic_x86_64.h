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

#ifndef X86_64_QATOMIC_H
#define X86_64_QATOMIC_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

#if defined(Q_CC_GNU) || defined(Q_CC_INTEL)

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{
    unsigned char ret;
    asm volatile("lock cmpxchgl %2,%3\n"
                 "sete %1\n"
                 : "=a" (newval), "=qm" (ret)
                 : "r" (newval), "m" (*ptr), "0" (expected)
                 : "memory");
    return static_cast<int>(ret);
}

inline int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval)
{
    return q_atomic_test_and_set_int(ptr, expected, newval);
}

inline int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval)
{
    return q_atomic_test_and_set_int(ptr, expected, newval);
}

inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{
    unsigned char ret;
    asm volatile("lock cmpxchgq %2,%3\n"
                 "sete %1\n"
                 : "=a" (newval), "=qm" (ret)
                 : "r" (newval), "m" (*reinterpret_cast<volatile long *>(ptr)), "0" (expected)
                 : "memory");
    return static_cast<int>(ret);
}

inline int q_atomic_increment(volatile int *ptr)
{
    unsigned char ret;
    asm volatile("lock incl %0\n"
                 "setne %1"
                 : "=m" (*ptr), "=qm" (ret)
                 : "m" (*ptr)
                 : "memory");
    return static_cast<int>(ret);
}

inline int q_atomic_decrement(volatile int *ptr)
{
    unsigned char ret;
    asm volatile("lock decl %0\n"
                 "setne %1"
                 : "=m" (*ptr), "=qm" (ret)
                 : "m" (*ptr)
                 : "memory");
    return static_cast<int>(ret);
}

inline int q_atomic_set_int(volatile int *ptr, int newval)
{
    asm volatile("xchgl %0,%1"
                 : "=r" (newval)
                 : "m" (*ptr), "0" (newval)
                 : "memory");
    return newval;
}

inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{
    asm volatile("xchgq %0,%1"
                 : "=r" (newval)
                 : "m" (*reinterpret_cast<volatile long *>(ptr)), "0" (newval)
                 : "memory");
    return newval;
}

#else

extern "C" {
    int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval);
    int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval);
} // extern "C"

inline int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval)
{
    return q_atomic_test_and_set_int(ptr, expected, newval);
}

inline int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval)
{
    return q_atomic_test_and_set_int(ptr, expected, newval);
}

inline int q_atomic_increment(volatile int *ptr)
{
    int expected;
    forever {
        expected = *ptr;
        if (q_atomic_test_and_set_int(ptr, expected, expected + 1))
            break;
    }
    return expected != -1;
}

inline int q_atomic_decrement(volatile int *ptr)
{
    int expected;
    forever {
        expected = *ptr;
        if (q_atomic_test_and_set_int(ptr, expected, expected - 1))
            break;
    }
    return expected != 1;
}

inline int q_atomic_set_int(volatile int *ptr, int newval)
{
    int expected;
    forever {
        expected = *ptr;
        if (q_atomic_test_and_set_int(ptr, expected, newval))
            break;
    }
    return expected;
}

inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{
    void *expected;
    forever {
        expected = *reinterpret_cast<void * volatile *>(ptr);
        if (q_atomic_test_and_set_ptr(ptr, expected, newval))
            break;
    }
    return expected;
}

#endif

QT_END_HEADER

#endif // X86_64_QATOMIC_H
