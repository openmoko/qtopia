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

#ifndef SPARC_QATOMIC_H
#define SPARC_QATOMIC_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

#if defined(_LP64)

extern "C" {
    Q_CORE_EXPORT int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval);
    Q_CORE_EXPORT int q_atomic_test_and_set_acquire_int(volatile int *ptr,
                                                        int expected,
                                                        int newval);
    Q_CORE_EXPORT int q_atomic_test_and_set_release_int(volatile int *ptr,
                                                        int expected,
                                                        int newval);
    Q_CORE_EXPORT int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval);
    Q_CORE_EXPORT int q_atomic_increment(volatile int *ptr);
    Q_CORE_EXPORT int q_atomic_decrement(volatile int *ptr);
    Q_CORE_EXPORT int q_atomic_set_int(volatile int *ptr, int newval);
    Q_CORE_EXPORT void *q_atomic_set_ptr(volatile void *ptr, void *newval);
}

#else

extern "C" {
    Q_CORE_EXPORT int q_atomic_lock_int(volatile int *addr);
    Q_CORE_EXPORT int q_atomic_lock_ptr(volatile void *addr);
    Q_CORE_EXPORT void q_atomic_unlock(volatile void *addr, int value);
    Q_CORE_EXPORT int q_atomic_set_int(volatile int *ptr, int newval);
    Q_CORE_EXPORT void *q_atomic_set_ptr(volatile void *ptr, void *newval);
} // extern "C"

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{
    int val = q_atomic_lock_int(ptr);
    if (val == expected) {
        q_atomic_unlock(ptr, newval);
        return 1;
    }
    q_atomic_unlock(ptr, val);
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

inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{
    void *val = reinterpret_cast<void *>(q_atomic_lock_ptr(ptr));
    if (val == expected) {
        q_atomic_unlock(ptr, reinterpret_cast<int>(newval));
        return 1;
    }
    q_atomic_unlock(ptr, reinterpret_cast<int>(val));
    return 0;
}

inline int q_atomic_increment(volatile int *ptr)
{
    const int val = q_atomic_lock_int(ptr);
    q_atomic_unlock(ptr, val + 1);
    return val != -1;
}

inline int q_atomic_decrement(volatile int *ptr)
{
    const int val = q_atomic_lock_int(ptr);
    q_atomic_unlock(ptr, val - 1);
    return val != 1;
}

#endif // !_LP64

QT_END_HEADER

#endif // SPARC_QATOMIC_H
