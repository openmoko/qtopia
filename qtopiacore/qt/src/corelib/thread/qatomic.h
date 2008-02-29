/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QATOMIC_H
#define QATOMIC_H

#if defined(QT_MOC) || defined(QT_BUILD_QMAKE) || defined(QT_RCC) || defined(QT_UIC)
#  include <QtCore/qatomic_generic.h>
#else
#  include <QtCore/qatomic_arch.h>
#endif
#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

#ifndef Q_SPECIALIZED_QATOMIC

/*
    We assume that the following 8 functions have been declared by the
    platform specific qatomic.h:

    int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval);
    int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval);
    int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval);

    int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval);
    int q_atomic_increment(volatile int *ptr);
    int q_atomic_decrement(volatile int *ptr);
    int q_atomic_set_int(volatile int *ptr, int newval);
    void *q_atomic_set_ptr(volatile void *ptr, void *newval);

    If you cannot implement these functions efficiently on your
    platform without great difficulty, consider defining
    Q_SPECIALIZED_QATOMIC.  By doing this, you need to implement:

    struct QBasicAtomic;
    template <typename T> struct QBasicAtomicPointer<T>;
    int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval);
    void *q_atomic_set_ptr(volatile void *ptr, void *newval);
*/

struct QBasicAtomic {
    volatile int atomic;

    void init(int x = 0)
    { atomic = x; }

    inline bool ref()
    { return q_atomic_increment(&atomic) != 0; }

    inline bool deref()
    { return q_atomic_decrement(&atomic) != 0; }

    inline bool operator==(int x) const
    { return atomic == x; }

    inline bool operator!=(int x) const
    { return atomic != x; }

    inline bool operator!() const
    { return atomic == 0; }

    inline operator int() const
    { return atomic; }

    inline QBasicAtomic &operator=(int x)
    {
        (void) q_atomic_set_int(&atomic, x);
        return *this;
    }

    inline bool testAndSet(int expected, int newval)
    { return q_atomic_test_and_set_int(&atomic, expected, newval) != 0; }

    inline bool testAndSetAcquire(int expected, int newval)
    { return q_atomic_test_and_set_acquire_int(&atomic, expected, newval) != 0; }

    inline bool testAndSetRelease(int expected, int newval)
    { return q_atomic_test_and_set_release_int(&atomic, expected, newval) != 0; }

    inline int exchange(int newval)
    { return q_atomic_set_int(&atomic, newval); }
};

template <typename T>
struct QBasicAtomicPointer
{
    volatile T *pointer;

    void init(T *t = 0)
    { pointer = t; }

    inline bool operator==(T *t) const
    { return pointer == t; }

    inline bool operator!=(T *t) const
    { return !operator==(t); }

    inline bool operator!() const
    { return operator==(0); }

    inline operator T *() const
    { return const_cast<T *>(pointer); }

    inline T *operator->() const
    { return const_cast<T *>(pointer); }

    inline QBasicAtomicPointer<T> &operator=(T *t)
    {
        (void) q_atomic_set_ptr(&pointer, t);
        return *this;
    }

    inline bool testAndSet(T *expected, T *newval)
    { return q_atomic_test_and_set_ptr(&pointer, expected, newval); }

    inline T *exchange(T * newval)
    { return static_cast<T *>(q_atomic_set_ptr(&pointer, newval)); }
};

#define Q_ATOMIC_INIT(a) { (a) }

#endif // Q_SPECIALIZED_QATOMIC

template <typename T>
inline T qAtomicSetPtr(volatile T *ptr, T newval)
{ return static_cast<T>(q_atomic_set_ptr(ptr, newval)); }

// High-level atomic integer operations
class QAtomic : public QBasicAtomic
{
public:
    inline QAtomic(int x = 0)
    { init(x); }
    inline QAtomic(const QAtomic &copy)
    { init(copy); }

    inline QAtomic &operator=(int x)
    {
        (void) QBasicAtomic::operator=(x);
        return *this;
    }

    inline QAtomic &operator=(const QAtomic &copy)
    {
        (void) QBasicAtomic::operator=(copy);
        return *this;
    }
};

// High-level atomic pointer operations
template <typename T>
class QAtomicPointer : public QBasicAtomicPointer<T>
{
public:
    inline QAtomicPointer(T *t = 0)
    { init(t); }
    inline QAtomicPointer(const QAtomicPointer<T> &copy)
    { init(copy); }

    inline QAtomicPointer<T> &operator=(T *t)
    {
        (void) QBasicAtomicPointer<T>::operator=(t);
        return *this;
    }

    inline QAtomicPointer<T> &operator=(const QAtomicPointer<T> &copy)
    {
        (void) QBasicAtomicPointer<T>::operator=(copy);
        return *this;
    }
};

/*!
    This is a helper for the assignment operators of implicitly
    shared classes. Your assignment operator should look like this:

    \code
        MyClass &MyClass:operator=(const MyClass &other)
        { qAtomicAssign(d, other.d); return *this; }
    \endcode
*/
template <typename T>
inline void qAtomicAssign(T *&d, T *x)
{
    x->ref.ref();
    x = qAtomicSetPtr(&d, x);
    if (!x->ref.deref())
        delete x;
}

/*! \internal
    \overload
*/
template <typename T>
inline void qAtomicAssign(QBasicAtomicPointer<T> &d, T *x)
{
    x->ref.ref();
    x = d.exchange(x);
    if (!x->ref.deref())
        delete x;
}

/*! \internal
    \overload
*/
template <typename T>
inline void qAtomicAssign(QBasicAtomicPointer<T> &d, const QBasicAtomicPointer<T> &x)
{ qAtomicAssign<T>(d, x); }

/*! \internal
    This is a helper for the detach function. Your private class needs
    a copy constructor which copies the members and sets the refcount
    to 1. After that, your detach function should look like this:

    \code
        void MyClass::detach()
        { qAtomicDetach(d); }
    \endcode
*/
template <typename T>
inline void qAtomicDetach(T *&d)
{
    if (d->ref == 1)
        return;
    T *x = new T(*d);
    x = qAtomicSetPtr(&d, x);
    if (!x->ref.deref())
        delete x;
}

/*! \internal
    \overload
*/
template <typename T>
inline void qAtomicDetach(QBasicAtomicPointer<T> &d)
{
    if (d->ref == 1)
        return;
    T *x = new T(*d);
    x = d.exchange(x);
    if (!x->ref.deref())
        delete x;
}

QT_END_HEADER

#endif // QATOMIC_H
