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
#ifndef QMATH_H
#define QMATH_H

#include <qtopia/qpeglobal.h>

#ifdef __cplusplus
extern "C"
{
#endif

QTOPIA_EXPORT double qSqrt( double value );
QTOPIA_EXPORT double qSin( double a );
QTOPIA_EXPORT double qCos( double a );
QTOPIA_EXPORT double qATan2( double y, double x );
QTOPIA_EXPORT double qATan( double a );
QTOPIA_EXPORT double qASin( double a );
QTOPIA_EXPORT double qTan( double a );
QTOPIA_EXPORT double qFloor( double a );
QTOPIA_EXPORT double qFabs( double a );

#ifdef __cplusplus
}
#endif

#endif
