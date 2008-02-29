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

#ifndef S390_QATOMIC_H
#define S390_QATOMIC_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

#define __CS_LOOP(ptr, op_val, op_string) ({				\
	volatile int old_val, new_val;					\
        __asm__ __volatile__("   l     %0,0(%3)\n"			\
                             "0: lr    %1,%0\n"				\
                             op_string "  %1,%4\n"			\
                             "   cs    %0,%1,0(%3)\n"			\
                             "   jl    0b"				\
                             : "=&d" (old_val), "=&d" (new_val),	\
			       "=m" (*ptr)	\
			     : "a" (ptr), "d" (op_val),			\
			       "m" (*ptr)	\
			     : "cc", "memory" );			\
	new_val;							\
})

#define __CS_OLD_LOOP(ptr, op_val, op_string) ({			\
	volatile int old_val, new_val;					\
        __asm__ __volatile__("   l     %0,0(%3)\n"			\
                             "0: lr    %1,%0\n"				\
                             op_string "  %1,%4\n"			\
                             "   cs    %0,%1,0(%3)\n"			\
                             "   jl    0b"				\
                             : "=&d" (old_val), "=&d" (new_val),	\
			       "=m" (*ptr)	\
			     : "a" (ptr), "d" (op_val),			\
			       "m" (*ptr)	\
			     : "cc", "memory" );			\
	old_val;							\
})

#ifdef __s390x__
#define __CSG_OLD_LOOP(ptr, op_val, op_string) ({				\
	long old_val, new_val;						\
        __asm__ __volatile__("   lg    %0,0(%3)\n"			\
                             "0: lgr   %1,%0\n"				\
                             op_string "  %1,%4\n"			\
                             "   csg   %0,%1,0(%3)\n"			\
                             "   jl    0b"				\
                             : "=&d" (old_val), "=&d" (new_val),	\
			       "=m" (*ptr)	\
			     : "a" (ptr), "d" (op_val),			\
			       "m" (*ptr)	\
			     : "cc", "memory" );			\
	old_val;							\
})
#endif

inline int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval)
{
        int retval;

        __asm__ __volatile__(
                "  lr   %0,%3\n"
                "  cs   %0,%4,0(%2)\n"
                "  ipm  %0\n"
                "  srl  %0,28\n"
                "0:"
                : "=&d" (retval), "=m" (*ptr)
                : "a" (ptr), "d" (expected) , "d" (newval),
		  "m" (*ptr) : "cc", "memory" );

        if(retval) return 0;
        else return 1;
}

inline int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval)
{
        int retval;

        __asm__ __volatile__(
                "  lr   %0,%3\n"
                "  cs   %0,%4,0(%2)\n"
                "  ipm  %0\n"
                "  srl  %0,28\n"
                "0:\n"
                "  bcr 15,0\n"
                : "=&d" (retval), "=m" (*ptr)
                : "a" (ptr), "d" (expected) , "d" (newval),
		  "m" (*ptr) : "cc", "memory" );

        if(retval) return 0;
        else return 1;
}

inline int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval)
{
        int retval;

        __asm__ __volatile__(
                "  bcr 15,0\n"
                "  lr   %0,%3\n"
                "  cs   %0,%4,0(%2)\n"
                "  ipm  %0\n"
                "  srl  %0,28\n"
                "0:"
                : "=&d" (retval), "=m" (*ptr)
                : "a" (ptr), "d" (expected) , "d" (newval),
		  "m" (*ptr) : "cc", "memory" );

        if(retval) return 0;
        else return 1;
}


inline int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void* newval)
{
        int retval;

#ifndef __s390x__
        __asm__ __volatile__(
                "  lr   %0,%3\n"
                "  cs   %0,%4,0(%2)\n"
                "  ipm  %0\n"
                "  srl  %0,28\n"
                "0:"
                : "=&d" (retval), "=m" (*reinterpret_cast<void * volatile *>(ptr))
                : "a" (ptr), "d" (expected) , "d" (newval),
		  "m" (*reinterpret_cast<void* volatile *>(ptr)) : "cc", "memory" );
#else
        __asm__ __volatile__(
                "  lgr   %0,%3\n"
                "  csg   %0,%4,0(%2)\n"
                "  ipm  %0\n"
                "  srl  %0,28\n"
                "0:"
                : "=&d" (retval), "=m" (*reinterpret_cast<void * volatile *>(ptr))
                : "a" (ptr), "d" (expected) , "d" (newval),
		  "m" (*reinterpret_cast<void* volatile *>(ptr)) : "cc", "memory" );
#endif

        if(retval) return 0;
        else return 1;
}

inline int q_atomic_increment(volatile int *ptr)
{ return __CS_LOOP(ptr, 1, "ar"); }

inline int q_atomic_decrement(volatile int *ptr)
{ return __CS_LOOP(ptr, 1, "sr"); }

inline int q_atomic_set_int(volatile int *ptr, int newval)
{
    return __CS_OLD_LOOP(ptr, newval, "lr");
}

inline void *q_atomic_set_ptr(volatile void *ptr, void *newval)
{
#ifndef __s390x__
    return (void*)__CS_OLD_LOOP(reinterpret_cast<volatile long*>(ptr), (int)newval, "lr");
#else
    return (void*)__CSG_OLD_LOOP(reinterpret_cast<volatile long*>(ptr), (long)newval, "lgr");
#endif
}

QT_END_HEADER

#endif // S390_QATOMIC_H
