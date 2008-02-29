/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QMATH_P_H
#define QMATH_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <math.h>

/*****************************************************************************
  Trigonometric function for QPainter

  We have implemented simple sine and cosine function that are called from
  QPainter::drawPie() and QPainter::drawChord() when drawing the outline of
  pies and chords.
  These functions are slower and less accurate than math.h sin() and cos(),
  but with still around 1/70000th sec. execution time (on a 486DX2-66) and
  8 digits accuracy, it should not be the bottleneck in drawing these shapes.
  The advantage is that you don't have to link in the math library.
 *****************************************************************************/

static const double Q_PI   = 3.14159265358979323846;   // pi
static const double Q_2PI  = 6.28318530717958647693;   // 2*pi
static const double Q_PI2  = 1.57079632679489661923;   // pi/2

inline int qFloor(double d)
{ return d >= 0.0 ? int(d) : int(d - 0.9999); }

#ifdef Q_WS_X11
#if defined(Q_CC_GNU) && defined(__i386__)

inline double qCos_x86(double a)
{
    double r;
    __asm__ (
        "fcos"
        : "=t" (r) : "0" (a));
    return r;
}
#define qCos qCos_x86

inline double qSin_x86(double a)
{
    double r;
    __asm__ (
        "fsin"
        : "=t" (r) : "0" (a));
    return r;
}
#define qSin qSin_x86

#else //GNU_CC && I386

inline double qSinCos(double a, bool calcCos=false)
{
    if (calcCos)                              // calculate cosine
        a -= Q_PI2;
    if (a >= Q_2PI || a <= -Q_2PI) {          // fix range: -2*pi < a < 2*pi
        int m = (int)(a/Q_2PI);
        a -= Q_2PI*m;
    }
    if (a < 0.0)                              // 0 <= a < 2*pi
        a += Q_2PI;
    int sign = a > Q_PI ? -1 : 1;
    if (a >= Q_PI)
        a = Q_2PI - a;
    if (a >= Q_PI2)
        a = Q_PI - a;
    if (calcCos)
        sign = -sign;
    double a2  = a*a;                           // here: 0 <= a < pi/4
    double a3  = a2*a;                          // make taylor sin sum
    double a5  = a3*a2;
    double a7  = a5*a2;
    double a9  = a7*a2;
    double a11 = a9*a2;
    return (a-a3/6+a5/120-a7/5040+a9/362880-a11/39916800)*sign;
}
#define qSin(a) qSinCos(a, false)
#define qCos(a) qSinCos(a, true)

#endif //GNU_CC && I386
#endif //Q_WS_X11

#ifndef qSin
# define qSin sin
#endif
#ifndef qCos
# define qCos cos
#endif

#endif // QMATH_P_H
