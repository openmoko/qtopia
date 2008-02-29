/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef MPEG3REAL_H
#define MPEG3REAL_H

#ifdef USE_FIXED_POINT

#include <limits.h>
#include <stdio.h>

#ifndef LONGLONG
#define LONGLONG long long
#endif

#define SCALEBITS   15

#define SC (1<<SCALEBITS)

#include <linux/config.h>
#ifdef CONFIG_ARM

/*
    This piece of assembler to muliply and scale the result
    by SCALEBITS was copied from libmad. This has been tested
    on a Zaurus and found to work well at speeding up libmpeg3
    using fixed point maths. It may be possible that similar
    pices of assember could be used for other CPUs.
*/
#define MUL_SCALE(x, y)  \
    ({ long __hi;  \
       long __lo;  \
       long __result;  \
       asm ("smull      %0, %1, %3, %4\n\t"  \
            "movs       %0, %0, lsr %5\n\t"  \
            "adc        %2, %0, %1, lsl %6"  \
            : "=&r" (__lo), "=&r" (__hi), "=r" (__result)  \
            : "%r" (x), "r" (y),  \
              "M" (SCALEBITS), "M" (32 - SCALEBITS)  \
            : "cc");  \
       __result;  \
    })

#else

#define MUL_SCALE(x, y) \
    (((LONGLONG)(x)*y)/SC)

#endif

class mpeg3_real_t {
    long v;
public:
    mpeg3_real_t() { } // Uninitialized, just like a float
    mpeg3_real_t(double d) { v=long(d*SC); }
    mpeg3_real_t(float f) { v=long(f*SC); }
    mpeg3_real_t(int i) { v=((long)i) << SCALEBITS; }
    long fixedPoint() const { return v; }
    operator float() const { return (float)(v >> SCALEBITS); }
    operator int() const { return (int)(v >> SCALEBITS); }
    mpeg3_real_t operator+() const;
    mpeg3_real_t operator-() const;
    mpeg3_real_t& operator= (const mpeg3_real_t&);
    mpeg3_real_t& operator+= (const mpeg3_real_t&);
    mpeg3_real_t& operator-= (const mpeg3_real_t&);
    mpeg3_real_t& operator*= (const mpeg3_real_t&);
    mpeg3_real_t& operator/= (const mpeg3_real_t&);
    friend mpeg3_real_t operator+ (const mpeg3_real_t&, const mpeg3_real_t&);
    friend mpeg3_real_t operator- (const mpeg3_real_t&, const mpeg3_real_t&);
    friend mpeg3_real_t operator* (const mpeg3_real_t&, const mpeg3_real_t&);
    friend mpeg3_real_t operator/ (const mpeg3_real_t&, const mpeg3_real_t&);
    friend mpeg3_real_t operator+ (const mpeg3_real_t&, const float&);
    friend mpeg3_real_t operator- (const mpeg3_real_t&, const float&);
    friend mpeg3_real_t operator* (const mpeg3_real_t&, const float&);
    friend mpeg3_real_t operator/ (const mpeg3_real_t&, const float&);
    friend mpeg3_real_t operator+ (const float&, const mpeg3_real_t&);
    friend mpeg3_real_t operator- (const float&, const mpeg3_real_t&);
    friend mpeg3_real_t operator* (const float&, const mpeg3_real_t&);
    friend mpeg3_real_t operator/ (const float&, const mpeg3_real_t&);
    friend mpeg3_real_t operator+ (const mpeg3_real_t&, const int&);
    friend mpeg3_real_t operator- (const mpeg3_real_t&, const int&);
    friend mpeg3_real_t operator* (const mpeg3_real_t&, const int&);
    friend mpeg3_real_t operator/ (const mpeg3_real_t&, const int&);
    friend mpeg3_real_t operator+ (const int&, const mpeg3_real_t&);
    friend mpeg3_real_t operator- (const int&, const mpeg3_real_t&);
    friend mpeg3_real_t operator* (const int&, const mpeg3_real_t&);
    friend mpeg3_real_t operator/ (const int&, const mpeg3_real_t&);
};

inline mpeg3_real_t mpeg3_real_t::operator+() const
{
    return *this;
}

