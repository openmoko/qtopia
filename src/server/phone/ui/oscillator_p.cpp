/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "oscillator_p.h"

#include <math.h>

Oscillator::Oscillator(qreal _yMin,qreal _yMax,qreal xMax,qreal _factor, qreal _origin)
        : yMax(_yMax)
        , yMin(_yMin)
        , m1(-(yMax-yMin)/2/xMax)
        , m2((yMax-yMin)/2/xMax)
        , factor(_factor)
        , origin(_origin)
{
}

qreal Oscillator::operator() (qreal x) {
    qreal max = m1 * x + yMax;
    qreal min = m2 * x + yMin;

    // If we need more oscillations or less, we add a factor and go like
    // this
    x *= factor;
    // for example, factor = 2 will give us twice as many peaks

    qreal hr = (max - min)/2;
    qreal rc = (hr * sin(x)) + origin;
    return rc;
}
