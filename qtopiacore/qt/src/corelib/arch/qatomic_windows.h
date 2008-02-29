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

#ifndef WINDOWS_QATOMIC_H
#define WINDOWS_QATOMIC_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

#if !defined(Q_CC_GNU) && !defined(Q_CC_BOR)

// MSVC++ 6.0 doesn't generate correct code when optimization are turned on!
#if _MSC_VER < 1300 && defined (_M_IX86)

inline int q_atomic_test_and_set_int(volatile int *pointer, int expected, int newval)
{
    __asm {
        mov EDX,pointer
        mov EAX,expected
        mov ECX,newval
        lock cmpxchg dword ptr[EDX],ECX
        mov newval,EAX
    }
    return newval == expected;
}

inline int q_atomic_test_and_set_ptr(volatile void *pointer, void *expected, void *newval)
{
    __asm {
        mov EDX,pointer
        mov EAX,expected
        mov ECX,newval
        lock cmpxchg dword ptr[EDX],ECX
        mov newval,EAX
    }
    return newval == expected;
}

inline int q_atomic_increment(volatile int *pointer)
{
    unsigned char retVal;
    __asm {
        mov ECX,pointer
        lock inc DWORD ptr[ECX]
        setne retVal
    }
    return static_cast<int>(retVal);
}

inline int q_atomic_decrement(volatile int *pointer)
{
    unsigned char retVal;
    __asm {
        mov ECX,pointer
        lock dec DWORD ptr[ECX]
        setne retVal
    }
    return static_cast<int>(retVal);
}

inline int q_atomic_set_int(volatile int *pointer, int newval)
{
    __asm {
        mov EDX,pointer
        mov ECX,newval
        lock xchg dword ptr[EDX],ECX
        mov newval,ECX
    }
    return newval;
}

inline void *q_atomic_set_ptr(volatile void *pointer, void *newval)
{
    __asm {
        mov EDX,pointer
        mov ECX,newval
        lock xchg dword ptr[EDX],ECX
        mov newval,ECX
    }
    return newval;
}

#else
// use compiler intrinsics for all atomic functions
extern "C" {
    long _InterlockedIncrement(volatile long *);
    long _InterlockedDecrement(volatile long *);
    long _InterlockedExchange(volatile long *, long);
    long _InterlockedCompareExchange(volatile long *, long, long);
}
#  pragma intrinsic (_InterlockedIncrement)
#  pragma intrinsic (_InterlockedDecrement)
#  pragma intrinsic (_InterlockedExchange)
#  pragma intrinsic (_InterlockedCompareExchange)

#  ifndef _M_IX86
extern "C" {
    void *_InterlockedCompareExchangePointer(void * volatile *, void *, void *);
    void *_InterlockedExchangePointer(void * volatile *, void *);
}
#    pragma intrinsic (_InterlockedCompareExchangePointer)
#    pragma intrinsic (_InterlockedExchangePointer)
#  else
#    define _InterlockedCompareExchangePointer(a,b,c) \
        reinterpret_cast<void *>(_InterlockedCompareExchange(reinterpret_cast<volatile long *>(a), reinterpret_cast<long>(b), reinterpret_cast<long>(c)))
#    define _InterlockedExchangePointer(a, b) \
        reinterpret_cast<void *>(_InterlockedExchange(reinterpret_cast<volatile long *>(a), reinterpret_cast<long>(b)))
#  endif

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{ return _InterlockedCompareExchange(reinterpret_cast<volatile long *>(ptr), newval, expected) == expected; }

inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{ return _InterlockedCompareExchangePointer(reinterpret_cast<void * volatile *>(ptr), newval, expected) == expected; }

inline int q_atomic_increment(volatile int *ptr)
{ return _InterlockedIncrement(reinterpret_cast<volatile long *>(ptr)); }

inline int q_atomic_decrement(volatile int *ptr)
{ return _InterlockedDecrement(reinterpret_cast<volatile long *>(ptr)); }

inline int q_atomic_set_int(volatile int *ptr, int newval)
{ return _InterlockedExchange(reinterpret_cast<volatile long *>(ptr), newval); }

inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{ return _InterlockedExchangePointer(reinterpret_cast<void * volatile *>(ptr), newval); }

#endif // _MSC_VER ...

#else

#if !(defined Q_CC_BOR) || (__BORLANDC__ < 0x560)

extern "C" {
    __declspec(dllimport) long __stdcall InterlockedCompareExchange(long *, long, long);
    __declspec(dllimport) long __stdcall InterlockedIncrement(long *);
    __declspec(dllimport) long __stdcall InterlockedDecrement(long *);
    __declspec(dllimport) long __stdcall InterlockedExchange(long *, long);
}

#else

extern "C" {
    __declspec(dllimport) long __stdcall InterlockedCompareExchange(long volatile*, long, long);
    __declspec(dllimport) long __stdcall InterlockedIncrement(long volatile*);
    __declspec(dllimport) long __stdcall InterlockedDecrement(long volatile*);
    __declspec(dllimport) long __stdcall InterlockedExchange(long volatile*, long);
}

#endif

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{ return InterlockedCompareExchange(reinterpret_cast<long *>(const_cast<int *>(ptr)), newval, expected) == expected; }

inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval)
{ return InterlockedCompareExchange(reinterpret_cast<long *>(const_cast<void *>(ptr)),
                                    reinterpret_cast<long>(newval),
                                    reinterpret_cast<long>(expected)) == reinterpret_cast<long>(expected); }

inline int q_atomic_increment(volatile int *ptr)
{ return InterlockedIncrement(reinterpret_cast<long *>(const_cast<int *>(ptr))); }

inline int q_atomic_decrement(volatile int *ptr)
{ return InterlockedDecrement(reinterpret_cast<long *>(const_cast<int *>(ptr))); }

inline int q_atomic_set_int(volatile int *ptr, int newval)
{ return InterlockedExchange(reinterpret_cast<long *>(const_cast<int *>(ptr)), newval); }

inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{ return reinterpret_cast<void *>(InterlockedExchange(reinterpret_cast<long *>(const_cast<void *>(ptr)),
                                  reinterpret_cast<long>(newval))); }

#endif // Q_CC_GNU

inline int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval)
{
    return q_atomic_test_and_set_int(ptr, expected, newval);
}

inline int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval)
{
    return q_atomic_test_and_set_int(ptr, expected, newval);
}

QT_END_HEADER

#endif // WINDOWS_QATOMIC_H