inline mpeg3_real_t mpeg3_real_t::operator-() const
{
    mpeg3_real_t r;
    r.v=-v;
    return r;
}

inline mpeg3_real_t& mpeg3_real_t::operator= (const mpeg3_real_t& o)
{
    v=o.v;
    return *this;
}

inline mpeg3_real_t& mpeg3_real_t::operator+= (const mpeg3_real_t& o)
{
    v += o.v;
    return *this;
}

inline mpeg3_real_t& mpeg3_real_t::operator-= (const mpeg3_real_t& o)
{
    v -= o.v;
    return *this;
}

inline mpeg3_real_t& mpeg3_real_t::operator*= (const mpeg3_real_t& o)
{
    *this = *this * o;
    return *this;
}

inline mpeg3_real_t& mpeg3_real_t::operator/= (const mpeg3_real_t& o)
{
    *this = *this / o; 
    return *this;
}


inline mpeg3_real_t operator+ (const mpeg3_real_t&a, const mpeg3_real_t&b)
{
    mpeg3_real_t r;
    r.v=a.v+b.v;
    return r;
}

inline mpeg3_real_t operator- (const mpeg3_real_t&a, const mpeg3_real_t&b)
{
    mpeg3_real_t r;
    r.v=a.v-b.v;
    return r;
}

inline mpeg3_real_t operator* (const mpeg3_real_t&a, const mpeg3_real_t&b)
{
    mpeg3_real_t r;
    r.v = MUL_SCALE(a.v, b.v);
    return r;
}

inline mpeg3_real_t operator/ (const mpeg3_real_t&a, const mpeg3_real_t&b)
{
    mpeg3_real_t r;
    r.v = (((LONGLONG)a.v) << SCALEBITS) / b.v;
    return r;
}

inline mpeg3_real_t operator+ (const mpeg3_real_t&a, const float&b)
{
    return a+mpeg3_real_t(b);
}

inline mpeg3_real_t operator- (const mpeg3_real_t&a, const float&b)
{
    return a-mpeg3_real_t(b);
}

inline mpeg3_real_t operator* (const mpeg3_real_t&a, const float&b)
{
    return a*mpeg3_real_t(b);
}

inline mpeg3_real_t operator/ (const mpeg3_real_t&a, const float&b)
{
    return a/mpeg3_real_t(b);
}


inline mpeg3_real_t operator+ (const float&a, const mpeg3_real_t&b)
{
    return mpeg3_real_t(a)+b;
}

inline mpeg3_real_t operator- (const float&a, const mpeg3_real_t&b)
{
    return mpeg3_real_t(a)-b;
}

inline mpeg3_real_t operator* (const float&a, const mpeg3_real_t&b)
{
    return mpeg3_real_t(a)*b;
}

inline mpeg3_real_t operator/ (const float&a, const mpeg3_real_t&b)
{
    return mpeg3_real_t(a)/b;
}


inline mpeg3_real_t operator+ (const mpeg3_real_t&a, const int&b)
{
    return a+mpeg3_real_t(b);
}

inline mpeg3_real_t operator- (const mpeg3_real_t&a, const int&b)
{
    return a-mpeg3_real_t(b);
}

inline mpeg3_real_t operator* (const mpeg3_real_t&a, const int&b)
{
    return a*mpeg3_real_t(b);
}

inline mpeg3_real_t operator/ (const mpeg3_real_t&a, const int&b)
{
    return a/mpeg3_real_t(b);
}


inline mpeg3_real_t operator+ (const int&a, const mpeg3_real_t&b)
{
    return mpeg3_real_t(a)+b;
}

inline mpeg3_real_t operator- (const int&a, const mpeg3_real_t&b)
{
    return mpeg3_real_t(a)-b;
}

inline mpeg3_real_t operator* (const int&a, const mpeg3_real_t&b)
{
    return mpeg3_real_t(a)*b;
}

inline mpeg3_real_t operator/ (const int&a, const mpeg3_real_t&b)
{
    return mpeg3_real_t(a)/b;
}

#else
typedef float mpeg3_real_t;
#endif

#endif
